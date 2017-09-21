#include <iostream>
#include <string.h>
#include <fstream>

using namespace std;

const int NumHalfModules = 2;
const int NumChanPerChip_x = 256;
const int NumChanPerChip_y = 256;
const int NumChip_x = 4;
const int NumChip_y = 2;
//single module geometry
const int npix_x_sm=(NumChanPerChip_x*NumChip_x);
const int npix_y_sm=(NumChanPerChip_y*NumChip_y);
//Gap pixels
const int GapPixelsBetweenChips_x =2;
const int GapPixelsBetweenChips_y =2;
const int GapPixelsBetweenModules_x =8;
const int GapPixelsBetweenModules_y = 36;

int getFileParameters(string file,  int &dr, int &tg, int &is, int &x, int &y,
		      string& timestamp, double& expTime, double& period){

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
       sstr >> str >> str >> str >> dummyint;
       cout<<"total frames:"<<dummyint<<endl;
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
   		
    infile.close();
  }else{
    cprintf(RED, "Error: Could not read file: %s\n", file.c_str());
    return slsReceiverDefs::FAIL;
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
  
      return slsReceiverDefs::OK;
}

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
