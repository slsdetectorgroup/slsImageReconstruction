WD				=	$(shell pwd)

CBFLIBDIR		=	/scratch/CBFlib-0.9.5
HDF5DIR		        =	/afs/psi.ch/project/sls_det_software/software_packages/linux6/hdf5/1.10.3
LIBHDF5			=	$(HDF5DIR)/lib/libhdf5.a ${HDF5DIR}/lib/libhdf5_hl.a	-lz -ldl 
LIBHDF5CBF		=	$(CBFLIBDIR)/lib/libhdf5.a $(CBFLIBDIR)/lib/libhdf5_hl.a -L$(CBFLIBDIR)/lib/ -lcbf
LIBLZ4			=	-L$(LZ4DIR)/lib/ -llz4
LIBCBFTIFF		=  -L/usr/lib64/ -ltiff 

INCLUDESCBF		=	-I $(CBFLIBDIR)/include
INCLUDESHDF5		=	-I $(HDF5DIR)/include 
INCLUDELZ4              =  -I $(LZ4DIR)/lib
INCLUDES		= 	-I. -Iincludes  

CCX			=	gcc -O3 -pthread
CFLAGS		+=  -Wall 
LDLIBS		+= 	-lm  -lstdc++ 

PROGS				= 	image
PROGS_QUAD			= 	imageQuad
PROGSROOT_SUM			= 	imageSum
PROGS_CBF			= 	cbfMaker
PROGS_CBFSUM			= 	cbfMakerSum
PROGS_CBF_QUAD		  	= 	cbfMakerQuad
PROGS_CBF_HALF			= 	cbfMakerHalf
PROGS_CBF_1.5M        		= 	cbfMaker1.5M
PROGS_CBF_OMNY        		= 	cbfMakerOMNY
PROGS_CBF_9M	                = 	cbfMaker9M
PROGS_HDF5		        = 	hdf5Maker
PROGS_TIFF			= 	tiffMaker
PROGS_TIFF_QUAD			= 	tiffMakerQuad
PROGS_TXT		        = 	txtMaker
PROGS_TXT_QUAD			= 	txtMakerQuad
PROGS_OMNY_HDF5			= 	hdf5MakerOMNY
PROGS_9M_HDF5			= 	hdf5Maker9M
PROGS_1.5M_HDF5			= 	hdf5Maker1.5M
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


all: clean $(PROGS) $(PROGS_CBF) $(PROGS_HDF5) $(PROGS_TIFF) $(PROGS_TXT) 	

boot: $(OBJS) $(OBJSCSAXS) $(OBJSSUM)

$(PROGS): 
	@echo $(WD)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_CLNT) -DMYROOT  $(INCLUDES) $(CFLAGS) $(LDLIBS) `$(ROOTSYS)/bin/root-config --cflags --libs` 
	mv $(PROGS) $(DESTDIR) 
	cd $(WD)

$(PROGS_QUAD): 
	@echo $(WD)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(CFLAGS) $(LDLIBS) `$(ROOTSYS)/bin/root-config --cflags --libs` 
	mv $(PROGS_QUAD) $(DESTDIR) 
	cd $(WD)

$(PROGSROOT_SUM): 
	@echo $(WD)
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@  $(SRC_SUM_CLNT) $(INCLUDES) $(CFLAGS) $(LDLIBS) `$(ROOTSYS)/bin/root-config --cflags --libs` 
	mv $(PROGSROOT_SUM) $(DESTDIR) 
	cd $(WD)

$(PROGS_CBF): 
	@echo $(WD)
        ## choose if you want header supperted at MS beamline  -DMSHeader	
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) -DMYCBF -DMSHeader $(INCLUDES)  $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5CBF) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CBF) $(DESTDIR) 
	cd $(WD)

$(PROGS_CBF_QUAD): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES)  $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5CBF) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CBF_QUAD) $(DESTDIR) 
	cd $(WD)

$(PROGS_HDF5): 
	@echo $(WD)
# CHOSE OPTIONS  -DZLIB (COMPRESSED ZLIB), -DMASTERVIRTUAL (MASTER FILE CONTAINS A VIRTUAL DATASET), -DMASTERLINK (MASETR FILE CONTAINS A LINKED DATASET)
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) -DHDF5f -DMASTERLINK $(INCLUDES)  $(INCLUDESHDF5) $(INCLUDELZ4) $(LIBHDF5) $(EXTPLUGINLIB) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_HDF5) $(DESTDIR) 
	cd $(WD)

