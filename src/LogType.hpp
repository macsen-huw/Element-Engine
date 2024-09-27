#pragma once

#ifndef LOGTYPE_HPP
#define LOGTYPE_HPP

namespace elmt
{

	/*
	Flags for logging.
	Created logs can use a combination of these flags to log different kinds of errors
	Created messages can use a combination of these flags to have the message sent to different logs
	Any created message should, ideally, have at least one SEVERITY and one CATEGORY flag set
	Combine flags using the "+" or "|" operators
	*/
	// SEVERITY
	enum class LOGSEV : unsigned int {
		SEV_ALL = 1, // Special case. A Message with the "SEV_ALL" flag set will be sent to all logs. A log with the "SEV_ALL" flag set will receive all messages

		SEV_INFO = 2, // General information, no necessarily bad
		SEV_WARNING = 4, // An issue which may cause a problem in future, but which might not be immediately critical
		SEV_ERROR = 8, // An issue which breaks engine functionality in some way, and must be handled ASAP
		SEV_TRACE = 16 // Verbose information, usually only used when debugging. Could be used several times in a function to record function progress, for example
	};

	// CATEGORY
	enum class LOGCAT : unsigned int {
		CAT_ALL = 1, // Special case. A Message with the "CAT_ALL" flag set will be sent to all logs. A log with the "CAT_ALL" flag set will receive all messages

		CAT_LOADING = 2, // To do with asset management
		CAT_RENDERING = 4, // To do with rendering
		CAT_PHYSICS = 8, // To do with physics, collision, etc
		CAT_CORE = 16, // To do with core systems, like engine setup/teardown, Entity/Components updates, etc
		CAT_LOGIC = 32, // To do with game-specific logic, like player control or enemy AI
		CAT_AUDIO = 64, // To do with audio
		CAT_MISC = 128 // Does not belong to any of the above categories
	};

	/*
	unsigned int operator| (LOGSEV a, LOGSEV b) {
		return ((unsigned int)a | (unsigned int)b);
	}
	unsigned int operator| (LOGCAT a, LOGCAT b) {
		return ((unsigned int)a | (unsigned int)b);
	}
	*/
	inline LOGSEV operator| (LOGSEV a, LOGSEV b) {
		return (LOGSEV)((unsigned int)a | (unsigned int)b);
	}
	inline LOGCAT operator| (LOGCAT a, LOGCAT b) {
		return (LOGCAT)((unsigned int)a | (unsigned int)b);
	}

	inline LOGSEV operator& (LOGSEV a, LOGSEV b) {
		return (LOGSEV)((unsigned int)a & (unsigned int)b);
	}
	inline LOGCAT operator& (LOGCAT a, LOGCAT b) {
		return (LOGCAT)((unsigned int)a & (unsigned int)b);
	}

	inline unsigned int operator| (unsigned int a, LOGSEV b) {
		return (a | (unsigned int)b);
	}

	inline unsigned int operator& (unsigned int a, LOGSEV b) {
		return (a & (unsigned int)b);
	}

	inline unsigned int operator| (unsigned int a, LOGCAT b) {
		return (a | (unsigned int)b);
	}

	inline unsigned int operator& (unsigned int a, LOGCAT b) {
		return (a & (unsigned int)b);
	}

};

#endif