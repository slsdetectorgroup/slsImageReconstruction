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
#include <stdlib.h>			


using namespace std;

int getParameters(int argc, char *argv[], int &dr, string &fntop, string &fnbottom, int &tg);


int main(int argc, char *argv[]) {
	//declare variables
	slsReceiverData <uint32_t> *receiverdata = NULL;
	char* buffer = NULL;
	ifstream infile;
	string fnametop = "", fnamebottom = "";
	int dynamicrange, tenGiga, bufferSize, dataSize, packetsPerFrame, ix, iy, numFrames, fnum;
	typedef struct
	{
		unsigned char num1[4];
		unsigned char num2[2];
		unsigned char num3[1];
		unsigned char num4[1];
	} eiger_packet_header;

	//get config parameters
	dynamicrange = -1;
	tenGiga = 0;
	if(getParameters(argc, argv, dynamicrange,fnametop,fnamebottom, tenGiga) == slsReceiverDefs::OK){
		//if no dynamic range given
		if(dynamicrange == -1){
			cout << "Getting Dynamic Range from File..." << endl;
			char *data = new char[1024];
			int dynamicrange2=-1;

			//top dynamic range
			if(!fnametop.empty()){
				infile.open(fnametop.c_str(),ios::in | ios::binary);
				if (infile.is_open()) {
					if(infile.read(data,1024)){
						dynamicrange = eigerHalfModuleData::getDynamicRange(data);
					}
					infile.close();
				}else{
					cprintf(RED, "Error: Could not read top file: %s\n", fnametop.c_str());
					delete [] data;
					exit(-1);
				}
			}

			//bottom dynamic range
			if(!fnamebottom.empty()){
				infile.open(fnamebottom.c_str(),ios::in | ios::binary);
				if (infile.is_open()) {
					if(infile.read(data,1024)){
						dynamicrange2 = eigerHalfModuleData::getDynamicRange(data);
						if ((dynamicrange != -1) && (dynamicrange != dynamicrange2)){
							cprintf(RED, "Error: Dynamic range of top %d and bottom %d does not match\n", dynamicrange,dynamicrange2);
							exit(-1);
						}
						else dynamicrange = dynamicrange2; //only bottom checked
					}
					infile.close();
				}else{
					cprintf(RED, "Error: Could not read bottom file: %s\n", fnamebottom.c_str());
					delete [] data;
					exit(-1);
				}
			}

			delete [] data;
		}

		//verify dynamic range
		switch(dynamicrange){
		case 4:
		case 8:
		case 16:
		case 32: break;
		default:
			cprintf(RED, "Error: Invalid dynamic range %d. Options: 4,8,16,32.\n", dynamicrange);
			exit(-1);
		}
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
					//while((buffer = receiverdata->readNextFrame(infile))){
					cout << "Reading top values for frame #" << fnum << endl;
					//getting values


					iy = 255;
					for(ix = 512; ix < 516; ix++)
						cprintf(BLUE,"%d,%d :%f\n",ix,iy,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));

					/*
					for(iy = 0; iy < 5; iy++){
						for(ix = 0; ix < 2; ix++)
							cprintf(BLUE,"%d,%d :%f\n",ix,iy,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
						for(ix = 254; ix < 258; ix++)
							cprintf(BLUE,"%d,%d :%f\n",ix,iy,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
					}


					for(iy = 0; iy < 256 ; iy++){
						for(ix = 0; ix < 1024 ; ix++){
							cprintf(BLUE,"%d,%d :%f\t",ix,iy,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
						}
					}
*/


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
					//while((buffer = receiverdata->readNextFrame(infile))){
					cout << "Reading bottom values for frame #" << fnum << endl;

					//getting values



					/*
					for(iy = 0; iy < 3; iy++){
						for(ix = 0; ix < 2; ix++)
							cprintf(BLUE,"%d,%d :%f\n",ix,iy,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
						for(ix = 254; ix < 258; ix++)
							cprintf(BLUE,"%d,%d :%f\n",ix,iy,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
					}
					*/
					for(iy = 0; iy < 256 ; iy++){
						for(ix = 0; ix < 1024 ; ix++){
							cprintf(BLUE,"%d,%d :%f\t",ix,iy,(receiverdata->getValue((char*)buffer,ix,iy,dynamicrange)));
						}
					}



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
      //verification done later
      /*switch(dr){
	case 4:
	case 8:
	case 16:
	case 32: break;
	default:
	cprintf(RED,"ERROR: Invalid dynamic range. Options: 4,8,16,32.\n");
	return slsReceiverDefs::FAIL;
	}*/
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
