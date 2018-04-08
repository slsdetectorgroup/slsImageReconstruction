WD				=	$(shell pwd)
LIBDIR	 		= 	$(WD)/../build/bin
CALDIR			= 	$(WD)/../slsDetectorCalibration
LIBRARYRXRDIR 	=	$(WD)/../slsReceiverSoftware
LIBRARYDETDIR 	=	$(WD)/../slsDetectorSoftware
LIBRARYCALDIR	=	$(WD)/../slsDetectorCalibration

#CBFLIBDIR		=	../../CBFlib-0.9.5
CBFLIBDIR		=	/scratch/CBFlib-0.9.5
#CBFLIBDIR		=	~/local/Software/CBFlib/CBFlib-0.9.5
LIBRARYCBF		=	$(CBFLIBDIR)/lib/*.o
LIBHDF5			=	-L$(CBFLIBDIR)/lib/ -lhdf5
INCLUDESCBF		=	-I $(CBFLIBDIR)/include

LDFLAGRXR 		= 	-L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -pthread -lrt -L$(LIBRARYRXRDIR)/include -lzmq
LDFLAGDET 		= 	-L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -pthread -lrt -L$(LIBRARYRXRDIR)/include -lzmq
INCLUDESRXR		=	-I $(LIBRARYRXRDIR)/include
INCLUDESDET		=	-I $(LIBRARYDETDIR)/slsDetector -I $(LIBRARYDETDIR)/slsDetectorAnalysis  -I $(LIBRARYDETDIR)/commonFiles 
INCLUDES		= 	-I. -Iincludes -I $(LIBRARYRXRDIR)/include -I $(LIBRARYCALDIR) -I $(CALDIR) 

CCX			=	g++
CFLAGS		+=  -Wall 
LDLIBS		+= 	-lm  -lstdc++ 

PROGS				= 	image
PROGS_CSAXS			= 	cbfMaker
PROGS_CSAXSGAPS			= 	cbfMakerGaps
PROGS_SUM			= 	cbfMakerSum
PROGS_HALF			= 	cbfMakerHalf
PROGS_CSAXS_1.5M        	= 	cbfMaker1.5M
PROGS_CSAXS_9M	                = 	cbfMaker9M
PROGS_CSAXSGAPS_9M		= 	cbfMakerGaps9M
DESTDIR				= 	bin
INSTMODE			= 	0777


SRC_CLNT		=	src/main_root.cpp 
SRC_CSAXS_CLNT	=	src/main_csaxs.cpp
SRC_CSAXSGAPS_CLNT =	src/main_gaps.cpp
SRC_SUM_CLNT	=	src/main_pxSum.cpp
SRC_HALF_CLNT	=	src/main_half.cpp 
SRC_CSAXS_MULTI	=	src/main_csaxs_multi.cpp
SRC_CSAXSGAPS_MULTI =	src/main_csaxs_multi.cpp
OBJS 			= 	$(SRC_CLNT:.cpp=.o)
OBJSCSAXS 		= 	$(SRC_CSAXS_CLNT:.cpp=.o)
OBJSCSAXSGAPS 		= 	$(SRC_CSAXSGAPS_CLNT:.cpp=.o)
OBJSSUM 		= 	$(SRC_SUM_CLNT:.cpp=.o)	
OBJSCSAXSGAPSMULTI	= 	$(SRC_CSAXSGAPS_MULTI:.cpp=.o)


all: clean $(PROGS_CSAXS) $(PROGS) $(PROGS_CSAXS_1.5M)  $(PROGS_CSAXS_9M) $(PROGS_SUM) #$(PROGS_HALF) 

boot: $(OBJS) $(OBJSCSAXS) $(OBJSHALF) $(OBJSCSAXSMULTI) $(OBJSSUM)

$(PROGS): 
	@echo $(WD)
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(LDFLAGDET)  $(CFLAGS) $(LDLIBS) `$(ROOTSYS)/bin/root-config --cflags --libs` 
	cp $(PROGS) ../build/bin
	rm -f $(PROGS) 

$(PROGS_CSAXS): 
	@echo $(WD)
	echo $(OBJS_CSAXS)
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_CSAXS) ../build/bin
	rm -f $(PROGS_CSAXS) 

$(PROGS_CSAXSGAPS): 
	@echo $(WD)
	echo $(OBJS_CSAXSGAPS)
	$(CCX)  -o $@  $(SRC_CSAXSGAPS_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_CSAXSGAPS) ../build/bin
	rm -f $(PROGS_CSAXSGAPS) 


$(PROGS_HALF): 
	@echo $(WD)
	echo $(OBJS_HALF)
	$(CCX)  -o $@  $(SRC_HALF_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_HALF) ../build/bin
	rm -f $(PROGS_HALF) 

$(PROGS_CSAXS_1.5M): 
	@echo $(WD)
	echo $(OBJSCSAXSMULTI)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_CSAXS_1.5M) ../build/bin
	rm -f $(PROGS_CSAXS_1.5M) 

$(PROGS_CSAXS_9M): 
	@echo $(WD)
	echo $(OBJSCSAXSMULTI)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_CSAXS_9M) ../build/bin
	rm -f $(PROGS_CSAXS_9M) 

$(PROGS_CSAXSGAPS_9M): 
	@echo $(WD)
	echo $(OBJSCSAXSGAPSMULTI)
	$(CCX)  -o $@  $(SRC_CSAXSGAPS_MULTI) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_CSAXSGAPS_9M) ../build/bin
	rm -f $(PROGS_CSAXSGAPS_9M) 

$(PROGS_SUM): 
	@echo $(WD)
	echo $(OBJS_SUM)
	$(CCX)  -o $@  $(SRC_SUM_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_SUM) ../build/bin
	rm -f $(PROGS_SUM) 

clean:
	rm -rf ../build/bin/$(PROGS)  *.o $(DESTDIR)/$(PROGS)
	rm -rf ../build/bin/$(PROGS_CSAXS)  *.o $(DESTDIR)/$(PROGS_CSAXS)
	rm -rf ../build/bin/$(PROGS_HALF)  *.o $(DESTDIR)/$(PROGS_HALF)
	rm -rf ../build/bin/$(PROGS_CSAXS_1.5M)  *.o $(DESTDIR)/$(PROGS_CSAXS_1.5M)
	rm -rf ../build/bin/$(PROGS_CSAXS_9M)  *.o $(DESTDIR)/$(PROGS_CSAXS_9M)	
