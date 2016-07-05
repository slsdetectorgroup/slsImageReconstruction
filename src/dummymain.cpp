/* Reads files to get values for images
 * main.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: maliakal_d
 */

#include "sls_receiver_defs.h"
#include "slsReceiverData.h"
#include "eigerHalfModuleData.h"
#include "ansi.h"

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>			
#include <map>
#include <getopt.h>

using namespace std;

int getCommandParameters(int argc, char *argv[], string &file, int &bottom);

int getFileParameters(string file, int &hs, int &dr, int &ps, int &x, int &y);


int main(int argc, char *argv[]) {

	//declare variables
	slsReceiverData <uint32_t> *receiverdata = NULL;
	char* buffer = NULL;
	ifstream infile;
	string file = "";
	int bottom, headersize, dynamicrange, xpix, ypix, packetSize;

	//get command parameters
	if(getCommandParameters(argc,argv,file,bottom) != slsReceiverDefs::OK)
		return -1;

	//get file parameters
	if(getFileParameters(file, headersize, dynamicrange, packetSize, xpix, ypix) != slsReceiverDefs::OK)
		return -1;


	//validations
	switch(dynamicrange){
	case 4: case 8: case 16: case 32: break;
	default:
		cout << "Error: Invalid dynamic range " << dynamicrange << " read from file " << file << endl;
		return -1;
	}

	int tenGiga, dataSize, packetsPerFrame;
	switch(packetSize){
	case 1040: tenGiga = 0; dataSize = 1024; packetsPerFrame = 16 * dynamicrange * 2; break;
	case 4112: tenGiga = 1; dataSize = 4096; packetsPerFrame = 4 * dynamicrange * 2;  break;
	default:
		cprintf(RED, "Error: Invalid packet size %d read from file %s\n", packetSize,file.c_str());
		return -1;
	}

	//print variables
	cout << "\n\nFile name\t:" << file <<
			"\nX pixels\t:" << xpix <<
			"\nY pixels\t:" << ypix <<
			"\nDynamic range\t:"<< dynamicrange <<
			"\nTen giga\t:" << tenGiga <<
			"\nPacket Size\t:" << packetSize <<
			"\nData Size\t:" << dataSize <<
			"\nPacket per Frame:" << packetsPerFrame <<
			"\nHalfMoule\t:";	if(bottom)	cout << "bottom";	else cout << "top";
	cout << endl << endl;


	//read values
	int ix=0, iy=0, numFrames, fnum;

	if(!file.empty()){
		//construct top datamapping object
		numFrames = 0;
		fnum = -1;
		receiverdata = new eigerHalfModuleData(dynamicrange,packetsPerFrame,packetSize, dataSize, (bottom?0:1));
		//open file
		infile.open(file.c_str(),ios::in | ios::binary);
		if(infile.is_open()){

			char data[headersize];
			infile.read(data,headersize);

			//get frame buffer
			while((buffer = receiverdata->readNextFrame(infile, fnum))){
				//while((buffer = receiverdata->readNextFrame(infile))){
				cout << "Reading values for frame #" << fnum << endl;
				//getting values


				for(iy = 0; iy < 5; iy++){
					for(ix = 0; ix < 2; ix++)
						cprintf(BLUE,"%d,%d :%f\n",ix,iy,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
					for(ix = 254; ix < 258; ix++)
						cprintf(BLUE,"%d,%d :%f\n",ix,iy,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
				}

				delete [] buffer;
				numFrames++;
			}
			//close file
			infile.close();
		}else cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
		delete receiverdata;
		cout  << "Found " << numFrames << " frames in file." << endl << endl;
	}


	cout << endl << "Goodbye!" << endl;
	return slsReceiverDefs::OK;

}


/**
 * Parse command line parameters to get dynamic range, ten giga enable and file names
 */
int getCommandParameters(int argc, char *argv[], string &file, int &bottom){

	map<string, string> configuration_map;
	size_t detPosition,framePosition,filePosition,endPosition;

	static struct option long_options[] = {
			{"file",     		required_argument,       0, 'f'},
			{"help",  			no_argument,      		 0, 'h'},
			{0, 0, 0, 0}
	};

	int option_index = 0, c;

	if(argc<3){
		cprintf(RED,"ERROR: Invalid arguments. image --file [file]\n");
		return slsReceiverDefs::FAIL;
	}

	while ( c != -1 ){
		c = getopt_long (argc, argv, "tfh", long_options, &option_index);
		if (c == -1)//end of options
			break;
		switch(c){

			case 'f':
				file = optarg;
				if(file.empty()){
					cprintf(RED,"ERROR: Empty file name.\n");
					return slsReceiverDefs::FAIL;
				}

				detPosition=file.rfind("_d");
				if(detPosition == string::npos){
					cprintf(RED, "Error: Invalid file name. Cannot find detector index from %s\n",file.c_str());
					return slsReceiverDefs::FAIL;
				}
				framePosition=file.rfind("_f00",detPosition);
				filePosition=file.rfind("_");
				endPosition = framePosition;
				if(framePosition == string::npos)
					endPosition = filePosition;
				if (!sscanf(file.substr(detPosition+1,endPosition-1-detPosition).c_str(),"d%d",&bottom)) {
					cprintf(RED, "Error: Invalid file name. Cannot parse detector index from %s\n",file.c_str());
					return slsReceiverDefs::FAIL;
				}

				bottom%=2;
				break;
			case 'h':
				string help_message = """\nSLS Image Reconstruction\n\n""";
				help_message += """usage: image [--f file_name]\n\n""";
				help_message += """\t--file:\t file name of any raw file\n""";
				cout << help_message << endl;
				break;

		}
	}

	return slsReceiverDefs::OK;
}



int getFileParameters(string file, int &hs, int &dr, int &ps, int &x, int &y){
	cout << "Getting File Parameters from " << file << endl;
	string str;
	ifstream infile;

	infile.open(file.c_str(),ios::in | ios::binary);
	if (infile.is_open()) {

		//header size
		if(getline(infile,str)){
			istringstream sstr(str);
			sstr >> str >> hs;
		}

		//dynamic range
		if(getline(infile,str)){
			istringstream sstr(str);
			sstr >> str >> str >> dr;
		}

		//packet size
		if(getline(infile,str)){
			istringstream sstr(str);
			sstr >> str >> ps;
		}

		//x
		if(getline(infile,str)){
			istringstream sstr(str);
			sstr >> str >> x;
		}

		//y
		if(getline(infile,str)){
			istringstream sstr(str);
			sstr >> str >> y;
		}

		infile.close();
	}else{
		cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
		return slsReceiverDefs::FAIL;
	}

	return slsReceiverDefs::OK;
}
