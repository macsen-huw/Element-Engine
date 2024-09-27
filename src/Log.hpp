#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include "LogType.hpp"

namespace elmt {
	struct Message;
	class Logger;
}


namespace elmt {

	class Log
	{
		friend std::ostream& operator<< (std::ostream& os, const Message& m);
		friend class Logger;


		// Properties
	public:
		// The number of messages that can be logged before an automatic flush occurs
		// If set to 0, no automatic flushing occurs
		unsigned int autoFlushMessageCount;

		// Maximum number of characters allowed in the auto flush file
		// Once this is exceeded, the file will be cleared to preserve storage space
		unsigned int maxAutoFlushCharacters;
	private:
		std::string name;

		// Set this to a combination of LOGSEV
		LOGSEV logSeverity;
		// Set this to a combination of LOGCAT
		LOGCAT logCategory;

		// All the messages sent to this log
		std::vector<Message> messages;
		std::string autoFlushFilename;

		// The amount of characters that have been sent to the log,
		// since the last time the auto flush file was cleared
		unsigned int autoFlushCharacterCount;

		// Whether to send new messages to cout or cerr
		bool toCout;
		bool toCerr;

		// Methods
	public:
		Log(const char* name, LOGCAT logCategory, LOGSEV logSeverity, unsigned int autoFlushMessageCount, const char* autoFlushFilename, bool toCout, bool toCerr);
		Log(const char* name, LOGCAT logCategory, LOGSEV logSeverity, unsigned int autoFlushMessageCount, const char* autoFlushFilename);

		Log(const char* name, LOGCAT logCategory, LOGSEV logSeverity, bool toCout, bool toCerr);
		Log(const char* name, LOGCAT logCategory, LOGSEV logSeverity);

		LOGCAT getCategory() { return logCategory; };
		LOGSEV getSeverity() { return logSeverity; };
		const char* getName() { return name.c_str(); };
		const char* getAutoFlushFilename() { return autoFlushFilename.c_str(); };

		int addMessage(Message message);
		std::string getAllMessages();

		int flushToFile(const char* fileName, std::ios_base::openmode openType);
		int flushToFile(const char* fileName) { return flushToFile(fileName, std::ios_base::out); };

	private:
		int doAutoFlush(Message* lastMessage);

	};

}
