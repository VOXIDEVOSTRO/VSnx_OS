#ifndef STARTUP_H
#define STARTUP_H
/*
	Header files
*/
#include "../fat32/gristle.h" // For fat
#include "../elf/process.h" // For spawn process
#include "../threading/thread.h"
#include "../utilities/utility.h" 
#include <stdint.h>
/*
	Constants for the startup
*/
#define CONFIG_PATH "/KERNEL/CONFIGS/STARTLIST.CFG" /*fixed place*/
#define MAX_LINE_LENGTH 256
#define MAX_PROCESSES_TO_SPAWN 32/*should be unlimited*/
/*
	Prototypes
*/
void parse_startup_list(void);
#endif
