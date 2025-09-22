/*
	Some includs
*/
#include "hookorder.h"
/*
	Declaring the vars
*/
hookorder_entry_t g_entries[HOOKORDER_MAX_ENTRIES];
int g_entry_count = 0;
char g_cfg_cache[HOOKORDER_MAX_CFG_SIZE];
const char* g_cfg_path = "/KERNEL/CONFIGS/HOOKORDER.CFG";

/*
	Simple string helpers
*/

static void str_trim(char* s) {
    if (!s || !*s) return;

    // skip leading whitespace
    char* start = s;
    while (*start==' '||*start=='\t'||*start=='\r'||*start=='\n') start++;

    // find end
    char* end = start + strlen(start);
    while (end > start && (end[-1]==' '||end[-1]=='\t'||end[-1]=='\r'||end[-1]=='\n')) end--;
    *end = '\0';

    // shift left into original buffer
    if (start != s) {
        memmove(s, start, (size_t)(end - start) + 1);
    }
}

static void str_to_upper(char* s) {
	/*
		Because of our path convention
	*/
    for (; *s; ++s) {
        if (*s >= 'a' && *s <= 'z') *s = *s - 'a' + 'A';
    }
}

static int match_token(const char* pattern, const char* value) {
	/*
		Tokens
	*/
    if (pattern[0]=='*' && pattern[1]=='\0') return 1;
    return strcasecmp(pattern, value) == 0;
}

/*
	Parse up
	follwoing or : CLASS:<Subclass>:<driver>:<path_to_driver>
*/

static int parse_line(const char* raw, hookorder_entry_t* out) {
	/*
		Calculate the line
	*/
    char line[HOOKORDER_MAX_TOKEN*4 + HOOKORDER_MAX_PATH];
    strncpy(line, raw, sizeof(line)-1);
	/*
		NUll terminate
	*/
    line[sizeof(line)-1] = '\0';
	/*
		Strip of comments
	*/
    char* c = strchr(line, '#'); if (c) *c = '\0';
    c = strchr(line, ';'); if (c) *c = '\0';

    str_trim(line);
    if (!*line) return 1;

    // split path via '>' else last ':'
    char left[HOOKORDER_MAX_TOKEN*3];
    char path[HOOKORDER_MAX_PATH];
	/*
		split via the seperator:
		Could be the LAST colon
		or any ">"
	*/
    char* gt = strchr(line, '>');
    if (gt) {
        *gt = '\0';
        strncpy(left, line, sizeof(left)-1); left[sizeof(left)-1]=0;
        strncpy(path, gt+1, sizeof(path)-1); path[sizeof(path)-1]=0;
    } else {
		/*
			Handle the colon
		*/
        char* lastc = strrchr(line, ':');
        if (!lastc) return -1;
        *lastc = '\0';
        strncpy(left, line, sizeof(left)-1); left[sizeof(left)-1]=0;
        strncpy(path, lastc+1, sizeof(path)-1); path[sizeof(path)-1]=0;
    }
	/*
		trim both sides
	*/
    str_trim(left);
    str_trim(path);
    if (!*left || !*path) return -2;
	/*
		split up the 3 token on the left side
	*/
    char* p = left;
    char* c1 = strchr(p, ':'); if (!c1) return -2;
    *c1 = '\0';
    char* tok0 = p; p = c1 + 1;

    char* c2 = strchr(p, ':'); if (!c2) return -2;
    *c2 = '\0';
    char* tok1 = p; p = c2 + 1;

    char* tok2 = p;
    if (!*tok0 || !*tok1 || !*tok2) return -2;
	/*
		Get lenth of each token
	*/
    str_trim(tok0);
    str_trim(tok1);
    str_trim(tok2);
	/*
		Include it
	*/
	/*
		CLASS
	*/
    strncpy(out->category, tok0, sizeof(out->category)-1);
    out->category[sizeof(out->category)-1]=0;
	/*
		<subclass>
	*/
    strncpy(out->subclass, tok1, sizeof(out->subclass)-1);
    out->subclass[sizeof(out->subclass)-1]=0;
	/*
		<driver>
	*/
    strncpy(out->driver, tok2, sizeof(out->driver)-1);
    out->driver[sizeof(out->driver)-1]=0;
	/*
		<path>
	*/
    strncpy(out->path, path, sizeof(out->path)-1);
    out->path[sizeof(out->path)-1]=0;
	/*
		Set all as upper case
	*/
    str_to_upper(out->category);
    str_to_upper(out->subclass);
    str_to_upper(out->driver);
	/*
		reset
	*/
    out->claimed = 0;
	/*
		done
	*/
    return 0;
}

