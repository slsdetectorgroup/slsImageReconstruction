/**************************************************************************/
/* This sample program provides a code for a connectionless server.       */
/**************************************************************************/

/**************************************************************************/
/* Header files needed for this sample program                            */
/**************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>  /* exit() */
#include <string.h>  /* memset(), memcpy() */
#include <sys/utsname.h>   /* uname() */
#include <sys/types.h>
#include <sys/socket.h>   /* socket(), bind(),
                  listen(), accept() */
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>  /* fork(), write(), close() */
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>

#define OK 0
#define FAIL -1
#define MAX_BUFLEN   1048576

/**************************************************************************/
/* Constants used by this program                                         */
/**************************************************************************/
#define BUFFER_LENGTH    8214
#define FALSE              0

#define NBIN 500
#define NCH 4096*128
#define ADCPERBIN 8

int sockfd;
FILE * sfilefd;
int display,save,framesinfile;
int* hchptr;
int* hchptrwg;

int findclumax(int startindex,int16_t * pcimaged){
   int max,ix,iy,idx;
   int idxofmax;
   max=0;idxofmax=0;
   for (ix=-1;ix<2;ix++) {
     for (iy=-1;iy<2;iy++) {
       idx=startindex+ix+iy*1024;
       if ((idx>0)&&(idx<NCH)) {
     if (max<pcimaged[idx]) {
       max=pcimaged[idx];
       idxofmax=idx;
     }
       }
     }
   }
   if (idxofmax!=startindex) {
     return findclumax(idxofmax,pcimaged); //recursion!
   }
   else   {
     return idxofmax;
   }
}
void zeroclu(int startindex,int16_t * pcimaged){
   int ix,iy,idx;
   for (ix=-1;ix<2;ix++) {
     for (iy=-1;iy<2;iy++) {
       idx=startindex+ix+iy*1024;
       if ((idx>0)&&(idx<NCH)) {
     pcimaged[idx]=0;
       }
     }
   }
}



