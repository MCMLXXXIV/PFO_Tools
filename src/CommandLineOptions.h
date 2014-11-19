#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <string>

using namespace std;

class CommandLineOptionsEncapsulation {
 public:
    CommandLineOptionsEncapsulation();
    bool ParseArgs(int argc, char **argv);
    void ShowHelp();

    bool ShowHelpOpt;
    bool DumpItems;
    bool DumpItemReqs;
    bool GetPlanForItem;
    bool ParseError;
    bool SearchForItemsThatRequire;
    bool ParseProgressionFile;

    string LoggingTags;
    int VerbosityLevel;

    string ErrMsg;

    string Items;
};


#endif
