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
//#include <omp.h>
#include <cassert>	
#include <algorithm> 
#include <sys/time.h>
#include <inttypes.h>

#include "image.h"

#define MYCBF //choose 
//#define MYROOT //choose 
//#define HDF5f
//#define MSHeader

#ifdef HDF5f
//#include "hdf5.h"
#include "H5Cpp.h"
#endif

#ifdef MYCBF
#include "cbf.h" 
#endif

#ifdef MYROOT
#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"
#endif

using namespace std;
//using namespace H5;

#ifdef MYCBF
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
#endif
 
#ifdef MYROOT
void FillROOT(TH2F* hmap,  int longedge_x, int x_t, int y_t, int k_t)		 
{
  if(longedge_x) 
    hmap->SetBinContent(x_t+1, y_t+1, k_t);
  else
    hmap->SetBinContent(y_t+1, x_t+1, k_t);
}

void FillROOTEdge(TH2F* hmap,  int longedge_x, int x_t, int y_t, int k_t,
		  int  xvirtual1, int yvirtual1, int kvirtual1)
{
  FillROOT(hmap,longedge_x, x_t, y_t,k_t);
  FillROOT(hmap,longedge_x, xvirtual1, yvirtual1, kvirtual1);
}

void FillROOTCorner(TH2F* hmap,  int longedge_x, int x_t, int y_t, int k_t,
		    int  xvirtual1, int yvirtual1, int kvirtual1,
		    int  xvirtual2, int yvirtual2, int kvirtual2,
		    int  xvirtual3, int yvirtual3, int kvirtual3)
{
  FillROOT(hmap,longedge_x, x_t, y_t,k_t);
  FillROOT(hmap,longedge_x, xvirtual1, yvirtual1, kvirtual1);
  FillROOT(hmap,longedge_x, xvirtual2, yvirtual2, kvirtual2);
  FillROOT(hmap,longedge_x, xvirtual3, yvirtual3, kvirtual3);
}
#endif

int main(int argc, char *argv[]) {

  //double tdif=0;
  //user set geometry
  int npix_x_user= npix_x_sm;
  int npix_y_user= npix_y_sm;
  
  //get command line arguments
  string file, datasetname;
  int fileIndex, fileFrameIndex=0,startdet=0;
  int longedge_x;
  int fillgaps;
  bool isFileFrameIndex = false;
  getCommandParameters(argc, argv, file, fileIndex, isFileFrameIndex, 
		       fileFrameIndex, npix_x_user, npix_y_user, 
		       longedge_x,fillgaps,datasetname,startdet);

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
  int* mapr=new int[npix_x_g*npix_y_g];

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
		       ypix, timestamp, expTime, period,imgs ) != 1) return -1;
 
  vector <int*> buffer;
  buffer.reserve(n_v *n_h*2*2);
  FILE *out;
  //nr high again
  int numFrames = fileFrameIndex+1 ;

  // open all files at once
  //loop on each receiver to get frame buffer
  for(int inr=0; inr<nr; inr++){
    sprintf(fname, "%s_d%d%s_%d.raw",file.c_str(),inr,frames,fileIndex);
    
    //open file
    if(!infile[inr].is_open())
      infile[inr].open(fname,ios::in | ios::binary);
  }//loop on receivers
  
#ifdef MYROOT
  //now open a single root file
  TFile* ofile = new TFile(TString::Format("%s-det%d%s_%d.root",file.c_str(),
					   nr,frames,fileIndex).Data(),
			   "RECREATE");
#endif  //If ROOT

  //now loop over all frames
  //for each frame
  
  int Nimgsperfile=2000; //to be fixed in next realease when configurable
  int Nimagesexpected=Nimgsperfile+numFrames; //assumes 2000 more tahn number   
  if(imgs<Nimagesexpected)  Nimagesexpected=imgs+1;
  cout<< "last image expected for this file is "<<Nimagesexpected-1<<endl;


