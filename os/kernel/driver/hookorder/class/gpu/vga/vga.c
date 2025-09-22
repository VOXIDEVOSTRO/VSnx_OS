/*
	Headz
*/
#include "vga.h"
/*
	Main for VGA
*/
int vga_spawn_driver(void) {
	#ifdef DEBUG
	printf("Vga hookorder started\n");
	#endif
    int found = 0;
	/*
		MODE 12h
	*/
    if (vga_detect_mode_12h() == 1) {
        #ifdef DEBUG
        printf("[vga] VGA mode 12h detected\n");
        #endif
        found = 1;
    }
	/*
		MODE 13h
	*/
    if (vga_detect_mode_13h() == 1) {
        #ifdef DEBUG
        printf("[vga] VGA mode 13h detected\n");
        #endif
        found = 1;
    }
	/*
		any found?
	*/
    if (!found) {
        #ifdef DEBUG
        printf("[vga] No VGA modes detected, skipping\n");
        #endif
        return 0;
    }
	/*
		Is hookorder valid?
	*/
    if (hookorder_count() == 0) {
        int loaded = hookorder_load(NULL);
        if (loaded < 0) {
            #ifdef DEBUG
            printf("[vga] hookorder load failed (%d)\n", loaded);
            #endif
			/*Nope*/
            return -1;
        }
    }
	/*
		Spawn up the thread!
	*/
    process_t* proc = NULL;
    int rc = hookorder_spawn_for("GPU", "VGA", &proc);
    if (rc < 0) {
		/*
			Failure
		*/
        #ifdef DEBUG
        printf("[vga] Failed to spawn VGA driver (rc=%d)\n", rc);
        #endif
        return -3;
    }
	/*
		DONE!
	*/
    #ifdef DEBUG
    printf("[vga] VGA driver spawned successfully (pid=%u)\n", proc->pid);
    #endif
    return 1;
}
