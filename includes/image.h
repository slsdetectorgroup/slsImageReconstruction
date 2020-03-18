#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <typeinfo>
#include <vector>
#include <unistd.h>

using namespace std;

//typedef unsigned int uint64_t;
typedef unsigned short uint16_t;

typedef double double32_t;
typedef float float32_t;
typedef int int32_t;
typedef unsigned int uint32_t;

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
int imagesize=0;
int dynamicrange;
string outdir;


//gap pixel threatement 
enum { kZero, kDivide, kInterpolate, kMask, kInterpolate2, kIgnore};

//class container
//{

//public:
  
//container();
//container(int range) 
//{
      //    if(range==32) uint32_t m_val;
      //if(range==8) uint8_t m_val;
      //if(range==16) uint16_t m_val;
     
      //}
      // ~container() { }

//uint32_t value() {return m_val32;}
// uint16_t value() {return m_val16;}
//uint8_t  value() {return m_val8;}

// private:
 //template <typename T>
  
 // uint32_t m_val32;
 //uint16_t m_val16;
 //uint8_t  m_val8;
 //T m_val;

      //};


template <typename T>
class vec : public std::vector<T>  {};

int getFileParameters(string file, int &tg,  int &ih, int &is, int &x, int &y,
		      string& timestamp, double& expTime,  double& subexptime, double& period, double& subperiod, int& imgs, int& imgspfile, int& gpenabled, int& quad){

  cout << "Getting File Parameters from " << file << endl;
  string str;
  string strdayw, strmonth, strday, strtime,  stryear;
  ifstream infile;
  int dummyint;
  string timestamp_s;
  string period_s;
  int dr;
    
  /*
    Version                    : 4.0
    Detector Type              : 3
    Dynamic Range              : 32
    Ten Giga                   : 1
    Image Size                 : 524288 bytes
    nPixelsX                   : 512 pixels
    nPixelsY                   : 256 pixels
    Max Frames Per File        : 10000
    Total Frames               : 3
    Exptime (ns)               : 10000000
    SubExptime (ns)            : 2621440
    SubPeriod(ns)              : 2621440
    Period (ns)                : 0
    Gap Pixels Enable          : 0
    Quad Enable                : 0
    Timestamp                  : Wed Aug 21 16:30:20 2019
   */

  infile.open(file.c_str(),ios::in | ios::binary);
  if (infile.is_open()) {

    //empty line
    // getline(infile,str);

    //version
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> str >> str;
      cout<<"Version:"<<str<<endl;  
    }
    //detector type
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> str;
    }
    
    //dynamic range
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >>  str >> dr;
      dynamicrange=dr;
     cout<<"dynamic range:"<<dr<<endl;
    }

    //ten giga
    if(getline(infile,str)){
      istringstream sstr(str);
      //      cout<<"Str ten giga:"<<str<<endl;
      sstr >> str >> str >> str >> tg;
      cout<<"ten giga :"<<tg<<endl; 
    }
    
    //image size
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> str >> is;
      imagesize=is;
    }
    
    //x
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> x;
    }

    //y
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> y;
    }
    
    // Max. Frames Per File       : 10000
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> str >> str >> str >>  imgspfile;
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
    //  SubExptime (ns)    		: 2621440
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> str >> subexptime;
    }

    //SubPeriod(ns)              : 2661440
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >>  subperiod;
    }
    
    //Period (ns)	: 1000000000
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> str >> period;
    }
    expTime*=1e-9;
    period*= 1e-9;
    subexptime*=1e-9;
    subperiod*= 1e-9;
   
    //Gap Pixels Enable          : 0
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> str >> str >> gpenabled;
    
    }
    //Quad Enable                : 0
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> str >> quad;
    }
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
    
    /*
      #Frame Header
      Frame Number                    : 8 bytes
      SubFrame Number/ExpLength       : 4 bytes
      Packet Number                   : 4 bytes
      Bunch ID                        : 8 bytes
      Timestamp                       : 8 bytes
      Module Id                       : 2 bytes
      X Coordinate                    : 2 bytes
      Y Coordinate                    : 2 bytes
      Z Coordinate                    : 2 bytes
      Debug                           : 4 bytes
      Round Robin Number              : 2 bytes
      Detector Type                   : 1 byte
      Header Version                  : 1 byte
      Packets Caught Mask             : 64 bytes
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
    //Packets Caught Mask        
    if(getline(infile,str)){
      istringstream sstr(str);
      sstr >> str >> str >> str >> str>> dummyint;
      frameheadersize+=dummyint;
    }

    if(frameheadersize!= 8+4+4+8+8+2+2+2+2+4+2+1+1+64) {
      assert(0);
    } 

    ih= frameheadersize;  
    infile.close();
  }else{
    cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
    return 0;
  }

  //validations
  switch(dynamicrange){
  case 4: case 8: case 16: case 32: break;
  default:
    cout << "Error: Invalid dynamic range " << dynamicrange << " read from file " << file << endl;
    return -1;
  }
  int packetsPerFrame;
  if(!tg)
    packetsPerFrame = 16 * dynamicrange;
  else
    packetsPerFrame = 4 * dynamicrange;
  if(!tg){
    if((gpenabled==0) && (is!=(packetsPerFrame*1024))){
	cout << "Error: Invalid packet size " << is << " for 1g read from file " << file <<" gappixels enabled "<<gpenabled<< endl;
	return -1;
    }
    //so far nly 32 bit implemented
    if((gpenabled==1) && (is!=(((512+2)*257*4)))){
      cout << "Error: Invalid packet size " << is << " for 1g read from file " << file <<" gappixels enabled "<<gpenabled<<endl;
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
string GetFileNoDir(string file){
  string::size_type position=0;
  string::size_type last_position=0;
  
  while(1){
    position = file.find ('/',position+1);
    if (position == string::npos) break;
    last_position=position;
  }
  //now I know position
  file.erase (0,last_position+1);
 
  return file;
}

string GetDir(string file){
  cout<<file<<endl;
  string::size_type position=0;
  string::size_type last_position=0;
  
  while(1){
    position = file.find ('/',position+1);
    if (position == string::npos) break;
    last_position=position;
  }
  //  cout<<file<<"   "<<last_position+1<<"   "<<file.size()<<endl;
  file.erase (last_position,file.size());
 
  cout<<file<<endl;
  return file;
}

int  getCommandParameters(int argc, char *argv[], string &file, int &fileIndex, bool &isFileFrameIndex, int &fileFrameIndex, int &npix_x_user, int &npix_y_user, int& longedge_x, int& fillgaps, string& datasetname, bool& maskpix, int& thratecorr){
  
  int c;
  string s;
  size_t uscore;
  //default
  npix_x_user=1024;
  npix_y_user=512;
  longedge_x=1;
  fillgaps=kInterpolate;
  datasetname="data";
  maskpix=false; 
  thratecorr=0; 
  
  outdir="";
  while ( (c = getopt(argc, argv, "f:d:x:y:n:vg:mr:")) != -1) {
    switch (c) 
      {
      case 'f': //[file]
	int i;  
	file=optarg;
        s=file;
	uscore=file.rfind("_");
	if (sscanf(s.substr(uscore+1,s.size()-uscore-1).c_str(),"%d",&i)) {
	  fileIndex=i;
	  s=file.substr(0,uscore);
	}
	uscore=s.rfind("_");
	if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"f%d",&i)) {
	  isFileFrameIndex = true;
	  fileFrameIndex = i;
	  s=file.substr(0,uscore);
	}
	uscore=s.rfind("_");
	if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"d%d",&i)){
	  s=file.substr(0,uscore);
	}
	
	//put all defaults here [file needs to be the first argument tough]
	file=s; 
	if(outdir.empty()) outdir= GetDir(file);
	break;
      case 'd': //[dir]
	outdir=optarg;
	break;    
      case 'x': //[npix_x]
	npix_x_user=atoi(optarg);
	break; 
      case 'y':// [npix_y]
	npix_y_user=atoi(optarg);
	break; 
      case 'v': //[vertical]
	longedge_x=0;
	break;
      case 'g':  //[gap pix]
	fillgaps=atoi(optarg);//0 no filling, 1 division, 2 interpolation 3 mask, interpolate alternative, 4 ignore
	break;
      case 'n': //[name of the dataset ]
	datasetname=optarg;
	break;
      case 'm': //[mask pix]
	maskpix=true;
	break;
      case 'r':
	thratecorr=atoi(optarg);
	break;
	
  }//switch
    
  }//while options
  
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
}


bool CheckFrames( int fnum, int numFrames)
{
  if(fnum!= numFrames) return false;
  return true;
}

void decodeData(unsigned int *datain, unsigned int* dataout, const int size, const int nx, const int ny) 
{
  
  int dataBytes = size;
  int nch = nx*ny;
  //  int* dataout = new int [nch+1];
  char *ptr=(char*)datain;
  char iptr;

  const int bytesize=8;
  unsigned int ival=0;
  int  ipos=0, ichan=0, ibyte;
  
  switch (dynamicrange) {
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
    //for (ichan=0; ichan<nch; ++ichan) { 	//for every pixel
    //dataout[ichan]=datain[ichan];
    //}  
    memcpy(&dataout[0], datain,	nch*sizeof(int));
  }
  
}

/*void decodeData(unsigned int *datain, std::vector <unsigned int*>& buffer,  
		const int dataBytes, 
		const int nx, const int ny, int Nimgs) 
{

  char *ptr=(char*)datain;
  int nch = nx*ny;
  char iptr;
  const int bytesize=8;
  
  for(int im=0; im<Nimgs; ++im){
    //will delete it in buffer
    unsigned int* dataout = new unsigned int[nx* ny]; 

    unsigned int ival=0;
    int  ipos=0, ichan=0;
    
    //skip header
    switch (dynamicrange) {
    case 4:
      //for (ibyte=0; ibyte<dataBytes; ++ibyte) {//for every byte (1 pixel = 1/2 byte)
      //iptr=ptr[ibyte]&0xff;				//???? a byte mask
      //for (ipos=0; ipos<2; ++ipos) {		//loop over the 8bit (twice)
      //ival=(iptr>>(ipos*4))&0xf;		//pick the right 4bit
      //dataout[ichan]=ival;
      //ichan++;
      //}
      //}
      for (int ibyte=frameheadersize*(im+1)+(dataBytes)*im; 
	   ibyte<frameheadersize*(im+1)+(dataBytes)*im+dataBytes; 
	   ++ibyte) {//for every byte (1 pixel = 1/2 byte)
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
      ival=ptr[ichan+frameheadersize*(im+1)+dataBytes*im]&0xff;				//????? a byte mask
      dataout[ichan]=ival;
      }
      break;
    case 16:
      for(ichan=0; ichan<nch; ++ichan){ 	//for every pixel
	ival=0;
	for (int ibyte=0; ibyte<2;++ibyte){ 	//for each byte (concatenate 2 bytes to get 16 bit value)
	  iptr=ptr[ichan*2+ibyte+frameheadersize*(im+1)+dataBytes*im];
	  ival|=((iptr<<(ibyte*bytesize))&(0xff<<(ibyte*bytesize)));
      	}
      	dataout[ichan]=ival;
      }
      break;
    default:
      memcpy(&dataout[0], (char*)datain+frameheadersize*(im+1)+dataBytes*im, nch*sizeof(unsigned int));
    }
    buffer.push_back(dataout); 
  }//loop on images

}
*/

