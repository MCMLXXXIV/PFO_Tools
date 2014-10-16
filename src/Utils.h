#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <list>

using namespace std;

class Utils {
 public:
    static bool RankInName(const char* origName, char **name, int &rank);
    static list<string> SplitCommaSeparatedValuesWithQuotedFields(const char* line);
};
#endif
