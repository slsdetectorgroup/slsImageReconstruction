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
#include <map>
#include <getopt.h>
using namespace std;

int getParameters(int argc, char *argv[], int &dr, string &fntop, string &fnbottom, int &tg);


int main(int argc, char *argv[]) {
	//declare variables
	slsReceiverData <uint32_t> *receiverdata = NULL;
	char* buffer = NULL;
	ifstream infile;
	string fnametop = "", fnamebottom = "";
	int dynamicrange, tenGiga, bufferSize, dataSize, packetsPerFrame, ix, iy, numFrames, fnum;


	//get config parameters
	dynamicrange = 16;
	tenGiga = 0;
	if(getParameters(argc, argv, dynamicrange,fnametop,fnamebottom, tenGiga) == slsReceiverDefs::OK){
		//cprintf(BLUE,"\n\nDynamic range:%d\nTop File name:%s\nBottom File name:%s\nTen giga:%d\n\n",dynamicrange,fnametop,fnamebottom,tenGiga);
		cout << "\n\nDynamic range:"<< dynamicrange << "\nTop File name:" << fnametop << "\nBottom File name:" << fnamebottom << "\nTen giga:"<< tenGiga << endl << endl;


		//initialize variables for 1g and 10g
		ix=0;
		iy = 0;
		bufferSize = 1040;
		dataSize = 1024;
		packetsPerFrame = 16 * dynamicrange * 2;
		if(tenGiga){
			packetsPerFrame = 4 * dynamicrange*2;
			bufferSize = 4112;
			dataSize = 4096;
		}


//top
		if(!fnametop.empty()){
			//construct top datamapping object
			numFrames = 0;
			fnum = -1;
			receiverdata = new eigerHalfModuleData(dynamicrange,packetsPerFrame,bufferSize, dataSize, true);
			//open file
			infile.open(fnametop.c_str(),ios::in | ios::binary);
			if(infile.is_open()){
				//get frame buffer
				while((buffer = receiverdata->readNextFrame(infile, fnum))){

					cout << "Reading top values for frame #" << numFrames << endl;
					//getting values
					iy = 0;
					for(ix = 0; ix < 2; ix++)
						cprintf(BLUE,"%d,%d :%f\n",iy,ix,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
					for(ix = 254; ix < 258; ix++)
						cprintf(BLUE,"%d,%d :%f\n",iy,ix,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
					iy = 2;
					for(ix = 0; ix < 2; ix++)
						cprintf(BLUE,"%d,%d :%f\n",iy,ix,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
					for(ix = 254; ix < 258; ix++)
						cprintf(BLUE,"%d,%d :%f\n",iy,ix,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));

					delete [] buffer;
					numFrames++;
				}
				//close file
				infile.close();
			}else cprintf(RED, "Error: Could not read top file: %s\n", fnametop.c_str());
			delete receiverdata;

			cout  << "Found " << numFrames << " frames in top file." << endl << endl;
		}


//bottom
		if(!fnamebottom.empty()){
			//construct bottom datamapping object
			numFrames = 0;
			fnum = -1;
			receiverdata = new eigerHalfModuleData(dynamicrange,packetsPerFrame,bufferSize, dataSize, false);
			//open file
			infile.open(fnamebottom.c_str(),ios::in | ios::binary);
			if(infile.is_open()){
				//get frame buffer
				while((buffer = receiverdata->readNextFrame(infile,fnum))){

					cout << "Reading bottom values for frame #" << numFrames << endl;
					//getting values
					iy = 0;
					for(ix = 0; ix < 2; ix++)
						cprintf(BLUE,"%d,%d :%f\n",iy,ix,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
					for(ix = 254; ix < 258; ix++)
						cprintf(BLUE,"%d,%d :%f\n",iy,ix,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
					iy = 2;
					for(ix = 0; ix < 2; ix++)
						cprintf(BLUE,"%d,%d :%f\n",iy,ix,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
					for(ix = 254; ix < 258; ix++)
						cprintf(BLUE,"%d,%d :%f\n",iy,ix,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));

					delete [] buffer;
					numFrames++;
				}
				//close file
				infile.close();
			}else cprintf(RED, "Error: Could not read bottom file: %s\n", fnamebottom.c_str());
			delete receiverdata;

			cout  << "Found " << numFrames << " frames in bottom file." << endl;
		}

	}
	cout << endl << "Goodbye!" << endl;
	return slsReceiverDefs::OK;
}


/**
 * Parse command line parameters to get dynamic range, ten giga enable and file names
 */
int getParameters(int argc, char *argv[], int &dr, string &fntop, string &fnbottom,int &tg){

	map<string, string> configuration_map;
	static struct option long_options[] = {
			{"dr",				required_argument,       0, 'd'},
			{"tengiga",    		required_argument,       0, 't'},
			{"top",     		required_argument,       0, 'p'},
			{"bottom",     		required_argument,       0, 'm'},
			{"help",  			no_argument,      		 0, 'h'},
			{0, 0, 0, 0}
	};
	int option_index = 0, c;
	while ( c != -1 ){
		c = getopt_long (argc, argv, "dtpmh", long_options, &option_index);
		if (c == -1)//end of options
			break;
		switch(c){
		case 'd':
			if(sscanf(optarg, "%d", &dr) <=0){
				cprintf(RED,"ERROR: Cannot parse dynamic range. Options: 4,8,16,32.\n");
				return slsReceiverDefs::FAIL;
			}
			switch(dr){
			case 4:
			case 8:
			case 16:
			case 32: break;
			default:
				cprintf(RED,"ERROR: Invalid dynamic range. Options: 4,8,16,32.\n");
				return slsReceiverDefs::FAIL;
			}
			break;
			case 't':
				if(sscanf(optarg, "%d", &tg) <=0){
					cprintf(RED,"ERROR: Cannot parse ten giga parameter. Options: 0 or 1.\n");
					return slsReceiverDefs::FAIL;
				}
				switch(tg){
				case 0:
				case 1: break;
				default:
					cprintf(RED,"ERROR: Invalid ten giga parameter. Options: 0 or 1.\n");
					return slsReceiverDefs::FAIL;
				}
				break;
		case 'p':
			fntop = optarg;
			if(fntop.empty()){
				cprintf(RED,"ERROR: Empty top file name.\n");
				return slsReceiverDefs::FAIL;
			}
			break;
		case 'm':
			fnbottom = optarg;
			if(fnbottom.empty()){
				cprintf(RED,"ERROR: Empty bottom file name.\n");
				return slsReceiverDefs::FAIL;
			}
			break;
		case 'h':
			string help_message = """\nSLS Image Reconstruction\n\n""";
			help_message += """usage: image --p top_fname [--m bottom_fname] [--dr dynamic_range]\n\n""";
			help_message += """\t--dr:\t dynamic range or bit mode. Default:16. Options: 4,8,16,32.\n""";
			help_message += """\t--tengiga:\t 1 if 10Gbe or 0 for 1Gbe. Default: 0\n""";
			help_message += """\t--top:\t file name of top image\n""";
			help_message += """\t--bottom:\t file name of bottom image\n\n""";
			cout << help_message << endl;
			break;

		}
	}

	//cout << " dynamic range:"<< dr << " top file name:" << fntop << " bottom file name:" << fnbottom << " ten giga:"<< tg << endl;
	return slsReceiverDefs::OK;
}
