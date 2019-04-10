#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <mutex>
#define USING_LOGGER   				(1)

//enable/disable logger in specific parts

#include <fstream>
#include <string.h>
#include <pthread.h>
#if USING_LOGGER
#define USING_LOG_COLLECTOR         (1)
#define USING_LOG_OPCUA     		(1)
#define USING_LOG_OPCSERVER         (1)
#define USING_LOG_HANDLER   		(1)
#define USING_LOG_JOBMANAGER   		(1)
#define USING_LOG_SCHEDULER         (1)
#define USING_LOG_WEBXMLREADER      (0)

#define MAX_NUM_LOGFILE             (10 + 2) // 10 files pluse dir . and ..
#define LOG_PATH                 "/home/pi/BfxFundingBot/logs"
#define DEFAULT_LOG_LEVEL        LEVEL_ALL
#define DEFAULT_LOG_TYPE         TYPE_FILE

class Logger
{
public:
typedef enum
{
	LEVEL_ALL,		//All levels
	LEVEL_TRACE,	//finer-grained informational events than the DEBUG
	LEVEL_DEBUG,	//fine-grained informational events that are most useful to debug
	LEVEL_INFO,		//informational messages that highlight the progress of the application
	LEVEL_WARN,		//potentially harmful situations
	LEVEL_ERROR,	//error events that might still allow the application to continue running
	LEVEL_FATAL,	//very severe error events
	LEVEL_OFF		//turn off logging
}LogLevel;

typedef enum
{
	TYPE_NOLOG,
	TYPE_CONSOLE,
	TYPE_FILE,
	TYPE_BOTH_FILECONSOLE
}LogType;

public:
    static Logger* getInstance();
    ~Logger();

    void trace (const char* text) { log("[TRACE]: ",  LEVEL_TRACE, text); }
    void debug (const char* text) { log("[DEBUG]: ",  LEVEL_DEBUG, text); }
    void info  (const char* text) { log("[INFO]: ",   LEVEL_INFO,  text); }
    void warn  (const char* text) { log("[WARN]: ",   LEVEL_WARN,  text); }
    void error (const char* text) { log("[ERROR]: ",  LEVEL_ERROR, text); }
    void fatal (const char* text) { log("[FATAL]: ",  LEVEL_FATAL, text); }

    void trace (const std::string& text) { trace(text.data()); }
    void debug (const std::string& text) { debug(text.data()); }
    void info  (const std::string& text) { info (text.data()); }
    void warn  (const std::string& text) { warn (text.data()); }
    void error (const std::string& text) { error(text.data()); }
    void fatal (const std::string& text) { fatal(text.data()); }

    void setLevel(const LogLevel level) { m_LogLevel = level; }
    void enableLog() { m_LogLevel = LEVEL_ALL; }
    void disableLog(){ m_LogLevel = LEVEL_OFF; }

    void setType(const LogType type){ m_LogType = type; }
    void enableConsoleLog(){ m_LogType = TYPE_CONSOLE; }
    void enableFileLog()   { m_LogType = TYPE_FILE;}

private:
    static Logger*          m_Instance;
    std::ofstream           m_File;

    std::mutex              m_Mutex;

    LogLevel                m_LogLevel;
    LogType                 m_LogType;

    std::string             m_LogFileName;
private:
    Logger();
    Logger(const Logger& obj);
    void operator=(const Logger& obj);

    std::string getCurrentTime()
    {
        time_t now = time(0);
    	std::string tmp(ctime(&now));

    	// remove the ended "\n"
        return tmp.substr(0, tmp.size() - 1);
    }

    void log(const char* strLevel, const LogLevel level, const char* logText);
};

  #define LOG_TRACE(x)    	 Logger::getInstance()->trace(x)
  #define LOG_DEBUG(x)    	 Logger::getInstance()->debug(x)
  #define LOG_INFO(x)     	 Logger::getInstance()->info (x)
  #define LOG_WARN(x)	     Logger::getInstance()->warn (x)
  #define LOG_ERROR(x)    	 Logger::getInstance()->error(x)
  #define LOG_FATAL(x)    	 Logger::getInstance()->fatal(x)
#else //USING_LOGGER
  #define LOG_TRACE(x)
  #define LOG_DEBUG(x)
  #define LOG_INFO(x)
  #define LOG_WARN(x)
  #define LOG_ERROR(x)
  #define LOG_FATAL(x)
