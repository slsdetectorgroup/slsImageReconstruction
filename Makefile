WD				=	$(shell pwd)

#CBFLIBDIR		=	../../CBFlib-0.9.5
CBFLIBDIR		=	/scratch/CBFlib-0.9.5
#CBFLIBDIR		=	~/local/Software/CBFlib/CBFlib-0.9.5
LIBRARYCBF		=	$(CBFLIBDIR)/lib/*.o
LIBHDF5			=	-L$(CBFLIBDIR)/lib/ -lhdf5
INCLUDESCBF		=	-I $(CBFLIBDIR)/include

INCLUDES		= 	-I. -Iincludes  

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
DESTDIR				= 	./bin/
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


all: clean $(PROGS)  $(PROGS_CSAXS)  $(PROGS_CSAXS_1.5M)  $(PROGS_CSAXS_9M) $(PROGS_CSAXSGAPS) $(PROGS_CSAXSGAPS_9M) #$(PROGS_SUM) #$(PROGS_HALF) 

boot: $(OBJS) $(OBJSCSAXS) $(OBJSHALF) $(OBJSCSAXSMULTI) $(OBJSSUM)

$(PROGS): 
	@echo $(WD)
	echo $(OBJS)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_CLNT) $(INCLUDES) $(CFLAGS) $(LDLIBS) `$(ROOTSYS)/bin/root-config --cflags --libs` 
	mv $(PROGS) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS): 
	@echo $(WD)
	echo $(OBJS_CSAXS)
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) $(INCLUDES)  $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXSGAPS): 
	@echo $(WD)
	echo $(OBJS_CSAXSGAPS)
	$(CCX)  -o $@  $(SRC_CSAXSGAPS_CLNT) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXSGAPS) $(DESTDIR) 
	cd $(WD)

$(PROGS_HALF): 
	@echo $(WD)
	echo $(OBJS_HALF)
	$(CCX)  -o $@  $(SRC_HALF_CLNT) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5)  $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_HALF) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS_1.5M): 
	@echo $(WD)
	echo $(OBJSCSAXSMULTI)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS_1.5M) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS_9M): 
	@echo $(WD)
	echo $(OBJSCSAXSMULTI)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5)  $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS_9M) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXSGAPS_9M): 
	@echo $(WD)
	echo $(OBJSCSAXSGAPSMULTI)
	$(CCX)  -o $@  $(SRC_CSAXSGAPS_MULTI) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXSGAPS_9M) $(DESTDIR) 
	cd $(WD)

$(PROGS_SUM): 
	@echo $(WD)
	echo $(OBJS_SUM)
	$(CCX)  -o $@  $(SRC_SUM_CLNT) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_SUM) $(DESTDIR) 
	cd $(WD)

clean:
	rm -rf $(DESTDIR)$(PROGS)
	rm -rf $(DESTDIR)$(PROGS_CSAXS)
	#rm -rf $(DESTDIR)$(PROGS_HALF)
	rm -rf $(DESTDIR)$(PROGS_CSAXS_1.5M)
	rm -rf $(DESTDIR)$(PROGS_CSAXS_9M)	
	rm -rf $(DESTDIR)$(PROGS_CSAXSGAPS) 
	rm -rf $(DESTDIR)$(PROGS_CSAXSGAPS_9M)	