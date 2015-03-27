
WD				=	$(shell pwd)
LIBDIR	 		= 	$(WD)/../bin
LIBRARYRXRDIR 	=	$(WD)/../slsReceiverSoftware
LIBRARYDETDIR 	=	$(WD)/../slsDetectorSoftware
LIBRARYCALDIR	=	$(WD)/../slsDetectorCalibration
LDFLAGRXR 		= 	-L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -lpthread
LDFLAGDET 		= 	-L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -lpthread
INCLUDESRXR		=	-I $(LIBRARYRXRDIR)/slsReceiver
INCLUDESDET		=	-I $(LIBRARYDETDIR)/slsDetector -I $(LIBRARYDETDIR)/slsDetectorAnalysis



CCX			=	g++
CFLAGS		+=  -Wall 
LDLIBS		+= 	-lm  -lstdc++ 

PROGS		= 	eigerImageReconstruct
DESTDIR		?= 	bin
INSTMODE	= 	0777

INCLUDES	= 	-I. -Iincludes -I $(LIBRARYRXRDIR)/include -I $(LIBRARYCALDIR)

SRC_CLNT	=	src/main.cpp 
OBJS 		= 	$(SRC_CLNT:.cpp=.o)


#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

all: clean $(PROGS)

	
boot: $(OBJS)

$(PROGS): 
	@echo $(WD)
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_CLNT) $(INCLUDES)  $(INCLUDESRXR) $(LDFLAGRXR)  $(INCLUDESDET) $(LDFLAGDET)  $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o
	
	