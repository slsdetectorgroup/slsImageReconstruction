/* Reads files to get values for images
 * main.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: maliakal_d
 */

#include "sls_receiver_defs.h"
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

int* decodeData(int *datain, const int size, const int nx, const int ny, const int dynamicRange);

int getCommandParameters(int argc, char *argv[], string &file);

int getFileParameters(string file, int &hs, int &tp, int &lt, int &act, int &dr, int &tg, int &is, int &x, int &y);


int main(int argc, char *argv[]) {

	//declare variables
	ifstream infile;
	string file = "";
	int fileheadersize, top, left, active, dynamicrange, tenGiga, imageSize, xpix, ypix;

	//get command parameters
	if(getCommandParameters(argc,argv,file) != slsReceiverDefs::OK)
		return -1;

	//get file parameters
	if(getFileParameters(file, fileheadersize, top, left, active, dynamicrange, tenGiga, imageSize, xpix, ypix) != slsReceiverDefs::OK)
		return -1;

	//validations
	switch(dynamicrange){
	case 4: case 8: case 16: case 32: break;
	default:
		cout << "Error: Invalid dynamic range " << dynamicrange << " read from file " << file << endl;
		return -1;
	}
	int packetsPerFrame;
	if(!tenGiga)
		packetsPerFrame = 16 * dynamicrange;
	else
		packetsPerFrame = 4 * dynamicrange;
	if(!tenGiga){
		if(imageSize!=(packetsPerFrame*1024)){
			cout << "Error: Invalid packet size " << imageSize << " for 1g read from file " << file << endl;
			return -1;
		}
	}else{
		if(imageSize!=(packetsPerFrame*4096)){
			cout << "Error: Invalid packet size " << imageSize << " for 10g read from file " << file << endl;
			return -1;
		}
	}

	//print variables
	cout << "\n\nFile name\t:" << file <<
			"\nHalfMoule\t:";	if(top)	cout << "top ";	else cout << "bottom ";
								if(left)cout << "left";	else cout << "right";

	cout<<"\nStatus\t\t:";
	if(active) cout << "active"; else cout << "inactive";
	cout << "\nDynamic range\t:"<< dynamicrange <<
			"\nTen giga\t:" << tenGiga <<
			"\nPackets/Frame\t:" << packetsPerFrame <<
			"\nImage Size\t:" << imageSize <<
			"\nX pixels\t:" << xpix <<
			"\nY pixels\t:" << ypix << endl << endl;

	//read values
	int numFrames;
	uint64_t fnum, snum, bnum;

	int* value;


	if(!file.empty()){
		struct timespec begin,end; //requires -lrt in Makefile
		slsReceiverDefs::sls_detector_header detheader;
		int* intbuffer = new int[imageSize];

		clock_gettime(CLOCK_REALTIME, &begin);


		numFrames = 0;
		fnum = -1;
		snum=-1;
		bnum=-1;
		//open file
		infile.open(file.c_str(),ios::in | ios::binary);
		if(infile.is_open()){

			//read file header
			char data[fileheadersize];
			infile.read(data,fileheadersize);



			//read header
			while(infile.read((char*)&detheader,sizeof(detheader))) {
				fnum = detheader.frameNumber;
				cout << "Reading values for frame #" << fnum << endl;
				cout <<"bunchid#" << detheader.bunchId << "\texplength#" << detheader.expLength << endl;

				//read data
				if(!infile.read((char*)intbuffer,(imageSize)))
					break;
				value = decodeData(intbuffer, imageSize, xpix, ypix, dynamicrange);
				/*if(fnum==1)
					for(int iy = 60; iy < 61; iy++){
						for(int ix = 90; ix < 140; ix++){
							cprintf(BLUE,"%d,%d :%d\n",ix,iy,value[iy*xpix+ix]);
						}
					}
*/
				numFrames++;
				delete [] value;
			}

			//close file
			infile.close();
		}else cprintf(RED, "Error: Could not read file: %s\n", file.c_str());


		clock_gettime(CLOCK_REALTIME, &end);
		cprintf(BLUE,"Elapsed time:%f seconds\n",( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0);


		delete [] intbuffer;

		cout  << "Found " << numFrames << " frames in file." << endl << endl;
	}


	cout << endl << "Goodbye!" << endl;
	return slsReceiverDefs::OK;

}



int* decodeData(int *datain, const int size, const int nx, const int ny, const int dynamicRange) {

		int dataBytes = size;
		int nch = nx*ny;
		int* dataout = new int [nch];
		char *ptr=(char*)datain;
		char iptr;

		const int bytesize=8;
		int ival=0;
		int  ipos=0, ichan=0, ibyte;

		switch (dynamicRange) {
		case 4:
			for (ibyte=0; ibyte<dataBytes; ++ibyte) {//for every byte (1 pixel = 1/2 byte)
				iptr=ptr[ibyte]&0xff;				//???? a byte mask
				for (ipos=0; ipos<2; ++ipos) {		//loop over the 8bit (twice)
					ival=(iptr>>(ipos*4))&0xf;		//pick the right 4bit
					dataout[ichan]=ival;
					ichan++;
				}
			}
			break;
		case 8:
			for (ichan=0; ichan<dataBytes; ++ichan) {//for every pixel (1 pixel = 1 byte)
				ival=ptr[ichan]&0xff;				//????? a byte mask
				dataout[ichan]=ival;
			}
			break;
		case 16:
			for (ichan=0; ichan<nch; ++ichan) { 	//for every pixel
				ival=0;
				for (ibyte=0; ibyte<2; ++ibyte) { 	//for each byte (concatenate 2 bytes to get 16 bit value)
					iptr=ptr[ichan*2+ibyte];
					ival|=((iptr<<(ibyte*bytesize))&(0xff<<(ibyte*bytesize)));
				}
				dataout[ichan]=ival;
			}
			break;
		default:
		  ;								//for every 32 bit (every element in datain array)
		  //for (ichan=0; ichan<nch; ++ichan) { 	//for every pixel
			  //ival=datain[ichan]&0xffffff;
		  //	dataout[ichan]=ival;
		  //	}
		}



		return dataout;

	};


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



int getFileParameters(string file, int &hs, int &tp, int &lt, int &act,int &dr, int &tg, int &is, int &x, int &y){
	cout << "Getting File Parameters from " << file << endl;
	string str;
	ifstream infile;

/*
	Header			: 500 bytes
	Top				: 1
	Left			: 1
	Active			: 1
	Frames Caught	: 15
	Frames Lost		: 0
	Dynamic Range	: 16
	Ten Giga		: 0
	Image Size		: 262144 bytes
	x				: 512 pixels
	y				: 256 pixels

*/


	infile.open(file.c_str(),ios::in | ios::binary);
	if (infile.is_open()) {

		//empty line
		getline(infile,str);

		//header size
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str header size:"<<str<<endl;
			sstr >> str >> str >> hs;
		}

		//top
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str top:"<<str<<endl;
			sstr >> str >> str >> tp;
		}

		//left
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str left:"<<str<<endl;
			sstr >> str >> str >> lt;
		}

		//active
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str active:"<<str<<endl;
			sstr >> str >> str >> act;
		}
cout<<"activeL:"<<act<<endl;
		//frames caught
		getline(infile,str);
		//cout<<"Str frames caught:"<<str<<endl;
		//frames lost
		getline(infile,str);
		//cout<<"Str frames lost:"<<str<<endl;

		//dynamic range
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str dynamic range:"<<str<<endl;
			sstr >> str >> str >>  str >> dr;
		}

		//ten giga
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str ten giga:"<<str<<endl;
			sstr >> str >> str >> str >> tg;
		}

		//image size
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str image size:"<<str<<endl;
			sstr >> str >> str >> str >> is;
		}
		//x
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str x:"<<str<<endl;
			sstr >> str >> str >> x;
		}

		//y
		if(getline(infile,str)){
			istringstream sstr(str);
			//cout<<"Str y:"<<str<<endl;
			sstr >> str >> str >> y;
		}


		infile.close();
	}else{
		cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
		return slsReceiverDefs::FAIL;
	}

	return slsReceiverDefs::OK;
}
