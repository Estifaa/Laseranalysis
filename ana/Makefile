#
# Makefile for the MIDAS analyzer component of ROOTANA
#
# Options:
# make NO_ROOT=1 # build without ROOT support
#

CXXFLAGS += -g -O2 -Wall -Wuninitialized -I.

ifndef ROOTANASYS
norootanasys:
	@echo Error: ROOTANASYS in not defined, please source thisrootana.{sh,csh}
endif

# check for ROOT

ifdef NO_ROOT
ROOTSYS:=
endif

ifneq ($(ROOTSYS),)
HAVE_ROOT:=1
LIBS   += -lSpectrum
endif

# get the rootana Makefile settings

CXXFLAGS += -I$(ROOTANASYS)/include $(shell cat $(ROOTANASYS)/include/rootana_cflags.txt)
LIBS     += -L$(ROOTANASYS)/lib -lrootana $(shell cat $(ROOTANASYS)/include/rootana_libs.txt)

# select the main program - local custom main()
# or standard main() from rootana

#MAIN := manalyzer_main.o
MAIN := $(ROOTANASYS)/obj/manalyzer_main.o

# uncomment and define analyzer modules here

#RMODULES = reco_module.o
#RLIBS = -L$(ANALYSIS_TPC) -lAGTPC -lAGUTILS -lGeom -lRGL

MODULES += ncfm.o unpack_module.o adc_module.o pwb_module.o Alpha16.o feam_module.o TsSync.o Feam.o FeamEVB.o FeamAsm.o PwbAsm.o AgEvent.o AgEVB.o TrgAsm.o Unpack.o AgAsm.o $(RMODULES) wfexport_module.o final_module.o

ALL     += agana.exe
#ALL     += ncfm.exe

# examples

EXAMPLE_ALL += manalyzer.exe
EXAMPLE_ALL += manalyzer_example_cxx.exe
EXAMPLE_ALL += manalyzer_example_flow.exe
ifdef HAVE_ROOT
EXAMPLE_ALL += manalyzer_example_root.exe
EXAMPLE_ALL += manalyzer_example_root_graphics.exe
endif

all:: $(EXAMPLE_ALL)
all:: $(MODULES)
all:: $(ALL)

%.exe: %.o manalyzer_main.o
	$(CXX) -o $@ manalyzer_main.o $< $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread

$(EXAMPLE_ALL): %.exe:
	$(CXX) -o $@ $(ROOTANASYS)/obj/manalyzer_main.o $< $(CXXFLAGS) $(LIBS) -lm -lz -lpthread

%.exe: $(MAIN) $(MODULES)
	$(CXX) -o $@ $(MAIN) $(MODULES) $(CXXFLAGS) $(RLIBS) $(LIBS) -lm -lz -lpthread -lMathMore -lMinuit -lPhysics

ncfm.exe: %.exe: %.o
	$(CXX) -o $@ $< $(CXXFLAGS) $(LIBS) -lm -lz -lpthread

reco_module.o: reco_module.cxx
	$(CXX) -o $@ -I$(AGTPC_ANALYSIS) -I$(ANALYSIS_TPC)/include -I$(GARFIELDPP) -I${SOURCE_TPC}/include $(CXXFLAGS) -c $<

# Signals.o: HAVE_ROOT = ""
# Signals.o: $(AGTPC_ANALYSIS)/Signals.cc
# 	echo HAVE_ROOT = $(HAVE_ROOT)
# 	$(CXX) -o $@ -I$(AGTPC_ANALYSIS) -I$(GARFIELDPP) $(CXXFLAGS) -c $<

# SpacePoints.o: HAVE_ROOT = ""
# SpacePoints.o: $(AGTPC_ANALYSIS)/SpacePoints.cc
# 	echo HAVE_ROOT = $(HAVE_ROOT)
# 	$(CXX) -o $@ -I$(AGTPC_ANALYSIS) -I$(ANALYSIS_TPC)/include -I$(GARFIELDPP) $(CXXFLAGS) -c $<

# TLookUpTable.o: $(ANALYSIS_TPC)/src/TLookUpTable.cc
# 	echo HAVE_ROOT = $(HAVE_ROOT)
# 	$(CXX) -o $@ -I$(AGTPC_ANALYSIS) -I$(ANALYSIS_TPC)/include -I$(GARFIELDPP) $(CXXFLAGS) -c $<

# TPCBase.o: $(GARFIELDPP)/TPCBase.cc
# 	$(CXX) -o $@ -I$(GARFIELDPP) $(CXXFLAGS) -c $<

%.o: %.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

html/index.html:
	-mkdir html
	-make -k dox
	touch html/index.html

dox:
	doxygen

clean::
	-rm -f *.o *.a *.exe

clean::
	-rm -f $(ALL)

clean::
	-rm -f $(EXAMPLE_ALL)

clean::
	-rm -rf *.exe.dSYM

clean::
	-rm -rf html

# end
