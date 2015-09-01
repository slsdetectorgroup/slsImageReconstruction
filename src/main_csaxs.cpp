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
#include <map>
#include <vector>
#include <getopt.h>
#include <stdlib.h>			

#include <cassert>	

#include <algorithm> 

#include "cbf.h"

using namespace std;

int local_exit(int status) {
  exit(status);
  return status;    /* to avoid warning messages */
}

#undef cbf_failnez
#define cbf_failnez(x) \
 {int err; \
  err = (x); \
  if (err) { \
    fprintf(stderr,"\nCBFlib fatal error %x \n",err); \
    local_exit(-1); \
  } \
 }

bool CheckFrames( int fnum, int numFrames)
{
  if(fnum!= numFrames) return false;
  return true;
}

int main(int argc, char *argv[]) {
  //declare variables
 
  string fname;
  int dynamicrange, tenGiga, bufferSize, dataSize, packetsPerFrame, numFrames; 
  cbf_handle cbf;
 
  typedef struct
  {
    unsigned char num1[4];
    unsigned char num2[2];
    unsigned char num3[1];
    unsigned char num4[1];
  } eiger_packet_header;

  string dir;
  string file;
  int n;
  
  //single module  
  const int npix_x_sm=(256*4);
  const int npix_y_sm=(256*2);


  //user set geometry
  int npix_x_user= npix_x_sm;
  int npix_y_user= npix_y_sm;

  const int Nframes=1;
  
  //get config parameters
  dynamicrange = -1;
  tenGiga = 0;
  
  char *data = new char[1024];
  int dynamicrange2=-1;
  
  int startdet=0;
  //top dynamic range
  dir=argv[1];
      file=argv[2];
      n=atoi(argv[3]);
      if(argc>4){  
	npix_x_user=atoi(argv[4]);
	npix_y_user=atoi(argv[5]);
	startdet=atoi(argv[6]);
      }

      const int nx_=npix_x_user;
      const int ny_=npix_y_user;
      int map[nx_*ny_];

      //work out how many modules in vertical and how many in horizontal
      int n_v= npix_y_user/npix_y_sm;
      int n_h= npix_x_user/npix_x_sm;

      cout<<"geometery has "<<n_h<<" modules in horizontal and "<<n_v<<" modules in vertical"<<endl;
      FILE *out;

      slsReceiverData <uint32_t> *receiverdata[n_v *n_h*2];
      vector <char*> buffer;
      buffer.reserve(n_v *n_h*2);
      ifstream infile[n_v *n_h*2];
      int fnum[n_v *n_h*2];

      char* buffer0 = NULL;
    
      int nr=0;
      for(int imod_v=0; imod_v<n_v; ++imod_v){
	for(int imod_h=0; imod_h<n_h;++imod_h){
	  for( int it=0;it<2;it++){
	    receiverdata[nr]=NULL;
	    fnum[nr]=0;
	    nr++;	
	  }
	}
      }
      
      string frames="";//"f000000000000";

      int nfile=startdet;
      //put master on top always
     
     	nr=0;
    
      for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
	for(int imod_h=0; imod_h<n_h;imod_h++){
	  for( int it=0;it<2;it++){
	    char buff[256];
	    snprintf(buff, sizeof(buff),"%s/%s_d%d%s_%d.raw",dir.c_str(),file.c_str(),/*(it+(imod_h*2)+n_h*imod_v)*/nfile,frames.c_str(),n);
	    fname=buff;
	      
	    if(!fname.empty()){
	      infile[nr].open(fname.c_str(),ios::in | ios::binary);
	      if (infile[nr].is_open()) {
		if(infile[nr].read(data,1024)){
		  dynamicrange = eigerHalfModuleData::getDynamicRange(data);
		}
		infile[nr].close();
	      }else cprintf(RED, "Error: Could not read top file: %s\n", fname.c_str());
	    }
	    
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

      //nr high again
      numFrames = 1;
      
      char buff[256];
      
      while(fnum[0]>-1){
      //here nr is not volatile anymore
	for(int inr=0; inr<nr; inr++){  
	  snprintf(buff, sizeof(buff),"%s/%s_d%d%s_%d.raw",dir.c_str(),file.c_str(),/*(it+(imod_h*2)+n_h*imod_v)*/inr,frames.c_str(),n);
	  fname=buff;
	  cout<<fname<<endl;
	  
	  if(!fname.empty()){
	    //open file
	    infile[inr].open(fname.c_str(),ios::in | ios::binary);
	    if(infile[inr].is_open()){
	      //get frame buffer
	      char* tempbuffer=(receiverdata[inr]->readNextFrame(infile[inr], fnum[inr]));
	      if(!CheckFrames(fnum[inr],numFrames)) {
		continue; 
	      }
	      buffer.push_back(tempbuffer);
	    }
	  }
	}//loop on receivers	      
 
	if(buffer.size()!=nr) continue;

	int inr=0;
	for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
	  for(int imod_h=0; imod_h<n_h;imod_h++){
	    for(int it=0;it<2;it++){
		
	      /* Make a cbf version of the image */
	      //getting values //top
	      if(it==0){
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
	
	
	buffer.clear();
	
	  /* Write the new file */
	  //---> here I should also fill
	char buff[256];
	cbf_failnez (cbf_make_handle (&cbf));	
	snprintf(buff, sizeof(buff),"%s/%s_%d_%d.cbf",dir.c_str(),file.c_str(),numFrames, n); 
	out = fopen (buff, "w");
	  
	
	
	
	/* Make a new data block */
	
	cbf_failnez (cbf_new_datablock (cbf, "image_1"))
	  

	 
	    /* Make the _diffrn category */
	    cbf_failnez (cbf_new_category (cbf, "diffrn"))
	    cbf_failnez (cbf_new_column   (cbf, "id"))
	    cbf_failnez (cbf_set_value    (cbf, "DS1"))
	    





	  /* Make the _array_data category */

	  //cbf_failnez (cbf_new_datablock (cbf, "image_1"))

	  //cbf_failnez (cbf_new_category     (cbf, "array_data"));
	  //assert(0);	  	
	  //cbf_failnez (cbf_new_column       (cbf, "array_id"));
	  //cbf_failnez (cbf_set_value        (cbf, "image_1"));
	  //cbf_failnez (cbf_new_column       (cbf, "binary_id"));
	  //cbf_failnez (cbf_set_integervalue (cbf, 1));
	  //cbf_failnez (cbf_new_column       (cbf, "data"));
	  
	  /* Save the binary data */
	

       	  cbf_failnez (cbf_set_integerarray_wdims_fs (cbf, CBF_PACKED|CBF_FLAT_IMAGE, 1,
						      &(map[0]), sizeof (int), 1,
						      npix_y_user * npix_x_user,
						      "little_endian", npix_x_user, npix_y_user,0,0 ));
	  
	
	  cbf_failnez (cbf_write_file (cbf, out, 1, CBF, MSG_DIGEST | MIME_HEADERS  , 0));
	
	  
	  numFrames++;
      }
      
      //close file when not frame yet
      for(int inr=0; inr<nr; inr++) infile[inr].close();
      
      buffer.clear();      
      /* Free the cbf */
      cbf_failnez (cbf_free_handle (cbf));
  

      for(int inr=0; inr<nr; ++inr){
	   delete receiverdata[inr];
      }

      return slsReceiverDefs::OK;
}