int main(int argc,char *argv[])
{

   struct  timeval tss,tsss; //for timing
   double tdif;

#pragma pack(push)
#pragma pack(2)
   struct mystruct{
     char emptyheader[6]; //was 2
     uint64_t framenum;
     uint64_t packetnum;
     uint16_t data[BUFFER_LENGTH];
   };
   struct mystruct packet;
#pragma pack(pop)


   int ibin;

   FILE* outfile;
   uint16_t imaged[NCH];
   float fpeded[NCH];
   uint16_t ipeded[NCH];
   int16_t pcimaged[NCH];

   int hcumul[NBIN];


   char datafilename[128];
   char pedefilename[128];
   char pedefileprefix[128];
   char datafileprefix[128];



   int fileindex=0;
   int nframes=0;

   int ipx=0;
   int threshold;
   int threshold2;
   int th_id;int clumax;

   printf("allocating memory PF  ....\n");

   if ((argc!=5)) {printf("USAGE: com fname pedename lowthreshold highthreshold(for window) \n"); return -1  ;}
   if ((argc=5)) { sprintf(&datafileprefix,"%s",argv[1]);
     sprintf(&pedefileprefix,"%s",argv[2]);
     threshold=atoi(argv[3]);
     threshold2=atoi(argv[4]);
     printf( " \n");
     printf( "opening those  %sxxxxxxxx.dat \n",datafileprefix);
     printf( "with pede %s000000.dat \n",pedefileprefix);
     printf( "and photon threshold %d  \n",threshold);

   }

   framesinfile=0;

   sprintf(pedefilename,"%s000000.dat",pedefileprefix);
   printf("opening pede file : %s ....\n",pedefilename);
   sfilefd=fopen((const char *)(pedefilename),"r");
   for (ipx=0;ipx<NCH;ipx++) fpeded[ipx]=0;


   while ((fread(&packet.framenum, 8 ,1,sfilefd)+fread(imaged, 2 ,NCH,sfilefd)>524288)&&(framesinfile<200)){
     framesinfile++;nframes++;
     for (ipx=0;ipx<NCH;ipx++) fpeded[ipx]=(fpeded[ipx]*(nframes-1)+(float)(imaged[ipx]&0x3fff))/(float)(nframes);

   }

   for (ipx=0;ipx<NCH;ipx++) { ipeded[ipx]=(uint16_t)(fpeded[ipx]);

     if (ipx%10033==0) printf("i=%d pede= %d  %f .\n",ipx, ipeded[ipx],fpeded[ipx]);
   }



   for (ibin=0;ibin<NBIN;ibin++)hcumul[ibin] =0;

   fclose(sfilefd);
   printf("opened %d images as pede.\n",framesinfile);
   hchptr = (int*) malloc(NCH*sizeof(int));
   hchptrwg = (int*) malloc(NCH*sizeof(int));
   framesinfile=0;

   omp_set_dynamic(0);     // Explicitly disable dynamic teams
#define NTHREADS 6
   omp_set_num_threads(NTHREADS); // Use NTHREADS threads
#pragma omp parallel private(th_id,sfilefd,ipx,ibin,framesinfile,fileindex,datafilename,tss,tsss,tdif,packet,imaged,pcimaged,clumax)
   { //parallele
     fileindex=0;
     th_id = omp_get_thread_num();
sprintf(datafilename,"%s%06d.dat",datafileprefix,fileindex+th_id);

     sfilefd=fopen((const char *)(datafilename),"r");
     printf("Thread %d opened  %s ...  \n", th_id,datafilename);

     while (((sfilefd!=NULL))&&(fileindex<600)) { // loop on files

       while ((fread(&packet.framenum, 8 ,1,sfilefd)+fread(imaged, 2 ,NCH,sfilefd)>524288)){

     framesinfile++;
     // printf("Thread %d inside file while  precessed %d frames \n", th_id,framesinfile);

     for (ipx=0;ipx<NCH;ipx++) pcimaged[ipx]=imaged[ipx]-ipeded[ipx];
     // asm("; after pede sub");

     for (ipx=0;ipx<NCH;ipx++) {
       if (pcimaged[ipx]>threshold)
         {
           if (pcimaged[ipx]<threshold2) hchptrwg[(ipx)]= hchptrwg[(ipx)]+1;

           hchptr[(ipx)]= hchptr[(ipx)]+1;


         }
     }//end of loop on channels
     /* for (ipx=0;ipx<NCH;ipx++) { */
     /*   if (pcimaged[ipx]>threshold){ */

     /*     clumax=findclumax(ipx,pcimaged); */
     /*     hchptrwg[clumax]++; */
     /*     zeroclu(clumax,pcimaged); */

     /*   } */
     /* }//end of loop on channels*/




#define NPRI 1000
     if  (((framesinfile%NPRI)==NPRI-1)&&(1==1)) {
       tss=tsss;
       gettimeofday(&tsss,NULL);
       tdif=(1e6*(tsss.tv_sec - tss.tv_sec)+(long)(tsss.tv_usec)-(long)(tss.tv_usec));
       printf("Thread %d :average Rate = %f frames/s \n",th_id, 1e6/(tdif/NPRI) ); ///for timing purposes
     }

       }//end of loop on images
       fclose(sfilefd);
       fileindex=fileindex+NTHREADS;
sprintf(datafilename,"%s%06d.dat",datafileprefix,fileindex+th_id);
       sfilefd=fopen((const char *)(datafilename),"r");
       printf("Thread %d opened  %s   \n", th_id,datafilename);
       framesinfile=0;
     } //end on file loop


   } //parallele



   sprintf(datafilename,"%s_photonmap_MT.bin",datafileprefix);

   printf( "writing photonmap to  %s \n",datafilename);



   outfile=fopen(datafilename,"w");
   fwrite(hchptr, 4 ,NCH,outfile);
   fclose(outfile);

   sprintf(datafilename,"%s_photonmap_MT_WD.bin",datafileprefix);
   printf( "writing photonmap window to  %s \n",datafilename);
   outfile=fopen(datafilename,"w");
   fwrite(hchptrwg, 4 ,NCH,outfile);
   fclose(outfile);



   // for (ibin=0;ibin<NBIN;ibin++) { printf( "bin = %d content %d \n",ibin,hcumul[ibin]);}

   return 0;
}
