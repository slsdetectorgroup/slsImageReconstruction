/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "sls_receiver_defs.h"
#include "slsReceiverUsers.h"
#include "slsReceiver.h"

#include <iostream>
#include <string.h>
#include <signal.h>	//SIGINT
#include <cstdlib>		//system
#include <cassert>

#include "utilities.h"
#include "logger.h"

#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait
#include <unistd.h> 	//usleep
#include <syscall.h>


bool keeprunning;

void sigInterruptHandler(int p){
	keeprunning = false;
}

FILE *fptr[2];//2 as left and right
int portn;
char iptr;
const int bytesize=8;
unsigned int ival;
unsigned int* ivalp;

int ichanmin4b=(256*4)/2;
int ichanmax4b=(256)*256*2/2;
int ichanmin=(256*4);
int ichanmax=(256)*256*2;


int startAcquisitionCallBack(char* filepath, char* filename, uint64_t fileindex, uint32_t datasize, void*p){
  cprintf(BLUE,"#### StartAcq:  filepath:%s  filename:%s fileindex:%lld  datasize:%u ####\n",
	 filepath, filename, fileindex, datasize);
 
  char str[300];
  for(int iport =0; iport<2; ++iport){
    sprintf(str,"%s/%sd%d_%lld.txt",filepath, filename, 
	    (portn==1954 ? 0+iport : 2+iport),
	    fileindex);
    
    fptr[iport]=fopen(str,"w");
  }
  cprintf(BLUE, "--StartAcq: returning 0\n");
  return 0;
}


void acquisitionFinishedCallBack(uint64_t frames, void*p){
  cprintf(BLUE, "#### AcquisitionFinished: frames:%llu ####\n",frames);
  for(int iport =0; iport<2; ++iport) fclose(fptr[iport]);
}

void rawDataReadyCallBack(char* metadata, char* datapointer, uint32_t datasize, void* p){
  slsReceiverDefs::sls_receiver_header* header = (slsReceiverDefs::sls_receiver_header*)metadata;
  slsReceiverDefs::sls_detector_header detectorHeader = header->detHeader;
  
  uint64_t sum= 0;
  /*	
	PRINT_IN_COLOR (detectorHeader.modId?detectorHeader.modId:detectorHeader.row,
	"#### %d GetData: ####\n"
	"frameNumber: %llu\t\texpLength: %u\t\tpacketNumber: %u\t\tbunchId: %llu"
	"\t\ttimestamp: %llu\t\tmodId: %u\t\t"
	"xCrow%u\t\tcolumn: %u\t\tcolumn: %u\t\tdebug: %u"
	"\t\troundRNumber: %u\t\tdetType: %u\t\tversion: %u"
	//"\t\tpacketsMask:%s"
	"\t\tfirstbytedata: 0x%x\t\tdatsize: %u\n\n",
	detectorHeader.row, detectorHeader.frameNumber,
	detectorHeader.expLength, detectorHeader.packetNumber, detectorHeader.bunchId,
	detectorHeader.timestamp, detectorHeader.modId,
	detectorHeader.row, detectorHeader.column, detectorHeader.column,
	detectorHeader.debug, detectorHeader.roundRNumber,
	detectorHeader.detType, detectorHeader.version,
	//header->packetsMask.to_string().c_str(),
	  ((uint8_t)(*((uint8_t*)(datapointer)))), datasize);
  */
  
  //skip 2 raws of pixels near the center of the module for hot pixels
  
  //use datasize to know dynamic range

  //need to get the coirrect size of data at this point
  if(datasize==65536){
    for (int ichan=ichanmin4b; ichan<ichanmax4b; ++ichan) {
      uint8_t iptr = datapointer[ichan];
      sum+=(iptr &0xf);
      sum+= (iptr>>4);// &0xf;
 /*
      iptr=datapointer[ichan];//&0xff; //read the byte
      for (int ipos=0; ipos<2; ++ipos) {
	//loop over the 8bit (twice)
	sum+=(iptr>>(ipos*4))&0xf;		//pick the right 4bit
      }
*/
    }//ichan
  }//4 bit
  
  if(datasize==131072){
    //8 bits
    //skip 2 raws of pixels near the center of the module for hot pixels
    for (int ichan=ichanmin; ichan<ichanmax; ++ichan) {
      sum+=(uint8_t)datapointer[ichan]; //(datapointer[ichan]&0xff);
    }
  }//8 bit
  
  if(datasize==262144){
    unsigned short *datapointer16=(unsigned short*) datapointer;
    //16 bits
    //skip 2 raws of pixels near the center of the module for hot pixels
    for (int ichan=ichanmin; ichan<ichanmax; ++ichan) {
      sum+= datapointer16[ichan];
    }
  }//16 bit
  if(datasize==524288){
    unsigned int *datapointer32=(unsigned int*) datapointer;
    //32 bits
    //skip 2 raws of pixels near the center of the module for hot pixels
    for (int ichan=ichanmin; ichan<ichanmax; ++ichan) {
      sum+=datapointer32[ichan];
    }
  }
  
  //column here tells me if left or right
  fprintf(fptr[ detectorHeader.column],"%d %lld \n",
	  detectorHeader.frameNumber, sum);
}


