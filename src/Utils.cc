#include "Utils.h"

#include <cstring>
#include <stdlib.h>

bool Utils::RankInName(const char *origName, char **name,  int &rank) {
    int len = strlen(origName);

    const char *ptr = origName + len - 1;
    while (ptr >= origName && *ptr >= '0' && *ptr <= '9') {
	ptr--;
    }

    if (ptr == origName + len - 1) {
	// there were no numbers at the end
	return false;
    }

    if (*ptr != '+') {
	// there were numbers but no plus sign
	return false;
    }

    int nameLen = ptr - origName - 1;

    rank = atoi(ptr+1);
    *name = new char[nameLen+1];
    strncpy(*name, origName, nameLen);
    (*name)[nameLen] = '\0';
    return true;
}

list<string> Utils::SplitCommaSeparatedValuesWithQuotedFields(const char* line) {
    // and trim space characters on the ends
    // and if the whole field is quoted, don't include the quote characters
    const char* start = line;
    const char* end = line;

    list<string> retVal;

    if (line == NULL || *line == '\0') {
	return retVal;
    }

    while (true) {
	// skip any leading space
	while (*start == ' ') { 
	    ++start;
	}
	if (*start == '\0') { 
	    // add empty field
	    retVal.push_back("");
	    break; 
	}

	// find the end
	end = start;
	while (true) {
	    if (*end == '\0') { break; }

	    // eat quoted strings
	    if (*end == '"') {
		++end; // get past the first quote
		while (*end != '\0' && *end != '"') {
		    ++end;
		}
		if (*end == '\0') { break; }
	    }

	    if (*end == ',') { break; }
	    ++end;
	}

	// now trim any trailing space
	const char *endAfterTrim = end;
	while (endAfterTrim != start && *(endAfterTrim-1) == ' ') {
	    --endAfterTrim;
	}

	// if the whole field is quoted, omit the quotes
	if (start < endAfterTrim) {
	    if (*start == '"' && *(endAfterTrim-1) == '"') {
		++start;
		--endAfterTrim;
	    }
	}
	retVal.push_back(string (start, (size_t)(endAfterTrim - start)));

	if (*end != '\0') {
	    ++end;
	} else {
	    break;
	}
	start = end;
    }
    return retVal;
}
    
    
