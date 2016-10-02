#include <StdInc.h>
#include <AyriaLog.h>

#include <mutex>
#include <thread> // for lock_guard
#include <vector>

#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>

inline uint64_t GetSystemTimer()
{
	return GetTickCount64();
}
#endif

#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)

#ifndef AYRIA_EXTENSION_NAME
#define AYRIA_EXTENSION_NAME UnknownExt
#endif

#ifndef BUILDHOST
#define BUILDHOST_STRING ""
#else
#define BUILDHOST_STRING "on " STRINGIFY(BUILDHOST) " "
#endif

static constexpr const char* GetLogPath()
{
	return "Plugins/Logs/" STRINGIFY(AYRIA_EXTENSION_NAME) ".log";
}

static constexpr FILE* InvalidLogFile = (FILE*)-1;

void Trace(const char* format, ...)
{
	static FILE* logFile;
	static std::mutex logMutex;

	// open the log file
	if (!logFile)
	{
		static std::once_flag initFlag;
		static std::once_flag bannerFlag;

		std::call_once(initFlag, [] ()
		{
			if ((logFile =_fsopen(GetLogPath(), "w", _SH_DENYNO)) == nullptr)
			{
				logFile = InvalidLogFile;
			}
		});

		std::call_once(bannerFlag, [] ()
		{
			Trace("%s initializing (built %sat %s %s in %s configuration)\n",
				  STRINGIFY(AYRIA_EXTENSION_NAME),
				  BUILDHOST_STRING,
				  __DATE__,
				  __TIME__,
#ifdef _DEBUG
				  "Debug",
#else
				  "Release",
#endif
				  nullptr);
		});
	}

	// enter the logging mutex
	std::lock_guard<std::mutex> lock(logMutex);

	// format the log string
	static thread_local std::vector<char> logBuffer(32768);
	uint32_t lengthWritten;

	{
		va_list ap;

		va_start(ap, format);
		lengthWritten = _vsnprintf(&logBuffer[0], logBuffer.size() - 1, format, ap);
		va_end(ap);

		// retry if the buffer is too small
		if (lengthWritten >= logBuffer.size())
		{
			logBuffer.resize(lengthWritten + 1);

			va_start(ap, format);
			lengthWritten = _vsnprintf(&logBuffer[0], logBuffer.size() - 1, format, ap);
			va_end(ap);
		}

		// if it failed, return
		if (lengthWritten < 0)
		{
			return;
		}
	}

	// write to the file
	if (logFile != InvalidLogFile)
	{
		// TODO: only print time if there's a new line
		static uint64_t startTimer = GetSystemTimer();

		fprintf(logFile, "[%08llu] %s", GetSystemTimer() - startTimer, logBuffer.data());
		fflush(logFile);
	}

	// write to a console
	{
		fprintf(stdout, "%s", logBuffer.data());
		fflush(stdout);
	}

	// write to debuggers
	OutputDebugStringA(logBuffer.data());
}