#include "Log.h"

#include <iostream>
#include <sstream>
#include <cstdarg>
#include <cstdio>

using namespace std;

const string Logger::LegalTagCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_:-+.";

Logger* Logger::m_pInstance = NULL;
Logger* Logger::Instance() {
    if (!m_pInstance) {
	m_pInstance = new Logger();
    }
    return m_pInstance;
}

Logger::Logger() {
    LoggingLevel = Warn;
}

void Logger::SetLoggingLevel(Level level) {
    LoggingLevel = level;
}

enum Logger::Level Logger::GetLoggingLevel() {
    return LoggingLevel;
}

bool Logger::SetTagsFromCsv(string listCsv) {
    stringstream optstream(listCsv);
    string val;
    while (getline(optstream, val, ',')) {
	if (SetTag(val) == false) {
	    return false;
	}
    }
    return true;
}

bool Logger::SetTag(string tag) {
    if (tag.size() < 1) { 
	return true;
    }

    for (string::iterator itr = tag.begin(); itr != tag.end(); ++itr) {
	if (LegalTagCharacters.find(*itr) == string::npos) {
	    cout << "ERROR: illegal characters in tag, '" << tag << "'; legal characters are [A-Za-z0-9_:-+.]" << endl;
	    return false;
	}
    }

    Tags.insert(tag);

    return true;
}

void Logger::RemoveTag(string tag) {
}

void Logger::Log(Level level, string tag, const char *fmt, ...) {
    if (level > LoggingLevel) {
	return;
    }
    if (tag.size() > 0 && Tags.size() > 0 && Tags.find(tag) == Tags.end()) {
	return;
    }

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