/*
	Helpers
*/
int hookorder_is_claimed(const char* subclass) {
    for (int i=0; i<g_entry_count; i++) {
        if (strcasecmp(g_entries[i].subclass, subclass) == 0) {
            return g_entries[i].claimed;
        }
    }
    return 0;
}

int hookorder_mark_claimed(const char* subclass, const char* driver) {
    for (int i=0; i<g_entry_count; i++) {
        if (strcasecmp(g_entries[i].subclass, subclass) == 0 &&
            strcasecmp(g_entries[i].driver, driver) == 0) {
            g_entries[i].claimed = 1;
            return 0;
        }
    }
    return -1;
}
/*
	Main APIs
*/

int hookorder_load(const char* path) {
	/*
		Verfify
	*/
    if (!path) path = g_cfg_path;
	/*
		Open up
	*/
    int err=0, fd = fat_open(path, 0, 0, &err);
	/*
		ERROR
	*/
    if (fd < 0) { 
		#ifdef DEBUG
		printf("[hookorder] open failed %d\n", err);
		#endif
		return -1; 
	}

    g_entry_count = 0;
    size_t off = 0;
	/*
		Read up
	*/
    while (off+1 < sizeof(g_cfg_cache)) {
		/*
			read
		*/
        int n = fat_read(fd, g_cfg_cache+off, sizeof(g_cfg_cache)-off-1, &err);
		/*
			Close if nothing or invalid
		*/
        if (n < 0) { fat_close(fd,&err); return -2; }
		/*
			If really nothing
		*/
        if (n == 0) break;
        off += (size_t)n;
    }
	/*
		Null terminate
	*/
    g_cfg_cache[off] = '\0';
    fat_close(fd, &err);
	/*
		Split up (once again)
	*/
    const char* p = g_cfg_cache;
    while (*p && g_entry_count < HOOKORDER_MAX_ENTRIES) {
		/*
			find up the last newline
		*/
        const char* nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
		/*
			Copy to buf
		*/
        char linebuf[HOOKORDER_MAX_TOKEN*4 + HOOKORDER_MAX_PATH];
        if (len >= sizeof(linebuf)) len = sizeof(linebuf)-1;
        memcpy(linebuf, p, len);
		/*
			Null terminate
		*/
        linebuf[len] = '\0';
		/*
			Advance the pointer
		*/
        p = nl ? nl + 1 : p + len;
		/*
			Parse the line
		*/
        hookorder_entry_t e;
        if (parse_line(linebuf, &e) == 0) {
            g_entries[g_entry_count++] = e;
        }
    }
	/*
		Count up the entries
		and get them
	*/
    for (int i=0; i<g_entry_count; i++) {
        const hookorder_entry_t* e = hookorder_get(i);
        #ifdef DEBUG
        printf("[hookorder] entry %d: %s:%s:%s -> %s\n", i, e->category, e->subclass, e->driver, e->path);
        #endif
    }
	/*
		Done
	*/
    #ifdef DEBUG
    printf("[hookorder] loaded %d entries\n", g_entry_count);
    #endif
    return g_entry_count;
}

int hookorder_reload(void) {
	/*
		No use prolly
	*/
    return hookorder_load(NULL);
}

int hookorder_count(void) {
    return g_entry_count;
}

const hookorder_entry_t* hookorder_get(int i) {
	/*
		Count up
	*/
    if (i < 0 || i >= g_entry_count) return NULL;
    return &g_entries[i];
}

