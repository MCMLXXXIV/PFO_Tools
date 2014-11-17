#include "CommandLineOptions.h"

#include <iostream>

#include <getopt.h>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>

CommandLineOptionsEncapsulation::CommandLineOptionsEncapsulation() {
    ShowHelpOpt = false;
    DumpItems = false;
    DumpItemReqs = false;
    GetPlanForItem = false;
    ParseError = false;
    SearchForItemsThatRequire = false;
    ParseProgressionFile = false;
    Verbosity = 0;
}

bool CommandLineOptionsEncapsulation::ParseArgs(int argc, char **argv) {
    int flags, opt;

    while ((opt = getopt(argc, argv, "hdr:p:P:R:v:")) != -1) {
	switch (opt) {
	case 'h':
	    ShowHelpOpt = true;
	    break;
	case 'd':
	    DumpItems = true;
	    break;
	case 'r':
	    DumpItemReqs = true;
	    Items = optarg;
	    break;
	case 'p':
	    GetPlanForItem = true;
	    Items = optarg;
	    break;
	case 'P':
	    ParseProgressionFile = true;
	    Items = optarg;
	    break;
	case 'R':
	    SearchForItemsThatRequire = true;
	    Items = optarg;
	    break;
	case 'v':
	    Verbosity = atoi(optarg);
	    break;
	default:
	    ParseError = true;
            printf("?? getopt returned character code 0%o ??\n", opt);
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    if (!ShowHelpOpt && !DumpItems && !DumpItemReqs && !GetPlanForItem && !SearchForItemsThatRequire && !ParseProgressionFile) {
	ParseError = true;
	ErrMsg = "no (valid) args";
    }

    return true;
}

void CommandLineOptionsEncapsulation::ShowHelp() {
    cout << "arch_test [-h] [-d] [[-r] item[,item2]] [[-p] item[,item2]] [-P file] [-R item] [-v level]" << endl;
    cout << "   [-h]               show this mesage" << endl
	 << "   [-d]               dump all parsed entities" << endl
	 << "   [-r item[,item2]]  dump the known requirements for the item(s)" << endl
	 << "   [-p item[,item2]]  show a plan to create the item(s)" << endl
	 << "   [-P file]          parse and dump a progression file" << endl
	 << "   [-R item]          debug: search all known enities for those that have item as a requirement" << endl
	 << "   [-v level]         set the verbosity level to 'level'" << endl
	 << endl;
}
