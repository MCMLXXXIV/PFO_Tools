#ifndef LOG_H
#define LOG_H

#include <set>
#include <string>

using namespace std;

class Logger {
 public:
    enum Level { Quiet = 0, Error, Warn, Note, Verbose  };

    static Logger* Instance();

    void Log(Level level, string tag, const char *fmt, ...);
    void SetLoggingLevel(Level level);
    Level GetLoggingLevel();
    bool SetTagsFromCsv(string listCsv);
    bool SetTag(string tag);
    void RemoveTag(string tag);
    
 private:
    Logger();

    set<string> Tags;
    Level LoggingLevel;
    static Logger* m_pInstance;

    static const string LegalTagCharacters;

};


#endif
