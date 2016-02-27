/**
@file
@brief logging system
@author Sergei Kachkov
*/
#pragma once

enum LOG_TYPE
{
	LOG_INFO,
	LOG_FAIL
};

/**
@brief opens log
*/
void InitLog ();
/**
@brief writes to log; format: [time] [label] [your message] 
@param type type of error: LOG_INFO - write note, LOG_FAIL - write error
@warning application automatically closes after writing with type LOG_FAIL
@param format_str format string like in printf ()
@note log forcedly writes information on disk; it can cause drops in performance
*/
void log (LOG_TYPE type, const char *format_str, ...);
void CloseLog ();