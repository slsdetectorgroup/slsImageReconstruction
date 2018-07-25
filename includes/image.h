#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>
#include <string.h>
#include <fstream>

using namespace std;

//typedef unsigned int uint64_t;
typedef unsigned short uint16_t;

typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;

const int NumHalfModules = 2;
const int NumChanPerChip_x = 256;
const int NumChanPerChip_y = 256;
const int NumChip_x = 4;
const int NumChip_x_port = 2;
const int NumChip_y = 2;
//single module geometry
const int npix_x_sm=(NumChanPerChip_x*NumChip_x);
const int npix_y_sm=(NumChanPerChip_y*NumChip_y);
//Gap pixels
const int GapPixelsBetweenChips_x =2;
const int GapPixelsBetweenChips_y =2;
const int GapPixelsBetweenModules_x =8;
const int GapPixelsBetweenModules_y = 36;
int frameheadersize=0;

//gap pixel threatement 
enum { kZero, kDivide, kInterpolate, kMask, kInterpolate2 };

int getFileParameters(string file,  int &dr, int &tg,  int &ih, int &is, int &x, int &y,
		      string& timestamp, double& expTime, double& period, int& imgs ){

  cout << "Getting File Parameters from " << file << endl;
  string str;
  string strdayw, strmonth, strday, strtime,  stryear;
  ifstream infile;
  int dummyint;
  string timestamp_s;
  string period_s;
  
  /*
    Version            		: 1.0
    Dynamic Range      		: 32
    Ten Giga           		: 0
    Image Size         		: 524288 bytes
    x                  		: 512 pixels
    y                  		: 256 pixels
    Total Frames       		: 1
    Exptime (ns)       		: 100000000000
    SubExptime (ns)    		: 2621440
    Period (ns)        		: 1000000000
    Timestamp          		: Wed Sep 13 11:58:11 2017   
  */
  infile.open(file.c_str(),ios::in | ios::binary);
  if (infile.is_open()) {

    //empty line
    // getline(infile,str);

    //version
    if(getline(infile,str)){
    istringstream sstr(str);
    sstr >> str >> str >> str;
    cout<<"Version:"<<str<<endl;  
  }
    //dynamic range
    if(getline(infile,str)){
      istringstream sstr(str);
      cout<<"Str dynamic range:"<<str<<endl;
      sstr >> str >> str >>  str >> dr;
    }

    //ten giga
    if(getline(infile,str)){
      istringstream sstr(str);
      cout<<"Str ten giga:"<<str<<endl;
      sstr >> str >> str >> str >> tg;
    }

    //image size
    if(getline(infile,str)){
      istringstream sstr(str);
      cout<<"Str image size:"<<str<<endl;
      sstr >> str >> str >> str >> is;
    }
    //x
    if(getline(infile,str)){
      istringstream sstr(str);
      cout<<"Str x:"<<str<<endl;
      sstr >> str >> str >> x;
    }

    //y
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> y;
      cout<<"Str y:"<<y<<endl; 
   }

    //Total Frames 
    if(getline(infile,str)){
      istringstream sstr(str);
       sstr >> str >> str >> str >> imgs;
       cout<<"total frames:"<<imgs<<endl;
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
    expTime*=1e-9;
    period*= 1e-9;

    //Timestamp
    if(getline(infile,str)){
      istringstream sstr(str);
      //cout<<"Str:"<<str<<endl;
      sstr >> str >> str>> strdayw >> strmonth >> strday>> strtime >> stryear;
      timestamp = stryear+"/"+strmonth+"/"+strday+" "+strtime+".000 CEST";
    }
    //two empty lines
    getline(infile,str);
    getline(infile,str);
    getline(infile,str);
    /*
      #Frame Header
      Frame Number                    : 8 bytes
      SubFrame Number/ExpLength       : 4 bytes
      Packet Number                   : 4 bytes
      Bunch ID                        : 8 bytes
      Timestamp                       : 8 bytes
      Module Id                               : 2 bytes
      X Coordinate                    : 2 bytes
      Y Coordinate                    : 2 bytes
      Z Coordinate                    : 2 bytes
      Debug                           : 4 bytes
      Round Robin Number              : 2 bytes
      Detector Type                   : 1 byte
      Header Version                  : 1 byte
    */

    getline(infile,str);
    //Frame Number
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str>> str >> dummyint;
      frameheadersize+=dummyint;
    }
  
    //SubFrame Number/ExpLength
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str>> str >> dummyint;
      frameheadersize+=dummyint;
    }	

    //Packet Number
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str>> str >> dummyint;
      frameheadersize+=dummyint;
    }	
    
    //Bunch ID
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str>> str >> dummyint;
      frameheadersize+=dummyint;
    }	
    //Timestamp
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> dummyint;
      frameheadersize+=dummyint;
    }	
    //Module Id
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >>str >> str >> dummyint;
      frameheadersize+=dummyint;
    }	
    //X Coordinate
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >>str >> str >> dummyint;
      frameheadersize+=dummyint;
    }	
    //Y Coordinate 
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >>str >> str >> dummyint;
      frameheadersize+=dummyint;
    }	
    //Z Coordinate
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >>str >> str >> dummyint;
      frameheadersize+=dummyint;
    }	
    //Debug
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> dummyint;
      frameheadersize+=dummyint;
    }	
    //Round Robin Number
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >>str >> str >>str >> dummyint;
      frameheadersize+=dummyint;
    }	
    //Detector Type
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >>str >> str >> dummyint;
      frameheadersize+=dummyint;
    }	
    //Header Version
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >>str >> str >> dummyint;
      frameheadersize+=dummyint;
    }
    if(frameheadersize!= 8+4+4+8+8+2+2+2+2+4+2+1+1) {
      ;
    } 
    ih= frameheadersize;  
    infile.close();
  }else{
    cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
    return 0;
  }

  //validations
  switch(dr){
  case 4: case 8: case 16: case 32: break;
  default:
    cout << "Error: Invalid dynamic range " << dr << " read from file " << file << endl;
    return -1;
  }
  int packetsPerFrame;
  if(!tg)
    packetsPerFrame = 16 * dr;
  else
    packetsPerFrame = 4 * dr;
  if(!tg){
    if(is!=(packetsPerFrame*1024)){
      cout << "Error: Invalid packet size " << is << " for 1g read from file " << file << endl;
      return -1;
    }
  }
  else{
    if(is!=(packetsPerFrame*4096)){
      cout << "Error: Invalid packet size " << is << " for 10g read from file " << file << endl;
      return -1;
    }
  }
  
  return 1;
}
//const static int imageHeader=frameheadersize;

