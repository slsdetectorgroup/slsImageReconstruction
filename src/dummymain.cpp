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
#include <time.h>

using namespace std;

int getCommandParameters(int argc, char *argv[], string &file);

int getFileParameters(string file, int &hs, int &tp, int &lt, int &act, int& mp,  int &dr, int &tg, int &ps, int &ds, int &x, int &y);


int main(int argc, char *argv[]) {

	//declare variables
	slsReceiverData <uint32_t> *receiverdata = NULL;
	char* buffer = NULL;
	ifstream infile;
	string file = "";
	int headersize, top, left, active, missingpackets, dynamicrange, tenGiga, packetSize, dataSize, xpix, ypix;

	//get command parameters
	if(getCommandParameters(argc,argv,file) != slsReceiverDefs::OK)
		return -1;

	//get file parameters
	if(getFileParameters(file, headersize, top, left, active, missingpackets, dynamicrange, tenGiga, packetSize, dataSize, xpix, ypix) != slsReceiverDefs::OK)
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
	if(active) cout << "\nActive"; else cout << "\nInactive";
	cout << "\nMissing Packets:\t" << missingpackets <<
			"\nDynamic range\t:"<< dynamicrange <<
			"\nTen giga\t:" << tenGiga <<
			"\nPackets/Frame\t:" << packetsPerFrame <<
			"\nPacket Size\t:" << packetSize <<
			"\nData Size\t:" << dataSize <<
			"\nX pixels\t:" << xpix <<
			"\nY pixels\t:" << ypix << endl << endl;

	//read values
	int ix=0, iy=0, numFrames, fnum;

	if(!file.empty()){
		struct timespec begin,end; //requires -lrt in Makefile
		int oneframesize = packetsPerFrame * packetSize;
		int* intbuffer = NULL;





/*

		clock_gettime(CLOCK_REALTIME, &begin);
		double** value =new double*[ypix];
		for(int i=0;i<ypix;i++)
			value[i] = new double[xpix];
		if(!active){
			for(iy = 0; iy < ypix; iy++){
				for(ix = 0; ix < xpix; ix++){
					value[iy][ix] = -1;
				}
			}
		}else{
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
					//cout << "Reading values for frame #" << fnum << endl;
					//getting values
					for(iy = 0; iy < ypix; iy++){
						for(ix = 0; ix < xpix; ix++){
							value[iy][ix] = receiverdata->getValue((char*)buffer,ix,iy);
							//cprintf(BLUE,"%d,%d :%f\n",ix,iy,value[iy][ix]);
						}
					}


					delete [] buffer;
					numFrames++;
				}
				//close file
				infile.close();
			}else cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
			delete receiverdata;
		}
		for(int i=0;i<ypix;i++)
			delete [] value[i];
		clock_gettime(CLOCK_REALTIME, &end);
		cprintf(BLUE,"1 Elapsed time:%f seconds\n",( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0);







		clock_gettime(CLOCK_REALTIME, &begin);
		double* value2 = new double[xpix*ypix];
		if(!active){
			for(iy = 0; iy < ypix; iy++){
				for(ix = 0; ix < xpix; ix++){
					value2[iy*xpix+ix] = -1;
				}
			}
		}else{
			numFrames = 0;
			fnum = -1;
			receiverdata = new eigerHalfModuleData(top, left, dynamicrange, tenGiga, packetSize, dataSize, packetsPerFrame, xpix, ypix);
			//open file
			infile.open(file.c_str(),ios::in | ios::binary);
			if(infile.is_open()){

				char data[headersize];
				infile.read(data,headersize);

				buffer = new char[oneframesize];
				while(infile.read(buffer,oneframesize)) {
					fnum = receiverdata->getFrameNumber(buffer);
					//cout << "Reading values for frame #" << fnum << endl;
					receiverdata->getChannelArray(value2,buffer);
					//for(int i=0;i<xpix*ypix;++i)
					//		cprintf(BLUE,"%d,%d :%f\n",ix,iy,value2[i]);
					numFrames++;
					delete [] buffer;
					buffer = new char[oneframesize];
				}

				delete [] buffer;
				//close file
				infile.close();
			}else cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
			delete receiverdata;
		}
		delete [] value2;
		clock_gettime(CLOCK_REALTIME, &end);
		cprintf(BLUE,"2 Elapsed time:%f seconds\n",( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0);






		clock_gettime(CLOCK_REALTIME, &begin);
		int* value3 = new int[xpix*ypix];
		if(!active){
			for(iy = 0; iy < ypix; iy++){
				for(ix = 0; ix < xpix; ix++){
					value3[iy*xpix+ix] = -1;
				}
			}
		}else{
		receiverdata = new eigerHalfModuleData(top, left, dynamicrange, tenGiga, packetSize, dataSize, packetsPerFrame, xpix, ypix);
		numFrames = 0;
		fnum = -1;
		//open file
		infile.open(file.c_str(),ios::in | ios::binary);
		if(infile.is_open()){

			char data[headersize];
			infile.read(data,headersize);

			while((intbuffer = receiverdata->readNextFrameOnlyData(infile,fnum))) {
				//cout << "Reading values for frame #" << fnum << endl;
				value3 = receiverdata->decodeData(intbuffer);
				/*if(fnum==1){
					for(iy = 60; iy < 61; iy++){
						for(ix = 90; ix < 140; ix++){
							cprintf(BLUE,"%d,%d :%d\n",ix,iy,value3[iy*xpix+ix]);
						}
					}
				}*/
/*
				numFrames++;
				delete [] intbuffer;
			}

			//close file
			infile.close();
		}else cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
		delete receiverdata;
		}
		delete [] value3;
		clock_gettime(CLOCK_REALTIME, &end);
		cprintf(BLUE,"3 Elapsed time:%f seconds\n",( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0);



*/


		clock_gettime(CLOCK_REALTIME, &begin);
		int* value4 = new int[xpix*ypix];
		if(!active){
			for(iy = 0; iy < ypix; iy++){
				for(ix = 0; ix < xpix; ix++){
					value4[iy*xpix+ix] = -1;
				}
			}
		}else{
		receiverdata = new eigerHalfModuleData(top, left, dynamicrange, tenGiga, packetSize, dataSize, packetsPerFrame, xpix, ypix);
		numFrames = 0;
		fnum = -1;
		//open file
		infile.open(file.c_str(),ios::in | ios::binary);
		if(infile.is_open()){
			char data[headersize];
			infile.read(data,headersize);

			while((intbuffer = receiverdata->readNextFramewithMissingPackets(infile,fnum))) {
				//cout << "Reading values for frame #" << fnum << endl;
				value4 = receiverdata->decodeData(intbuffer);
				/*if(fnum==1)
					for(iy = 60; iy < 61; iy++){
						for(ix = 90; ix < 140; ix++){
							cprintf(BLUE,"%d,%d :%d\n",ix,iy,value4[iy*xpix+ix]);
						}
					}
*/
				numFrames++;
				delete [] intbuffer;
			}

			//close file
			infile.close();
		}else cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
		delete receiverdata;
		}

		clock_gettime(CLOCK_REALTIME, &end);
		cprintf(BLUE,"4 Elapsed time:%f seconds\n",( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0);






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



int getFileParameters(string file, int &hs, int &tp, int &lt, int &act, int& mp, int &dr, int &tg, int &ps, int &ds, int &x, int &y){
	cout << "Getting File Parameters from " << file << endl;
	string str;
	ifstream infile;

/*
	Header		 	400 bytes
	Top		 		1
	Left		 	1
	Active		 	1
	Packets Lost	6
	Dynamic Range	16
	Ten Giga	 	0
	Packet		 	1040 bytes
	Data		 	1024 bytes
	x		 		512 pixels
	y		 		256 pixels
*/


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

		//active
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> act;
		}
		//missing packets
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str:"<<str<<endl;
			sstr >> str >> mp;
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
