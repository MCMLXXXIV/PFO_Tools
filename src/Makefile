# 
#  beginner makefile
#

CXXFLAGS=-Wall -g -std=c++0x

OBJ = LineItem.o OfficialData.o Supply.o TrackedResources.o Cost.o Plan.o Planners.o EntityTypeHelper.o EntityDefinition.o Gate.o HierarchicalId.o Utils.o CommandLineOptions.o main.o

$(ODIR)/%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

arch_test: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

LineItem.o : LineItem.h
OfficialData.o : OfficialData.h EntityDefinition.h LineItem.h Utils.h
Supply.o : Supply.h
TrackedResources.o : TrackedResources.h
Cost.o : Cost.h
Plan.o : Plan.h
Planners.o : Planners.h EntityDefinition.h OfficialData.h Supply.h TrackedResources.h Cost.h Plan.h LineItem.h
EntityTypeHelper.o : EntityTypeHelper.h HierarchicalId.h
EntityDefinition.o : EntityDefinition.h
Gate.o : LineItem.h
HierarchicalId.o : HierarchicalId.h
Utils.o : Utils.h
CommandLineOptions.o : CommandLineOptions.h
main.o : OfficialData.h Planners.h Supply.h TrackedResources.h Cost.h Plan.h CommandLineOptions.h


.PHONY: clean

clean:
	rm -f *.o *~ core arch_test