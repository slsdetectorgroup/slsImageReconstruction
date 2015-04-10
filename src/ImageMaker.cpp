/********************************************//**
 * @file ImageMaker.h
 * @short creates image data from file
 ***********************************************/

#include "ImageMaker.h"
#include "fileIOStatic.h"

#include <fstream>

ImageMaker::ImageMaker(int dr, int tg, bool top):dynamicrange(dr), tenGiga(tg),dataSize(0){

	int multfactor = 16;
	int bufferSize = 1040;
	dataSize = 1024;
	if(tenGiga){
		multfactor = 4;
		bufferSize = 4112;
		dataSize = 4096;
	}
	int packetsPerFrame = multfactor * dynamicrange*2;


	receiverData = new eigerHalfModuleData(dynamicrange,packetsPerFrame,bufferSize, dataSize, top);
}


ImageMaker::~ImageMaker(){
	if(receiverData)
		delete receiverData;
}

int ImageMaker::processFile(string fname){

	ifstream infile;
	infile.open(fname.c_str(),ios::in | ios::binary);
	if(infile.is_open()){

		/*separate them into open file and close file and read next frame and get value.*/
		char*receiverData = topData->readNextFrame(infile);
		int inum;
		for(inum = 0; inum < 2; inum++)
			cprintf(BLUE,"0,%d :%f\n",inum,(receiverData->getValue((char*)buff,inum,0,dynamicrange)));
		for(inum = 254; inum < 258; inum++)
			cprintf(BLUE,"%d,0 :%f\n",inum,(receiverData->getValue((char*)buff,inum,0,dynamicrange)));
		for(inum = 0; inum < 2; inum++)
			cprintf(BLUE,"%d,2 :%f\n",inum,(receiverData->getValue((char*)buff,inum,2,dynamicrange)));
		for(inum = 254; inum < 258; inum++)
			cprintf(BLUE,"%d,2 :%f\n",inum,(receiverData->getValue((char*)buff,inum,2,dynamicrange)));



		infile.close();
	}else{
		cprintf(RED, "Error: Could not read file: %s\n", fname.c_str());
		return slsReceiverDefs::FAIL;
	}
	return slsReceiverDefs::OK;



}

