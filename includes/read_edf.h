#ifndef READ_EDF_H
#define READ_EDF_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <string.h>
#include <sstream>
using namespace std;

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;

unsigned int* decodeData(unsigned char *datain, const int size, const int nx, const int ny, int byteconvertfactor) 
{

  unsigned int* dataout = new unsigned int [nx*ny];  

  //for each byte (concatenate 2 bytes to get 16 bit value)
  for (int ichan=0; ichan<nx*ny; ++ichan) {
    if(byteconvertfactor==2){
      // sixteenBitNumber = (upperByte<<8) | lowerByte;
      dataout[ichan] = (datain[ichan*byteconvertfactor+1]<<8) | datain[ichan* byteconvertfactor];
    }
     if(byteconvertfactor==1){
       dataout[ichan] =datain[ichan];
     }
     if(byteconvertfactor==4){
       //unsigned int value = a | (b<<8) | (c<<16) | (d<<24);
       dataout[ichan]=(datain[ichan*byteconvertfactor+3]<<24) | (datain[ichan* byteconvertfactor+2]<<16) | 
	 (datain[ichan* byteconvertfactor+1]<<8)  | datain[ichan* byteconvertfactor] ; 
     }
 }
 return dataout;
}

void read_edf()
{
  char fname[1000]; 
  sprintf(fname, "/scratch/test_01_0015.edf");
  ifstream infile (fname);
  string line;
  string str;
  //open file
  if(!infile.is_open())
    infile.open(fname,ios::in | ios::binary);
  if(infile.is_open()){
    int nimg=0;

    while(!infile.eof()){
      nimg++;   
    //get first line
    getline( infile, line); //{
    getline(  infile, line); //H
    getline(  infile, line);//B
    //datatype
    getline(  infile, line);//D
    int byteconvertfactor;
    //1:1 =  8-bit/Byte
    //2:1 = 16-bit/Short
    //4:1 = 32-bit/Integer
    //uint64
  
    istringstream sstr(line);
    sstr   >> str >> str >> str;
    if(str.compare("UnsignedShort") ==0){
      byteconvertfactor=2;
    }
    if(str.compare("UnsignedByte") ==0){
      byteconvertfactor=1;
    }
    if(str.compare("UnsignedInteger") ==0){
      byteconvertfactor=4;
    }

    getline(  infile, line);//S
    istringstream sstr2(line);
    sstr2   >> str >> str >> str;
    const int datasize=atoi(str.c_str());
    getline(  infile, line); //Dim_1 = 1030 ;
    istringstream sstr3(line);
    sstr3   >> str >> str >> str;
    const int nx=atoi(str.c_str());
    getline(  infile, line);    //Dim_2 = 514 ;
    istringstream sstr4(line);
    sstr4   >> str >> str >> str;
    const int ny=atoi(str.c_str());
    
    while (getline(  infile, line, '}' )){
      break;
    }
    //read an extra byte ('\n')
    unsigned char* intbuffer = new unsigned char[datasize];   
    infile.read((char*)intbuffer,1);
    //concatenate 2
    //    unsigned short* obuffer = (unsigned short*)intbuffer;   
    unsigned int* obuffer = new unsigned int[nx*ny];
   
    //now start reading binaries   
    infile.read((char*)intbuffer,datasize);
    obuffer=decodeData(intbuffer, datasize, nx, ny, byteconvertfactor);
    //read an extra byte ('\n')
    infile.read((char*)intbuffer,1);
    
  }//endof the file 
    infile.close();
    cout<<"founf "<<nimg<<"   images"<<endl; 
 
  }//open
}

#endif
