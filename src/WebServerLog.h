#ifndef WEBSERVERLOG_H
#define WEBSERVERLOG_H

#include <string>
#include <boost/filesystem/fstream.hpp>

using namespace std;

class WebServerLog {
 public:
    static WebServerLog* Instance();
    
    void EmitLog(string logline);
    void CloseLog();

 private:
    bool Initialized;
    bool OpenLogFile();

    boost::filesystem::ofstream FileStream;

    const static string LogDirectory;

    WebServerLog();
    WebServerLog(WebServerLog const&);
    WebServerLog& operator=(WebServerLog const&);
    static WebServerLog* m_pInstance;
};

#endif