#endif //end of USING_LOGGER

#if USING_LOG_COLLECTOR
  #define STR_COLLECTOR            std::string("<Collector>: ")
  #define LOG_COLLECTOR_TRACE(x)   LOG_TRACE(STR_COLLECTOR + x)
  #define LOG_COLLECTOR_DEBUG(x)   LOG_DEBUG(STR_COLLECTOR + x)
  #define LOG_COLLECTOR_INFO(x)    LOG_INFO(STR_COLLECTOR + x)
  #define LOG_COLLECTOR_WARN(x)    LOG_WARN(STR_COLLECTOR + x)
  #define LOG_COLLECTOR_ERROR(x)   LOG_ERROR(STR_COLLECTOR + x)
  #define LOG_COLLECTOR_FATAL(x)   LOG_FATAL(STR_COLLECTOR + x)
#else //USING_LOG_COLLECTOR
  #define LOG_COLLECTOR_TRACE(x)
  #define LOG_COLLECTOR_DEBUG(x)
  #define LOG_COLLECTOR_INFO(x)
  #define LOG_COLLECTOR_WARN(x)
  #define LOG_COLLECTOR_ERROR(x)
  #define LOG_COLLECTOR_FATAL(x)
#endif //END OF USING_LOG_COLLECTOR

#if USING_LOG_OPCUA
  #define STROPCUA        	 std::string("<OpcUaCollector>: ")
  #define LOG_OPCUA_TRACE(x)   LOG_TRACE(STROPCUA + x)
  #define LOG_OPCUA_DEBUG(x)   LOG_DEBUG(STROPCUA + x)
  #define LOG_OPCUA_INFO(x)    LOG_INFO(STROPCUA + x)
  #define LOG_OPCUA_WARN(x)    LOG_WARN(STROPCUA + x)
  #define LOG_OPCUA_ERROR(x)   LOG_ERROR(STROPCUA + x)
  #define LOG_OPCUA_FATAL(x)   LOG_FATAL(STROPCUA + x)
#else //USING_LOG_OPCUA
  #define LOG_OPCUA_TRACE(x)
  #define LOG_OPCUA_DEBUG(x)
  #define LOG_OPCUA_INFO(x)
  #define LOG_OPCUA_WARN(x)
  #define LOG_OPCUA_ERROR(x)
  #define LOG_OPCUA_FATAL(x)
#endif //END OF USING_LOG_OPCUA

#if USING_LOG_OPCSERVER
  #define STROPCSERVER         std::string("<OpcUaServer>: ")
  #define LOG_OPCSERVER_TRACE(x)   LOG_TRACE(STROPCSERVER + x)
  #define LOG_OPCSERVER_DEBUG(x)   LOG_DEBUG(STROPCSERVER + x)
  #define LOG_OPCSERVER_INFO(x)    LOG_INFO(STROPCSERVER + x)
  #define LOG_OPCSERVER_WARN(x)    LOG_WARN(STROPCSERVER + x)
  #define LOG_OPCSERVER_ERROR(x)   LOG_ERROR(STROPCSERVER + x)
  #define LOG_OPCSERVER_FATAL(x)   LOG_FATAL(STROPCSERVER + x)
#else //USING_LOG_OPCSERVER
  #define LOG_OPCSERVER_TRACE(x)
  #define LOG_OPCSERVER_DEBUG(x)
  #define LOG_OPCSERVER_INFO(x)
  #define LOG_OPCSERVER_WARN(x)
  #define LOG_OPCSERVER_ERROR(x)
  #define LOG_OPCSERVER_FATAL(x)
#endif //END OF USING_LOG_OPCSERVER


#if USING_LOG_HANDLER
  #define STRHANDLER        	   std::string("<Handler>: ")
  #define LOG_HANDLER_TRACE(x)   LOG_TRACE(STRHANDLER + x)
  #define LOG_HANDLER_DEBUG(x)   LOG_DEBUG(STRHANDLER + x)
  #define LOG_HANDLER_INFO(x)    LOG_INFO(STRHANDLER + x)
  #define LOG_HANDLER_WARN(x)    LOG_WARN(STRHANDLER + x)
  #define LOG_HANDLER_ERROR(x)   LOG_ERROR(STRHANDLER + x)
  #define LOG_HANDLER_FATAL(x)   LOG_FATAL(STRHANDLER + x)
