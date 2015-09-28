WD				=	$(shell pwd)
LIBDIR	 		= 	$(WD)/../bin
CALDIR			= 	$(WD)/../slsDetectorCalibration
LIBRARYRXRDIR 	=	$(WD)/../slsReceiverSoftware
LIBRARYDETDIR 	=	$(WD)/../slsDetectorSoftware
LIBRARYCALDIR	=	$(WD)/../slsDetectorCalibration

#CBFLIBDIR		=	/scratch/CBFlib-0.9.5/ 
CBFLIBDIR		=	~/local/Software/CBFlib/CBFlib-0.9.5
LIBRARYCBF		=	$(CBFLIBDIR)/lib/*.o
LIBHDF5			=	-L$(CBFLIBDIR)/lib/ -lhdf5
INCLUDESCBF		=	-I $(CBFLIBDIR)/include

LDFLAGRXR 		= 	-L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -lpthread
LDFLAGDET 		= 	-L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -lpthread
INCLUDESRXR		=	-I $(LIBRARYRXRDIR)/slsReceiver
INCLUDESDET		=	-I $(LIBRARYDETDIR)/slsDetector -I $(LIBRARYDETDIR)/slsDetectorAnalysis  -I $(LIBRARYDETDIR)/commonFiles 
INCLUDES		= 	-I. -Iincludes -I $(LIBRARYRXRDIR)/include -I $(LIBRARYCALDIR) -I $(CALDIR)

CCX			=	g++
CFLAGS		+=  -Wall 
LDLIBS		+= 	-lm  -lstdc++ 

PROGS				= 	image
PROGS_CSAXS			= 	bcfMaker
PROGS_CSAXS_MULTI	= 	bcfMakerMulti
DESTDIR				= 	bin
INSTMODE			= 	0777


SRC_CLNT		=	src/main.cpp 
SRC_CSAXS_CLNT	=	src/main_csaxs.cpp 
SRC_CSAXS_MULTI	=	src/main_csaxs_multi.cpp 
OBJS 			= 	$(SRC_CLNT:.cpp=.o)
OBJSCSAXS 		= 	$(SRC_CSAXS_CLNT:.cpp=.o)
OBJSCSAXSMULTI	= 	$(SRC_CSAXS_MULTI:.cpp=.o)


all: clean $(PROGS_CSAXS) $(PROGS_CSAXS_MULTI) $(PROGS) 

boot: $(OBJS) $(OBJSCSAXS) $(OBJSCSAXSMULTI)

$(PROGS): 
	@echo $(WD)
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(LDFLAGDET)  $(CFLAGS) $(LDLIBS) `$(ROOTSYS)/bin/root-config --cflags --libs` 
	cp $(PROGS) ../bin
	mv $(PROGS) $(DESTDIR)

$(PROGS_CSAXS): 
	@echo $(WD)
	echo $(OBJS_CSAXS)
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_CSAXS) ../bin
	mv $(PROGS_CSAXS) $(DESTDIR)
	
$(PROGS_CSAXS_MULTI): 
	@echo $(WD)
	echo $(OBJSCSAXSMULTI)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_CSAXS_MULTI) ../bin
	mv $(PROGS_CSAXS_MULTI) $(DESTDIR)


clean:
	rm -rf ../bin/$(PROGS)  *.o $(DESTDIR)/$(PROGS)
	rm -rf ../bin/$(PROGS_CSAXS)  *.o $(DESTDIR)/$(PROGS_CSAXS)
	rm -rf ../bin/$(PROGS_CSAXS_MULTI)  *.o $(DESTDIR)/$(PROGS_CSAXS_MULTI)