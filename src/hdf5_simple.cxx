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
#include "image.h"

//#define MYCBF //choose 
//#define MYROOT //choose 
#define HDF5f
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
  string file;
  int fileIndex, fileFrameIndex=0,startdet=0;
  int longedge_x;
  int fillgaps;
  bool isFileFrameIndex = false;
  getCommandParameters(argc, argv, file, fileIndex, isFileFrameIndex, 
		       fileFrameIndex, npix_x_user, npix_y_user, 
		       longedge_x,fillgaps,startdet);

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
  
  
  int  ihd, rank, signal;
  /* HDF-5 handles */
  hid_t fid, fapl, gid, atts, atttype, attid;
  hid_t datatype, dataspace,dataspace2,dataspace3, dataprop, dataset, dataset2;
  rank = 3;
  hsize_t dim[3]={2,((longedge_x==1) ? npix_y_g : npix_x_g) ,
		  ((longedge_x==1) ? npix_x_g : npix_y_g)};
  hsize_t dim2[2]={((longedge_x==1) ? npix_y_g : npix_x_g) ,
		   ((longedge_x==1) ? npix_x_g : npix_y_g)};
  hsize_t start[3], count[3];
  hsize_t stride[3]={1,1,1};
  hsize_t block[3]={1,1,1};  
  /*
   * open the file. The file attribute forces normal file 
   * closing behaviour down HDF-5's throat
   */
  fapl = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fclose_degree(fapl,H5F_CLOSE_STRONG);
  fid = H5Fcreate("/scratch/testHDF5/myNXfile.h5", H5F_ACC_TRUNC, H5P_DEFAULT,fapl);  
  H5Pclose(fapl);
  /*
   * create scan:NXentry
   */
  gid = H5Gcreate(fid,"entry",H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  /*
   * store the NX_class attribute. Notice that you
   * have to take care to close those hids after use
   */
  atts = H5Screate(H5S_SCALAR);
  atttype = H5Tcopy(H5T_C_S1);
  H5Tset_size(atttype, 7);//H5T_VARIABLE);
  attid = H5Acreate(gid,"NX_class", atttype,atts, H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attid, atttype, (char *)"NXentry");
  H5Sclose(atts);
  H5Tclose(atttype);
  H5Aclose(attid);
  /*
   * same thing for data:Nxdata in scan:NXentry.
   * A subroutine would be nice to have here.......
   */
    
  hid_t dcpl = H5Pcreate (H5P_DATASET_CREATE);
  gid = H5Gcreate(fid,"entry/data",H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
  atts = H5Screate(H5S_SCALAR);
  atttype = H5Tcopy(H5T_C_S1);
  //H5Tset_size(atttype, 7);//H5T_VARIABLE);
  attid = H5Acreate(gid,"NX_class", atttype, atts, H5P_DEFAULT,H5P_DEFAULT);
  // H5Awrite(attid, atttype, (char *)"NXdata");
  H5Sclose(atts);
  H5Tclose(atttype);
  H5Aclose(attid);

  //general (all images)
  /* Initialize hyperslab values */
  
  dataspace = H5Screate_simple(rank,dim,NULL);
  datatype = H5Tcopy(H5T_STD_U32LE);  
  dataprop = H5Pcreate(H5P_DATASET_CREATE);

  dataset = H5Dcreate(gid,"Eiger", datatype,dataspace,H5P_DEFAULT, 
		     dataprop,H5P_DEFAULT);

  //first image
  int map2d[npix_y_g][npix_x_g];
  for(int iy=0; iy<npix_y_g; iy++){
    for(int ix=0; ix<npix_x_g; ix++){
      map2d[iy][ix]=100;
    }
  }
  start[0] = 0;
  start[1] = 0;
  start[2]= 0;
  count[0]=1;
  count[1] =((longedge_x==1) ? npix_y_g : npix_x_g) ;
  count[2] =((longedge_x==1) ? npix_x_g : npix_y_g) ;
  dataspace2 = H5Screate_simple(2, dim2, NULL);
  H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, stride, count, block);
  H5Dwrite(dataset, datatype , dataspace2, dataspace, H5P_DEFAULT, map2d);
  H5Sclose(dataspace2);

  
  //second image
  for(int iy=0; iy<npix_y_g; iy++)
    for(int ix=0; ix<npix_x_g; ix++)
      map2d[iy][ix]=77;
  start[0] = 1;
  start[1] = 0;
  start[2]= 0;
  count[0]=1;
  count[1] =((longedge_x==1) ? npix_y_g : npix_x_g) ;
  count[2] =((longedge_x==1) ? npix_x_g : npix_y_g) ; 
  dataspace2 = H5Screate_simple(2, dim2, NULL);
  H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, stride, count, block);
  H5Dwrite(dataset, datatype , dataspace2, dataspace, H5P_DEFAULT, map2d);
  H5Sclose(dataspace2);
  



  H5Tclose(datatype);
  H5Pclose(dataprop);  
  H5Dclose(dataset);
  
  H5Fclose(fid);
}
