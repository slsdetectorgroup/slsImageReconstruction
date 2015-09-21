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
#include <stdlib.h>			

#include <cassert>	
#include <algorithm>

#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"
using namespace std;


void getParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &tenGiga, int &npix_x_user, int &npix_y_user, int &startdet);




int main(int argc, char *argv[]) {

	//geometry
	const int NumChanPerChip_x = 256;
	const int NumChanPerChip_y = 256;
	const int NumChip_x = 4;
	const int NumChip_y = 2;
	//single module geometry
	const int npix_x_sm=(NumChanPerChip_x*NumChip_x);
	const int npix_y_sm=(NumChanPerChip_y*NumChip_y);
	//user set geometry
	int npix_x_user= npix_x_sm;
	int npix_y_user= npix_y_sm;


	//get command line arguments
	string file;
	int fileIndex, fileFrameIndex=0,startdet=0, tenGiga = 0;
	bool isFileFrameIndex = false;
	getParameters(argc, argv, file, fileIndex, isFileFrameIndex, fileFrameIndex, tenGiga, npix_x_user, npix_y_user, startdet);


	//number of modules in vertical and horizontal
	int n_v = npix_y_user/npix_y_sm;
	int n_h = npix_x_user/npix_x_sm;




	//get dynamic range and configure  depending on top and bottom
	slsReceiverData <uint32_t> *receiverdata = NULL;
	char* buffer = NULL;
	char fname[1000];
	char frames[20]="";
	if(isFileFrameIndex)
		sprintf(frames,"_f%012d",fileFrameIndex);//"f000000000000";
	ifstream infile;
	char *data = new char[1024];
	int dynamicrange = -100, dynamicrange2=-1;
	int nfile=startdet;
	int bufferSize, dataSize, packetsPerFrame;
	int numFrames, fnum;

	const int Nframes=1000;
	TH2F* hmap[Nframes] = {NULL};


	//put master on top always
	for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
		for(int imod_h=0; imod_h<n_h;imod_h++){
			for( int it=0;it<2;it++){
				sprintf(fname,"%s_d%d%s_%d.raw",file.c_str(),nfile,frames,fileIndex);//(it+(imod_h*2)+n_h*imod_v)nfile
				cout<<fname<<endl;
				//read file to get dynamic range
				infile.open(fname,ios::in | ios::binary);
				if (infile.is_open()) {
					if(infile.read(data,1024)){
						dynamicrange2 = eigerHalfModuleData::getDynamicRange(data);
						if(dynamicrange == -100)
							dynamicrange = dynamicrange2;
						else if(dynamicrange != dynamicrange2){
							cprintf(RED, "Error: The dynamic range %d read from: %s\n"
									"does not match dynamic range from previous files %d\n", dynamicrange2,fname,dynamicrange);
						}
					}
					infile.close();
				}else cprintf(RED, "Error: Could not read file: %s\n", fname);


				//initialize variables for 1g and 10g
				bufferSize = 1040;
				dataSize = 1024;
				packetsPerFrame = 16 * dynamicrange * 2;
				if(tenGiga){
					packetsPerFrame = 4 * dynamicrange*2;
					bufferSize = 4112;
					dataSize = 4096;
				}



				//construct top datamapping object
				numFrames = 0;
				fnum = -1;
				receiverdata = new eigerHalfModuleData(dynamicrange,packetsPerFrame,bufferSize,dataSize, it==0? true: false);

				//open file
				infile.open(fname,ios::in | ios::binary);
				if(infile.is_open()){
					//get frame buffer
					while((buffer = receiverdata->readNextFrame(infile, fnum))){

						//cout << "Reading values for frame #" << fnum-1 << endl;

						//getting values //top
						if(it==0){
							if(imod_v==(n_v-1) && imod_h==0){
								//top master is done first
								hmap[numFrames]= new TH2F(TString::Format("hmap%d",numFrames).Data(),
										TString::Format("hmap%d",numFrames).Data(),
										npix_x_user, 0, npix_x_user, npix_y_user, 0, npix_y_user);
							}
							for(int iy=((npix_y_sm/2)+imod_v*npix_y_sm); iy<(npix_y_sm+imod_v*npix_y_sm); iy++)
								for(int ix=0+imod_h*npix_x_sm; ix<npix_x_sm+imod_h*npix_x_sm; ix++)
									hmap[numFrames]->SetBinContent(ix+1, iy+1, (receiverdata->getValue((char*)buffer,(ix-imod_h*npix_x_sm),(iy-(npix_y_sm/2)-imod_v*npix_y_sm),dynamicrange)));
						}
						if(it==1){
							for(int iy=0+imod_v*npix_y_sm; iy<npix_y_sm/2+imod_v*npix_y_sm; iy++)
								for(int ix=0+imod_h*npix_x_sm; ix<npix_x_sm+imod_h*npix_x_sm; ix++)
									hmap[numFrames]->SetBinContent(ix+1, iy+1, (receiverdata->getValue((char*)buffer,
											(ix-imod_h*npix_x_sm),(iy-imod_v*npix_y_sm),dynamicrange)));
						}

						delete [] buffer;
						numFrames++;
					}
					//close file
					infile.close();
				}else cprintf(RED, "Error: Could not read file: %s\n", fname);
				delete receiverdata;

				cout  << "Found " << numFrames << " frames in "<<nfile/*it+(imod_h*2)+n_h*imod_v*/<<" file." << endl;
				//cout <<	"it:" << it << " imod_h:" << imod_h << " n_h:" << n_h << " imod_v:" << imod_v << endl << endl;

				nfile++;
			}

		} //hor
	}//vert

	TFile* ofile = new TFile(TString::Format("%s_det%d-%d_%d.root",file.c_str(),startdet,nfile-1,fileIndex).Data(),"RECREATE");
	for(int fnum=0; fnum<numFrames; fnum++){
		if( hmap[fnum]){
			hmap[fnum]->SetStats(kFALSE);
			hmap[fnum]->Draw("colz");
			hmap[fnum]->Write();
		}
	}
	ofile->Write();
	delete ofile;
	for(int i=0;i<Nframes;i++)
		delete hmap[i];
	cout << endl << "Goodbye!" << endl;
	return slsReceiverDefs::OK;
}




