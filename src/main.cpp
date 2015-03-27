/*
 * main.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: maliakal_d
 */

#include "sls_receiver_defs.h"
#include "slsReceiverData.h"


#include <iostream>
#include <string.h>
using namespace std;


int main(int argc, char *argv[]) {

	slsReceiverData <uint32_t> *receiverData = 0;

	if(receiverData)
		delete receiverData;

#include "slsReceiverData.h"
	slsReceiverData<uint32_t>  *receiverdata;

#include "eigerHalfModuleData.h"
	receiverdata(NULL)


	:eiger_read_frame()
	   int inum = 0;
	+/*                     //dr = 16, hence uint16_t
	+                       for(inum = 0; inum < 2; inum++)
	+                               cprintf(YELLOW,"before htonl %d,0 :%d\n",inum,((uint16_t)(*((uint16_t*)((char*)(retval+(inum*(dynamicrange/8))))))));
	+                       for(inum = 254; inum < 258; inum++)
	+                               cprintf(YELLOW,"before htonl %d,0 :%d\n",inum,((uint16_t)(*((uint16_t*)((char*)(retval+(inum*(dynamicrange/8))))))));
	+                       for(inum = 0; inum < 2; inum++)
	+                               cprintf(YELLOW,"before htonl %d,2 :%d\n",inum,((uint16_t)(*((uint16_t*)((char*)(retval+((2048+inum)*(dynamicrange/8))))))));
	+                       for(inum = 254; inum < 258; inum++)
	+                               cprintf(YELLOW,"before htonl %d,2 :%d\n",inum,((uint16_t)(*((uint16_t*)((char*)(retval+((2048+inum)*(dynamicrange/8))))))));
	+
	+*/
	+/*
	+                       for(inum = 0; inum < 2; inum++)
	+                               cprintf(BLUE,"before htonl 0,%d :%f\n",inum,(receiverdata->getValue((char*)origVal,inum,0,dynamicrange)));
	+                       for(inum = 254; inum < 258; inum++)
	+                               cprintf(BLUE,"before htonl %d,0 :%f\n",inum,(receiverdata->getValue((char*)origVal,inum,0,dynamicrange)));
	+                       for(inum = 0; inum < 2; inum++)
	+                               cprintf(BLUE,"before htonl %d,2 :%f\n",inum,(receiverdata->getValue((char*)origVal,inum,2,dynamicrange)));
	+                       for(inum = 254; inum < 258; inum++)
	+                               cprintf(BLUE,"before htonl %d,2 :%f\n",inum,(receiverdata->getValue((char*)origVal,inum,2,dynamicrange)));
	+*/
	                        //64 bit htonl cuz of endianness
	                        for(i=0;i<(1024*(16*dynamicrange)*2)/8;i++){
	                                (*(((uint64_t*)retval)+i)) = be64toh(((uint64_t)(*(((uint64_t*)retval)+i))));
	-                        /*
	+
	+                               /*
	                          int64_t temp;
	                          temp = ((uint64_t)(*(((uint64_t*)retval)+i)));
	                          temp = ((temp << 8) & 0xFF00FF00FF00FF00ULL ) | ((temp >> 8) & 0x00FF00FF00FF00FFULL );
	@@ -1501,6 +1528,18 @@ int      slsReceiverTCPIPInterface::eiger_read_frame(){
	                          (*(((uint64_t*)retval)+i)) = temp;
	                          */
	                        }
	+
	+/*                     //dr = 16, hence uint16_t
	+                       for(inum = 0; inum < 2; inum++)
	+                               cprintf(MAGENTA,"after htonl %d,0 :%d\n",inum,((uint16_t)(*((uint16_t*)((char*)(retval+(inum*(dynamicrange/8))))))));
	+                       for(inum = 254; inum < 258; inum++)
	+                               cprintf(MAGENTA,"after htonl %d,0 :%d\n",inum,((uint16_t)(*((uint16_t*)((char*)(retval+(inum*(dynamicrange/8))))))));
	+                       for(inum = 0; inum < 2; inum++)
	+                               cprintf(MAGENTA,"after htonl %d,2 :%d\n",inum,((uint16_t)(*((uint16_t*)((char*)(retval+((2048+inum)*(dynamicrange/8))))))));
	+                       for(inum = 254; inum < 258; inum++)
	+                               cprintf(MAGENTA,"after htonl %d,2 :%d\n",inum,((uint16_t)(*((uint16_t*)((char*)(retval+((2048+inum)*(dynamicrange/8))))))));
	+
	+*/




	 int slsReceiverTCPIPInterface::set_dynamic_range() {
	                        retval=receiverBase->setDynamicRange(dr);
	                        dynamicrange = dr;
	                         if(myDetectorType == EIGER){
	-                                if(!tenGigaEnable)
	+                                if(!tenGigaEnable){
	                                         packetsPerFrame = EIGER_ONE_GIGA_CONSTANT * dynamicrange * EIGER_MAX_PORTS;
	-                                else
	+                                        if(receiverdata){ delete receiverdata;receiverdata = NULL;}
	+                                        receiverdata = new eigerHalfModuleData(dynamicrange,packetsPerFrame,EIGER_ONE_GIGA_ONE_PACKET_SIZE, EIGER_ONE_GIGA_O
	+                                }else{
	                                         packetsPerFrame = EIGER_TEN_GIGA_CONSTANT * dynamicrange * EIGER_MAX_PORTS;
	+                                }
	                         }
	                }
	        }








	cout << "Goodbye!" << endl;
	return 0;
}