int main(int argc, char *argv[]) {

	keeprunning = true;
	portn=1954;
	if(argc>2) {
	  //std::cout<< argv[2]<<std::endl;
	  //std::cout<< atoi(argv[2])<<std::endl;
	  if(argc>2) portn=atoi(argv[2]);
	}

	cprintf(BLUE,"Created [ Tid: %ld ]\n", (long)syscall(SYS_gettid));

	// Catch signal SIGINT to close files and call destructors properly
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
	sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		cprintf(RED, "Could not set handler function for SIGINT\n");
	}


	// if socket crash, ignores SISPIPE, prevents global signal handler
	// subsequent read/write to socket gives error - must handle locally
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=SIG_IGN;					// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGPIPE, &asa, NULL) == -1) {
		cprintf(RED, "Could not set handler function for SIGPIPE\n");
	}


	int ret = slsReceiverDefs::OK;
	slsReceiverUsers *receiver = new slsReceiverUsers(argc, argv, ret);
	if(ret==slsReceiverDefs::FAIL){
		delete receiver;
		cprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
		exit(EXIT_FAILURE);
	}

	//register callbacks

	/**
	   callback arguments are
	   filepath
	   filename
	   fileindex
	   datasize

	   return value is 
	   0 raw data ready callback takes care of open,close,write file
	   1 callback writes file, we have to open, close it
	   2 we open, close, write file, callback does not do anything

	   registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg);
	 */
	receiver->registerCallBackStartAcquisition(startAcquisitionCallBack,NULL);


	/**
	  callback argument is
	  total farmes caught
	  registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg);
	 */
	receiver->registerCallBackAcquisitionFinished(acquisitionFinishedCallBack,NULL);


	/**
	  args to raw data ready callback are
	  sls_receiver_header frame metadata
	  datapointer
	  file descriptor
	  guidatapointer (NULL, no data required)
	  NEVER DELETE THE DATA POINTER
	  REMEMBER THAT THE CALLBACK IS BLOCKING
	  registerCallBackRawDataReady(void (*func)(char*, char*, uint32_t, void*),void *arg);
	 */
	receiver->registerCallBackRawDataReady(rawDataReadyCallBack,NULL);



	//start tcp server thread
	if (receiver->start() == slsReceiverDefs::FAIL){
		delete receiver;
		cprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
		exit(EXIT_FAILURE);
	}

	FILE_LOG(logINFO) << "Ready ... ";
	cprintf(RESET, "\n[ Press \'Ctrl+c\' to exit ]\n");
	while(keeprunning)
		pause();

	delete receiver;
	cprintf(BLUE,"Exiting [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
	FILE_LOG(logINFO) << "Goodbye!";
	return 0;
}