/**
 * Parse command line parameters to get dynamic range, ten giga enable and file names
 */

void getParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &tenGiga, int &npix_x_user, int &npix_y_user, int &startdet){
	if(argc < 2){
		cprintf(RED, "Error: Not enough arguments: bcfMaker [file_name_with_dir] \nExiting.\n");
		exit(-1);
	}
	file=argv[1];


	//extract file index and file name with _d0
    int i;
    size_t uscore=file.rfind("_");
    if (uscore==string::npos) {
    	cprintf(RED, "Error: Invalid file name. No underscore found\nExiting.\n");
    	exit(-1);
    }
    string s=file;
    if (sscanf(s.substr(uscore+1,s.size()-uscore-1).c_str(),"%d",&i)) {
    	fileIndex=i;
    	s=file.substr(0,uscore);
    } else{
    	cprintf(RED, "Error: Invalid file name. Cannot parse file index from %s\n",file.c_str());
    	exit(-1);
    }
    uscore=s.rfind("_");
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"f%d",&i)) {
    	isFileFrameIndex = true;
    	fileFrameIndex = i;
    	s=file.substr(0,uscore);
    }/*else      cout << "Warning: ******************************** cannot parse frame index from " << s << endl; \*/
    uscore=s.rfind("_");
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"d%d",&i)){
    	s=file.substr(0,uscore);
    }else{
    	cprintf(RED, "Error: Invalid file name. Cannot parse detector index from %s\n",file.c_str());
    	exit(-1);
    }
	file=s;

	//more parameters for ten giga, user pixels, startdet
	if(argc>2){
		if(argc < 6){
			cprintf(RED, "Error: Not enough arguments: bcfMaker [file_name_with_dir] "
					"[tengiga] [numpixels_x][numpixels_y] [start_detector_Index]\nExiting.\n");
			exit(-1);
		}
		tenGiga =atoi(argv[2]);
		npix_x_user=atoi(argv[3]);
		npix_y_user=atoi(argv[4]);
		startdet=atoi(argv[5]);

		cprintf(BLUE,
				"File Name:%s\n"
				"File Index:%d\n"
				"Is File Frame Index in File:%d\n"
				"Frame Index:%d\n"
				"Ten Giga: %d\n"
				"Number of pixels in x dir:%d\n"
				"Number of pixels in y dir:%d\n"
				"Start detector index:%d\n",
				file.c_str(),fileIndex,isFileFrameIndex,fileFrameIndex, tenGiga,npix_x_user,npix_y_user,startdet);
		return;
	}
	cprintf(BLUE,
			"File Name:%s\n"
			"File Index:%d\n"
			"Is File Frame Index in File:%d"
			"\nFile Frame Index:%d\n",
			file.c_str(),fileIndex,isFileFrameIndex,fileFrameIndex);
}
