/* Reads files to get values for images
 * main_csaxs.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: tinti_g
 */

#include "sls_receiver_defs.h"
#include "slsReceiverData.h"
#include "eigerHalfModuleData.h"
#include "ansi.h"

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <stdlib.h>			

#include <cassert>	
#include <algorithm> 
#include "cbf.h"
using namespace std;

#undef cbf_failnez
#define cbf_failnez(x) 	\
		{				\
			int err; 	\
			err = (x); 	\
			if (err) { 	\
				fprintf(stderr,"\nCBFlib fatal error %x \n",err); \
				local_exit(-1); \
			} 			\
		}

void getParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &tenGiga, int &npix_x_user, int &npix_y_user, int &startdet);
bool CheckFrames( int fnum, int numFrames);
int local_exit(int status);




int main(int argc, char *argv[]) {

	//geometry
	const int NumHalfModules = 2;
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


	//map of pixels
	const int nx_ = npix_x_user;
	const int ny_ = npix_y_user;
	int map[nx_*ny_];
	//number of modules in vertical and horizontal
	int n_v = npix_y_user/npix_y_sm;
	int n_h = npix_x_user/npix_x_sm;
	//Gap pixels
	//note bad coded valid only to 1.5M
	const int GapPixelsBetweenChips_x = 2;
	const int GapPixelsBetweenChips_y = 2;
	const int GapPixelsBetweenModules_x = 8;
	const int GapPixelsBetweenModules_y = 36;
	int gap_pix_x_sm = GapPixelsBetweenChips_x * (NumChip_x-1);
	int gap_pix_y_sm = GapPixelsBetweenChips_y * (NumChip_y-1);
	//number of pixels of single module * number of modules +
	//gap pixels within a module * number of modules
	//+ gap pixels between modules * (number of modules -1)
	int npix_x_g = npix_x_sm * n_h  +  gap_pix_x_sm *  n_h + GapPixelsBetweenModules_x  * (n_h-1);
	int npix_y_g = npix_y_sm * n_v  +  gap_pix_y_sm *  n_v + GapPixelsBetweenModules_y  * (n_v-1);
	cout<<"npix_x_g:"<<npix_x_g<<" npix_y_g:"<<npix_y_g<<endl;
	//map including gap pixels
	//int mapg[npix_x_g*npix_y_g];
	int* mapg = new int[npix_x_g*npix_y_g];
	for(int ik=0; ik<npix_x_g*npix_y_g; ik++)
		mapg[ik]=0.;
	cout << "\nNumber of modules in horizontal: " << n_h << "\nNumber of modules in vertical  : " << n_v << endl << endl;







	//initialize receiverdata and fnum for all half modules
	int numModules = n_v *n_h*NumHalfModules;
	slsReceiverData <uint32_t> *receiverdata[numModules];
	int fnum[n_v *n_h*2];
	int nr=0;
	for(int imod_v=0; imod_v<n_v; imod_v++){
		for(int imod_h=0; imod_h<n_h; imod_h++){
			for(int it=0;it<2;it++){
				receiverdata[nr]=NULL;
				fnum[nr]=0;
				nr++;
			}
		}
	}

	//get dynamic range and configure receiverdata depending on top and bottom
	char fname[1000];
	char frames[20]="";
	if(isFileFrameIndex)
		sprintf(frames,"_f%012d",fileFrameIndex);//"f000000000000";
	ifstream infile[numModules];
	char *data = new char[1024];
	int dynamicrange = -100, dynamicrange2=-1;
	int nfile=startdet;
	//put master on top always
	nr=0;
	int bufferSize, dataSize, packetsPerFrame;


	for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
		for(int imod_h=0; imod_h<n_h; imod_h++){
			for( int it=0;it<2;it++){
				sprintf(fname,"%s_d%d%s_%d.raw",file.c_str(),nfile,frames,fileIndex);
				//read file to get dynamic range
				infile[nr].open(fname,ios::in | ios::binary);
				if (infile[nr].is_open()) {
					if(infile[nr].read(data,1024)){
						dynamicrange2 = eigerHalfModuleData::getDynamicRange(data);
						if(dynamicrange == -100)
							dynamicrange = dynamicrange2;
						else if(dynamicrange != dynamicrange2){
							cprintf(RED, "Error: The dynamic range read from: %s does not match dynamic range from previous files\n", fname);
						}
					}
					infile[nr].close();
				}else cprintf(RED, "Error: Could not read top file: %s\n", fname);

				//initialize variables for 1g and 10g
				bufferSize = 1040;
				dataSize = 1024;
				packetsPerFrame = 16 * dynamicrange * 2;
				if(tenGiga){
					packetsPerFrame = 4 * dynamicrange*2;
					bufferSize = 4112;
					dataSize = 4096;
				}

				//construct datamapping object
				receiverdata[nr] = new eigerHalfModuleData(dynamicrange,packetsPerFrame,bufferSize, dataSize, it==0 ? true : false);
				nr++;
				nfile++;
			}
		}
	}
	delete [] data;




	//Create cbf files with data
	cbf_handle cbf;
	vector <char*> buffer;
	buffer.reserve(n_v *n_h*2);
	FILE *out;
	//nr high again
	int numFrames = 1;

	//for each frame
	while(fnum[0]>-1){

		//here nr is not volatile anymore
		//loop on each receiver to get frame buffer
		for(int inr=0; inr<nr; inr++){
			sprintf(fname, "%s_d%d%s_%d.raw",file.c_str(),inr,frames,fileIndex);
			if( numFrames == 1)
			cout << fname << endl;
			//open file
			if(!infile[inr].is_open())
				infile[inr].open(fname,ios::in | ios::binary);
			if(infile[inr].is_open()){
				//get frame buffer
				char* tempbuffer=(receiverdata[inr]->readNextFrame(infile[inr], fnum[inr])); /*creating memory has to be deleted*/
				if(!CheckFrames(fnum[inr],numFrames))
					continue;
				buffer.push_back(tempbuffer);
			}
		}//loop on receivers

		if(buffer.size()!=nr) continue;
		cout << "Number of Frames:" << numFrames << endl;

		//get a 2d map of the image
		int inr=0;
		for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
			for(int imod_h=0; imod_h<n_h;imod_h++){
				for(int it=0;it<2;it++){
					/* Make a cbf version of the image */
					//getting values //top
					if(it==0){
						//initialize the first time
						if(imod_v==(n_v-1) && imod_h==0)
							for(int ik=0; ik<npix_y_user*npix_x_user;++ik)
								map[ik]=-1;

						for(int iy=((npix_y_sm/2)+imod_v*npix_y_sm); iy<(npix_y_sm+imod_v*npix_y_sm); iy++){
							for(int ix=0+imod_h*npix_x_sm; ix<npix_x_sm+imod_h*npix_x_sm; ix++){
								int k=ix+n_h*npix_x_sm*iy;
								map[k]=(receiverdata[inr]->getValue(buffer.at(inr),
										(ix-imod_h*npix_x_sm),(iy-(npix_y_sm/2)-imod_v*npix_y_sm),dynamicrange));
							}
						}
					}
					//getting values for bottom
					if(it==1){
						for(int iy=0+imod_v*npix_y_sm; iy<npix_y_sm/2+imod_v*npix_y_sm; ++iy){
							for(int ix=0+imod_h*npix_x_sm; ix<npix_x_sm+imod_h*npix_x_sm; ++ix){
								int k=ix+n_h*npix_x_sm*iy;
								map[k]= (receiverdata[inr]->getValue(buffer.at(inr),
										(ix-imod_h*npix_x_sm),(iy-imod_v*npix_y_sm),dynamicrange));
							}
						}
					}
					inr++;
				}
			}
		} //close all loops


		//now add gap pixels - should be done above to be faster but I will rewrite the code later
		int knog,kg;
		for(int imody=0; imody<n_v;imody++){
			for(int ichipy=0; ichipy<2;ichipy++){
				for(int iy=0; iy<256;iy++){
					for(int imodx=0; imodx<n_h;imodx++){
						for(int ichipx=0; ichipx<4;ichipx++){
							for(int ix=0; ix<256;ix++){
								knog=(ix+256*ichipx+256*4*imodx)+ n_h*npix_x_sm*(iy+256*ichipy+256*2*imody);
								kg= ix+(256+2)*ichipx+(256*4+6+8)*imodx+ npix_x_g*(iy+(256+2)*ichipy+(256*2+2+36)*imody);
	//cout<<"imody:"<<imody<<" ichipy:"<<ichipy<<" iy:"<<iy<<" imodx:"<<imodx<<" ichipx:"<<ichipx<<" ix:"<<ix<<""
									//	" kg:"<<kg<<" knog:"<<knog<<" mapg[kg]:"<<mapg[kg]<<endl;
								//cout<<"map[knog]:"<<map[knog]<<endl;
								mapg[kg]=map[knog];
							}
						}
					}
				}
			}
		}
		buffer.clear();



		//---> here I should also fill
		/* Create and initializes new internal CBF Object*/
		cbf_failnez (cbf_make_handle (&cbf));
		sprintf(fname, "%s_%d_%d.cbf",file.c_str(),numFrames, fileIndex);
		out = fopen (fname, "w");




		//fake headers
		fprintf(out,
		"###CBF: VERSION 1.0, CBFlib v0.9.5 - SLS EIGER detector\r\n"
		"# Detector: Eiger\r\n"
		);
		//timestamp
		time_t rawtime = time(NULL);
		struct tm *timeinfo = localtime(&rawtime);
		char date[100],printDate[100];
		strftime(date, sizeof(date), "%Y/%b/%d %H:%M:%S.%j %Z", timeinfo);
		sprintf(printDate,"# %s\r\n",date);
		fprintf(out,printDate);
		fprintf(out,
		"# Exposure_time 1.0000000 s\r\n"
		"# Exposure_period 1.0000000 s\r\n"
		"# Tau = 0 s\r\n"
		"# Count_cutoff 0 counts\r\n"
		"# Threshold_setting 8000 eV\r\n"
		";\r\n"
		);


		/* Make a new data block */
		cbf_failnez (cbf_new_datablock (cbf, "image_1"))  //why not: cbf_new_saveframe(cbf,"image 1")
		/* Make the _diffrn category */
		cbf_failnez (cbf_new_category (cbf, "array_data"))
		/* Make new column at current data category */
		cbf_failnez (cbf_new_column   (cbf, "header_convention"))
		/* Set value to current column and row */
		cbf_failnez (cbf_set_value    (cbf, "SLS_1.0"))
		/* Make new column at current data category */
		cbf_failnez (cbf_new_column   (cbf, "header_contents"))
		/* Make new column at current data category */
		cbf_failnez (cbf_new_column   (cbf, "data"))



		/* Create the binary data */
		cbf_failnez (cbf_set_integerarray_wdims_fs (
				cbf, 								//cbf_handle handle
				CBF_BYTE_OFFSET| CBF_FLAT_IMAGE, 	// unsigned int compression
				1,									//int binary_id
				&(mapg[0]), 						//void *array
				sizeof (int),						 //size_t elsize
				1,									//int elsigned
				npix_y_g * npix_x_g,				//size_t elements
				"little_endian",					 // const char *byteorder
				npix_x_g,							 //size_t dimfast
				npix_y_g,							//size_t dimmid
				0,									//size_t dimslow
				4095 								//size_t padding
				));


		/** write everything to file */
		cbf_failnez (cbf_write_file (
				cbf, 		//cbf_handle handle
				out, 		//FILE *file
				1, 			//int readable
				CBF, 		//int ciforcbf
				MSG_DIGEST | MIME_HEADERS  , //int headers
				0));		//int encoding


		numFrames++;
	}





	//close file when not frame yet
	for(int inr=0; inr<nr; inr++)
		if (infile[inr].is_open())
			infile[inr].close();

	buffer.clear();

	/* Free the cbf */
	cbf_failnez (cbf_free_handle (cbf));


	for(int inr=0; inr<nr; ++inr){
		delete receiverdata[inr];
	}



	return slsReceiverDefs::OK;
}




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
		if(argc < 5){
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



bool CheckFrames( int fnum, int numFrames)
{
	if(fnum!= numFrames) return false;
	return true;
}


int local_exit(int status) {
	exit(status);
	return status;    /* to avoid warning messages */
}
