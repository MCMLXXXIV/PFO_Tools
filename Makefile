# 
#  beginner makefile
#

CXXFLAGS=-Wall -g -std=c++0x
#LIBS += -Wl -lmicrohttpd.a

# is this working?  The resulting binary seems to be the same size...
LIBS += -Wl,-Bstatic -lmicrohttpd -Wl,-Bdynamic -lpthread -lrt

ODIR := obj
SDIR := src
BDIR := bin

_OBJS = LineItem.o OfficialData.o Supply.o TrackedResources.o Cost.o Plan.o Planners.o EntityTypeHelper.o EntityDefinition.o Gate.o HierarchicalId.o Utils.o CommandLineOptions.o

OBJS=$(addprefix $(ODIR)/,$(_OBJS))

all : $(BDIR)/arch_test $(BDIR)/webService

$(ODIR)/%.o : $(SDIR)/%.cc
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(BDIR)/arch_test: $(OBJS) $(ODIR)/main.o | $(BDIR)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BDIR)/webService: $(OBJS) $(ODIR)/WebService.o | $(BDIR)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(OBJS): | $(ODIR)


$(ODIR)/LineItem.o :           $(patsubst %,$(SDIR)/%,LineItem.h)
$(ODIR)/OfficialData.o :       $(patsubst %,$(SDIR)/%,OfficialData.h EntityDefinition.h LineItem.h Utils.h)
$(ODIR)/Supply.o :             $(patsubst %,$(SDIR)/%,Supply.h)
$(ODIR)/TrackedResources.o :   $(patsubst %,$(SDIR)/%,TrackedResources.h)
$(ODIR)/Cost.o :               $(patsubst %,$(SDIR)/%,Cost.h)
$(ODIR)/Plan.o :               $(patsubst %,$(SDIR)/%,Plan.h)
$(ODIR)/Planners.o :           $(patsubst %,$(SDIR)/%,Planners.h EntityDefinition.h OfficialData.h Supply.h TrackedResources.h Cost.h Plan.h LineItem.h)
$(ODIR)/EntityTypeHelper.o :   $(patsubst %,$(SDIR)/%,EntityTypeHelper.h HierarchicalId.h)
$(ODIR)/EntityDefinition.o :   $(patsubst %,$(SDIR)/%,EntityDefinition.h)
$(ODIR)/Gate.o :               $(patsubst %,$(SDIR)/%,LineItem.h)
$(ODIR)/HierarchicalId.o :     $(patsubst %,$(SDIR)/%,HierarchicalId.h)
$(ODIR)/Utils.o :              $(patsubst %,$(SDIR)/%,Utils.h)
$(ODIR)/CommandLineOptions.o : $(patsubst %,$(SDIR)/%,CommandLineOptions.h)
$(ODIR)/main.o :               $(patsubst %,$(SDIR)/%,OfficialData.h Planners.h Supply.h TrackedResources.h Cost.h Plan.h CommandLineOptions.h)
$(ODIR)/WebService.o :         $(patsubst %,$(SDIR)/%,OfficialData.h Planners.h Supply.h TrackedResources.h Cost.h Plan.h CommandLineOptions.h)

$(ODIR):
	mkdir $(ODIR)

$(BDIR):
	mkdir $(BDIR)

.PHONY: clean

clean:
	rm -f $(SDIR)/*~ core
	rm -rf $(ODIR) $(BDIR)