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

int getCommandParameters(int argc, char *argv[], string &file);

int getFileParameters(string file, int &hs, int &tp, int &lt, int &dr, int &tg, int &ps, int &ds, int &x, int &y);


int main(int argc, char *argv[]) {

	//declare variables
	slsReceiverData <uint32_t> *receiverdata = NULL;
	char* buffer = NULL;
	ifstream infile;
	string file = "";
	int headersize, top, left, dynamicrange, tenGiga, packetSize, dataSize, xpix, ypix;

	//get command parameters
	if(getCommandParameters(argc,argv,file) != slsReceiverDefs::OK)
		return -1;

	//get file parameters
	if(getFileParameters(file, headersize, top, left, dynamicrange, tenGiga, packetSize, dataSize, xpix, ypix) != slsReceiverDefs::OK)
		return -1;

	//validations
	switch(dynamicrange){
	case 4: case 8: case 16: case 32: break;
	default:
		cout << "Error: Invalid dynamic range " << dynamicrange << " read from file " << file << endl;
		return -1;
	}
	if(!tenGiga){
		if(packetSize!=1040){
			cout << "Error: Invalid packet size " << packetSize << " for 1g read from file " << file << endl;
			return -1;
		}
		if(dataSize!=1024){
			cout << "Error: Invalid data size " << dataSize << " for 1g read from file " << file << endl;
			return -1;
		}
	}else{
		if(packetSize!=4112){
			cout << "Error: Invalid packet size " << packetSize << " for 10g read from file " << file << endl;
			return -1;
		}
		if(dataSize!=4096){
			cout << "Error: Invalid data size " << dataSize << " for 10g read from file " << file << endl;
			return -1;
		}
	}


	int packetsPerFrame;
	if(!tenGiga)
		packetsPerFrame = 16 * dynamicrange;
	else
		packetsPerFrame = 4 * dynamicrange;

	//print variables
	cout << "\n\nFile name\t:" << file <<
			"\nHalfMoule\t:";	if(top)	cout << "top ";	else cout << "bottom ";
								if(left)cout << "left";	else cout << "right";
	cout <<	"\nDynamic range\t:"<< dynamicrange <<
			"\nTen giga\t:" << tenGiga <<
			"\nPackets/Frame\t:" << packetsPerFrame <<
			"\nPacket Size\t:" << packetSize <<
			"\nData Size\t:" << dataSize <<
			"\nX pixels\t:" << xpix <<
			"\nY pixels\t:" << ypix << endl << endl;

	//read values
	int ix=0, iy=0, numFrames, fnum;

	if(!file.empty()){
		//construct top datamapping object
		numFrames = 0;
		fnum = -1;
		receiverdata = new eigerHalfModuleData(top, left, dynamicrange, tenGiga, packetSize, dataSize, packetsPerFrame, xpix, ypix);
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
				double value=0;

				for(iy = 0; ypix < 1; iy++){
					for(ix = 0; ix < xpix; ix++){
						value = receiverdata->getValue((char*)buffer,ix,iy);
						//cprintf(BLUE,"%d,%d :%f\n",ix,iy,value);
					}
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
int getCommandParameters(int argc, char *argv[], string &file){

	map<string, string> configuration_map;
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



int getFileParameters(string file, int &hs, int &tp, int &lt, int &dr, int &tg, int &ps, int &ds, int &x, int &y){
	cout << "Getting File Parameters from " << file << endl;
	string str;
	ifstream infile;

	infile.open(file.c_str(),ios::in | ios::binary);
	if (infile.is_open()) {

		//empty line
		getline(infile,str);

		//header size
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> hs;
		}

		//bottom
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> tp;
		}

		//right
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> lt;
		}

		//dynamic range
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> str >> dr;
		}

		//ten giga
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> str >> tg;
		}

		//packet size
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> ps;
		}

		//data size
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> ds;
		}
		//x
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> x;
		}

		//y
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> y;
		}


		infile.close();
	}else{
		cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
		return slsReceiverDefs::FAIL;
	}

	return slsReceiverDefs::OK;
}
