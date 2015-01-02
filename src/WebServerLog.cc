#include "WebServerLog.h"

#include <iostream>
#include <boost/filesystem.hpp>

const string WebServerLog::LogDirectory = "WebAccessLogs";

WebServerLog* WebServerLog::m_pInstance = NULL;

WebServerLog* WebServerLog::Instance() {
    if (!m_pInstance) {
	m_pInstance = new WebServerLog();
    }
    return m_pInstance;
}

WebServerLog::WebServerLog() : Initialized(false) {}

void WebServerLog::EmitLog(string logline) {
    if (Initialized == false) {
	if (OpenLogFile() == false) {
	    cerr << "Can't open log file" << endl;
	    return;
	}
	Initialized = true;
    }
    FileStream << logline << endl;
}

bool WebServerLog::OpenLogFile() {
    boost::filesystem::path dir(LogDirectory);
    boost::system::error_code ec;
    if (boost::filesystem::exists(dir) == false && boost::filesystem::create_directory(dir, ec) == false) {
	cerr << ec.message() << endl;
	return false;
    }

    char logFileName[128];
    memset(logFileName, 128, '\0');
    const time_t now = time(NULL);
    struct tm *timeInfo = gmtime(&now);
    strftime(logFileName, 127, "access_%Y%m%d_%H%M%S.txt", timeInfo);

    FileStream.open(dir / string(logFileName));

    return true;
}


void WebServerLog::CloseLog() {
    Initialized = false;
}
