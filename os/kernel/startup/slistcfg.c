/*
	header files or defines
*/
#include "startup.h"
/*

	MAIN parser for the config file
	So works by a path per line.
	meaning each line has the path to the 
	binary executable we wanna start.
	Like this:

	CONFIG_START

	/MODULES/SYS/TEST.ELF
	/MODULES/TESTING/TESTINGTHELOL.ELF
	...

	CONFIG_END
	Also we only execute in ring 3 never ring 0, drivers will have
	thier own startup list, but changed by kernel
*/
void parse_startup_list(void) {
	/*
		Init some variables
	*/
    int fd;
    int rerrno;
    char buffer[MAX_LINE_LENGTH/*256*/];
    char line[MAX_LINE_LENGTH/*256*/];
    int line_pos = 0;
    int bytes_read;
    int processes_spawned = 0;

    #ifdef DEBUG
    printf("STARTUP: Parsing startup configuration file: %s\n", CONFIG_PATH);
    #endif
	/*
		Open the file. duh...
		As readonly
	*/
    fd = fat_open(CONFIG_PATH/*/KERNEL/CONFIGS/STARTLIST.CFG*/, 0, 0, &rerrno);
    if (fd < 0) {
        #ifdef DEBUG
        printf("STARTUP: Failed to open config file %s (error %d)\n", CONFIG_PATH, rerrno);
        #endif
        return;
    }

    #ifdef DEBUG
    printf("STARTUP: Config file opened successfully\n");
    #endif
	/*
		READ it. Also line by line to
		keep the path per line
	*/
    while ((bytes_read = fat_read(fd, buffer, sizeof(buffer), &rerrno)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            char c = buffer[i];
			/*
				SImple handler for line endings
			*/
            if (c == '\n' || c == '\r') {
                if (line_pos > 0) {
                    line[line_pos] = '\0'; /*NULL TERMINATION*/
					/*
						Trim the start
					*/
                    char* start = line;
                    while (*start && (*start == ' ' || *start == '\t')) start++;
					/*
						As well as the end
					*/
                    char* end = start + strlen(start) - 1;
                    while (end > start && (*end == ' ' || *end == '\t' || *end == '\r')) end--;
                    *(end + 1) = '\0';
					/*
						SKIP comments
					*/
                    if (*start && *start != '#') {
                        #ifdef DEBUG
                        printf("STARTUP: Spawning process: %s\n", start);
                        #endif
						/*
							Loop and spawn them
						*/
                        process_t* proc = spawn_process(start, THREAD_RING3);
                        if (proc) {
                            #ifdef DEBUG
                            printf("STARTUP: Successfully spawned PID=%d for %s\n", proc->pid, start);
                            #endif
							/*
								Also execute it
							*/
							execute_process(proc);
                            processes_spawned++/*+1*/;
                        } else {
                            #ifdef DEBUG
                            printf("STARTUP: Failed to spawn process: %s\n", start);
                            #endif
                        }
						/*
							TO BE FIXED:
							remove limit
						*/
                        if (processes_spawned >= MAX_PROCESSES_TO_SPAWN) {
                            #ifdef DEBUG
                            printf("STARTUP: Maximum process limit reached (%d)\n", MAX_PROCESSES_TO_SPAWN);
                            #endif
                            break;
                        }
                    }

                    line_pos = 0;
                }
            } else if (line_pos < MAX_LINE_LENGTH - 1) {
				/*
					Add back
				*/
                line[line_pos++] = c;
            }
        }

		/*
			Simple limit check
		*/
        if (processes_spawned >= MAX_PROCESSES_TO_SPAWN/*32?*/) {
            break;
        }
    }
	/*
		Also handle no newlines
		In case
	*/
    if (line_pos > 0) {
        line[line_pos] = '\0';
		/*
			Trim start
		*/
        char* start = line;
        while (*start && (*start == ' ' || *start == '\t')) start++;
		/*
			End too
		*/
        char* end = start + strlen(start) - 1;
        while (end > start && (*end == ' ' || *end == '\t' || *end == '\r')) end--;
        *(end + 1) = '\0';
		/*
			Ignore comments
		*/
        if (*start && *start != '#') {
            #ifdef DEBUG
            printf("STARTUP: Spawning final process: %s\n", start);
            #endif
			/*
				Spawn UP
			*/
            process_t* proc = spawn_process(start, THREAD_RING3/*always keep ring3 to avoid exploition*/);
            if (proc) {
                #ifdef DEBUG
                printf("STARTUP: Successfully spawned PID=%d for %s\n", proc->pid, start);
                #endif
				/*
					Execute!
				*/
				execute_process(proc);
                processes_spawned++;
            } else {
                #ifdef DEBUG
                printf("STARTUP: Failed to spawn final process: %s\n", start);
                #endif
            }
        }
    }

	/*
		CLOSE
	*/
    fat_close(fd, &rerrno/*prolly 0?*/);

    #ifdef DEBUG
    printf("STARTUP: configuration parsing complete. Spawned %d processes.\n", processes_spawned);
    #endif
}