$(PROGS_TXT): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) -DTXT $(INCLUDES) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_TXT) $(DESTDIR) 
	cd $(WD)

$(PROGS_TXT_QUAD): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_TXT_QUAD) $(DESTDIR) 
	cd $(WD)

$(PROGS_TIFF): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_CLNT) -DTIFFFILE $(INCLUDES) $(INCLUDESCBF) $(CFLAGS) $(LDLIBS) $(LIBCBFTIFF) 
	mv $(PROGS_TIFF) $(DESTDIR) 
	cd $(WD)

$(PROGS_TIFF_QUAD): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESCBF) $(CFLAGS) $(LDLIBS) $(LIBCBFTIFF) 
	mv $(PROGS_TIFF_QUAD) $(DESTDIR) 
	cd $(WD)

$(PROGS_HALF): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_HALF_CLNT) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5)  $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_HALF) $(DESTDIR) 
	cd $(WD)

$(PROGS_CBF_1.5M): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5CBF) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CBF_1.5M) $(DESTDIR) 
	cd $(WD)

$(PROGS_1.5M_HDF5): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES)  $(INCLUDESHDF5) $(INCLUDELZ4) $(LIBHDF5) $(EXTPLUGINLIB) $(CFLAGS) $(LDLIBS)
	mv $(PROGS_1.5M_HDF5) $(DESTDIR) 
	cd $(WD)

$(PROGS_CBF_OMNY): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5CBF) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CBF_OMNY) $(DESTDIR) 
	cd $(WD)

$(PROGS_OMNY_HDF5): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI)  $(INCLUDES)  $(INCLUDESHDF5)  $(INCLUDELZ4) $(LIBHDF5) $(EXTPLUGINLIB) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_OMNY_HDF5) $(DESTDIR) 
	cd $(WD)

$(PROGS_CBF_9M): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESCBF)  $(LIBRARYCBF) $(LIBHDF5CBF)  $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_CBF_9M) $(DESTDIR) 
	cd $(WD)

$(PROGS_9M_HDF5): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_CSAXS_MULTI) $(INCLUDES) $(INCLUDESHDF5) $(INCLUDELZ4) $(LIBHDF5) $(EXTPLUGINLIB) $(CFLAGS) $(LDLIBS) 
	mv $(PROGS_9M_HDF5) $(DESTDIR) 
	cd $(WD)

$(PROGS_SUM): 
	@echo $(WD)
	$(CCX)  -o $@  $(SRC_SUM_CLNT) $(INCLUDES) $(INCLUDESCBF) $(LIBHDF5) $(LIBRARYCBF)  $(CFLAGS) $(LDLIBS) $(LIBHDF5CBF) 
	mv $(PROGS_SUM) $(DESTDIR) 
	cd $(WD)

clean:
	rm -rf $(DESTDIR)$(PROGS)
	rm -rf $(DESTDIR)$(PROGS_QUAD)
	rm -rf $(DESTDIR)$(PROGSROOT_SUM)
	rm -rf $(DESTDIR)$(PROGS_SUM)
	rm -rf $(DESTDIR)$(PROGS_CBF)
	rm -rf $(DESTDIR)$(PROGS_CBF_1.5M)
	rm -rf $(DESTDIR)$(PROGS_CBF_OMNY)
	rm -rf $(DESTDIR)$(PROGS_CBF_9M)	
	rm -rf $(DESTDIR)$(PROGS_CBFSUM)
	rm -rf $(DESTDIR)$(PROGS_CBF_QUAD)
	rm -rf $(DESTDIR)$(PROGS_CBF_HALF)
	rm -rf $(DESTDIR)$(PROGS_CBF_1.5M)
	rm -rf $(DESTDIR)$(PROGS_TIFF)
	rm -rf $(DESTDIR)$(PROGS_TIFF_QUAD)
	rm -rf $(DESTDIR)$(PROGS_TXT)
	rm -rf $(DESTDIR)$(PROGS_TXT_QUAD)	
	rm -rf $(DESTDIR)$(PROGS_OMNY_HDF5)	
	rm -rf $(DESTDIR)$(PROGS_HDF5)		
	rm -rf $(DESTDIR)$(PROGS_9M_HDF5)	
	rm -rf $(DESTDIR)$(PROGS_1.5M_HDF5)	