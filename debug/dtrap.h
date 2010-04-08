#ifndef _debugtrapheader
#define _debugtrapheader

#define DEBUG_LOG_LIST_LENGTH 50

typedef struct {
	char *list[DEBUG_LOG_LIST_LENGTH];
	int llen[DEBUG_LOG_LIST_LENGTH];
	int next;
} DEBUG_LOG;

extern DEBUG_LOG debug_log;

void debug_trap_init();
void execute_trap(BYTE id);

#endif
