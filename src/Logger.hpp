#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include "LogType.hpp"


namespace elmt {
	
		

	

	struct Message {
		std::string detail; //message contents
		LOGCAT category;// category; //combination of LOGCAT
		LOGSEV severity; //combination of LOGSEV
		std::time_t time; //message send time
	};


	

}

namespace elmt {
	class Log;
//	class Logger;
}


namespace elmt {
	class Logger
	{
	// Attributes
	private:
		friend class Log;
		

		inline static std::vector<Log*> logs = {};
		// Methods
	public:
		static int Print(const char* messageString, LOGCAT logCategory, LOGSEV logSeverity);
		static int Print(const std::string& messageString, LOGCAT logCategory, LOGSEV logSeverity);

		static Log* getLog(const char* logName);
		
	private:
		friend class core;
		static int cleanUp();
		static int flushAll(const char* logPath);
	};
}