int GetX(int ix, int ichipx, int imod_h)
{
  return ix+(NumChanPerChip_x+GapPixelsBetweenChips_x)*ichipx+(NumChanPerChip_x*NumChip_x+(NumChip_x-1)*GapPixelsBetweenChips_x+GapPixelsBetweenModules_x)*imod_h;
}

int GetY(int iy, int ichipy,int imod_v)
{
  return iy+(NumChanPerChip_y+GapPixelsBetweenChips_y)*ichipy+(NumChanPerChip_y*NumChip_y+(NumChip_y-1)*GapPixelsBetweenChips_y+GapPixelsBetweenModules_y)*imod_v;
}

int GetK(int xvirtual, int yvirtual, int npix_x_g)
{

  int  kvirtual= xvirtual+ npix_x_g*yvirtual;
  return kvirtual;
}

void FillCornerGapsBetweenChipZero(uint* map, int k, 
				   int kvirtual1,int kvirtual2, int kvirtual3)	
{
  map[kvirtual1]=0;
  map[kvirtual2]=0;
  map[kvirtual3]=0;
}

unsigned int Divide(int k, unsigned int i)
{
  if(!(k%i))
    return k/=i;
  return k/i+(k%i);
}

bool Saturated(unsigned int k)
{
  if(dynamicrange==4 && k==15) return true;
  if(dynamicrange==8 && k==255) return true;
  if(dynamicrange==16 && k==4095) return true;
  if(dynamicrange==32 && k==((long int)(pow(2,32))-1)) return true;
  return false;
}