const hookorder_entry_t* hookorder_find_exact(const char* subclass, const char* driver) {
	/*
		Find the exact match
	*/	
    char subU[HOOKORDER_MAX_TOKEN], drvU[HOOKORDER_MAX_TOKEN];
    strncpy(subU, subclass, sizeof(subU)-1); subU[sizeof(subU)-1]=0;
    strncpy(drvU, driver, sizeof(drvU)-1); drvU[sizeof(drvU)-1]=0;
    str_to_upper(subU); str_to_upper(drvU);
	/*
		Handle up the tokens
	*/
    for (int i=0; i<g_entry_count; i++) {
        if (strcasecmp(g_entries[i].category,"CLASS")==0 &&
            strcasecmp(g_entries[i].subclass,subU)==0 &&
            strcasecmp(g_entries[i].driver,drvU)==0) {
            return &g_entries[i];
        }
    }
    return NULL;
}

const hookorder_entry_t* hookorder_find_best(const char* subclass, const char* driver) {
	/*
		Find the approx match
	*/
    char subU[HOOKORDER_MAX_TOKEN], drvU[HOOKORDER_MAX_TOKEN];
    strncpy(subU, subclass, sizeof(subU)-1); subU[sizeof(subU)-1]=0;
    strncpy(drvU, driver, sizeof(drvU)-1); drvU[sizeof(drvU)-1]=0;
    str_to_upper(subU); str_to_upper(drvU);

    const hookorder_entry_t* best=NULL;
    int best_rank=999;
    for (int i=0; i<g_entry_count; i++) {
        const hookorder_entry_t* e=&g_entries[i];
        if (strcasecmp(e->category,"CLASS")!=0) continue;
        if (!match_token(e->subclass, subU)) continue;
        if (!match_token(e->driver, drvU)) continue;
		/*
			Compare
		*/
        int rank=0;
        if (strcmp(e->subclass,"*")==0) rank+=2;
        if (strcmp(e->driver,"*")==0) rank+=1;
        if (rank<best_rank) { best=e; best_rank=rank; if(rank==0) break; }
    }
    return best;
}

int hookorder_find_path(const char* subclass, const char* driver, char* out, size_t outsz) {
	/*
		Find up
	*/
    const hookorder_entry_t* e = hookorder_find_best(subclass, driver);
    if (!e) return -1;
    strncpy(out, e->path, outsz-1);
    out[outsz-1] = '\0';
    return 0;
}
/*
	THE MAIN loader
*/
int hookorder_spawn_for(const char* subclass, const char* driver, process_t** out_proc) {
    const hookorder_entry_t* e = hookorder_find_best(subclass, driver);
	if (hookorder_is_claimed(subclass)) {
        #ifdef DEBUG
        printf("[hookorder] %s already claimed, skipping %s\n", subclass, driver);
        #endif
        return -2;
    }
    if (!e) return -1;
	/*
		Spawn up driver
	*/
    process_t* proc = spawn_process(e->path, THREAD_RING0, THREAD_PRIORITY_BACKGROUND);
	if(proc) {
		/*
			Execute
		*/
		execute_process(proc);
	}
    if (!proc) return -2;
	hookorder_mark_claimed(subclass, driver);
	/*
		Done
	*/
    if (out_proc) *out_proc = proc;
    #ifdef DEBUG
    printf("[hookorder] spawned %s:%s -> %s (pid=%u)\n", subclass, driver, e->path, proc->pid);
    #endif
    return 0;
}

/*
	init THE HOOK ORDER!
*/
/*
	GPU header
*/
#include "class/gpu/gpu.h"
/*
	PCI header
*/
#include "class/pci/_pci_.h"

int start_hookorder(void) {
	/*
		PCI start
		it must be first
	*/
	pci_load_drivers(/*START THE PCI DRIVER!*/);
	/*
		GPU start
	*/
	gpu_load_drivers(/*START THE GPU DRIVER!*/);
    return 0;
}