int  getCommandParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &npix_x_user, int &npix_y_user)
{
  if(argc < 2){
    cprintf(RED, "Error: Not enough arguments: cbfMaker [file_name_with_dir] \nExiting.\n");
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
      cprintf(BLUE,
	      "\n"
	      "File Name                 : %s\n"
	      "File Index                : %d\n"
	      "Frame Index Enable        : %d\n"
	      "Frame Index               : %d\n"
	      "Number of pixels in x dir : %d\n"
	      "Number of pixels in y dir : %d\n",
	      file.c_str(),fileIndex,isFileFrameIndex,fileFrameIndex, npix_x_user,npix_y_user);
      return 1;
    }else{
      npix_x_user=1024;
      npix_y_user=512;
      cprintf(BLUE,
	      "\n"
	      "File Name                   : %s\n"
	      "File Index                  : %d\n"
	      "Frame Index Enable          : %d\n"
	      "File Frame Index            : %d\n",
	      file.c_str(),fileIndex,isFileFrameIndex,fileFrameIndex);
     return 1;
    }
    return 0;
}


bool CheckFrames( int fnum, int numFrames)
{
  if(fnum!= numFrames) return false;
  return true;
}

int* decodeData(int *datain, const int size, const int nx, const int ny, const int dynamicRange) 
{
  
  int dataBytes = size;
  int nch = nx*ny;
  int* dataout = new int [nch+1];
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
    //for every 32 bit (every element in datain array)
    for (ichan=0; ichan<nch; ++ichan) { 	//for every pixel
      //		ival=datain[ichan]&0xffffff;
      //  dataout[ichan]=ival;
      dataout[ichan]=datain[ichan];
    }
  }
  
 return dataout;
  
}

