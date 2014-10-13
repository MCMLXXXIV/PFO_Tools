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
