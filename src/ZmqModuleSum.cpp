#include "ZmqSocket.h"
#include "zmq.h"
#include "rapidjson/document.h" //json header in zmq stream
#include "rapidjson/error/error.h" //json header in zmq stream
#include "sls_receiver_defs.h"

//these 2 are picked up by sls_receiver_defs.h

#include "ansi.h"
#include <iostream>
#include <signal.h>	//SIGINT
using namespace std;

vector <ZmqSocket*> zmqsocket;
vector <ZmqSocket*> zmqsocket2;

/**
 * Control+C Interrupt Handler
 * Sets the variable keeprunning to false, to let all the processes know to exit properly
 */
void sigInterruptHandler(int p){
  // Disconnect resources
  
  for (int imod=0; imod<zmqsocket.size(); ++imod) 
    delete zmqsocket[imod];
  for (int imod=0; imod<zmqsocket2.size(); ++imod) 
    delete zmqsocket2[imod];
  
  zmqsocket.clear();
  zmqsocket2.clear();
}

int main(int argc, char *argv[]) {


	/** - Catch signal SIGINT to close files and call destructors properly */
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
        sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, NULL) == -1) {
	  cprintf(RED, "Could not set handler function for SIGINT\n");
	}

/**
 * trial.o [numsockets] [socket ip] [starting port number] [send_socket ip] [start send port number]
 *
 */
	// command line parameters
	bool send = false;
	int numSockets = 0;
	char* socketip = 0;
	uint32_t portnum = 0;
	char* socketip2 = 0;
	uint32_t portnum2 = 0;
	
	// help
	if (argc < 4 || (argc > 4 && argc !=6 )) {
	  cprintf(RED, "Help: ./zmqModule [numsockets] [receive socket ip] [receive starting port number] [send_socket ip] [send starting port number]\n");
	  cout<<"./bin/zmqModuleSum 4 \"129.129.202.84\" 30001 "<<endl;
	  return EXIT_FAILURE;
	}
	// recieve parameters
	numSockets=atoi(argv[1]);
	socketip = argv[2];
	portnum = atoi(argv[3]);

	//only two parameters don't send
	// send parameters if any
	if (argc >4 ) {
		send = true;
		socketip2 = argv[4];
		portnum2 = atoi(argv[5]);
	}
	cout << "\nnumsockets   : " << numSockets <<
			"\nrx socket ip : " << socketip <<
			"\nrx port num  : " <<  portnum ;
	if (send) {
	  cout << "\nsd socket ip : " << socketip2 <<
	    "\nsd port num  : " <<  portnum2;
	}
	cout << endl;
	
	// sockets and image buffer

	char* image          = NULL;
	char* multiframe     = NULL;
	int multisize        = 0;
	// only first message header
	uint32_t size = 0, nPixelsX = 0, nPixelsY = 0, dynamicRange = 0;
	float bytesPerPixel = 0;
	// header info every header
	std::string currentFileName           = "";
	uint64_t currentAcquisitionIndex = 0, currentFrameIndex = 0, currentFileIndex = 0;
	int flippedDataX = 0;
	char* additionalJsonHeader = 0;
	uint32_t currentPacketNumber=0, currentSubFrameIndex=0;
	uint64_t currentBunchId=0, currentTimeStamp=0;
	uint16_t currentModId=0, currentReserved=0,currentRR=0, coordX = 0, coordY = 0;
	uint32_t currentDebug=0;
	uint8_t  currentDetType=0,currentVersion=0;
	bool data = false;

	//need to keep track of image number by myself
	//const int Nsockets=numSockets;
	int fnum[4]={0,};

	// loop through number of sockets
	for (int imod=0; imod<numSockets; ++imod) {
		// receive socket
	  ZmqSocket* z = new ZmqSocket(socketip,portnum+imod);
	  zmqsocket.push_back(z);
	  
	  if (zmqsocket[imod]->Connect()) {
	    cprintf(RED, "Error: Could not create Zmq socket on port %d with ip %s\n", portnum+imod, socketip);
	    //delete zmqsocket[imod];
	    return EXIT_FAILURE;
	  }
	  printf("Zmq Client[%d] at %s\n",imod, zmqsocket[imod]->GetZmqServerAddress());
	  
	  // send socket
	  if (send) {
	   	ZmqSocket* z2 = new ZmqSocket(portnum2+imod, socketip2);
		zmqsocket2.push_back(z2);
		if (zmqsocket2[imod]->Connect()) {
	      bprintf(RED, "Error: Could not create Zmq socket server[%d] on port %d and ip %s\n", imod, portnum2+imod, socketip2);
	      //delete zmqsocket2[imod];
	      //delete zmqsocket[imod];
	      return EXIT_FAILURE;
	    }
	    printf("Zmq Server[%d] started at %s\n", imod, zmqsocket2[imod]->GetZmqServerAddress());
	  }
	}
	
	
	// to keep track of any socket sending dummy (end of acquisition) ahead of the others
	bool runningList[numSockets];
	for(int i = 0; i < numSockets; ++i)
			runningList[i] = true;
	int numRunning = numSockets;

	// infinite loop
	while(1) {
	  cout<<"waiting..."<<endl;

		// reset data
		data = false;
		if (multiframe != NULL)
			memset(multiframe, 0xFF, multisize);

		for(int isocket=0; isocket<numSockets; ++isocket){

			// if dummy is not sent (not end of acquisition)
			if (runningList[isocket]) {

				// HEADER
				{
					rapidjson::Document doc;
					if (!zmqsocket[isocket]->ReceiveHeader(isocket, doc,
									       SLS_DETECTOR_JSON_HEADER_VERSION)) {
					  // cprintf(RED, "%d Got Dummy\n", isocket);
					  // stream dummy  to socket2
					  if (send) {
					    zmqsocket2[isocket]->SendHeaderData(isocket, true, 
										SLS_DETECTOR_JSON_HEADER_VERSION);
					    //cprintf(RED, "%d Sent Dummy\n", isocket);
					  }
	// parse error, version error or end of acquisition for socket
					  runningList[isocket] = false;
					  --numRunning;
					  fnum[isocket]=0;					
					  continue;
					}

					// if first message, allocate (all one time stuff)
					//	if (image == NULL) {
					  // allocate
					  size       = doc["size"].GetUint();
					  multisize  = size*numSockets;
					  image      = new char[size];
					  multiframe = new char[multisize];
					  memset(multiframe, 0xFF, multisize);
					  // dynamic range
					  dynamicRange  = doc["bitmode"].GetUint();
					  bytesPerPixel = (float)dynamicRange / 8;
					  // shape
					  nPixelsX = doc["shape"][0].GetUint();
					  nPixelsY = doc["shape"][1].GetUint();
					  cprintf(BLUE, "(One Time Header Info:\n"
						  "size: %u\n"
						  "multisize: %u\n"
						  "dynamicRange: %u\n"
						  "bytesPerPixel: %f\n"
						  "nPixelsX: %u\n"
						  "nPixelsY: %u\n",
						  size, multisize, dynamicRange, bytesPerPixel,
						  nPixelsX, nPixelsY);
					  
					  //  dynamicrange= dynamicRange ;
 
					  //}

					// each time, parse rest of header
					//definition of header in ZmqSocket.h
					currentFileName         = doc["fname"].GetString();
					currentAcquisitionIndex = doc["acqIndex"].GetUint64();
					currentFrameIndex       = doc["fIndex"].GetUint64();
					currentFileIndex        = doc["fileIndex"].GetUint64();
					currentBunchId    = doc["bunchId"].GetUint64();
					currentTimeStamp    = doc["timestamp"].GetUint64();
					currentSubFrameIndex     = doc["expLength"].GetUint();
					currentPacketNumber    = doc["packetNumber"].GetUint();
					coordY                  = doc["row"].GetUint();
					coordX                  = doc["column"].GetUint();
					//coordYs                  = doc["row"].GetUint();
					//coordXs                  = doc["column"].GetUint();
					currentModId       = doc["modId"].GetUint();
					flippedDataX = doc["flippedDataX"].GetUint();
					currentReserved = doc["reserved"].GetUint();
					currentDebug = doc["debug"].GetUint();
					currentRR=doc["roundRNumber"].GetUint();
					currentDetType=doc["detType"].GetUint();
					currentVersion=doc["version"].GetUint();
					++fnum[isocket];
				}//HEADER
				//cprintf(GREEN, "%d Got Header \n", isocket);
								
				// DATA
				data = true;
				int length = zmqsocket[isocket]->ReceiveData(isocket, image, size);
				//cprintf(GREEN, "%d Got Data\n", isocket);

				//processing with image
				uint32_t sum= 0;
				if (dynamicRange!=8 ) assert(0 && "only 8 bit supported for now");
				//only 8 bit here 
				//unsigned char* a dataout = new unsigned int [nPixelsX* nPixelsY];
				
				//to do the sum interpret all dynamic range
				//decodeData(image, dataout, size, nPixelsX, nPixelsY);  
				for (int ichan=0; ichan<size/bytesPerPixel; ++ichan) {
				  sum+=(image[ichan]&0xff);
					}
				
				//decodeData(image, dataout, size, nPixelsX, nPixelsY);  
				//if (dynamicRange==16 ){
				//for (int ichan=0; ichan<size/bytesPerPixel; ++ichan) {
				//  iptr=image[ichan*2+ibyte];
				///  for (ibyte=0; ibyte<2; ++ibyte) { 
				//	iptr=image[ichan+ibyte];  
				//	ival|=((iptr<<(ibyte*bytesize))&(0xff<<(ibyte*bytesize)));   
				//	sum+=(image[ichan]&0xff);
				//}
				//}

	



				cout<< "Socket "<< isocket <<"  Image "<<  fnum[isocket] <<"    Sum is  "<< sum <<endl;
			  //				for (int iy=10; iy<nPixelsY-10; ++iy) {
			  //	  for (int ix=10; ix<nPixelsX-10; ++ix){
			  //	      dataout[ix]=
			  //	    
			  //	  }
			  //}


				//stream to socket 2
				if (send) {
				  char fname[10000];
				  strcpy(fname,currentFileName.c_str());
				  zmqsocket2[isocket]->SendHeaderData(isocket,false, SLS_DETECTOR_JSON_HEADER_VERSION,  dynamicRange, currentFileIndex,
								      nPixelsX ,  nPixelsY, 
								      size,currentAcquisitionIndex, currentFrameIndex, fname,
								      currentAcquisitionIndex ,	currentSubFrameIndex, 	currentPacketNumber,   
								      currentBunchId, currentTimeStamp,
								      currentModId , 
								      coordY, coordX, 
								      currentReserved ,
								      currentDebug, currentRR, currentDetType,currentVersion,&flippedDataX,additionalJsonHeader);
				  cprintf(GREEN, "%d Sent Header \n", isocket);
				  
				  zmqsocket2[isocket]->SendData((char*)image,length);
				  cprintf(GREEN, "%d Sent Data\n", isocket);
				}
			}//running sockets
		}// end of for loop; got one image from all sockets

		
		// if end of acquisition for all sockets, wait for new measurement
		if(!numRunning){
		  cprintf(BLUE,"waiting for new measurement\n");
		  for(int i = 0; i < numSockets; ++i)
		    runningList[i] = true;
		  numRunning = numSockets;
		}
	}// exiting infinite loop


	
	cout<<"Goodbye"<<  endl;
	return 0;
}

