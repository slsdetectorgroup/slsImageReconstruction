WD				=	$(shell pwd)

CBFLIBDIR		=	/scratch/CBFlib-0.9.5
#HDF5DIR		        =	/afs/psi.ch/project/sls_det_software/software_packages/linux6/hdf5/1.10.1
HDF5DIR		        =	$(HDF5_SERIAL_DIR)
LZ4DIR		        =	/scratch/lz4


LIBRARYCBF		=	$(CBFLIBDIR)/lib/*.o
LIBHDF5			=	-L$(HDF5DIR)/lib64/ -lhdf5 -lhdf5_hl -lhdf5_hl_cpp -lhdf5_cpp -lz #-lsz 
LIBHDF5CBF			=	-L$(CBFLIBDIR)/lib/ -lhdf5
#LIBLZ4			=	-L$(LZ4DIR)/lib/ -llz4
#EXTPLUGINDIR	= /scratch/HDF5-External-Filter-Plugins/plugins/
#EXTPLUGINLIB	= -L$(EXTPLUGINDIR) -lh5lz4


INCLUDESCBF		=	-I $(CBFLIBDIR)/include
INCLUDESHDF5		=	-I $(HDF5_SERIAL_INCLUDE_DIR)  
INCLUDELZ4              =  -I $(LZ4DIR)/lib
INCLUDES		= 	-I. -Iincludes  

CCX			=	gcc -O3 -pthread  -fopenmp 
CFLAGS		+=  -Wall 
LDLIBS		+= 	-lm  -lstdc++ 

PROGS				= 	image
PROGS_CSAXS			= 	cbfMaker
PROGS_CSAXS_HDF5		= 	hdf5Maker
PROGS_CSAXS_OMNY_HDF5		= 	hdf5MakerOMNY
PROGS_CSAXS_9M_HDF5		= 	hdf5Maker9M
PROGS_CSAXS_1.5M_HDF5		= 	hdf5Maker1.5M
PROGS_SUM			= 	cbfMakerSum
PROGS_HALF			= 	cbfMakerHalf
PROGS_CSAXS_1.5M        	= 	cbfMaker1.5M
PROGS_CSAXS_OMNY        	= 	cbfMakerOMNY
PROGS_CSAXS_9M	                = 	cbfMaker9M
DESTDIR				= 	./bin/
INSTMODE			= 	0777


SRC_CLNT		=	src/main_csaxs.cpp 
SRC_CSAXS_CLNT	=	src/main_csaxs.cpp
SRC_SUM_CLNT	=	src/main_csaxs_sum.cpp #src/main_pxSum.cpp
SRC_HALF_CLNT	=	src/main_half.cpp 
SRC_CSAXS_MULTI	=	src/main_csaxs_multi.cpp
OBJS 			= 	$(SRC_CLNT:.cpp=.o)
OBJSCSAXS 		= 	$(SRC_CSAXS_CLNT:.cpp=.o)
OBJSSUM 		= 	$(SRC_SUM_CLNT:.cpp=.o)	


all: clean $(PROGS)  $(PROGS_CSAXS)  $(PROGS_CSAXS_1.5M) $(PROGS_CSAXS_OMNY) $(PROGS_CSAXS_9M) #$(PROGS_SUM) #$(PROGS_HALF) 

boot: $(OBJS) $(OBJSCSAXS) $(OBJSSUM)

$(PROGS): 
	@echo $(WD)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_CLNT) $(INCLUDES) $(CFLAGS) $(LDLIBS) `$(ROOTSYS)/bin/root-config --cflags --libs` 
	mv $(PROGS) $(DESTDIR) 
	cd $(WD)


$(PROGS_CSAXS): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) $(INCLUDES)  $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5CBF) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS_HDF5): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) $(INCLUDES)  $(INCLUDESHDF5) $(INCLUDELZ4) $(LIBHDF5) $(EXTPLUGINLIB) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS_HDF5) $(DESTDIR) 
	cd $(WD)

$(PROGS_HALF): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_HALF_CLNT) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5)  $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_HALF) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS_1.5M): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5CBF) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS_1.5M) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS_1.5M_HDF5): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES)  $(INCLUDESHDF5) $(INCLUDELZ4) $(LIBHDF5) $(EXTPLUGINLIB) $(CFLAGS) $(LDLIBS)
	mv $(PROGS_CSAXS_1.5M_HDF5) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS_OMNY): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5CBF) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS_OMNY) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS_OMNY_HDF5): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI)  $(INCLUDES)  $(INCLUDESHDF5)  $(INCLUDELZ4) $(LIBHDF5) $(EXTPLUGINLIB) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS_OMNY_HDF5) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS_9M): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5CBF)  $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS_9M) $(DESTDIR) 
	cd $(WD)

$(PROGS_CSAXS_9M_HDF5): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESHDF5) $(INCLUDELZ4) $(LIBHDF5) $(EXTPLUGINLIB) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CSAXS_9M_HDF5) $(DESTDIR) 
	cd $(WD)



$(PROGS_SUM): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_SUM_CLNT) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_SUM) $(DESTDIR) 
	cd $(WD)

clean:
	rm -rf $(DESTDIR)$(PROGS)
	rm -rf $(DESTDIR)$(PROGS_CSAXS)
	#rm -rf $(DESTDIR)$(PROGS_HALF)
	rm -rf $(DESTDIR)$(PROGS_CSAXS_1.5M)
	rm -rf $(DESTDIR)$(PROGS_CSAXS_OMNY)
	rm -rf $(DESTDIR)$(PROGS_CSAXS_9M)	