void decodeData(int *datain, int* dataout, const int size, const int nx, const int ny, const int dynamicRange) 
{
  
  int dataBytes = size;
  int nch = nx*ny;
  //  int* dataout = new int [nch+1];
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
    //for every 32 bit (every element in datain array)
    for (ichan=0; ichan<nch; ++ichan) { 	//for every pixel
      //		ival=datain[ichan]&0xffffff;
      //  dataout[ichan]=ival;
      dataout[ichan]=datain[ichan];
    }
  }
  
  //return dataout;
  
}

int GetX(int ix, int ichipx, int imod_h)
{
  return ix+(NumChanPerChip_x+GapPixelsBetweenChips_x)*ichipx+(NumChanPerChip_x*NumChip_x+(NumChip_x-1)*GapPixelsBetweenChips_x+GapPixelsBetweenModules_x)*imod_h;
}
int GetY(int iy, int ichipy,int imod_v)
{
return iy+(NumChanPerChip_y+GapPixelsBetweenChips_y)*ichipy+(NumChanPerChip_y*NumChip_y+(NumChip_y-1)*GapPixelsBetweenChips_y+GapPixelsBetweenModules_y)*imod_v;
}

//int GetK(int xvirtual, int yvirtual, int longedge_x, 
//	 int npix_x_g, int npix_y_g)
//{
//int  kvirtual= xvirtual+ npix_x_g*yvirtual;
//if(!longedge_x)
//  kvirtual=(npix_y_g-yvirtual)+ npix_y_g*xvirtual;
//return kvirtual;
//}

int GetK(int xvirtual, int yvirtual, int npix_x_g)
{

  int  kvirtual= xvirtual+ npix_x_g*yvirtual;
   return kvirtual;
}

void FillCornerGapsBetweenChipZero(int* map, int k, 
				   int kvirtual1,int kvirtual2, int kvirtual3)	
{
  map[kvirtual1]=0;
  map[kvirtual2]=0;
  map[kvirtual3]=0;
}


int Divide(int k, int i)
{
  if(!(k%i))
    return k/=i;
  return k/i+(k%i);
}

bool Saturated(int k, int dynamicrange)
{
  if(dynamicrange==4 && k==15) return true;
  if(dynamicrange==8 && k==255) return true;
  if(dynamicrange==16 && k==4095) return true;
  if(dynamicrange==32 && k==(pow(2,32)-1)) return true;
  return false;
}

void FillCornerGapsBetweenChipDivide(int* map, int k, 
				     int kvirtual1,int kvirtual2, int kvirtual3,
				     int dynamicrange)	
{
  bool saturated=Saturated(map[k],dynamicrange);
  
  //divided by 4
  int koriginal=map[k];
  if(saturated==false){
    int gpixelc=(int) map[k]/4;			    
    map[k]=gpixelc;
    map[kvirtual1]=gpixelc;
    map[kvirtual2]=gpixelc;
    map[kvirtual3]=gpixelc;
    
    //asign randomly other hits
    for(int i=0; i<(koriginal%4);i++){
      double random_variable = std::rand()/(double)RAND_MAX;
      if(random_variable>=0 && random_variable<0.25) map[k]++;
      if(random_variable>=0.25 && random_variable<0.5) map[kvirtual1]++;
      if(random_variable>=0.5 && random_variable<0.75) map[kvirtual2]++;
      if(random_variable>=0.75 && random_variable<=1) map[kvirtual3]++;
    }//
  }else{
    map[k]=koriginal;
    map[kvirtual1]=koriginal;
    map[kvirtual2]=koriginal;
    map[kvirtual3]=koriginal;
  }
}

