# 
#  beginner makefile
#

CXXFLAGS=-Wall -g -std=c++0x

ODIR := obj
SDIR := src
BDIR := bin

_OBJS = LineItem.o OfficialData.o Supply.o TrackedResources.o Cost.o Plan.o Planners.o EntityTypeHelper.o EntityDefinition.o Gate.o HierarchicalId.o Utils.o CommandLineOptions.o main.o
OBJS=$(addprefix $(ODIR)/,$(_OBJS))

$(ODIR)/%.o : $(SDIR)/%.cc
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(BDIR)/arch_test: $(OBJS) | $(BDIR)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(OBJS): | $(ODIR)


obj/LineItem.o :           $(SDIR)/LineItem.h
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

$(ODIR):
	mkdir $(ODIR)

$(BDIR):
	mkdir $(BDIR)

.PHONY: clean

clean:
	rm -f $(SDIR)/*~ core
	rm -rf $(ODIR) $(BDIR)