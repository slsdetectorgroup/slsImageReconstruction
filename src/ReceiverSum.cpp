/* A simple server in the internet domain using TCP
   The port number is passed as an argument */


//#include "Implementation.h"
//#include "ServerSocket.h"
//#include "receiver_defs.h"
//#include "sls_detector_defs.h"
//#include "sls_detector_funcs.h"

#include "Receiver.h"
#include "container_utils.h"
#include "logger.h"
#include "sls_detector_defs.h"

#include <csignal> //SIGINT
#include <cstring>
#include <iostream>
#include <semaphore.h>
#include <sys/syscall.h>
#include <sys/wait.h> //wait
#include <unistd.h>

//#include <iostream>
#include <string.h>
//#include <signal.h>	//SIGINT
//#include <cstdlib>		//system
//#include <cassert>



#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait
#include <unistd.h> 	//usleep
#include <syscall.h>

sem_t semaphore;

void sigInterruptHandler(int p) { sem_post(&semaphore); }

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


int startAcquisitionCallBack(std::string filepath, std::string filename, uint64_t fileindex, uint32_t datasize, void*p){
  cprintf(BLUE,"#### StartAcq:  filepath:%s  filename:%s fileindex:%lu  datasize:%u ####\n",
	  filepath.c_str(), filename.c_str(), fileindex, datasize);
 
  char str[300];
  for(int iport =0; iport<2; ++iport){
    sprintf(str,"%s/%sd%d_%lu.txt",filepath.c_str(), filename.c_str(), 
	    (portn==1954 ? 0+iport : 2+iport),
	    fileindex);
    
    fptr[iport]=fopen(str,"w");
  }
  cprintf(BLUE, "--StartAcq: returning 0\n");
  return 0;
}


void acquisitionFinishedCallBack(uint64_t frames, void*p){
  cprintf(BLUE, "#### AcquisitionFinished: frames:%lu ####\n",frames);
  for(int iport =0; iport<2; ++iport) fclose(fptr[iport]);
}

void rawDataReadyCallBack(char* metadata, char* datapointer, uint32_t datasize, void* p){
  slsDetectorDefs::sls_receiver_header* header = (slsDetectorDefs::sls_receiver_header*)metadata;
  slsDetectorDefs::sls_detector_header detectorHeader = header->detHeader;
  
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
  fprintf(fptr[ detectorHeader.column],"%lu %lu \n",
	  detectorHeader.frameNumber, sum);
}


int main(int argc, char *argv[]) {

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
	asa.sa_handler = SIG_IGN;  // handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	
	// invocation of handler
	if (sigaction(SIGPIPE, &asa, nullptr) == -1) {
	  cprintf(RED, "Could not set handler function for SIGPIPE\n");
	}

	//	int numReceivers = 1;

	/** - loop over number of receivers */
	//	for (int i = 0; i < numReceivers; ++i) {
	  
	/**	- fork process to create child process */
	// pid_t pid = fork();
	
        /**	- if fork failed, raise SIGINT and properly destroy all child
         * processes */
        //if (pid < 0) {
	//cprintf(RED, "fork() failed. Killing all the receiver objects\n");
	//  raise(SIGINT);
        //}
	
        /**	- if child process */
	Receiver *receiver = new Receiver(portn);
	    // } catch (...) {
	    //LOG(logINFOBLUE)
	    //    << "Exiting Child Process [ Tid: " << syscall(SYS_gettid)
	    //<< " ]";
	    //throw;
	    //}
	  
	
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
	  receiver->registerCallBackStartAcquisition(startAcquisitionCallBack,nullptr);


	/**
	  callback argument is
	  total farmes caught
	  registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg);
	 */
	  receiver->registerCallBackAcquisitionFinished(acquisitionFinishedCallBack,nullptr);


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
	  receiver->registerCallBackRawDataReady(rawDataReadyCallBack,nullptr);

     /**	- as long as no Ctrl+C */
	  sem_wait(&semaphore);
	  sem_destroy(&semaphore);
	  cprintf(BLUE, "Exiting Child Process [ Tid: %ld ]\n",
		  (long)syscall(SYS_gettid));
	  exit(EXIT_SUCCESS);
	  //	  break;
	  //}
	/** - Parent process ignores SIGINT (exits only when all child process
	 * exits) */
	sa.sa_flags = 0;          // no flags
	sa.sa_handler = SIG_IGN;  // handler function
	sigemptyset(&sa.sa_mask); // dont block additional signals during invocation
                              // of handler
	if (sigaction(SIGINT, &sa, nullptr) == -1) {
	  cprintf(RED, "Could not set handler function for SIGINT\n");
	}

/** - Print Ready and Instructions how to exit */
    std::cout << "Ready ... \n";
    cprintf(RESET, "\n[ Press \'Ctrl+c\' to exit ]\n");

    /** - Parent process waits for all child processes to exit */
    for (;;) {
        pid_t childPid = waitpid(-1, nullptr, 0);

        // no child closed
        if (childPid == -1) {
            if (errno == ECHILD) {
                cprintf(GREEN, "All Child Processes have been closed\n");
                break;
            } else {
                cprintf(RED, "Unexpected error from waitpid(): (%s)\n",
                        strerror(errno));
                break;
            }
        }

        // child closed
        cprintf(BLUE, "Exiting Child Process [ Tid: %ld ]\n",
                (long int)childPid);
    }
    delete receiver;
    std::cout << "Goodbye!\n";
    return 0;
}

