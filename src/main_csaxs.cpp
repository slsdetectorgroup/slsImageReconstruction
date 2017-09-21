/* Reads files to get values for images
 * main_csaxs.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: tinti_g
 */

#include "sls_receiver_defs.h"
#include "ansi.h"

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>			
#include <map>
#include <getopt.h>
#include <cmath>

#include <cassert>	
#include <algorithm> 

#include "cbf.h"
#include "image.h"

using namespace std;

#undef cbf_failnez
#define cbf_failnez(x)					\
  {							\
    int err;						\
    err = (x);						\
    if (err) {						\
      fprintf(stderr,"\nCBFlib fatal error %x \n",err); \
      local_exit(-1);					\
    }							\
  }

int local_exit(int status) {
  exit(status);
  return status;    /* to avoid warning messages */
}

int  getCommandParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &npix_x_user, int &npix_y_user, int& longedge_x, int &startdet){
  if(argc < 2){
    cprintf(RED, "Error: Not enough arguments: cbfMaker [file_name_with_dir] \nExiting.\n");
    exit(-1);
  }
  file=argv[1];
  getCommandParameters(argc, argv, file, fileIndex, isFileFrameIndex, fileFrameIndex, npix_x_user, npix_y_user);

  if(argc>2){
        
      if(argc>4)longedge_x=atoi(argv[4]);
      else longedge_x=1;
      if(argc>5) startdet=atoi(argv[5]);
      else startdet=0;

      cprintf(BLUE,
              "Module long edge is on x  : %d\n"
	      "Start detector index      : %d\n",
	      longedge_x,startdet);
      return 1;
    }else{
    longedge_x=1;
    startdet=0;
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {

  
  //user set geometry
  int npix_x_user= npix_x_sm;
  int npix_y_user= npix_y_sm;

  //get command line arguments
  string file;
  int fileIndex, fileFrameIndex=0,startdet=0;
  int longedge_x;
  bool isFileFrameIndex = false;
  getCommandParameters(argc, argv, file, fileIndex, isFileFrameIndex, fileFrameIndex, npix_x_user, npix_y_user, longedge_x,startdet);

  //cheat and reverse if it is in vertical orientation 
  if (!longedge_x){
    int nxt=npix_x_user;
    int nyt=npix_y_user;
    npix_x_user=nyt;
    npix_y_user=nxt;
  }
  
  //number of modules in vertical and horizontal
  int n_v = npix_y_user/npix_y_sm;
  if( npix_y_user==256)  n_v=1;
  int n_h = npix_x_user/npix_x_sm;
  //Gap pixels
  int gap_pix_x_sm = GapPixelsBetweenChips_x * (NumChip_x-1);
  int gap_pix_y_sm = GapPixelsBetweenChips_y * (NumChip_y-1);
  //number of pixels of single module * number of modules +
  //gap pixels within a module * number of modules
  //+ gap pixels between modules * (number of modules -1)
  int npix_x_g = npix_x_sm * n_h  +  gap_pix_x_sm *  n_h + GapPixelsBetweenModules_x  * (n_h-1);
  int npix_y_g = npix_y_sm * n_v  +  gap_pix_y_sm *  n_v + GapPixelsBetweenModules_y  * (n_v-1);
  if( npix_y_user==256) { 
    npix_y_g = npix_y_user;
  }
 
  //map including gap pixels
  int* map=new int[npix_x_g*npix_y_g];
  
  cprintf(BLUE,
	  "Number of Pixels (incl gap pixels) in x dir : %d\n"
	  "Number of Pixels (incl gap pixels) in y dir : %d\n"
	  "Number of modules in horizontal             : %d\n"
	  "Number of modules in vertical               : %d\n",
	  longedge_x ? npix_x_g : npix_y_g ,
	  longedge_x ? npix_y_g : npix_x_g ,
	  longedge_x ? n_h: n_v,
	  longedge_x ? n_v : n_h);
  
  //initialize receiverdata and fnum for all half modules
  int numModules = n_v *n_h*NumHalfModules*2;
  if( npix_y_user==256)   numModules=2;
  int fnum;
  int nr=0;
  for(int imod_h=0; imod_h<n_h; imod_h++){
    for(int imod_v=0; imod_v<n_v; imod_v++){
      for(int it=0;it<2;it++){
	for(int ileft=0; ileft<2;ileft++){
	  if( npix_y_user==256 && it==1 ) continue;
	  fnum=0;
	  nr++;
	}
      }
    }
  }

  //get dynamic range and configure receiverdata depending on top and bottom
  char fname[1000]; 
  char frames[20]="";
  if(isFileFrameIndex)
    sprintf(frames,"_f%012d",fileFrameIndex);//"f000000000000";
  ifstream infile[numModules];
  // int nfile=startdet;
  //put master on top always
  int dynamicrange, tenGiga, xpix, ypix, imageSize;
  string timestamp;
  double expTime, period;
  
  sprintf(fname,"%s_master_%d.raw",file.c_str(),fileIndex);
  if(getFileParameters(fname, dynamicrange, tenGiga, imageSize, xpix, 
		       ypix, timestamp, expTime, period ) != slsReceiverDefs::OK)return -1;
  
  vector <int*> buffer;
  buffer.reserve(n_v *n_h*2);
  FILE *out;
  //nr high again
  int numFrames = fileFrameIndex+1 ;
  
  const static int imageHeader = 8+4+4+8+8+2+2+2+2+4+2+1+1; //bytes
  
  //for each frame
  while(fnum>-1){

    //Create cbf files with data
    cbf_handle cbf;
   
    //here nr is not volatile anymore
    //loop on each receiver to get frame buffer
    for(int inr=0; inr<nr; inr++){
      sprintf(fname, "%s_d%d%s_%d.raw",file.c_str(),inr,frames,fileIndex);
    
      //open file
      if(!infile[inr].is_open())
	infile[inr].open(fname,ios::in | ios::binary);
      if(infile[inr].is_open()){
	
	int* intbuffer = new int[imageSize+1];
      	int* bufferheader=new int[imageHeader+1];
	//read data
	if(infile[inr].read((char*)bufferheader,imageHeader))
	  fnum = (*((uint64_t*)(char*)bufferheader));
	else fnum=-1;
	if(fnum==-1) {
	  exit(1);
	}
		
	infile[inr].read((char*)intbuffer,imageSize);
	if(!CheckFrames(fnum,numFrames)) continue; 	
     
	buffer.push_back(decodeData(intbuffer, imageSize, xpix, ypix, dynamicrange));
	
      } //while read images
      //close files
      infile[inr].close();
    }//loop on receivers

    if(buffer.size()!=nr) continue;
    
    //get a 2d map of the image
    //initialize
    for(int ik=0; ik<npix_y_g*npix_x_g;++ik)
      map[ik]=4095;//-1; 
	
    int startchipx=0;
    int startchipy=0;
    int endchipx=4;
    int endchipy=1;


    for(int imod_h=0; imod_h<n_h;imod_h++){
      for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
	for( int it=0;it<2;it++){	
	  for( int ileft=0;ileft<2;ileft++){
		
	    if( npix_y_user==256 && it==1) continue; 
		
	    /* Make a cbf version of the image */
	    //getting values //top
	    if(it==0){
	      startchipy=1;    
	      endchipy=2;

	      if(ileft==0){		  
		startchipx=0;
		endchipx=2;
	      }
	      if(ileft==1){		  
		startchipx=2;
		endchipx=4;
	      }
	      if(npix_y_user==256){
		startchipy=0;    
		endchipy=1;
	      } 
	      for(int ichipy=startchipy; ichipy<endchipy;ichipy++){
		for(int iy=0; iy<NumChanPerChip_y;iy++){
		  for(int ichipx=startchipx; ichipx<endchipx;ichipx++){
		    for(int ix=0; ix<NumChanPerChip_x;ix++){
		      int x_t= ix+(NumChanPerChip_x+GapPixelsBetweenChips_x)*ichipx+(NumChanPerChip_x*NumChip_x+(NumChip_x-1)*GapPixelsBetweenChips_x+GapPixelsBetweenModules_x)*imod_h ;
		      int y_t= iy+(NumChanPerChip_y+GapPixelsBetweenChips_y)*ichipy+(NumChanPerChip_y*NumChip_y+(NumChip_y-1)*GapPixelsBetweenChips_y+GapPixelsBetweenModules_y)*imod_v;

		      int k= x_t+ npix_x_g*y_t;
		      if(!longedge_x) {
			//now apply rotation 
			//y'=x
			//x'=(ngap_pix_y-iy)
			k=(npix_y_g-y_t)+ npix_y_g*x_t;
		      }
		      map[k]=buffer[2*it+ileft][ix+(ichipx%2)*256+ 256*2*iy];
		    }
		  }
		}
	      }
		  
	    } //it ==0 		
	    //getting values for bottom
	    if(it==1 ) {
	      startchipy=0;    
	      endchipy=1;
	      if(ileft==0){		  
		startchipx=0;
		endchipx=2;
	      }
	      if(ileft==1){		  
		startchipx=2;
		endchipx=4;
	      }		 
		 		    
	      for(int ichipy=startchipy; ichipy<endchipy;ichipy++){
		for(int iy=0; iy<NumChanPerChip_y;iy++){
		  for(int ichipx=startchipx; ichipx<endchipx;ichipx++){
		    for(int ix=0; ix<NumChanPerChip_x;ix++){
		      int x_t=ix+(NumChanPerChip_x+GapPixelsBetweenChips_x)*ichipx+(NumChanPerChip_x*NumChip_x+(NumChip_x-1)*GapPixelsBetweenChips_x+GapPixelsBetweenModules_x)*imod_h;
		      int y_t= iy+(NumChanPerChip_y+GapPixelsBetweenChips_y)*ichipy+(NumChanPerChip_y*NumChip_y+(NumChip_y-1)*GapPixelsBetweenChips_y+GapPixelsBetweenModules_y)*imod_v;
			    
		      int k=  x_t+npix_x_g*y_t;
		      if(!longedge_x){//now apply rotation 
			//y'=x
			//x'=(ngap_pix_y-iy)
			k=(npix_y_g-y_t)+ npix_y_g*x_t;
		      }
		      map[k]=buffer[2*it+ileft][ix+(ichipx%2)*256+ 256*2*(255-iy)];
		    }
		  }
		}
	      }
	    } //it==1
	  }//ileft
	} //it
	    
      }//v mods
    } //h mods close all loops
		
  
    
    buffer.clear();
    
    //---> here I should also fill
    /* Create and initializes new internal CBF Object*/
    cbf_failnez (cbf_make_handle (&cbf));
    sprintf(fname, "%s_%05d_%05d.cbf",file.c_str(),fileIndex, numFrames);
    out = fopen (fname, "w");
		
    //fake headers
    fprintf(out,
	    "###CBF: VERSION 1.0, CBFlib v0.9.5 - SLS EIGER detector\r\n"
	    "# Detector: Eiger\r\n"
	    );
    //timestamp
    char printDate[100];
    char limits[100];
    fprintf(out,"_array_data.header_contents\r\n"
	    ";\r\n");
    sprintf(printDate,"# %s\r\n",timestamp.c_str());
    fprintf(out,printDate);
    fprintf(out,
	    "# Exposure_time 1.0000000 s\r\n"
	    "# Exposure_period 1.0000000 s\r\n"
	    "# Tau = 0 s\r\n");
    long int max=pow(2,dynamicrange)-1;
    if(dynamicrange==16) max=pow(2,12)-1;
    sprintf(limits, "# Count_cutoff %ld counts\r\n",max );
    fprintf(out,limits);
    fprintf(out,
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
						  &(map[0]), 						//void *array
						  sizeof (int),						 //size_t elsize
						  1,									//int elsigned
						  longedge_x ?  npix_y_g * npix_x_g: npix_x_g * npix_y_g,	    //size_t elements
						  "little_endian",					 // const char *byteorder
						  longedge_x? npix_x_g : npix_y_g,		       		 //size_t dimfast
						  longedge_x? npix_y_g : npix_x_g,				  //size_t dimmid
						  0,							       	//size_t dimslow
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
    

    
    cbf_failnez (cbf_free_handle (cbf));
    numFrames++;
    
    buffer.clear();

  } //loop on frames
  
  //buffer.clear();


  return 1;
}




