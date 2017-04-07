WD				=	$(shell pwd)
LIBDIR	 		= 	$(WD)/../bin
CALDIR			= 	$(WD)/../slsDetectorCalibration
LIBRARYRXRDIR 	=	$(WD)/../slsReceiverSoftware
LIBRARYDETDIR 	=	$(WD)/../slsDetectorSoftware
LIBRARYCALDIR	=	$(WD)/../slsDetectorCalibration

CBFLIBDIR		=	/sls/X12SA/data/x12saop/EigerPackage/CBFlib-0.9.5
#CBFLIBDIR		=	~/local/Software/CBFlib/CBFlib-0.9.5
LIBRARYCBF		=	$(CBFLIBDIR)/lib/*.o
LIBHDF5			=	-L$(CBFLIBDIR)/lib/ -lhdf5
INCLUDESCBF		=	-I $(CBFLIBDIR)/include

LDFLAGRXR 		= 	-L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -pthread
LDFLAGDET 		= 	-L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -pthread
INCLUDESRXR		=	-I $(LIBRARYRXRDIR)/slsReceiver
INCLUDESDET		=	-I $(LIBRARYDETDIR)/slsDetector -I $(LIBRARYDETDIR)/slsDetectorAnalysis  -I $(LIBRARYDETDIR)/commonFiles 
INCLUDES		= 	-I. -Iincludes -I $(LIBRARYRXRDIR)/include -I $(LIBRARYCALDIR) -I $(CALDIR)

CCX			=	g++
CFLAGS		+=  -Wall 
LDLIBS		+= 	-lm  -lstdc++ 

PROGS				= 	image
PROGS_CSAXS			= 	cbfMaker
PROGS_SUM			= 	cbfMakerSum
PROGS_HALF			= 	cbfMakerHalf
PROGS_CSAXS_1.5M        	= 	cbfMaker1.5M
PROGS_CSAXS_9M	                = 	cbfMaker9M
DESTDIR				= 	bin
INSTMODE			= 	0777


SRC_CLNT		=	src/dummymain.cpp 
SRC_CSAXS_CLNT	=	src/main_csaxs.cpp 
SRC_SUM_CLNT	=	src/main_pxSum.cpp
SRC_HALF_CLNT	=	src/main_half.cpp 
SRC_CSAXS_MULTI	=	src/main_csaxs_multi.cpp 
OBJS 			= 	$(SRC_CLNT:.cpp=.o)
OBJSCSAXS 		= 	$(SRC_CSAXS_CLNT:.cpp=.o)
OBJSSUM 		= 	$(SRC_SUM_CLNT:.cpp=.o)	
OBJSCSAXSMULTI	= 	$(SRC_CSAXS_MULTI:.cpp=.o)


all: clean $(PROGS_CSAXS) $(PROGS) $(PROGS_CSAXS_1.5M)  $(PROGS_CSAXS_9M) $(PROGS_SUM) #$(PROGS_HALF) 

boot: $(OBJS) $(OBJSCSAXS) $(OBJSHALF) $(OBJSCSAXSMULTI) $(OBJSSUM)

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
$(PROGS_HALF): 
	@echo $(WD)
	echo $(OBJS_HALF)
	$(CCX)  -o $@  $(SRC_HALF_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_HALF) ../bin
	mv $(PROGS_HALF) $(DESTDIR)

$(PROGS_CSAXS_1.5M): 
	@echo $(WD)
	echo $(OBJSCSAXSMULTI)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_CSAXS_1.5M) ../bin
	mv $(PROGS_CSAXS_1.5M) $(DESTDIR)

$(PROGS_CSAXS_9M): 
	@echo $(WD)
	echo $(OBJSCSAXSMULTI)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_CSAXS_9M) ../bin
	mv $(PROGS_CSAXS_9M) $(DESTDIR)

$(PROGS_SUM): 
	@echo $(WD)
	echo $(OBJS_SUM)
	$(CCX)  -o $@  $(SRC_SUM_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(LDFLAGDET) $(CFLAGS) $(LDLIBS) 
	cp $(PROGS_SUM) ../bin
	mv $(PROGS_SUM) $(DESTDIR)

clean:
	rm -rf ../bin/$(PROGS)  *.o $(DESTDIR)/$(PROGS)
	rm -rf ../bin/$(PROGS_CSAXS)  *.o $(DESTDIR)/$(PROGS_CSAXS)
	rm -rf ../bin/$(PROGS_HALF)  *.o $(DESTDIR)/$(PROGS_HALF)
	rm -rf ../bin/$(PROGS_CSAXS_1.5M)  *.o $(DESTDIR)/$(PROGS_CSAXS_1.5M)
	rm -rf ../bin/$(PROGS_CSAXS_9M)  *.o $(DESTDIR)/$(PROGS_CSAXS_9M)	