void FillCornerGapsBetweenChipDivide(uint* map, int k, 
				     int kvirtual1,int kvirtual2, int kvirtual3)	
{
  bool saturated=Saturated(map[k]);
  
  //divided by 4
  unsigned int koriginal=map[k];
  if(saturated==false){
    unsigned int gpixelc=(unsigned int) map[k]/4;			    
    map[k]=gpixelc;
    map[kvirtual1]=gpixelc;
    map[kvirtual2]=gpixelc;
    map[kvirtual3]=gpixelc;
    
    //asign randomly other hits
    for(unsigned int i=0; i<(koriginal%4);i++){
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

void FillGapsBetweenChipDivide(uint* map, int k, int kvirtual)	
{
  bool saturated=Saturated(map[k]);
  unsigned int koriginal=map[k];

  if(saturated==false){
    unsigned int gpixelc=(unsigned int)map[k]/2;			    
    map[k]=gpixelc;
    map[kvirtual]=gpixelc;
    
    //asign randomly other hits
    for(unsigned int i=0; i<(koriginal%2);i++){
      double random_variable = std::rand()/(double)RAND_MAX;
      if(random_variable>=0 && random_variable<0.5) map[k]++;
      if(random_variable>=0.5 && random_variable<=1) map[kvirtual]++;
    }//
  }else{
    map[k]=koriginal;
    map[kvirtual]=koriginal;
  }
}

void FillGapsBetweenChipZero(uint* map, int k, int kvirtual, 
			     int kvirtual2, int k2)	
{
  map[kvirtual]=0;
  map[kvirtual2]=0;
}

void FillGapsBetweenChipDivide(uint* map, int k, int kvirtual, 
			       int kvirtual2, int k2)	
{
  FillGapsBetweenChipDivide(map, k,kvirtual );
  FillGapsBetweenChipDivide(map, k2,kvirtual2);
}

void FillGapsBetweenChipInterpolate(unsigned int* map, int k, int kvirtual, 
				    int kvirtual2, int k2)	
{
  bool saturated=Saturated(map[k]);
  bool saturated2=Saturated(map[k2]);

  if(map[k]==map[k2] || saturated || saturated2  || (map[k]==0) || map[k2]==0) {
    FillGapsBetweenChipDivide( map, k, kvirtual, kvirtual2, k2);
  }
  else{ 
    unsigned int c1=map[k];
    unsigned int c4=map[k2];
    long int ik2,ikvirtual2,ik, ikvirtual;
    
    if(c4>c1){
      //here need to temporarly use long int for interpolation 
      ik2=((15.*c4-3.*c1)/24.);
      ikvirtual2=(c4-map[k2]);
      ik=(3.*c4-5.*map[k2]);
      ikvirtual=(c1-map[k]);
    }
    else{
      ik= ((15.*c1-3.*c4)/24.);
      ikvirtual=(c1-map[k]);
      ik2=(3.*c1-5.*map[k]);
      ikvirtual2=(c4-map[k2]);
    }
    
    //check for negartives in long int
    if(ik2<0 || ikvirtual2<0 ||
       ik<0 || ikvirtual<0){
      //if negative then don't use (othehrwise 
      //it is converted as a high number)
      FillGapsBetweenChipDivide(map,k2,kvirtual2);
      FillGapsBetweenChipDivide(map,k,kvirtual);
    }else{
      //now i am sure i can convert long int to unsigned int
      map[k]=ik;
      map[kvirtual]=ikvirtual;
      map[k2]=ik2;
      map[kvirtual2]=ikvirtual2;
      
      if( Saturated(map[ik]) ||
	  Saturated(map[kvirtual]) || 
	  Saturated(map[k2])  || 
	  Saturated(map[kvirtual2])){
	//i after interpolation negative or saturate divide then
	map[k]=c1;//reset value
	map[k2]=c4;//reset value
	FillGapsBetweenChipDivide(map,k2,kvirtual2);
	FillGapsBetweenChipDivide(map,k,kvirtual);
      }
    }//else
  }//else
}
void FillGapsBetweenChipInterpolate2(unsigned int* map, int k_b, int k, int kvirtual, 
				     int kvirtual2, int k2,int k2_a)	
{
 
  //should be properly corrected  
  bool saturated=Saturated(map[k]);
  bool saturated2=Saturated(map[k2]);

  if(map[k]==map[k2] || saturated || saturated2 || (map[k]==0) || map[k2]==0) {
    FillGapsBetweenChipDivide( map, k, kvirtual, kvirtual2, k2);
  }
  else{ 
    unsigned int c1=map[k];
    unsigned int c4=map[k2];
    
    //now interpolate according to Sophie's scheme
    unsigned int a=map[k_b]+1./3*(c4/2.-map[k_b]);
    unsigned int b=map[k_b]+2./3*(c4/2.-map[k_b]);
    unsigned int c=c1/2.+1./3.*(map[k2_a]-c1/2.);
    unsigned int d=c1/2.+2./3.*(map[k2_a]-c1/2.);
    
    //now rescale. always keep lower integer
    map[k]=(double)a*c1/(a+b);
    map[kvirtual]=(double)b*c1/(a+b);
    map[k2]=(double)d*c4/(c+d);
    map[kvirtual2]=(double)c*c4/(c+d);

    for(unsigned int i=0; i<(c1%2);i++){
      double random_variable = std::rand()/(double)RAND_MAX;
      if(random_variable>=0 && random_variable<0.5) map[k]++;
      if(random_variable>=0.5 && random_variable<=1) map[kvirtual]++;
    }
    for(unsigned int i=0; i<(c4%2);i++){
      double random_variable = std::rand()/(double)RAND_MAX;
      if(random_variable>=0 && random_variable<0.5) map[kvirtual2]++;
      if(random_variable>=0.5 && random_variable<=1) map[k2]++;
    }

    if(map[k2]<0 || map[kvirtual2]<0 ||
       map[k]<0 || map[kvirtual]<0){
      //divide then
      map[k]=c1;//reset value
      map[k2]=c4;//reset value
      FillGapsBetweenChipDivide(map,k2,kvirtual2);
      FillGapsBetweenChipDivide(map,k,kvirtual);
    }
  }//else
  //}
  //}
}

void FillGapsBetweenChipMask(uint* map, int k, int kvirtual, 
			     int kvirtual2, int k2)	
{
  map[k]=0;
  map[kvirtual]=0;
  map[kvirtual2]=0;
  map[k2]=0;
}

 

int local_exit(int status) {
  exit(status);
  return status;    /* to avoid warning messages */
}

//32 bit
std::vector <unsigned int> SetRateCorrectionTau(double tau, double sub_expure_time) //tau in sec
{
  vector <unsigned int> returnv;
  unsigned int rate_correction_table[1024];
  double meas[16384];
  double ratemax=-1;
  
  unsigned int np = 16384; //max slope 16 * 1024 -> 14 bit
  double b0[1024];
  double m[1024];
  
  if(tau<0||sub_expure_time<0)
    {
      printf("Error tau %f and sub_expure_time %f must be greater than 0.\n", tau, sub_expure_time);
      return returnv;
    }
  cout<<"Calculating table for tau of "<<tau*1e9<<" ns."<<endl;
  
  // Basic rate correction table
  for(unsigned int i=0;i<np;i++) meas[i]=0;
 
	for(unsigned int i=0;i<np;i++){
	  meas[i]  = i*exp(-i/sub_expure_time*tau);
	  if(meas[i]> ratemax) ratemax=meas[i];
	}

		//	cout<<i<<" meas "<<meas[i] <<endl;
		//if(meas[i]>4095) {
		//np=i;// limit max even before
		  //cout<<"l"<<endl;
		  //break;
		  //cout<<"max limited by 12 bit counter"<<endl; 		
		//}
		//if(i>0 && meas[i]<meas[i-1]){
		// np=i-1;
		  //cout<<"ll"<<endl;
		  //break;
		  //cout<<"max limited by maximum counter"<<endl; 		
		//}


	// b  :  index/address of block ram/rate correction table
	// b0 :  base in vhdl
	// m  :  slope in vhdl
	//
	// Firmware:
	//    data_in(11..2) -> memory address  --> memory
	//    data_in( 1..0) -> lsb
	//
	//    mem_data_out(13.. 0) -> base
	//    mem_data_out(17..14) -> slope
	//
	//    delta = slope*lsb
	//    corr  = base+delta

	unsigned int next_i=0;
	double beforemax;
	b0[0] = 0;
	m[0]  = 1;

	//initialize as max
	//for(int i=0; i<1024; i++) 
	//rate_correction_table[i]  =  (((int)(15+0.5)&0xf)<<14) | ((int)(max+0.5)&0x3fff); //65535;
	
	rate_correction_table[0]  = (((int)(m[0]+0.5)&0xf)<<14) | ((int)(b0[0]+0.5)&0x3fff);

	for(int b=1;b<1024;b++)
	{
	  if(m[b-1]<15)
	    {
	      double s=0,sx=0,sy=0,sxx=0,sxy=0;
	      for(;;next_i++)
		{
		  //give some space to stats
		  if(next_i>=np)
		    {
		      printf("Error bin problem ???????\n");
		      //not jump anymore but increase smootly

		      for(; b<1024; b++){
			//cout<<b<<endl;
			//b=1024;
			b0[b] = std::max( beforemax, ratemax);//16383;
			m[b]  = 15; 
			rate_correction_table[b]  = (((int)(m[b]+0.5)&0xf)<<14) | ((int)(b0[b]+0.5)&0x3fff);
		      }
		      b=1024;
		      break;
		    }
		  
		  double x    = meas[next_i] - b*4;
		  double y    = next_i;
		  //	cout<<b<<"    "<<next_i<<"   "<<meas[next_i]<<"    "<< np<<"   "<<np/16<<endl;
		  //printf("Start Loop  x: %f,\t y: %f,\t  s: %f,\t  sx: %f,\t  sy: %f,\t  sxx: %f,\t  sxy: %f,\t  next_i: %d,\t  b: %d,\t  meas[next_i]: %f\n", x, y, s, sx, sy, sxx, sxy, next_i, b, meas[next_i]);
		  
		  if(x < -0.5) continue;
		  if(x >  3.5) break;
		  s   += 1;
		  sx  += x;
		  sy  += y;
		  sxx += x*x;
		  sxy += x*y;
		  //printf("End   Loop  x: %f,\t y: %f,\t  s: %f,\t  sx: %f,\t  sy: %f,\t  sxx: %f,\t  sxy: %f,\t  next_i: %d,\t  b: %d,\t  meas[next_i]: %f\n", x, y, s, sx, sy, sxx, sxy, next_i, b, meas[next_i]);
		}
	      double delta = s*sxx - sx*sx;
	      b0[b] = (sxx*sy - sx*sxy)/delta;
	      m[b]  = (s*sxy  - sx*sy) /delta;
	      beforemax= b0[b];
 
	      if(m[b]<0 || m[b]>15){
		m[b]=15;
		b0[b]=std::max( beforemax, ratemax);//16383;
	      }

	      //	printf("After Loop  s: %f,\t  sx: %f,\t  sy: %f,\t  sxx: %f,\t  sxy: %f,\t  next_i: %d,\t  b: %d,\t  meas[next_i]: %f\n", s, sx, sy, sxx, sxy, next_i, b, meas[next_i]);
	      //	cout<<s<<"   "<<sx<<"   "<<sy<<"   "<<sxx<<"   "<<"   "<<sxy<<"   "<<delta<<"   "<<m[b]<<"    "<<b0[b]<<endl;
	    }else
	    {
	      b0[b] = std::max( beforemax,  ratemax);//(int)(max+0.5);//16383;
	      m[b]  = 15;
	    }
	  rate_correction_table[b]  = (((int)(m[b]+0.5)&0xf)<<14) | ((int)(b0[b]+0.5)&0x3fff);
	  //cout<<b<<"    "<<next_i<<"   "<<meas[next_i]<<"    "<< m[b]<<"   "<<b0[b]<<endl;
	  returnv.push_back(rate_corr_table[b]);
	}//b
	delete rate_corr_table[];	
	return returnv;
}

#if 0
unsigned int CorrectedValue(unsigned int i)
{
  int delta, slope, base, lsb;
  unsigned int corr;
	lsb   = i&3; //here is every how many is done
	base  = rate_correction_table[i>>2] & 0x3fff; //i>>2=i/4
	slope = ((rate_correction_table[i>>2] & 0x3c000) >> 14);
	
	//original here
	//	if(base==16383 && slope==15) return max;
	if(slope==15) return 3*15+base;//1000/*16383*/;
 
	delta = slope*lsb;
	corr  = delta+base;

	//	if(i==40 || i==80 || i==4000)
	//printf("Readout Input: %d,\tBase:%d,\tSlope:%d,\tLSB:%d,\tDelta:%d\tResult:%d\tReal:%f\n", i, base, slope, lsb, delta, corr, meas[i]); 

	//if(corr>i-4) 
	return corr;
}
#endif

#endif
