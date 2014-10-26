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
}

bool CommandLineOptionsEncapsulation::ParseArgs(int argc, char **argv) {
    int c;
    //int digit_optind = 0;

    while (1) {
        // int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"help",       no_argument, 0, 0 },
            {"reqs", required_argument, 0, 0 },
            {"dump",       no_argument, 0, 0 },
            {"plan", required_argument, 0, 0 },
            {0,         0,              0, 0 }
        };

	c = getopt_long(argc, argv, "r:dp:h", long_options, &option_index);
        if (c == -1)
            break;
	
	switch (c) {
        case 0:
	    switch (option_index) {
	    case 0:
		ShowHelpOpt = true;
		break;
	    case 1:
		DumpItemReqs = true;
		Items = optarg;
		break;
	    case 2:
		DumpItems = true;
		break;
	    case 3:
		GetPlanForItem = true;
		Items = optarg;
		break;
	    default:	
		printf("option %s", long_options[option_index].name);
		if (optarg)
		    printf(" with arg %s", optarg);
		printf("\n");
		break;
	    }
	    break;
	case 'r':
	    DumpItemReqs = true;
	    Items = optarg;
	    break;
	case 'h':
	    ShowHelpOpt = true;
	    break;
	case 'd':
	    DumpItems = true;
	    break;
	case 'p':
	    GetPlanForItem = true;
	    Items = optarg;
	    break;
	default: ;
            // printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    if (!ShowHelpOpt && !DumpItems && !DumpItemReqs && !GetPlanForItem) {
	ParseError = true;
	ErrMsg = "no (valid) args";
    }

    return true;
}

void CommandLineOptionsEncapsulation::ShowHelp() {
    cout << "arch_test [-h|--help] [-d|--dump] [[-r|--reqs] item[,item2]] [[-p|--plan] item[,item2]]" << endl;
    cout << "   [-h|--help]                 show this mesage" << endl
	 << "   [-d|--dump]                 dump all parsed entities" << endl
	 << "   [[-r|--reqs] item[,item2]]  dump the known requirements for the item(s)" << endl
	 << "   [[-p|--plan] item[,item2]]  show a plan to create the item(s)" << endl;
}
