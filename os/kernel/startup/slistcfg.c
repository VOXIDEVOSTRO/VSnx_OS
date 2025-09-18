/*
	header files or defines
*/
#include "startup.h"
/*
	Helper function to parse path and priority from line
	just to handle and get the priority
*/
void parse_line(const char* line, char* path, thread_priority_t* priority) {
	/*
		Copy
	*/
    char temp[MAX_LINE_LENGTH];
    strcpy(temp, line);
	/*
		Find the last space on the line
	*/
    char* last_space = strrchr(temp, ' ');
    if (last_space) {
        *last_space = '\0';/*NULL TERMINATE*/
        strcpy(path, temp); 
		/*
			After the line. we have the priority
		*/
        char* pri_str = last_space + 1;
		/*
			Handle the lows
		*/
        if (strcmp(pri_str, "LOW") == 0) {
            *priority = THREAD_PRIORITY_LOW;
		/*
			handle normals
		*/
        } else if (strcmp(pri_str, "NORMAL") == 0) {
            *priority = THREAD_PRIORITY_NORMAL;
		/*
			Aswell as the HIGHs
		*/
        } else if (strcmp(pri_str, "HIGH") == 0) {
            *priority = THREAD_PRIORITY_HIGH;
		/*
			Ultras too
		*/
		} else if (strcmp(pri_str, "ULTRA") == 0) {
            *priority = THREAD_PRIORITY_ULTRA;
		/*
			And backbenchers
		*/
		} else if (strcmp(pri_str, "BACKGROUND") == 0) {
            *priority = THREAD_PRIORITY_BACKGROUND;
		/*
			And some critical and high performance ones too.
		*/
		} else if (strcmp(pri_str, "IMMEDIATES") == 0) {
            *priority = THREAD_PRIORITY_IMMEDIATES;
        } else {
			/*
				If none normal up
			*/
            *priority = THREAD_PRIORITY_NORMAL;
        }
    } else {
		/*
			Same as the above
		*/
        strcpy(path, temp);
        *priority = THREAD_PRIORITY_NORMAL;
    }
}
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
                        char path[MAX_LINE_LENGTH];
                        thread_priority_t priority;
                        parse_line(start, path, &priority);
                        #ifdef DEBUG
                        printf("STARTUP: Spawning process: %s with priority %d\n", path, priority);
                        #endif
						/*
							Loop and spawn them
						*/
                        process_t* proc = spawn_process(path, THREAD_RING3, priority);
                        if (proc) {
                            #ifdef DEBUG
                            printf("STARTUP: Successfully spawned PID=%d for %s\n", proc->pid, path);
                            #endif
							/*
								Also execute it
							*/
							execute_process(proc);
                            processes_spawned++/*+1*/;
                        } else {
                            #ifdef DEBUG
                            printf("STARTUP: Failed to spawn process: %s\n", path);
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
            char path[MAX_LINE_LENGTH];
            thread_priority_t priority;
            parse_line(start, path, &priority);
            #ifdef DEBUG
            printf("STARTUP: Spawning final process: %s with priority %d\n", path, priority);
            #endif
			/*
				Spawn UP
			*/
            process_t* proc = spawn_process(path, THREAD_RING3/*always keep ring3 to avoid exploition*/, priority);
            if (proc) {
                #ifdef DEBUG
                printf("STARTUP: Successfully spawned PID=%d for %s\n", proc->pid, path);
                #endif
				/*
					Execute!
				*/
				execute_process(proc);
                processes_spawned++;
            } else {
                #ifdef DEBUG
                printf("STARTUP: Failed to spawn final process: %s\n", path);
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