void FillGapsBetweenChipDivide(int* map, int k, int kvirtual, int dynamicrange)	
{
  bool saturated=Saturated(map[k],dynamicrange);
  int koriginal=map[k];

  if(saturated==false){
    int gpixelc=(int)map[k]/2;			    
    map[k]=gpixelc;
    map[kvirtual]=gpixelc;
    
    //asign randomly other hits
    for(int i=0; i<(koriginal%2);i++){
      double random_variable = std::rand()/(double)RAND_MAX;
      if(random_variable>=0 && random_variable<0.5) map[k]++;
      if(random_variable>=0.5 && random_variable<=1) map[kvirtual]++;
    }//
  }else{
    map[k]=koriginal;
    map[kvirtual]=koriginal;
  }
}

void FillGapsBetweenChipZero(int* map, int k, int kvirtual, 
			       int kvirtual2, int k2)	
{
  map[kvirtual]=0;
  map[kvirtual2]=0;
}

void FillGapsBetweenChipDivide(int* map, int k, int kvirtual, 
			       int kvirtual2, int k2,int dynamicrange)	
{
  FillGapsBetweenChipDivide(map, k,kvirtual , dynamicrange);
  FillGapsBetweenChipDivide(map, k2,kvirtual2, dynamicrange);
}

void FillGapsBetweenChipInterpolate(int* map, int k, int kvirtual, 
				    int kvirtual2, int k2,int dynamicrange)	
{
  bool saturated=Saturated(map[k],dynamicrange);
  bool saturated2=Saturated(map[k2],dynamicrange);

  if(map[k]==map[k2] || saturated || saturated2) {
    FillGapsBetweenChipDivide( map, k, kvirtual, kvirtual2, k2,dynamicrange);
  }
  else{ 
    if(map[k]==0){
      map[k]=0;
      map[kvirtual]=0;
      FillGapsBetweenChipDivide(map,k2,kvirtual2,dynamicrange);
    }else {
      if(map[k2]==0){
	map[k2]=0;
	map[kvirtual2]=0;
	FillGapsBetweenChipDivide(map,k,kvirtual,dynamicrange);
      } else{
	int c1=map[k];
	int c4=map[k2];

	if(c4>c1){
	  map[k2]= (int)((15.*c4-3.*c1)/24.);
	  map[kvirtual2]=(int)(c4-map[k2]);
	  map[k]= (int)(3.*c4-5.*map[k2]);
	  map[kvirtual]=(int)(c1-map[k]);
	}
	else{
	  map[k]= (int)((15.*c1-3.*c4)/24.);
	  map[kvirtual]=(int)(c1-map[k]);
	  map[k2]= (int)(3.*c1-5.*map[k]);
	  map[kvirtual2]=(int)(c4-map[k2]);
	}
	
	if(map[k2]<0 || map[kvirtual2]<0 ||
	   map[k]<0 || map[kvirtual]<0){
	  //divide then
	  map[k]=c1;//reset value
	  map[k2]=c4;//reset value
	  FillGapsBetweenChipDivide(map,k2,kvirtual2,dynamicrange);
	  FillGapsBetweenChipDivide(map,k,kvirtual,dynamicrange);
	}
      }//else
    }
  }
}
void FillGapsBetweenChipInterpolate2(int* map, int k_b, int k, int kvirtual, 
				     int kvirtual2, int k2,int k2_a, 
				     int dynamicrange)	
{
  bool saturated=Saturated(map[k],dynamicrange);
  bool saturated2=Saturated(map[k2],dynamicrange);

  if(map[k]==map[k2] || saturated || saturated2) {
    FillGapsBetweenChipDivide( map, k, kvirtual, kvirtual2, k2,dynamicrange);
  }
  else{ 
    if(map[k]==0){
      map[k]=0;
      map[kvirtual]=0;
      FillGapsBetweenChipDivide(map,k2,kvirtual2,dynamicrange);
    }else {
      if(map[k2]==0){
	map[k2]=0;
	map[kvirtual2]=0;
	FillGapsBetweenChipDivide(map,k,kvirtual,dynamicrange);
      } else{
	int c1=map[k];
	int c4=map[k2];

	//now interpolate according to Sophie's scheme
	int a=map[k_b]+1./3*(c4/2.-map[k_b]);
	int b=map[k_b]+2./3*(c4/2.-map[k_b]);
	int c=c1/2.+1./3.*(map[k2_a]-c1/2.);
	int d=c1/2.+2./3.*(map[k2_a]-c1/2.);

	//now rescale. always keep lower integer
      	map[k]=(double)a*c1/(a+b);
	map[kvirtual]=(double)b*c1/(a+b);
       	map[k2]=(double)d*c4/(c+d);
	map[kvirtual2]=(double)c*c4/(c+d);

	for(int i=0; i<(c1%2);i++){
	  double random_variable = std::rand()/(double)RAND_MAX;
	  if(random_variable>=0 && random_variable<0.5) map[k]++;
	  if(random_variable>=0.5 && random_variable<=1) map[kvirtual]++;
	}
	for(int i=0; i<(c4%2);i++){
	  double random_variable = std::rand()/(double)RAND_MAX;
	  if(random_variable>=0 && random_variable<0.5) map[kvirtual2]++;
	  if(random_variable>=0.5 && random_variable<=1) map[k2]++;
	}

	if(map[k2]<0 || map[kvirtual2]<0 ||
	   map[k]<0 || map[kvirtual]<0){
	  //divide then
	  map[k]=c1;//reset value
	  map[k2]=c4;//reset value
	  FillGapsBetweenChipDivide(map,k2,kvirtual2,dynamicrange);
	  FillGapsBetweenChipDivide(map,k,kvirtual,dynamicrange);
	}
      }//else
    }
  }
}

