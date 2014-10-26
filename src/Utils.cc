#include "Utils.h"

#include <cstring>
#include <stdlib.h>
#include <cassert>

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

    if (*ptr == '+') {
	rank = atoi(ptr+1);
    } else {
	rank = atoi(ptr);
    }	

    // trim spaces and any plus
    while (ptr >= origName && (*ptr == ' ' || *ptr == '+')) {
	ptr--;
    }
    int nameLen = ptr - origName + 1;
    *name = new char[nameLen+1];
    strncpy(*name, origName, nameLen);
    (*name)[nameLen] = '\0';

    // finally, if the final name ends with " Level", truncate it off.
    const char *postFix = " Level";
    int postFixLen = strlen(postFix);
    if (nameLen > postFixLen) {
	if (strncmp(postFix, (*name + (nameLen - postFixLen)), postFixLen) == 0) {
	    (*name)[nameLen - postFixLen] = '\0';
	}
    }
	
    return true;
}

bool Utils::SplitKeyValueOnChar(const char* kvp, const char *delim, string &key, string &val) {
    // find the sep (separator)
    // c0 = walk forwards from the start to the first non-space
    // c1 = walk backwards from sep to first non-space
    // c2 = walk forwards from sep to first non-space
    // c3 = walk backwards from the end to the first non-space

    // make key from c0-c1
    // make val from c2-c3

    const char *c0, *c1, *c2, *c3, *s;

    c0 = kvp;
    int len = strlen(kvp);
    while (*c0 == ' ' && c0 < (kvp + len)) { ++c0; }
    if (c0 == (kvp + len)) { return false; }

    s = strstr(c0, delim);
    if (s == NULL) { return false; }

    c1 = s;
    while (*(c1 - 1) == ' ' && (c1 - 1) > c0) { --c1; }
    if (c1 == c0) { return false; }

    c2 = s + strlen(delim);
    while (*c2 == ' ' && c2 < (kvp + len)) { ++c2; }
    if (c2 == (kvp + len)) { return false; }

    // we want c3 to be on the last space
    c3 = kvp + len;
    while (*(c3 - 1) == ' ' && (c3 - 1) > c2) { --c3; }
    if (c3 == c2) { return false; }

    key = string (c0, (size_t)(c1 - c0));
    val = string (c2, (size_t)(c3 - c2));
    return true;

}

vector<string> Utils::SplitDelimitedValues(const char* line, const char* delimiter) {
    // forget the fancy trimming here - at least until later.
    vector<string> retVal;
    const char* endPos = line + strlen(line);
    int delimLen = strlen(delimiter);
    const char* p0 = line;
    const char* p1;
    do {
	p1 = strstr(p0, delimiter);
	if (p1 == NULL) {
	    p1 = endPos;
	}
	retVal.push_back(string (p0, (size_t)(p1 - p0)));
	p0 = p1 + delimLen;
    } while (p0 < endPos);

    return retVal;
}

vector<string> Utils::SplitCommaSeparatedValuesWithQuotedFields(const char* line) {
    // and trim space characters on the ends
    // and if the whole field is quoted, don't include the quote characters
    const char* start = line;
    const char* end = line;

    vector<string> retVal;

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
    
    
