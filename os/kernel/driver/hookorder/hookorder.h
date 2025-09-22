#ifndef HOOKORDER_H
#define HOOKORDER_H
/*
	header files
*/
#include <stdint.h>
#include <stddef.h>
#include "../../elf/process.h" /*Spawning of driver*/
#include "../../threading/thread.h" /*And threading stuff*/
#include "../../utilities/utility.h" /*printf... what more can i say?*/
#include "../../hal/pci/pci.h"
/*
	Main Constants
*/
#define HOOKORDER_MAX_ENTRIES   256
#define HOOKORDER_MAX_TOKEN     64
#define HOOKORDER_MAX_PATH      260
#define HOOKORDER_MAX_CFG_SIZE  (64*1024)/*64kb*/
/*
	Main struct for each line
	follwing this format
	CLASS:<Subclass>:<driver>:<path_to_driver>
*/
typedef struct hookorder_entry {
    char category[HOOKORDER_MAX_TOKEN]; /*Fixed CLASS*/
    char subclass[HOOKORDER_MAX_TOKEN]; /*SUBCLASS*/
    char driver[HOOKORDER_MAX_TOKEN]; /*The driver*/
    char path[HOOKORDER_MAX_PATH]; /*The path to driver*/
	int  claimed;                       /* 0 = not claimed, 1 = claimed */
	/*
		So standard path of the drivers is:
		/MODULES/SYS/DRIVERS/
		and the cfg file is in /KERNEL/CONFIGS/HOOKORDER.CFG
	*/
} hookorder_entry_t;
/*
	Globals
*/
extern hookorder_entry_t g_entries[HOOKORDER_MAX_ENTRIES];
extern int g_entry_count;
extern char g_cfg_cache[HOOKORDER_MAX_CFG_SIZE];
extern const char* g_cfg_path;
/*
	Prototypes
*/
int hookorder_load(const char *path);
int hookorder_reload(void);
int hookorder_count(void);
const hookorder_entry_t *hookorder_get(int i);
const hookorder_entry_t *hookorder_find_exact(const char *subclass, const char *driver);
const hookorder_entry_t *hookorder_find_best(const char *subclass, const char *driver);
int hookorder_find_path(const char *subclass, const char *driver, char *out, size_t outsz);
int hookorder_spawn_for(const char* subclass, const char* driver, process_t** out_proc);
/*
	Main start
*/
int start_hookorder(void);
#endif