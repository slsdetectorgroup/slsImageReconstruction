/* Reads files to get values for images
 * main_csaxs.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: tinti_g
 */

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

#include "image.h"

#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"

using namespace std;

/*
int  getCommandParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &npix_x_user, int &npix_y_user, int& longedge_x, int &startdet){
  if(argc < 2){
    cprintf(RED, "Error: Not enough arguments: image [file_name_with_dir] \nExiting.\n");
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
*/
int main(int argc, char *argv[]) {
    
  //user set geometry
  int npix_x_user= npix_x_sm;
  int npix_y_user= npix_y_sm;


  //get command line arguments
  string file;
  int fileIndex, fileFrameIndex=0,startdet=0;
  int longedge_x;
  bool isFileFrameIndex = false;
  getCommandParameters(argc, argv, file, fileIndex, isFileFrameIndex, fileFrameIndex, npix_x_user, npix_y_user, longedge_x,fillgaps,startdet);

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
  int dynamicrange, tenGiga, xpix, ypix, imageHeader, imageSize,imgs;
  string timestamp;
  double expTime, period;
  
  sprintf(fname,"%s_master_%d.raw",file.c_str(),fileIndex);
  if(getFileParameters(fname, dynamicrange, tenGiga, imageHeader, imageSize, xpix, 
		       ypix, timestamp, expTime, period,imgs ) != 1)return -1;
  
  vector <int*> buffer;
  buffer.reserve(n_v *n_h*2*2);
  FILE *out;
  //nr high again
  int numFrames = fileFrameIndex+1 ;

  // open all files at once
  //here nr is not volatile anymore
  //loop on each receiver to get frame buffer
  for(int inr=0; inr<nr; inr++){
    sprintf(fname, "%s_d%d%s_%d.raw",file.c_str(),inr,frames,fileIndex);
    
    //open file
    if(!infile[inr].is_open())
      infile[inr].open(fname,ios::in | ios::binary);
  }//loop on receivers
  

 
  //now open a single root file
  TFile* ofile = new TFile(TString::Format("%s-det%d%s_%d.root",file.c_str(),
					   nr,frames,fileIndex).Data(),
			   "RECREATE");
  
  //now loop over all frames
  //for each frame
  while(numFrames<(imgs+1)){
        
    //check horizontal or vertical
    TH2F* hmap= new TH2F(TString::Format("hmap%d",numFrames).Data(),
			 TString::Format("hmap%d",numFrames).Data(),
			 ( longedge_x ? npix_x_g : npix_y_g), 0, 
			 ( longedge_x ? npix_x_g : npix_y_g), 
			 ( longedge_x ? npix_y_g : npix_x_g), 0,
			 ( longedge_x ? npix_y_g : npix_x_g));
			 
      //here nr is not volatile anymore
    //loop on each receiver to get frame buffer
    for(int inr=0; inr<nr; inr++){
      
      int* intbuffer = new int[imageSize];
      int* bufferheader=new int[imageHeader];
      //read data
      if(infile[inr].read((char*)bufferheader,imageHeader)){
	fnum = (*((uint64_t*)(char*)bufferheader));
      }
      else {
      fnum=-1;
      exit(1);
      }
            
      if(!CheckFrames(fnum,numFrames)) continue; 	 
      infile[inr].read((char*)intbuffer,imageSize);
      
      buffer.push_back(decodeData(intbuffer, imageSize, xpix, ypix, dynamicrange));
      
      delete[] bufferheader;
      delete[] intbuffer;
      bufferheader=NULL;
      intbuffer=NULL;
 
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
    
    int nnr=0;
    for(int imod_h=0; imod_h<n_h;imod_h++){
      for(int imod_v=(n_v-1); imod_v>-1; imod_v--){

	//for(int imod_h=(n_h-1); imod_h>-1;imod_h--){
	//for(int imod_v=0; imod_v<n_v; imod_v++){

	for( int it=0;it<2;it++){	
	  for( int ileft=0;ileft<2;ileft++){
	    
	    if( npix_y_user==256 && it==1) continue; 
		
	    //int nnr=(imod_h*n_v*2*2)+(imod_v*2*2)+2*it+ileft;
	    //cout<<"nnr "<<nnr<<"   "<<imod_h<<"   "<<imod_v<<"  "
	    //	<<it<<"   "<<ileft<< endl;

	     
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
		      map[k]=buffer[nnr][ix+(ichipx%2)*256+ 256*2*iy];
		      if(longedge_x) hmap->SetBinContent(x_t+1,y_t+1,map[k]);
		      if(!longedge_x) hmap->SetBinContent(y_t+1,x_t+1,map[k]);

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
		      map[k]=buffer[nnr][ix+(ichipx%2)*256+ 256*2*(255-iy)];
		      if(longedge_x) hmap->SetBinContent(x_t+1,y_t+1,map[k]);
		      if(!longedge_x) hmap->SetBinContent(y_t+1,x_t+1,map[k]);
		    }
		  }
		}
	      }
	    } //it==1
	    nnr++;
	  }//ileft
	} //it
      }//v mods
    } //h mods close all loops
		
  
    buffer.clear();
    //fill root histo from map
    //   for(int ik=0; ik<npix_y_g*npix_x_g; ++ik)
    // hmap->SetBinContent(ik+1,map[ik]);
    
    numFrames++;
    hmap->SetStats(kFALSE);
    hmap->Draw("colz");
    hmap->Write();
    
    delete hmap;
  } //loop on frames
  
  
  for(int inr=0; inr<nr; inr++)    
    infile[inr].close();
  
  
  return 1;
}




