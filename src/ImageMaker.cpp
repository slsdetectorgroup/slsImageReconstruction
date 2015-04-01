/********************************************//**
 * @file ImageMaker.h
 * @short creates image data from file
 ***********************************************/

#include "ImageMaker.h"

ImageMaker::ImageMaker(string fn, int dr, int tg):fname(fn), dynamicrange(dr), tenGiga(tg){

	int packetsPerFrame;
	switch(dr){
	case 4:		packetsPerFrame = 128; break;
	case 8:		packetsPerFrame = 256; break;
	case 16:	packetsPerFrame = 512; break;
	case 32:	packetsPerFrame = 1024; break;
	}

	int bufferSize = 1040;
	int dataSize = 1024;
	if(tenGiga){
		bufferSize = 4112;
		dataSize = 4096;
	}

	receiverData = new eigerHalfModuleData(dynamicrange,packetsPerFrame,bufferSize, dataSize);
}


ImageMaker::~ImageMaker(){
	if(receiverData)
		delete receiverData;
}

void ImageMaker::processFile(){

}


/*
for(inum = 0; inum < 2; inum++)
	cprintf(BLUE,"before htonl 0,%d :%f\n",inum,(receiverdata->getValue((char*)origVal,inum,0,dynamicrange)));
for(inum = 254; inum < 258; inum++)
	cprintf(BLUE,"before htonl %d,0 :%f\n",inum,(receiverdata->getValue((char*)origVal,inum,0,dynamicrange)));
for(inum = 0; inum < 2; inum++)
	cprintf(BLUE,"before htonl %d,2 :%f\n",inum,(receiverdata->getValue((char*)origVal,inum,2,dynamicrange)));
for(inum = 254; inum < 258; inum++)
	cprintf(BLUE,"before htonl %d,2 :%f\n",inum,(receiverdata->getValue((char*)origVal,inum,2,dynamicrange)));

	one frame
	after htonl 0,0 :2
	after htonl 1,0 :2
	after htonl 254,0 :2
	after htonl 255,0 :2
	after htonl 256,0 :0
	after htonl 257,0 :0
	after htonl 0,2 :2
	after htonl 1,2 :2
	after htonl 254,2 :2
	after htonl 255,2 :2
	after htonl 256,2 :0
	after htonl 257,2 :0


	two frames
	after htonl 0,0 :2
	after htonl 1,0 :2
	after htonl 254,0 :2
	after htonl 255,0 :2
	after htonl 256,0 :0
	after htonl 257,0 :0
	after htonl 0,2 :2
	after htonl 1,2 :2
	after htonl 254,2 :2
	after htonl 255,2 :2
	after htonl 256,2 :0
	after htonl 257,2 :0
	after htonl 0,0 :2
	after htonl 1,0 :2
	after htonl 254,0 :2
	after htonl 255,0 :2
	after htonl 256,0 :0
	after htonl 257,0 :145
	after htonl 0,2 :2
	after htonl 1,2 :2
	after htonl 254,2 :2
	after htonl 255,2 :2
	after htonl 256,2 :0
	after htonl 257,2 :0


*/
