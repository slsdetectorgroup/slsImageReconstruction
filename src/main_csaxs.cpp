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

bool CheckFrames( int fnum, int numFrames)
{
  if(fnum!= numFrames) return false;
  return true;
}

int local_exit(int status) {
  exit(status);
  return status;    /* to avoid warning messages */
}

int getFileParameters(string file,  int &hs, int &tp, int &lt, int &act, int &dr, int &tg, int &is, int &x, int &y,
		      string& timestamp, int& expTime, int& period){
   	cout << "Getting File Parameters from " << file << endl;
	string str;
	string strdayw, strmonth, strday, strtime,  stryear;
	ifstream infile;
	int dummyint;
	/*
	  Header		: 500 bytes
	  Top		: 1
	  Left		: 1
	  Active		: 1
	  Frames Caught	: 1
	  Frames Lost	: 0
	  Dynamic Range	: 16
	  Ten Giga	: 0
	  Image Size	: 262144 bytes
	  x		: 512 pixels
	  y		: 256 pixels
	  Total Frames	: 1
	  Exptime (ns)	: 1000000000
	  Period (ns)	: 1000000000
	  Timestamp	: Fri Dec  2 12:40:33 2016
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

		//Total Frames 
		if(getline(infile,str)){
		  istringstream sstr(str);
		  sstr >> str >> str >> str >> dummyint;
		}


		// Exptime (ns)	: 1000000000
		if(getline(infile,str)){
		  istringstream sstr(str);
		  sstr >> str >> str >> str >> expTime;
		}
		//Period (ns)	: 1000000000
		if(getline(infile,str)){
		  istringstream sstr(str);
		  sstr >> str >> str >> str >> period;
		}

		//Timestamp
		if(getline(infile,str)){
		  istringstream sstr(str);
		  //cout<<"Str:"<<str<<endl;
		  sstr >> str >> str>> strdayw >> strmonth >> strday>> strtime >> stryear;
		  timestamp = stryear+"/"+strmonth+"/"+strday+" "+strtime+".000 CEST";
		}
		
		infile.close();
	}else{
		cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
		return slsReceiverDefs::FAIL;
	}

	return slsReceiverDefs::OK;
}

int  getCommandParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &npix_x_user, int &npix_y_user, int& longedge_x, int &startdet){
  if(argc < 2){
    cprintf(RED, "Error: Not enough arguments: cnbfMaker [file_name_with_dir] \nExiting.\n");
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
      if(argc < 4){
	cprintf(RED, "Error: Not enough arguments: cbfMaker [file_name_with_dir] "
		"[numpixels_x][numpixels_y] [modulelongedge_x] [start_detector_Index]\nExiting.\n");
	exit(-1);
      }
      npix_x_user=atoi(argv[2]);
      npix_y_user=atoi(argv[3]);
      if(argc>4)longedge_x=atoi(argv[4]);
      else longedge_x=1;
      if(argc>5) startdet=atoi(argv[5]);
      else startdet=0;

      cprintf(BLUE,
	      "\n"
	      "File Name                 : %s\n"
	      "File Index                : %d\n"
	      "Frame Index Enable        : %d\n"
	      "Frame Index               : %d\n"
	      "Number of pixels in x dir : %d\n"
	      "Number of pixels in y dir : %d\n"
              "Module long edge is on x  : %d\n"
	      "Start detector index      : %d\n",
	      file.c_str(),fileIndex,isFileFrameIndex,fileFrameIndex, npix_x_user,npix_y_user,longedge_x,startdet);
      return 1;
    }else{
      npix_x_user=1024;
      npix_y_user=512;
      longedge_x=1;
      startdet=0;
      cprintf(BLUE,
	      "\n"
	      "File Name                   : %s\n"
	      "File Index                  : %d\n"
	      "Frame Index Enable          : %d\n"
	      "File Frame Index            : %d\n",
	      file.c_str(),fileIndex,isFileFrameIndex,fileFrameIndex);
    }
}

int* decodeData(int *datain, const int size, const int nx, const int ny, const int dynamicRange) 
{
  
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
    ; //for every 32 bit (every element in datain array)
    //for (ichan=0; ichan<nch; ++ichan) { 	//for every pixel
    //		ival=datain[ichan]&0xffffff;
    //		dataout[ichan]=ival;
    //	}
  }
  
  
  
  return dataout;
  
}

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
  if( npix_y_user==256)  npix_y_g = npix_y_user;
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
  int nfile=startdet;
  //put master on top always
  int fileheadersize, top, left, active, dynamicrange, tenGiga, packetSize, xpix, ypix, imageSize;
  string timestamp;
  double expTime, period;
 int iexpTime, iperiod;

  for(int imod_h=0; imod_h<n_h; imod_h++){
    for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
      for( int it=0;it<2;it++){
	for( int ileft=0;ileft<2;ileft++){
	  if( npix_y_user==256 && it==1 ) continue;  
	  
	  sprintf(fname,"%s_d%d%s_%d.raw",file.c_str(),nfile,frames,fileIndex);
	  //get file parameters
	  if(getFileParameters(fname, fileheadersize, top, left, active, dynamicrange, tenGiga, imageSize, xpix, 
			       ypix, timestamp, iexpTime, iperiod ) != slsReceiverDefs::OK)return -1;
	  
	  expTime=iexpTime*1e-9;
	  period=iperiod* 1e-9;
	  
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
	  }
	  else{
	    if(imageSize!=(packetsPerFrame*4096)){
	      cout << "Error: Invalid packet size " << imageSize << " for 10g read from file " << file << endl;
	      return -1;
	    }
	  }
	  
	  nfile++; 	  
	}
      }
    }
  }
  
  //Create cbf files with data
  cbf_handle cbf;
  vector <int*> buffer;
  buffer.reserve(n_v *n_h*2);
  FILE *out;
  //nr high again
  int numFrames = fileFrameIndex+1 ;

  const static int imageHeader = 16;
  
    
  //for each frame
  while(fnum>-1){
    int* value;
  
    //here nr is not volatile anymore
    //loop on each receiver to get frame buffer
    for(int inr=0; inr<nr; inr++){
      sprintf(fname, "%s_d%d%s_%d.raw",file.c_str(),inr,frames,fileIndex);
    
      //open file
      if(!infile[inr].is_open())
	infile[inr].open(fname,ios::in | ios::binary);
      if(infile[inr].is_open()){

	//read file header
	if( numFrames == fileFrameIndex+1){
	  //read file header
	  char data[fileheadersize];
	  infile[inr].read(data,fileheadersize);
	}
	int* intbuffer = new int[imageSize];
	//read data
	infile[inr].read((char*)intbuffer,(imageSize+imageHeader));
	fnum = (*((uint64_t*)(char*)intbuffer));
	if(!CheckFrames(fnum,numFrames)) continue; 	
	buffer.push_back(decodeData(intbuffer, imageSize, xpix, ypix, dynamicrange));
	
      } //while read images
    }//loop on receivers
	
	if(fnum==-1) {
	  exit(1);
	}
	
	if(buffer.size()!=nr) continue;
	
	//get a 2d map of the image
	int inr=0;
	//initialize
	for(int ik=0; ik<npix_y_g*npix_x_g;++ik)
	  map[ik]=-1; 
	
	for(int imod_h=0; imod_h<n_h;imod_h++){
	  for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
	    for( int it=0;it<2;it++){	
	      for( int ileft=0;ileft<2;ileft++){	
		if( npix_y_user==256 && it==1) continue; 
		
		/* Make a cbf version of the image */
		//getting values //top
		if(it==0){
		  if(ileft==0){
		    
		    if( npix_y_user!=256 ){			      
		      
		      for(int ichipy=1; ichipy<NumChip_y;ichipy++){
		    for(int iy=0; iy<NumChanPerChip_y;iy++){
		      for(int ichipx=0; ichipx<NumChip_x-2;ichipx++){
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
			  
			  // map2[ix+(NumChanPerChip_x)*ichipx + 1024*iy ]=buffer[inr][ix+ichipx*256+ 512*iy]; 
			  map[k]=buffer[inr][ix+ichipx*256+ 512*iy];
			}
		      }
		    }
		  }
		}
		else{
		  //single half module
		  for(int ichipy=0; ichipy<1;ichipy++){
		    for(int iy=0; iy<NumChanPerChip_y;iy++){
		      for(int ichipx=0; ichipx<NumChip_x;ichipx++){
			for(int ix=0; ix<NumChanPerChip_x;ix++){
			  
			  int k= ix+(NumChanPerChip_x+GapPixelsBetweenChips_x)*ichipx+(NumChanPerChip_x*NumChip_x+(NumChip_x-1)*GapPixelsBetweenChips_x+ GapPixelsBetweenModules_x)*imod_h + npix_x_g*(iy+(NumChanPerChip_y+GapPixelsBetweenChips_y)*ichipy+(NumChanPerChip_y*NumChip_y+(NumChip_y-1)*GapPixelsBetweenChips_y +GapPixelsBetweenModules_y )*imod_v);
			  map[k]=buffer[inr][ix+ichipx*256+ 512*iy];
			}
		      }
		    }
		  }
		}
	      }
	      //right 
	      else{

		if( npix_y_user!=256 ){			      

		  for(int ichipy=1; ichipy<NumChip_y;ichipy++){
		    for(int iy=0; iy<NumChanPerChip_y;iy++){
		      for(int ichipx=2; ichipx<NumChip_x;ichipx++){
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

			  // map2[ix+ichipx*256+ 1024*iy]=buffer[inr][ix+(ichipx-2)*256+ 512*iy];
			  map[k]=buffer[inr][ix+(ichipx-2)*256+ 512*iy];
			}
		      }
		    }
		  }
		}
		else{
		  //single half module		
		  for(int ichipy=0; ichipy<1;ichipy++){
		    for(int iy=0; iy<NumChanPerChip_y;iy++){
		      for(int ichipx=0; ichipx<NumChip_x;ichipx++){
			for(int ix=0; ix<NumChanPerChip_x;ix++){
			  
			  int k= ix+(NumChanPerChip_x+GapPixelsBetweenChips_x)*ichipx+(NumChanPerChip_x*NumChip_x+(NumChip_x-1)*GapPixelsBetweenChips_x+ GapPixelsBetweenModules_x)*imod_h + npix_x_g*(iy+(NumChanPerChip_y+GapPixelsBetweenChips_y)*ichipy+(NumChanPerChip_y*NumChip_y+(NumChip_y-1)*GapPixelsBetweenChips_y +GapPixelsBetweenModules_y )*imod_v);
			  map[k]=buffer[inr][ix+(ichipx-2)*256+ 512*iy];
			}
		      }
		    }
		  }
		}
	      }


	    }

	    //getting values for bottom
	    if(it==1 ) {
	      //left
	    if(ileft==0){

	      for(int ichipy=0; ichipy<1;ichipy++){
		for(int iy=0; iy<NumChanPerChip_y;iy++){
		  for(int ichipx=0; ichipx<NumChip_x-2;ichipx++){
		    for(int ix=0; ix<NumChanPerChip_x;ix++){
		      int x_t=ix+(NumChanPerChip_x+GapPixelsBetweenChips_x)*ichipx+(NumChanPerChip_x*NumChip_x+(NumChip_x-1)*GapPixelsBetweenChips_x+GapPixelsBetweenModules_x)*imod_h;
		      int y_t= iy+(NumChanPerChip_y+GapPixelsBetweenChips_y)*ichipy+(NumChanPerChip_y*NumChip_y+(NumChip_y-1)*GapPixelsBetweenChips_y+GapPixelsBetweenModules_y)*imod_v;
		      
		      int k=  x_t+npix_x_g*y_t;
		      if(!longedge_x){//now apply rotation 
			//y'=x
			//x'=(ngap_pix_y-iy)
			k=(npix_y_g-y_t)+ npix_y_g*x_t;
		      }
		      map[k]=buffer[inr][ix+ichipx*256+ 512*(255-iy)];
		  }
		}
	      }
	    }
	  }
	    else {
	      //right
	      for(int ichipy=0; ichipy<1;ichipy++){
		for(int iy=0; iy<NumChanPerChip_y;iy++){
		  for(int ichipx=2; ichipx<NumChip_x;ichipx++){
		    for(int ix=0; ix<NumChanPerChip_x;ix++){
		      int x_t=ix+(NumChanPerChip_x+GapPixelsBetweenChips_x)*ichipx+(NumChanPerChip_x*NumChip_x+(NumChip_x-1)*GapPixelsBetweenChips_x+GapPixelsBetweenModules_x)*imod_h;
		      int y_t= iy+(NumChanPerChip_y+GapPixelsBetweenChips_y)*ichipy+(NumChanPerChip_y*NumChip_y+(NumChip_y-1)*GapPixelsBetweenChips_y+GapPixelsBetweenModules_y)*imod_v;
		      
		      int k=  x_t+npix_x_g*y_t;
		      if(!longedge_x){//now apply rotation 
			//y'=x
			//x'=(ngap_pix_y-iy)
			k=(npix_y_g-y_t)+ npix_y_g*x_t;
		      }
		      map[k]=buffer[inr][ix+(ichipx-2)*256 + 512*(255-iy)];
		    }
		  }
		}
	      }
	    }
	    } 
	    inr++;
	  }
	}
      }
    } //close all loops
		
  
    
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
    time_t rawtime = time(NULL);
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

    //cprintf(GREEN,"CBF File Created: %s\n\n",fname);

    numFrames++;

    cbf_failnez (cbf_free_handle (cbf));

  
  }


  //close file when not frame yet
  for(int inr=0; inr<nr; inr++)
    if (infile[inr].is_open())
      infile[inr].close();
	
  //buffer.clear();
  
  /* Free the cbf */
  cbf_failnez (cbf_free_handle (cbf));
  
  return 1;
}




