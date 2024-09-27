#include "Logger.hpp"
#include "Log.hpp"

#include <sstream>
#include <filesystem>
#include <iomanip>
#include <time.h>

using namespace elmt;

/*
Output a message
*/
std::ostream& elmt::operator <<(std::ostream& os, const elmt::Message& m)
{
	// Show severity as string
	std::string severity = "????";
	if ((bool)(m.severity & LOGSEV::SEV_ERROR))        { severity = "ERR"; }
	else if ((bool)(m.severity & LOGSEV::SEV_WARNING)) { severity = "WARN"; }
	else if ((bool)(m.severity & LOGSEV::SEV_INFO))    { severity = "INFO"; }
	else if ((bool)(m.severity & LOGSEV::SEV_TRACE))   { severity = "TRCE"; }
	else if ((bool)(m.severity & LOGSEV::SEV_ALL)) { severity = "ALL"; }

	 // Show category as string
	std::string messageType = "";
	unsigned int catCount = 0;
	if ((bool)(m.category & LOGCAT::CAT_LOADING))   { if (!messageType.empty()) { messageType.append("+"); } messageType.append("LOAD"); catCount++;}
	if ((bool)(m.category & LOGCAT::CAT_RENDERING)) { if (!messageType.empty()) { messageType.append("+"); } messageType.append("REND"); catCount++;}
	if ((bool)(m.category & LOGCAT::CAT_PHYSICS))   { if (!messageType.empty()) { messageType.append("+"); } messageType.append("PHYS"); catCount++;}
	if ((bool)(m.category & LOGCAT::CAT_CORE))      { if (!messageType.empty()) { messageType.append("+"); } messageType.append("CORE"); catCount++;}
	if ((bool)(m.category & LOGCAT::CAT_MISC))      { if (!messageType.empty()) { messageType.append("+"); } messageType.append("MISC"); catCount++;}
	if ((bool)(m.category & LOGCAT::CAT_AUDIO))     { if (!messageType.empty()) { messageType.append("+"); } messageType.append("AUDI"); catCount++; }
	if ((bool)(m.category & LOGCAT::CAT_LOGIC))     { if (!messageType.empty()) { messageType.append("+"); } messageType.append("LOGC"); catCount++;}
	if ((bool)(m.category & LOGCAT::CAT_ALL))   { if (!messageType.empty()) { messageType.append("+"); } messageType.append("ALL" ); catCount++;}

	// Show time as string
	char messageTime[32];
	std::strftime(messageTime, 32, "%X|%e/%m", localtime(&m.time) );

	int sevWidth = 4;

	int typeWidth;
	if (catCount) {
		typeWidth = ((catCount) * 5)-1;
	}
	else {
		typeWidth = 4;
	}
	

	os << "[" << std::setw(sevWidth) << severity << "|";
	//os << " ";
	os << std::setw(typeWidth) << messageType << "|";
	//os << " ";
	os << messageTime << "]";
	os << " ";
	os << m.detail;
	return os;
}

Log::Log(const char* name, LOGCAT logCategory, LOGSEV logSeverity, unsigned int autoFlushMessageCount, const char* autoFlushFilename, bool toCout, bool toCerr) :
	name(name), logCategory(logCategory), logSeverity(logSeverity), autoFlushMessageCount(autoFlushMessageCount), autoFlushFilename(autoFlushFilename), toCout(toCout), toCerr(toCerr)
{

	Logger::logs.push_back(this);


	maxAutoFlushCharacters = 1000;
	autoFlushCharacterCount = 0;

	// Open auto logging file and count existing characters
	// This isn't perfect since it takes into account date character count and stuff,
	// which standard cautoFlushCharacterCount addition doesn't do
	std::filesystem::path filePath(autoFlushFilename);
	if (std::filesystem::exists(filePath)) {
		auto fileSize = std::filesystem::file_size(filePath);
		autoFlushCharacterCount += fileSize;
	}
	
}
Log::Log(const char* name, LOGCAT logCategory, LOGSEV logSeverity, unsigned int autoFlushMessageCount, const char* autoFlushFilename) :
	Log(name, logCategory, logSeverity, autoFlushMessageCount, autoFlushFilename, true, false) {}

Log::Log(const char* name, LOGCAT logCategory, LOGSEV logSeverity, bool toCout, bool toCerr) : Log(name, logCategory, logSeverity, 32, name, toCout, toCerr) {}
Log::Log(const char* name, LOGCAT logCategory, LOGSEV logSeverity) : Log(name, logCategory, logSeverity, 32, name, false, false) {}

/*
Add a message to this log
*/
int Log::addMessage(Message message)
{
	messages.push_back(message);

	// Regularly, automatically flush the contents of the log to prevent it from building up
	if (autoFlushMessageCount) {
		if (messages.size() >= autoFlushMessageCount) {

			doAutoFlush(&message);
		}
	}

	if (toCout) {
		std::cout << message << std::endl;
	}
	if (toCerr) {
		std::cerr << message << std::endl;
	}

	return 0;
}



/*
Flush this log's messages to the file referred to by autoFlushFilename
*/
int Log::doAutoFlush(Message* lastMessage) {
	// Check whether to clear the auto flush file first
	std::ios_base::openmode openType;
	if (autoFlushCharacterCount >= maxAutoFlushCharacters) {
		openType = std::ios_base::out;
		autoFlushCharacterCount = 0;
	}
	else {
		openType = std::ios_base::app;
	}

	if (lastMessage) {
		autoFlushCharacterCount += lastMessage->detail.size();
	}

	flushToFile(autoFlushFilename.c_str(), openType);

	return 0;
}

/*
Concatenate all sent message since last flush into a single string
*/
std::string Log::getAllMessages()
{
	std::stringstream outStream;

	std::string out = "";
	for (Message& message : messages) {
		outStream << message << "\n";
	}
	out = outStream.str();
	return out;
}

/*
Output all stored messages to a file and clear
*/
int Log::flushToFile(const char* fileName, std::ios_base::openmode openType)
{
	std::string out = getAllMessages();
	
	messages.clear();

	std::ofstream outFile;
	outFile.open(fileName, openType);
	outFile << out;
	outFile.close();

	return 0;
}