#ifdef HDF5f
  int  rank=3;//3dim imgs
  /* HDF-5 handles */
  hid_t fid, fapl, gid, atts, atttype, attid;
  hid_t datatype, dataspace, dataspaceimg, vspace,dataprop, dataset;
  hsize_t dim[3]={Nimagesexpected-1,((longedge_x==1) ? npix_y_g : npix_x_g) ,
		  ((longedge_x==1) ? npix_x_g : npix_y_g)};
  hsize_t dim2[2]={((longedge_x==1) ? npix_y_g : npix_x_g) ,
		   ((longedge_x==1) ? npix_x_g : npix_y_g)};
  hsize_t maxdim[3]={H5S_UNLIMITED,((longedge_x==1) ? npix_y_g : npix_x_g) ,
		     ((longedge_x==1) ? npix_x_g : npix_y_g)};
  /* Hyperslab parameters */
  hsize_t start[3], count[3];
  hsize_t stride[3]={1,1,1};
  hsize_t block[3]={1,1,1}; 
 
  /*
   * open the file. The file attribute forces normal file 
   * closing behaviour down HDF-5's throat
   */
  fapl = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fclose_degree(fapl,H5F_CLOSE_STRONG);
  sprintf(fname, "%s_%05d.hdf5",file.c_str(),fileIndex);
  fid = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT,fapl);  
  H5Pclose(fapl);
  /*
   * create scan:NXentry
   */
  gid = H5Gcreate(fid,"entry",H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  /*
   * store the NX_class attribute. Notice that you
   * have to take care to close those hids after use
   */
  //  atts = H5Screate(H5S_SCALAR);
  //atttype = H5Tcopy(H5T_C_S1);
  //H5Tset_size(atttype, 7);//H5T_VARIABLE);
  //attid = H5Acreate(gid,"NX_class", atttype,atts, H5P_DEFAULT,H5P_DEFAULT);
  //H5Awrite(attid, atttype, (char *)"NXentry");
  //H5Sclose(atts);
  //H5Tclose(atttype);
  //H5Aclose(attid);
  /*
   * same thing for data:Nxdata in scan:NXentry.
   * A subroutine would be nice to have here.......
   */
    
  hid_t dcpl = H5Pcreate (H5P_DATASET_CREATE);
  gid = H5Gcreate(fid,"entry/data",H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  atts = H5Screate(H5S_SCALAR);
  atttype = H5Tcopy(H5T_C_S1);
  H5Tset_size(atttype, 7);//H5T_VARIABLE);
  attid = H5Acreate(gid,"NX_class", atttype, atts, H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attid, atttype, (char *)"NXdata");
  H5Sclose(atts);
  H5Tclose(atttype);
  H5Aclose(attid);
  //general (all images)
  /* Initialize hyperslab values */  
  dataspace = H5Screate_simple(rank,dim, maxdim); //here
  
  //here I don't know how to change it
  //i believe datatype depends on dr
  if(dynamicrange==4) datatype = H5Tcopy(H5T_STD_U8LE);  
  if(dynamicrange==8) datatype = H5Tcopy(H5T_STD_U8LE);  
  if(dynamicrange==16) datatype = H5Tcopy(H5T_STD_U16LE);  
  if(dynamicrange==32) datatype = H5Tcopy(H5T_STD_U32LE);  
  
  dataprop = H5Pcreate(H5P_DATASET_CREATE);

  /* Dataset must be chunked for compression */  
  hsize_t cdims[3]={1,dim[1],dim[2]};
  H5Pset_chunk (dataprop , rank, cdims);
  
  //do not remove here
  //SZIP Compression
  //unsigned szip_options_mask;
  //unsigned szip_pixels_per_block;
  //szip_options_mask = H5_SZIP_NN_OPTION_MASK;
  //szip_pixels_per_block = 16;
  //H5Pset_szip (dataprop, szip_options_mask, szip_pixels_per_block);
 
  //fill value
  int          fill_value =0;            /* Fill value for VDS */
  H5Pset_fill_value(dataprop,datatype, &fill_value);

  // Set ZLIB / DEFLATE Compression using compression level 2
   H5Pset_shuffle(dataprop); 
   H5Pset_deflate (dataprop, 1);//4 originale

  dataset = H5Dcreate(gid,datasetname.c_str(), datatype,dataspace,
		      H5P_DEFAULT, dataprop, H5P_DEFAULT);

  //one for all and overwritten
  int map2d[(longedge_x ? npix_y_g : npix_x_g)][(longedge_x ? npix_x_g : npix_y_g)];
  for(int iy=0; iy<(longedge_x ? npix_y_g : npix_x_g) ; iy++){
    for(int ix=0; ix<(longedge_x ? npix_x_g : npix_y_g) ; ix++){ 
      if(dynamicrange==4 ) map2d[iy][ix]=15; //change saturation
      if(dynamicrange==8 ) map2d[iy][ix]=255;
      if(dynamicrange==16 ) map2d[iy][ix]=4095;
      if(dynamicrange==32 ) map2d[iy][ix]=(pow(2,32)-1);
    }
  }

  //now create attributes
  //Count_cutoff 
  atts = H5Screate(H5S_SCALAR);
  atttype = H5Tcopy(H5T_STD_I64LE);
  int value=199998;
  H5Tset_size(atttype,8);
  attid = H5Acreate(dataset,"Count_cutoff", atttype,atts, 
		    H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attid, atttype,&value);
  H5Sclose(atts);
  H5Tclose(atttype);
  H5Aclose(attid);
  
  //Detector

  //Exposure_period
  atts = H5Screate(H5S_SCALAR);
  atttype = H5Tcopy(H5T_IEEE_F64LE);
  H5Tset_size(atttype,8);
  attid = H5Acreate(dataset,"Exposure_period", atttype,atts, 
		    H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attid, atttype,&period);
  H5Sclose(atts);
  H5Tclose(atttype);
  H5Aclose(attid);

  //Exposure_time
  atts = H5Screate(H5S_SCALAR);
  atttype = H5Tcopy(H5T_IEEE_F64LE);
  H5Tset_size(atttype,8);
  attid = H5Acreate(dataset,"Exposure_time", atttype,atts, 
		    H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attid, atttype,&expTime);
  H5Sclose(atts);
  H5Tclose(atttype);
  H5Aclose(attid);

  //Gain_setting
  //Image_path
  //Pixel_size
  double valued2d[2]={75e-6,75e-6};
  hsize_t d[2]={1,1};
  atts = H5Screate_simple(2, d, NULL);
  atttype = H5Tcopy(H5T_IEEE_F64LE);
  H5Tset_size(atttype,8);
  attid = H5Acreate(dataset,"Pixel_size", atttype,atts, 
		    H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attid, atttype,&valued2d);
  H5Sclose(atts);
  H5Tclose(atttype);
  H5Aclose(attid);
  //Silicon
  double valued=320e-6;
  atts = H5Screate(H5S_SCALAR);
  atttype = H5Tcopy(H5T_IEEE_F64LE);
  H5Tset_size(atttype,8);
  attid = H5Acreate(dataset,"Silicon", atttype,atts, 
		    H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attid, atttype,&valued);
  H5Sclose(atts);
  H5Tclose(atttype);
  H5Aclose(attid);
  //Tau
  valued=0;
  atts = H5Screate(H5S_SCALAR);
  atttype = H5Tcopy(H5T_IEEE_F64LE);
  H5Tset_size(atttype,8);
  attid = H5Acreate(dataset,"Tau", atttype,atts, 
		    H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attid, atttype,&valued);
  H5Sclose(atts);
  H5Tclose(atttype);
  H5Aclose(attid);
  //Threshold_setting
  //Trim_file

#endif  //HDF5f
    
  //now reause the same all the times
  int* intbuffer = new int[imageSize/sizeof(int)];
  int* bufferheader=new int[imageHeader/sizeof(int)];
  
  while(numFrames< Nimagesexpected){
    //Create cbf files with data
#ifdef MYCBF
    cbf_handle cbf;
#endif
#ifdef MYROOT
    //check horizontal or vertical
    TH2F* hmap= new TH2F(TString::Format("hmap%d",numFrames).Data(),
			 TString::Format("hmap%d",numFrames).Data(),
			 ( longedge_x ? npix_x_g : npix_y_g), 0, 
			 ( longedge_x ? npix_x_g : npix_y_g), 
			 ( longedge_x ? npix_y_g : npix_x_g), 0,
			 ( longedge_x ? npix_y_g : npix_x_g));
    
#endif  //If ROOT
    
    //here nr is not volatile anymore
    //loop on each receiver to get frame buffer
    
    for(int inr=0; inr<nr; inr++){
      int* dataout = new int [ xpix* ypix]; //will delete it in buffer
      //read data
      if(infile[inr].read((char*)bufferheader,imageHeader)){
	fnum = (*((uint64_t*)(char*)bufferheader));
      }
      if(!CheckFrames(fnum,numFrames)) continue; 	 
      infile[inr].read((char*)intbuffer,imageSize);
      decodeData(intbuffer, dataout, imageSize, xpix, ypix, 
		 dynamicrange);  
      
      buffer.push_back(dataout);
    }//loop on receivers
  
    
    if(buffer.size()!=nr) continue;
    
    //get a 2d map of the image
    //initialize
    for(int ik=0; ik<npix_y_g*npix_x_g;++ik){
      if(dynamicrange==4 ) map[ik]=15; //change saturation
      if(dynamicrange==8 ) map[ik]=255;
      if(dynamicrange==16 ) map[ik]=4095;
      if(dynamicrange==32 ) map[ik]=(pow(2,32)-1);
    }
	
    int startchipx=0;
    int startchipy=0;
    int endchipx=4;
    int endchipy=1;
    
    //omp_set_dynamic(0);     // Explicitly disable dynamic teams
    //omp_set_num_threads(nr); //set n receivers

    //can be changed in a loop over receivers 
    int nnr=0;
    struct  timeval tss,tsss; //for timing
    //   gettimeofday(&tss,NULL);

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
		      int x_t= GetX(ix, ichipx, imod_h);
		      int y_t= GetY(iy, ichipy,imod_v);
		      int k=GetK(x_t,y_t,npix_x_g);
		      map[k]=buffer[nnr][ix+(ichipx%2)*NumChanPerChip_x+ NumChanPerChip_x*NumChip_x_port*iy];
		    }//num ch chipx 
		  }//ichipx
		} //num ch chip y
	      }//ichipy
	    } //it ==0 		
	    
	    //getting values for bottom
	    if(it==1) {
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
		      int x_t=GetX(ix, ichipx, imod_h);
		      int y_t= GetY(iy,ichipy,imod_v);
		      int k=GetK(x_t,y_t,npix_x_g);
		      map[k]=buffer[nnr][ix+(ichipx%2)*NumChanPerChip_x+ NumChanPerChip_x*NumChip_x_port*(NumChanPerChip_y-1-iy)];
		    }
		  }
		}
	      }
	    } //it==1
	    nnr++;
	  }//ileft
	} //it

	//interpolation easier at the end of the module map(thanks chage later even divide here )
	//corner gap pixels gap pixels
	
	//gettimeofday(&tsss,NULL);
	//tdif+=(1e6*(tsss.tv_sec - tss.tv_sec)+(long)(tsss.tv_usec)-(long)(tss.tv_usec));
	//tss=tsss;
       
	//fillgaps
	//edge
	int ix=NumChanPerChip_x-1;
	int kdebug;
	//start from end pixel of the chip right 
	for( int ichipx=0; ichipx<3; ichipx++){
	  for( int ichipy=0; ichipy<2; ichipy++){
	    for(int iy=0; iy<NumChanPerChip_y;iy++){ 
	      //exclude border y
	      if(ichipy==1 && iy==0) {
		//first corner
		int x_t= GetX(ix, ichipx, imod_h);
		int y_t= GetY(iy, ichipy,imod_v);
		int k=GetK(x_t,y_t,npix_x_g);

		//right
		int xvirtual1=x_t+1;
		int yvirtual1=y_t;
		int kvirtual1=GetK(xvirtual1,yvirtual1,npix_x_g);
		//bottom
		int xvirtual2=x_t;
		int yvirtual2=y_t-1;
		int kvirtual2=GetK(xvirtual2, yvirtual2,npix_x_g);
		//bottom right
		int xvirtual3=x_t+1;
		int yvirtual3=y_t-1;
		int kvirtual3=GetK(xvirtual3,yvirtual3,npix_x_g);
		if(fillgaps==kZero) FillCornerGapsBetweenChipZero(map, k, kvirtual1,kvirtual2, kvirtual3 );
		if(fillgaps==kDivide) FillCornerGapsBetweenChipDivide(map, k, kvirtual1,kvirtual2, kvirtual3,dynamicrange);	
		if(fillgaps==kMask) FillGapsBetweenChipMask(map, k, kvirtual1,kvirtual2, kvirtual3 );	

		//second corner
		int x2_t= x_t+3;
		int y2_t= y_t;
		int k2=GetK(x2_t,y2_t,npix_x_g);
		//left
		int x2virtual1= x2_t-1;
		int y2virtual1= y2_t;
		int k2virtual1=GetK(x2virtual1,y2virtual1,npix_x_g);
		//bottom
		int x2virtual2= x2_t;
		int y2virtual2= y2_t-1;
		int k2virtual2=GetK(x2virtual2,y2virtual2,npix_x_g);
		//bottom left
		int x2virtual3= x2_t-1;
		int y2virtual3= y2_t-1;
		int k2virtual3=GetK(x2virtual3,y2virtual3,npix_x_g);
		if(fillgaps==kZero) FillCornerGapsBetweenChipZero(map, k2, k2virtual1,k2virtual2, k2virtual3 );	
		if(fillgaps==kDivide) FillCornerGapsBetweenChipDivide(map, k2, k2virtual1,k2virtual2, k2virtual3,dynamicrange );	
		if(fillgaps==kMask) FillGapsBetweenChipMask(map,k2, k2virtual1,k2virtual2, k2virtual3 );	

		//third corner
		int x3_t= x_t;
		int y3_t= y_t-3;
		int k3=GetK(x3_t,y3_t,npix_x_g);
		//right
		int x3virtual1=x3_t+1;
		int y3virtual1=y3_t;
		int k3virtual1=GetK(x3virtual1,y3virtual1,npix_x_g);
		//top
		int x3virtual2=x3_t;
		int y3virtual2=y3_t+1;
		int k3virtual2=GetK(x3virtual2, y3virtual2,npix_x_g);
		//top right
		int x3virtual3=x3_t+1;
		int y3virtual3=y3_t+1;
		int k3virtual3=GetK(x3virtual3,y3virtual3,npix_x_g);
		if(fillgaps==kZero) FillCornerGapsBetweenChipZero(map, k3, k3virtual1,k3virtual2, k3virtual3 );	
		if(fillgaps==kDivide) FillCornerGapsBetweenChipDivide(map, k3, k3virtual1,k3virtual2, k3virtual3,dynamicrange);	
		if(fillgaps==kMask) FillGapsBetweenChipMask(map,k3, k3virtual1,k3virtual2, k3virtual3 );

		//fourth
		int x4_t= x_t+3;
		int y4_t= y_t-3;
		int k4=GetK(x4_t,y4_t,npix_x_g);
		//left
		int x4virtual1=x4_t-1;
		int y4virtual1=y4_t;
		int k4virtual1=GetK(x4virtual1,y4virtual1,npix_x_g);
		//top
		int x4virtual2=x4_t;
		int y4virtual2=y4_t+1;
		int k4virtual2=GetK(x4virtual2, y4virtual2,npix_x_g);
		//top left
		int x4virtual3=x4_t-1;
		int y4virtual3=y4_t+1;
		int k4virtual3=GetK(x4virtual3,y4virtual3, npix_x_g);
		if(fillgaps==kZero) FillCornerGapsBetweenChipZero(map, k4, k4virtual1,k4virtual2, k4virtual3 );	
		if(fillgaps==kDivide) FillCornerGapsBetweenChipDivide(map, k4, k4virtual1,k4virtual2, k4virtual3,dynamicrange );	
		if(fillgaps==kMask) FillGapsBetweenChipMask(map,k4, k4virtual1,k4virtual2, k4virtual3 );

		//here do interpolation for corners
		if(fillgaps==kInterpolate || fillgaps==kInterpolate2 ){

		  bool saturated=Saturated(map[k],dynamicrange);
		  bool saturated2=Saturated(map[k2],dynamicrange);
		  bool saturated3=Saturated(map[k3],dynamicrange);
		  bool saturated4=Saturated(map[k4],dynamicrange);

		  if(saturated==false && saturated2==false && saturated3==false && saturated4==false){
		    int koriginal=map[k];
		    int k2original=map[k2];
		    int k3original=map[k3];
		    int k4original=map[k4];
		    //vertical left
		    map[k]=Divide(map[k],2);		    
		    map[k2]=Divide(map[k2],2);		    
		    map[k3]=Divide(map[k3],2);		    
		    map[k4]=Divide(map[k4],2);		    
		    FillGapsBetweenChipInterpolate(map,k,kvirtual2,k3virtual2,k3,dynamicrange);
		    FillGapsBetweenChipInterpolate(map,k2,k2virtual2,k4virtual2,k4,dynamicrange);
		    map[k]= Divide(koriginal,2);
		    map[k2]= Divide(k2original,2);
		    map[k3]= Divide(k3original,2);
		    map[k4]= Divide(k4original,2);
		    //horizontal
		    FillGapsBetweenChipInterpolate(map,k,kvirtual1,k2virtual1,k2,dynamicrange);
		    FillGapsBetweenChipInterpolate(map,k3,k3virtual1,k4virtual1,k4,dynamicrange);
		    map[k]= Divide(koriginal,2);
		    map[k2]= Divide(k2original,2);
		    map[k3]= Divide(k3original,2);
		    map[k4]= Divide(k4original,2);
		    //diagonal
		    FillGapsBetweenChipInterpolate(map,k,kvirtual3,k4virtual3,k4,dynamicrange);
		    FillGapsBetweenChipInterpolate(map,k3,k3virtual3,k2virtual3,k2,dynamicrange);
		  }//saturated
		  else{
		    FillCornerGapsBetweenChipDivide(map, k, kvirtual1,kvirtual2, kvirtual3,dynamicrange );
		    FillCornerGapsBetweenChipDivide(map, k2, k2virtual1,k2virtual2, k2virtual3,dynamicrange );
		    FillCornerGapsBetweenChipDivide(map, k3, k3virtual1,k3virtual2, k3virtual3,dynamicrange );
		    FillCornerGapsBetweenChipDivide(map, k4, k4virtual1,k4virtual2, k4virtual3,dynamicrange );
		  }		  
		      
		}//kinterpolate

	      }else{
		if(ichipy==0 && iy==NumChanPerChip_y-1) continue; //already taken care
		else{
		  int x_t= GetX(ix, ichipx, imod_h);
		  int y_t= GetY(iy, ichipy,imod_v);
		  int k=GetK(x_t,y_t, npix_x_g);
		  //first vitual right
		  int xvirtual= x_t+1;
		  int yvirtual= y_t;
		  int kvirtual=GetK(xvirtual,yvirtual,npix_x_g);
		    
		  //now find the matching pair on the other side
		  //virtual 
		  int xvirtual2= x_t+2;
		  int yvirtual2= y_t;
		  int kvirtual2=GetK(xvirtual2,yvirtual2,npix_x_g);
		  //real
		  int x_t2= x_t+3;
		  int y_t2= y_t;
		  int k2=GetK(x_t2,y_t2,npix_x_g);
		    
		  if(fillgaps==kZero) FillGapsBetweenChipZero(map,k,kvirtual,kvirtual2,k2);
		  if(fillgaps==kDivide) FillGapsBetweenChipDivide(map,k,kvirtual,kvirtual2,k2,dynamicrange);			    
		  if(fillgaps==kInterpolate) FillGapsBetweenChipInterpolate(map,k,kvirtual,kvirtual2,k2,dynamicrange);	
		  if(fillgaps==kMask) FillGapsBetweenChipMask(map,k,kvirtual,kvirtual2,k2);	
		  if(fillgaps==kInterpolate2) FillGapsBetweenChipInterpolate2(map,GetK(x_t-1,y_t, npix_x_g),k,kvirtual,
									      kvirtual2,k2,GetK(x_t2+1,y_t2,npix_x_g),dynamicrange);	
		}//edge
	      }//other corner
	    }//loop on y
	  } //loop on chip y
	} //loop on chip x

	  //other edge
	int iy=NumChanPerChip_y-1;
	int ichipy=0;//bottom
	for(int ichipx=0; ichipx<NumChip_x;ichipx++){
	  for(int ix=0; ix<NumChanPerChip_x;ix++){ 
	    //exclude border x
	    if(ichipx!=(NumChip_x-1) && ix==(NumChanPerChip_x-1)) continue;
	    if(ichipx!=0 && ix==0) continue;
	    int x_t= GetX(ix, ichipx, imod_h);
	    int y_t= GetY(iy, ichipy,imod_v);
	    int k=GetK(x_t,y_t,npix_x_g);
	    int xvirtual= x_t;
	    int yvirtual= y_t+1;
	    int kvirtual=GetK(xvirtual,yvirtual,npix_x_g);
	    //now find the matching pair on the other side
	    //virtual
	    int xvirtual2= x_t;
	    int yvirtual2= y_t+2;
	    int kvirtual2=GetK(xvirtual2,yvirtual2, npix_x_g);
	    //real
	    int x_t2= x_t;
	    int y_t2= y_t+3;
	    int k2=GetK(x_t2,y_t2,npix_x_g);
	      
	    if(fillgaps==kZero) FillGapsBetweenChipZero(map,k,kvirtual,kvirtual2,k2);
	    if(fillgaps==kDivide) FillGapsBetweenChipDivide(map,k,kvirtual,kvirtual2,k2,dynamicrange);			    
	    if(fillgaps==kInterpolate) FillGapsBetweenChipInterpolate(map,k,kvirtual,kvirtual2,k2, dynamicrange);	
	    if(fillgaps==kMask) FillGapsBetweenChipMask(map,k,kvirtual,kvirtual2,k2);	
	    if(fillgaps==kInterpolate2) FillGapsBetweenChipInterpolate2(map,GetK(x_t,y_t-1,npix_x_g),k,kvirtual,
									kvirtual2,k2,GetK(x_t2,y_t2+1,npix_x_g), dynamicrange);	
	   
	  }//xchannels
	} //chips	
   		    
      }//v mods
    } //h mods close all loops

    //delete dataout;
    buffer.clear();

