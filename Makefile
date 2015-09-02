
WD				=	$(shell pwd)
LIBDIR	 		= 	$(WD)/../bin
CALDIR			= 	$(WD)/../slsDetectorCalibration
LIBRARYRXRDIR 	=	$(WD)/../slsReceiverSoftware
LIBRARYDETDIR 	=	$(WD)/../slsDetectorSoftware
LIBRARYCALDIR	=	$(WD)/../slsDetectorCalibration
LIBRARYCBF	=	/scratch/CBFlib-0.9.5/lib/*.o
LIBHDF5		=	-L/scratch/CBFlib-0.9.5/lib/ -lhdf5
LDFLAGRXR 		= 	-L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -lpthread
LDFLAGDET 		= 	-L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -lpthread
INCLUDESRXR		=	-I $(LIBRARYRXRDIR)/slsReceiver
INCLUDESDET		=	-I $(LIBRARYDETDIR)/slsDetector -I $(LIBRARYDETDIR)/slsDetectorAnalysis  -I $(LIBRARYDETDIR)/commonFiles 
INCLUDESCBF	=	-I /scratch/CBFlib-0.9.5/include

CCX			=	g++
CFLAGS		+=  -Wall 
LDLIBS		+= 	-lm  -lstdc++ 

PROGS		= 	image
PROGS_CSAXS	= 	bcfMaker
DESTDIR		?= 	bin
INSTMODE	= 	0777

INCLUDES	= 	-I. -Iincludes -I $(LIBRARYRXRDIR)/include -I $(LIBRARYCALDIR) -I $(CALDIR)

#SRC_CLNT	=	src/dummymain.cpp
SRC_CLNT	=	src/main.cpp 
SRC_CSAXS_CLNT	=	src/main_csaxs.cpp 
OBJS 		= 	$(SRC_CLNT:.cpp=.o)
OBJSCSAXS 	= 	$(SRC_CSAXS_CLNT:.cpp=.o)

#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

all: clean $(PROGS_CSAXS) $(PROGS) 

boot: $(OBJS) $(OBJSCSAXS)

$(PROGS): 
	@echo $(WD)
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(LDFLAGDET)  $(CFLAGS) $(LDLIBS) `$(ROOTSYS)/bin/root-config --cflags --libs` 
#	$(CCX)  -o $@  $(SRC_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(LDFLAGDET)  $(CFLAGS) $(LDLIBS)  
	mv $(PROGS) ../bin

$(PROGS_CSAXS): 
	@echo $(WD)
	echo $(OBJS_CSAXS)
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS) ../bin

clean:
#	rm -rf ../bin/$(PROGS)  *.o
	rm -rf ../bin/$(PROGS_CSAXS)  *.o