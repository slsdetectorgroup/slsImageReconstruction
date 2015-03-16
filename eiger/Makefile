
WD				=	$(shell pwd)
LIBDIR	 		= 	$(WD)/../bin
LIBRARYRXRDIR 	=	$(WD)/../slsReceiverSoftware
LIBRARYDETDIR 	=	$(WD)/../slsDetectorSoftware
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

INCLUDES	= 	-I. -Iincludes -I

SRC_CLNT	=	src/main.cpp 
OBJS 		= 	$(SRC_CLNT:.cpp=.o)


#FLAGS+=  #-DVERBOSE -DVERYVERBOSE

all: clean $(PROGS)

	
boot: $(OBJS2)

$(PROGS): 
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_CLNT) $(INCLUDES) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)

clean:
	rm -rf $(DESTDIR)/$(PROGS)  *.o
	
	
	#receiver
#INCLUDES = -I ../slsDetectorSoftware/include -I ../slsReceiverSoftware/includes  -I ../slsReceiverSoftware/slsReceiver -I ../slsDetectorSoftware/slsDetector -I ../slsReceiverSoftware/MySocketTCP -I ../slsDetectorSoftware/commonFiles -I ../slsDetectorSoftware/slsDetectorAnalysis
#CLFAGS = -DSLS_RECEIVER_FUNCTION_LIST  
#LIBS =  -L ../bin -l SlsDetector -lm -lpthread  -L/usr/lib64/  -lstdc++

#all: users receivers

#users:
#	g++ mainClient.cpp $(INCLUDES) $(LIBS) -o users
	
#receivers:
#	g++ mainReceiver.cpp $(INCLUDES) $(LIBS) $(CLFAGS) -o receivers 
	

#clean:
#	rm users.o receivers.o



WD				=	$(shell pwd)
LIBDIR	 		= 	$(WD)/../bin
LIBRARYRXRDIR 	=	$(WD)/../slsReceiverSoftware
LIBRARYDETDIR 	=	$(WD)/../slsDetectorSoftware
LDFLAGRXR 		= 	-L$(LIBDIR) -lSlsReceiver -L/usr/lib64/ -lpthread
LDFLAGDET 		= 	-L$(LIBDIR) -lSlsDetector -L/usr/lib64/ -lpthread
INCLUDESRXR		=	-I $(LIBRARYRXRDIR)/slsReceiver
INCLUDESDET		=	-I $(LIBRARYDETDIR)/slsDetector -I $(LIBRARYDETDIR)/slsDetectorAnalysis



all: receivers users

receivers:
	@echo $(WD)
	g++ mainReceiver.cpp   $(INCLUDESRXR) $(LDFLAGRXR) -o userReceiver

users:
	@echo $(WD)
	g++ mainClient.cpp   $(INCLUDESDET) $(LDFLAGDET) -o userClient 


clean:
	rm userReceiver userClient
	