#ifdef MYCBF
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
    //timestamp
    time_t rawtime = time(NULL);
    struct tm *timeinfo = localtime(&rawtime);
    char date[100];
    strftime(date, sizeof(date), "%Y/%b/%d %H:%M:%S.%j %Z", timeinfo);
    sprintf(printDate,"# %s\r\n",date);
    
    fprintf(out,printDate);
    fprintf(out,
	    "# Exposure_time %f s\r\n"
	    "# Exposure_period %f s\r\n"
	    "# Tau = 0 s\r\n",expTime,period);
    long int max=pow(2,dynamicrange)-1;
    if(dynamicrange==16) max=pow(2,12)-1;
    sprintf(limits, "# Count_cutoff %ld counts\r\n",max );
    fprintf(out,limits);
    fprintf(out,
	    "# Threshold_setting 8000 eV\r\n"  );
    
#ifdef MSHeader
    fprintf(out,
	    "# Gain_setting: low gain (vrf = -0.300)"
	    "\r\n");
    fprintf(out,
	    "# N_excluded_pixels = 1132"
	    "\r\n");
    fprintf(out,
	    "# Excluded_pixels: badpix_mask.tif"
	    "\r\n");
    fprintf(out,
	    "# Flat_field: FF_p2m0111_E26000_T13000_vrf_m0p30.tif"
	    "\r\n");
    fprintf(out,
	    "# Trim_file: p2m0111_E26000_T13000_vrf_m0p30.bin"
	    "\r\n");
    fprintf(out,
	    "# Image_path: /ramdisk/"
	    "\r\n");
    fprintf(out,
	    "# Wavelength 0.7 A"
	    "\r\n");
    fprintf(out,
	    "# Start_angle -89.50 deg."
	    "\r\n");
    fprintf(out,
	    "# Angle_increment 0.50 deg."
	    "\r\n");
    fprintf(out,
	    "# Omega 57.05 deg."
	    "\r\n");
    fprintf(out,
	    "# Omega_increment 0.00 deg."
	    "\r\n");
    fprintf(out,
	    "# Phi 57.55 deg."
	    "\r\n");
    fprintf(out,
	    "# Phi_increment 0.50 deg."
	    "\r\n");
    fprintf(out,
	    "# Kappa -134.76 deg."
	    "\r\n");
    fprintf(out,
	    "# Oscillation_axis OMEGA CW"
	    "\r\n");
    fprintf(out,
	    "# Detector_distance 0.26212 m"
	    "\r\n");
    fprintf(out,
	    "# Detector_Voffset 0.0 m"
	    "\r\n");
    fprintf(out,
	    "# Beam_xy (1197.16, 1249.34) pixels"
	    "\r\n");
    fprintf(out,
	    "# Flux 0.0 counts"
	    "\r\n");
