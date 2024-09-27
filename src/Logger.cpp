#include "Logger.hpp"
#include "Log.hpp"


#include <chrono>

using namespace elmt;




/*
Output a message to (a) log(s)
*/
int Logger::Print(const char* messageString, LOGCAT logCategory, LOGSEV logSeverity)
{
	Message message;
	message.detail = messageString;
	message.time = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
	message.category = logCategory;
	message.severity = logSeverity;

	bool foundLog = false;


	for (Log* log : logs) {
		// Check to see if we want to output to this log
		if ( ((bool)(log->logCategory & logCategory) || (bool)(logCategory & LOGCAT::CAT_ALL) || (bool)(log->logCategory & LOGCAT::CAT_ALL) ) &&
			 ((bool)(log->logSeverity & logSeverity) || (bool)(logSeverity & LOGSEV::SEV_ALL) || (bool)(log->logSeverity & LOGSEV::SEV_ALL) )
			) {
			
			foundLog = true;
			log->addMessage(message);
		}
	}

	if (!foundLog) {

	}
	return 0;
}

/*
Output a message to (a) log(s)
*/
int Logger::Print(const std::string& messageString, LOGCAT logCategory, LOGSEV logSeverity) {
	return Print(messageString.c_str(), logCategory, logSeverity);
}

/*
Get the first log who's name that matches the given name
If no log is found, nullptr is returned
*/
Log* Logger::getLog(const char* logName)
{
	for (Log* log : logs) {
		if (log->name == logName) {
			return log;
		}
	}
	return nullptr;
}

/*
Delete all logs and clean up the logging system
THis is called automatically by core
*/
int Logger::cleanUp()
{
	// Perform final flush
	flushAll("logs");

	for (Log* log : logs) {
		delete log;
	}
	logs.clear();
	return 0;
}

/*
Flush every log to a file based off it's name (logPath/LOGNAME.log)
*/
int Logger::flushAll(const char* logPath)
{
	// From https://stackoverflow.com/questions/12971499/how-to-get-the-file-separator-symbol-in-standard-c-c-or
	const char* kPathSeparator =
	#ifdef _WIN32
			"\\";
	#else
			"/";
	#endif


	for (Log* log : logs) {
		std::string logFilePath;
		//logFilePath.append(logPath);
		//logFilePath.append(kPathSeparator);
		logFilePath.append(log->autoFlushFilename);
		//logFilePath.append(".");
		//logFilePath.append("log");
		log->flushToFile(logFilePath.c_str());
	}
	return 0;
}