void FillGapsBetweenChipMask(int* map, int k, int kvirtual, 
			     int kvirtual2, int k2)	
{
  map[k]=0;
  map[kvirtual]=0;
  map[kvirtual2]=0;
  map[k2]=0;
}

int  getCommandParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &npix_x_user, int &npix_y_user, int& longedge_x, int& fillgaps, int &startdet){
  if(argc < 2){
    cprintf(RED, "Error: Not enough arguments: cbfMaker [file_name_with_dir] \nExiting.\n");
    exit(-1);
  }
  file=argv[1];
  getCommandParameters(argc, argv, file, fileIndex, isFileFrameIndex, fileFrameIndex, npix_x_user, npix_y_user);

  if(argc>2){
        
    if(argc>4) longedge_x=atoi(argv[4]);
    else longedge_x=1;
    if(argc>5) fillgaps=atoi(argv[5]);
    else fillgaps=kInterpolate; //0 no filling, 1 division, 2 interpolation 3 mask
    if(argc>6) startdet=atoi(argv[6]);
    else startdet=0;
    

    cprintf(BLUE,
	    "Module long edge is on x  : %d\n"
	    "Fill gaps between chips   : %d\n"
	    "Start detector index      : %d\n",
	    longedge_x, fillgaps, startdet);
    return 1;
  }else{
    longedge_x=1;
    fillgaps=kInterpolate;
    startdet=0;
    return 1;
  }
  return 0;
}

int local_exit(int status) {
  exit(status);
  return status;    /* to avoid warning messages */
}



#endif