#else //USING_LOG_HANDLER
  #define LOG_HANDLER_TRACE(x)
  #define LOG_HANDLER_DEBUG(x)
  #define LOG_HANDLER_INFO(x)
  #define LOG_HANDLER_WARN(x)
  #define LOG_HANDLER_ERROR(x)
  #define LOG_HANDLER_FATAL(x)
#endif //END OF USING_LOG_HANDLER

#if USING_LOG_JOBMANAGER
  #define STRJOBMGR	          	    std::string("<JobManager>: ")
  #define LOG_JOBMANAGER_TRACE(x)   LOG_TRACE(STRJOBMGR + x)
  #define LOG_JOBMANAGER_DEBUG(x)   LOG_DEBUG(STRJOBMGR + x)
  #define LOG_JOBMANAGER_INFO(x)    LOG_INFO(STRJOBMGR + x)
  #define LOG_JOBMANAGER_WARN(x)    LOG_WARN(STRJOBMGR + x)
  #define LOG_JOBMANAGER_ERROR(x)   LOG_ERROR(STRJOBMGR + x)
  #define LOG_JOBMANAGER_FATAL(x)   LOG_FATAL(STRJOBMGR + x)
#else //USING_LOG_JOBMANAGER
  #define LOG_JOBMANAGER_TRACE(x)
  #define LOG_JOBMANAGER_DEBUG(x)
  #define LOG_JOBMANAGER_INFO(x)
  #define LOG_JOBMANAGER_WARN(x)
  #define LOG_JOBMANAGER_ERROR(x)
  #define LOG_JOBMANAGER_FATAL(x)
#endif //END OF USING_LOG_JOBMANAGER

#if USING_LOG_SCHEDULER
  #define STRSCHEDULER	           std::string("<Scheduler>: ")
  #define LOG_SCHEDULER_TRACE(x)   LOG_TRACE(STRSCHEDULER + x)
  #define LOG_SCHEDULER_DEBUG(x)   LOG_DEBUG(STRSCHEDULER + x)
  #define LOG_SCHEDULER_INFO(x)    LOG_INFO(STRSCHEDULER + x)
  #define LOG_SCHEDULER_WARN(x)    LOG_WARN(STRSCHEDULER + x)
  #define LOG_SCHEDULER_ERROR(x)   LOG_ERROR(STRSCHEDULER + x)
  #define LOG_SCHEDULER_FATAL(x)   LOG_FATAL(STRSCHEDULER + x)
#else //USING_LOG_SCHEDULER
  #define LOG_SCHEDULER_TRACE(x)
  #define LOG_SCHEDULER_DEBUG(x)
  #define LOG_SCHEDULER_INFO(x)
  #define LOG_SCHEDULER_WARN(x)
  #define LOG_SCHEDULER_ERROR(x)
  #define LOG_SCHEDULER_FATAL(x)
#endif //END OF USING_LOG_SCHEDULER

#if USING_LOG_WEBXMLREADER
  #define STRWEBXMLREADER   	      std::string("<WebXmlReader>: ")
  #define LOG_WEBXMLREADER_TRACE(x)   LOG_TRACE(STRWEBXMLREADER + x)
  #define LOG_WEBXMLREADER_DEBUG(x)   LOG_DEBUG(STRWEBXMLREADER + x)
  #define LOG_WEBXMLREADER_INFO(x)    LOG_INFO(STRWEBXMLREADER + x)
  #define LOG_WEBXMLREADER_WARN(x)    LOG_WARN(STRWEBXMLREADER + x)
  #define LOG_WEBXMLREADER_ERROR(x)   LOG_ERROR(STRWEBXMLREADER + x)
  #define LOG_WEBXMLREADER_FATAL(x)   LOG_FATAL(STRWEBXMLREADER + x)
#else //USING_LOG_HANDLER
  #define LOG_WEBXMLREADER_TRACE(x)
  #define LOG_WEBXMLREADER_DEBUG(x)
  #define LOG_WEBXMLREADER_INFO(x)
  #define LOG_WEBXMLREADER_WARN(x)
  #define LOG_WEBXMLREADER_ERROR(x)
  #define LOG_WEBXMLREADER_FATAL(x)
#endif //END OF USING_LOG_HANDLER

std::string uint2string(unsigned int num, unsigned int base);
#endif // End of _LOGGER_H_

