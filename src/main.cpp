/* Reads files to get values for images
 * main.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: tinti_g
 */

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <stdlib.h>			
#include <sstream>
#include <cassert>	
#include <algorithm> 


#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"

#include "sls_receiver_defs.h"
#include "slsReceiverData.h"
#include "eigerHalfModuleData.h"
#include "image.h"

#include "ansi.h"

using namespace std;

void getParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &npix_x_user, int &npix_y_user, int &startdet){

  getCommandParameters(argc, argv,file, fileIndex, isFileFrameIndex, fileFrameIndex, npix_x_user, npix_y_user);


  startdet=0;

    if(argc>2){
      startdet=atoi(argv[4]);

      cprintf(BLUE,
	      "Start detector index      : %d\n",startdet);
      return;
    }
}

int main(int argc, char *argv[]) {

  //user set geometry
  int npix_x_user= npix_x_sm;
  int npix_y_user= npix_y_sm;

  //get command line arguments
  string file;
  int fileIndex, fileFrameIndex=0,startdet=0;
  bool isFileFrameIndex = false;
  getParameters(argc, argv, file, fileIndex, isFileFrameIndex, fileFrameIndex, npix_x_user, npix_y_user, startdet);

  TH2F* hmap;

  //number of modules in vertical and horizontal
  int n_v = npix_y_user/npix_y_sm;
  if( npix_y_user==256)  n_v=1;
  int n_h = npix_x_user/npix_x_sm;

  TFile* ofile;
  if(npix_y_user!=256)
    ofile= new TFile(TString::Format("%s_det%d-%d_%d.root",file.c_str(),startdet,(n_v*n_h)*4-1,fileIndex).Data(),"RECREATE");
  else   ofile= new TFile(TString::Format("%s_%d.root",file.c_str(),fileIndex).Data(),"RECREATE");
  ofile->Close();
  delete ofile;

  //initialize receiverdata and fnum for all half modules
  int numModules = n_v *n_h*NumHalfModules*2;
  if (npix_y_user==256) numModules=2;
  int fnum;
  int nr=0;
  for(int imod_v=0; imod_v<n_v; imod_v++){
    for(int imod_h=0; imod_h<n_h; imod_h++){
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
  int dynamicrange, tenGiga, xpix, ypix, imageSize;
  string timestamp;
  double expTime, period;
  
  sprintf(fname,"%s_master_%d.raw",file.c_str(),fileIndex);
  if(getFileParameters(fname, dynamicrange, tenGiga, imageSize, xpix, 
		       ypix, timestamp, expTime, period ) != slsReceiverDefs::OK)return -1;
  
  vector <int*> buffer;
  buffer.reserve(numModules);
  
 
  int numFrames = fileFrameIndex+1 ;
  const static int imageHeader = 8+4+4+8+8+2+2+2+2+4+2+1+1; //bytes

  //for each frame
  while(fnum>-1){
    
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
	
	if(infile[inr].read((char*)bufferheader,imageHeader))
	  fnum = (*((uint64_t*)(char*)bufferheader));
	else fnum=-1;
	if(fnum==-1) {
	  exit(1);
	}
		
	infile[inr].read((char*)intbuffer,imageSize);
	if(!CheckFrames(fnum,numFrames)) continue; 	
	
	if(inr==0){
	  //create a hmap
	  hmap= new TH2F(TString::Format("hmap%d",numFrames-1).Data(),
			 TString::Format("hmap%d",numFrames-1).Data(),
			 npix_x_user, 0, npix_x_user, npix_y_user, 0, npix_y_user);
	}

	buffer.push_back(decodeData(intbuffer, imageSize, xpix, ypix, dynamicrange));
	
      } //while read images
      //close files
      infile[inr].close();
    }//loop on receivers

    if(buffer.size()!=nr) continue;
   
    int startchipx=0;
    int startchipy=0;
    int endchipx=4;
    int endchipy=1;
   
    for(int imod_h=0; imod_h<n_h;imod_h++){
      for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
	for( int it=0;it<2;it++){	
	  for( int ileft=0;ileft<2;ileft++){
	    
	    if( npix_y_user==256 && it==1) continue; 
	    
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
		       int x_t= ix+(NumChanPerChip_x)*ichipx+(NumChanPerChip_x*NumChip_x)*imod_h ;
		       int y_t= iy+(NumChanPerChip_y)*ichipy+(NumChanPerChip_y*NumChip_y)*imod_v;

		      hmap->SetBinContent(x_t+1 , y_t+1, buffer[2*it+ileft][ix+(ichipx%2)*256+ 256*2*iy]);
		    }
		  }
		}
	      }
	    }
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
		      int x_t=ix+(NumChanPerChip_x)*ichipx+(NumChanPerChip_x*NumChip_x)*imod_h;
		      int y_t= iy+(NumChanPerChip_y)*ichipy+(NumChanPerChip_y*NumChip_y)*imod_v;
		      hmap->SetBinContent(x_t+1, y_t+1,buffer[2*it+ileft][ix+(ichipx%2)*256+ 256*2*(255-iy)]);
		    }
		  }
		}
	      } 
	    }//it==1
	  }//LEFT
	}//it
      }//v
    }//h 
  
    buffer.clear();
    //for every image
    TFile* ofile;
    if(npix_y_user!=256)
      ofile= new TFile(TString::Format("%s_det%d-%d_%d.root",file.c_str(),startdet,(n_v*n_h)*4-1,fileIndex).Data(),"UPDATE");
    else   ofile= new TFile(TString::Format("%s_%d.root",file.c_str(),fileIndex).Data(),"UPDATE");
    
    hmap->SetStats(kFALSE);
    hmap->Draw("colz");
    hmap->Write();
    ofile->Close();
    delete ofile;       
    delete hmap; 	
    
    numFrames++;
  }//fnum
    
 
  if(npix_y_user!=256)
    cprintf(GREEN,"Root File Created: %s_det%d-%d_%d.root\n\n",file.c_str(),startdet,(n_v*n_h)*4-1,fileIndex);
  else     cprintf(GREEN,"Root File Created: %s_%d.root\n\n",file.c_str(),fileIndex);
  

 //close file when not frame yet
  for(int inr=0; inr<nr; inr++)
    if (infile[inr].is_open())
      infile[inr].close();
  
  
  return slsReceiverDefs::OK;
}




