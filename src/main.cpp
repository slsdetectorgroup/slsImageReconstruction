/* Reads files to get values for images
 * main.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: maliakal_d
 */

#include "sls_receiver_defs.h"
#include "slsReceiverData.h"
#include "eigerHalfModuleData.h"
#include "ansi.h"

#include <iostream>
#include <string.h>
#include <fstream>
#include <map>
#include <getopt.h>
#include <stdlib.h>			

#include <cassert>	

#include <algorithm> 

#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"

using namespace std;

int getParameters(int argc, char *argv[], int &dr, string &fntop, string &fnbottom, int &tg);


int main(int argc, char *argv[]) {
  //declare variables
  slsReceiverData <uint32_t> *receiverdata = NULL;
  char* buffer = NULL;
  ifstream infile;
  string fname;
  int dynamicrange, tenGiga, bufferSize, dataSize, packetsPerFrame, numFrames, fnum;

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
  TH2F* hmap[Nframes];

  //get config parameters
  dynamicrange = -1;
  tenGiga = 0;
  //if(getParameters(argc, argv, dynamicrange,fnametop,fnamebottom, tenGiga) == slsReceiverDefs::OK){

    //if no dynamic range given

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
      //work out how many modules in vertical and how many in horizontal
      int n_v= npix_y_user/npix_y_sm;
      int n_h= npix_x_user/npix_x_sm;

      cout<<"geometery has "<<n_h<<" modules in horizontal and "<<n_v<<" modules in vertical"<<endl;

      string frames="";//"f000000000000";

      int nfile=startdet;
      //put master on top always
      for(int imod_v=(n_v-1); imod_v>-1; imod_v--){
	for(int imod_h=0; imod_h<n_h;imod_h++){
	
	  for( int it=0;it<2;it++){
	    fname=TString::Format("%s/%s_d%d%s_%d.raw",dir.c_str(),file.c_str(),/*(it+(imod_h*2)+n_h*imod_v)*/nfile,frames.c_str(),n).Data();
	    cout<<fname<<endl;
	    if(!fname.empty()){
	      infile.open(fname.c_str(),ios::in | ios::binary);
	      if (infile.is_open()) {
		if(infile.read(data,1024)){
		  dynamicrange = eigerHalfModuleData::getDynamicRange(data);
		}
		infile.close();
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
	  
	    //top
	    if(!fname.empty()){
	      //construct top datamapping object
	      numFrames = 0;
	      fnum = -1;
	      receiverdata = new eigerHalfModuleData(dynamicrange,packetsPerFrame,bufferSize, dataSize, it==0? true: false);
	      
	      //open file
	      infile.open(fname.c_str(),ios::in | ios::binary);
	      if(infile.is_open()){
		//get frame buffer
		while((buffer = receiverdata->readNextFrame(infile, fnum))){
	  
		  cout << "Reading top values for frame #" << fnum << endl;
 
		  //getting values //top
		  if(it==0){
		    if(imod_v==(n_v-1) && imod_h==0){
		      //top master is done first
		      hmap[fnum-1]= new TH2F(TString::Format("hmap%d",fnum-1).Data(),
					     TString::Format("hmap%d",fnum-1).Data(),
					     npix_x_user, 0, npix_x_user, npix_y_user, 0, npix_y_user);		 
		    }    
		    for(int iy=((npix_y_sm/2)+imod_v*npix_y_sm); iy<(npix_y_sm+imod_v*npix_y_sm); iy++)
		      for(int ix=0+imod_h*npix_x_sm; ix<npix_x_sm+imod_h*npix_x_sm; ix++)
			hmap[fnum-1]->SetBinContent(ix+1, iy+1, (receiverdata->getValue((char*)buffer,(ix-imod_h*npix_x_sm),(iy-(npix_y_sm/2)-imod_v*npix_y_sm),dynamicrange)));
		  }
		  if(it==1){
		    for(int iy=0+imod_v*npix_y_sm; iy<npix_y_sm/2+imod_v*npix_y_sm; iy++)
		      for(int ix=0+imod_h*npix_x_sm; ix<npix_x_sm+imod_h*npix_x_sm; ix++)
			hmap[fnum-1]->SetBinContent(ix+1, iy+1, (receiverdata->getValue((char*)buffer,
											(ix-imod_h*npix_x_sm),(iy-imod_v*npix_y_sm),dynamicrange)));
		  }
		  
		  delete [] buffer;
	  numFrames++;
	}
	//close file
	infile.close();
      }else cprintf(RED, "Error: Could not read file: %s\n", fname.c_str());
      delete receiverdata;

      cout  << "Found " << numFrames << " frames in "<<nfile/*it+(imod_h*2)+n_h*imod_v*/<<" file." << endl << endl;
      cout<<	it<< "  "<< imod_h <<"  "<<n_h <<"    "<<imod_v<<endl;    
	    }
	    nfile++;	  
	  }
	  
	} //hor
      }//vert
 
      TFile* ofile = new TFile(TString::Format("%s/%s_det%d-%d_%d.root",dir.c_str(),file.c_str(),startdet,nfile-1,n).Data(),"RECREATE");
    for(int fnum=0; fnum<numFrames; fnum++){
      if( hmap[fnum]){
	hmap[fnum]->SetStats(kFALSE);
	hmap[fnum]->Draw("colz");
	hmap[fnum]->Write();
      }
    }
    ofile->Write();
    delete ofile;
    cout << endl << "Goodbye!" << endl;
    return slsReceiverDefs::OK;
}


/**
 * Parse command line parameters to get dynamic range, ten giga enable and file names
 */
int getParameters(int argc, char *argv[], int &dr, string &fntop, string &fnbottom,int &tg){

  map<string, string> configuration_map;
  static struct option long_options[] = {
    {"dr",				required_argument,       0, 'd'},
    {"tengiga",    		required_argument,       0, 't'},
    {"top",     		required_argument,       0, 'p'},
    {"bottom",     		required_argument,       0, 'm'},
    {"help",  			no_argument,      		 0, 'h'},
    {0, 0, 0, 0}
  };
  int option_index = 0, c;
  while ( c != -1 ){
    c = getopt_long (argc, argv, "dtpmh", long_options, &option_index);
    if (c == -1)//end of options
      break;
    switch(c){
    case 'd':
      if(sscanf(optarg, "%d", &dr) <=0){
	cprintf(RED,"ERROR: Cannot parse dynamic range. Options: 4,8,16,32.\n");
	return slsReceiverDefs::FAIL;
      }
      //verification done later
      /*switch(dr){
	case 4:
	case 8:
	case 16:
	case 32: break;
	default:
	cprintf(RED,"ERROR: Invalid dynamic range. Options: 4,8,16,32.\n");
	return slsReceiverDefs::FAIL;
	}*/
      break;
    case 't':
      if(sscanf(optarg, "%d", &tg) <=0){
	cprintf(RED,"ERROR: Cannot parse ten giga parameter. Options: 0 or 1.\n");
	return slsReceiverDefs::FAIL;
      }
      switch(tg){
      case 0:
      case 1: break;
      default:
	cprintf(RED,"ERROR: Invalid ten giga parameter. Options: 0 or 1.\n");
	return slsReceiverDefs::FAIL;
      }
      break;
    case 'p':
      fntop = optarg;
      if(fntop.empty()){
	cprintf(RED,"ERROR: Empty top file name.\n");
	return slsReceiverDefs::FAIL;
      }
      break;
    case 'm':
      fnbottom = optarg;
      if(fnbottom.empty()){
	cprintf(RED,"ERROR: Empty bottom file name.\n");
	return slsReceiverDefs::FAIL;
      }
      break;
    case 'h':
      string help_message = """\nSLS Image Reconstruction\n\n""";
      help_message += """usage: image --p top_fname [--m bottom_fname] [--dr dynamic_range]\n\n""";
      help_message += """\t--dr:\t dynamic range or bit mode. Default:16. Options: 4,8,16,32.\n""";
      help_message += """\t--tengiga:\t 1 if 10Gbe or 0 for 1Gbe. Default: 0\n""";
      help_message += """\t--top:\t file name of top image\n""";
      help_message += """\t--bottom:\t file name of bottom image\n\n""";
      cout << help_message << endl;
      break;

    }
  }

  //cout << " dynamic range:"<< dr << " top file name:" << fntop << " bottom file name:" << fnbottom << " ten giga:"<< tg << endl;
  return slsReceiverDefs::OK;
}
