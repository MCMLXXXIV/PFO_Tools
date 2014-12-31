#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

using namespace std;

class Utils {
 public:
    static bool SplitKeyValueOnChar(const char* kvp, const char *delim, string &key, string &val);
    static bool RankInName(const char* origName, char **name, unsigned &rank);
    static vector<string> SplitCommaSeparatedValuesWithQuotedFields(const char* line);
    static vector<string> SplitDelimitedValues(const char* line, const char* delimiter);
};
#endif
