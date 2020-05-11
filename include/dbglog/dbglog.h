#ifndef DBG_LOG
#define DBG_LOG 1

#include "log.h"

#define DBGLOG_TRACE(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define DBGLOG_DEBUG(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)


#endif /* DBG_LOG */
