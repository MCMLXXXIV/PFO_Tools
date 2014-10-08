# 
#  beginner makefile
#

CXXFLAGS=-Wall -g

OBJ = OfficialData.o Supply.o TrackedResources.o Cost.o Plan.o Planners.o RequirementNode.o ProvidesNode.o EntityDefinition.o main.o

$(ODIR)/%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

arch_test: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

OfficialData.o : OfficialData.h EntityDefinition.h RequirementNode.h ProvidesNode.h
Supply.o : Supply.h
TrackedResources.o : TrackedResources.h
Cost.o : Cost.h
Plan.o : Plan.h
Planners.o : Planners.h EntityDefinition.h OfficialData.h Supply.h TrackedResources.h Cost.h Plan.h
RequirementNode.o : RequirementNode.h EntityDefinition.h
ProvidesNode.o: ProvidesNode.h EntityDefinition.h
EntityDefinition.o : EntityDefinition.h
main.o : OfficialData.h Planners.h Supply.h TrackedResources.h Cost.h Plan.h


.PHONY: clean

clean:
	rm -f *.o *~ core