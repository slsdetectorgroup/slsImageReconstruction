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


int ImageMaker::openFile(string fname, ifstream &infile){

	infile.open(fname.c_str(),ios::in | ios::binary);
	if(!infile.is_open()){
		cprintf(RED, "Error: Could not read file: %s\n", fname.c_str());
		return slsReceiverDefs::FAIL;
	}
	return slsReceiverDefs::OK;
}


void ImageMaker::closeFile(ifstream &infile){
	if(infile.is_open())
		infile.close();
}


char* ImageMaker::readOneFrame(ifstream &infile){
	return receiverData->readNextFrame(infile);
}


double ImageMaker::getValue(int ix, int iy, char* buff){
	return (receiverData->getValue((char*)buff,ix,iy,dynamicrange));
}



/*run_one_frame_8bit_d0_0.raw

 * after htonl 0,0 :0
after htonl 1,0 :0
after htonl 254,0 :0
after htonl 255,0 :46080
after htonl 256,0 :46260
after htonl 257,0 :29876
after htonl 0,2 :0
after htonl 1,2 :0
after htonl 254,2 :0
after htonl 255,2 :46080
after htonl 256,2 :46260
after htonl 257,2 :46260
 */

