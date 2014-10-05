#include "OfficialData.h"

using namespace std;

int main() {

    OfficialData gData;
    gData.ProcessSpreadsheetDir("official_data");

    return 0;
}