#endif
    fprintf(out,
	    ";\r\n"
	    );

    //now rotate everything - so far inside the cbf map
    if(!longedge_x){
      //here test ro ratate on the output matrix
      for(int ix=0; ix<npix_x_g; ix++){
	for(int iy=0; iy<npix_y_g; iy++){
	  int kold=ix+ npix_x_g*iy;
	  int  knew=(npix_y_g-iy)+ npix_y_g*ix;
	  mapr[knew]=map[kold];
	}
      }
    } //short edege
   
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
						  ( longedge_x ? &(map[0]) : &(mapr[0])), 						//void *array
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
    
#endif  //If CBF
    
    
#ifdef MYROOT
    for(int ix=0; ix<npix_x_g; ix++){
      for(int iy=0; iy<npix_y_g; iy++){
	int kold=ix+ npix_x_g*iy;
	FillROOT(hmap,longedge_x, ix, iy, map[kold]);
      }
    }
    hmap->SetStats(kFALSE);
    hmap->Draw("colz");
    hmap->Write();
    delete hmap;
#endif  //If ROOT

#ifdef HDF5f
    /*
     * store the counts dataset
     */

    start[0] = numFrames-1;
    start[1] = 0;
    start[2]= 0;
    count[0]=1;
    count[1] =((longedge_x==1) ? npix_y_g : npix_x_g) ;
    count[2] =((longedge_x==1) ? npix_x_g : npix_y_g) ;
    dataspaceimg = H5Screate_simple(2, dim2, NULL);
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, stride, count, block);
    
    //now fill teh map2d - map is not rotated
    for(int ix=0; ix<npix_x_g; ix++){
      for(int iy=0; iy<npix_y_g; iy++){
	int k=ix+ npix_x_g*iy;
	//map is never the rotated ones
	if(longedge_x) 
	  map2d[iy][ix]=map[k];
	else
	  map2d[ix][iy]=map[k];
      }
    }
    H5Dwrite(dataset, datatype , dataspaceimg, dataspace, H5P_DEFAULT, map2d);
    H5Sclose(dataspaceimg);
    //atts = H5Screate(H5S_SCALAR);
    //atttype = H5Tcopy(H5T_C_S1);
    //H5Tset_size(atttype, H5T_VARIABLE);
    //attid = H5Acreate(gid,"NX_class", atttype, atts, H5P_DEFAULT,H5P_DEFAULT);
    //H5Awrite(attid, atttype, (char *)"NXdata");
    //H5Sclose(atts);
    //H5Tclose(atttype);
    //H5Aclose(attid);

   //check for memory leak
#endif  //HDF5f
    
    numFrames++;
    
    buffer.clear();
    for(int inr=0; inr<nr; inr++) {   
      delete buffer[inr]; //remove memory 
    }
    
  } //loop on frames

#ifdef HDF5f
  /*
   * close the file 
   */
  H5Tclose(datatype);
  H5Pclose(dataprop);  
  H5Dclose(dataset);
  H5Fclose(fid);
#endif
  
  delete[] bufferheader;
  delete[] intbuffer;
  
  delete[]  map;
  delete[]  mapr;
  
  for(int inr=0; inr<nr; inr++)
    infile[inr].close();
  

  //cout<<"only to read took "<<tdif/1e6<<endl;
  return 1;
}



