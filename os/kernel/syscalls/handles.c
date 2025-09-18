/*
	Header files
*/
#include <stdbool.h>
#include "syscall.h"
#include "../threading/thread.h"
#include "../elf/process.h"
#include "../fat32/gristle.h" // For fat32
#include "../hal/mem/u_mem/u_mem.h" // For memory
#include "../ipc/ipc.h"
#include "../systemclock/clock.h"
/*
    PRINTF SYSCALL HANDLER
	For DEBUG use only LOL
*/
int64_t printf_handler(uint64_t format_ptr, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {

    char* format = (char*)format_ptr;
    
    printf(/*So we print PID too. because we can keep track for debug*/"USER[PID=%d]: %s\n", 
           current_thread ? current_thread->owner_pid/*use PID instead of TID to keep them contained*/ : 0, 
           format);
    
    return 0; // Success
}

/*
	FILESYSTEM/FAT32 Gristle driver handlers
*/
int64_t open_handler(uint64_t filename_ptr, uint64_t flags, uint64_t mode, uint64_t unused1, uint64_t unused2, uint64_t unused3) {
    char* filename = (char*)filename_ptr;
    int rerrno = 0;
    
    int fd = fat_open(filename, (int)flags, (int)mode, &rerrno);
    
    if (fd < 0) {
        return -rerrno/*-0 (Negetive Zero????????????? (nvm the fat_open returns))*/; // Return negative error code to avoid massively random uint64_t numbers
    }
    
    return fd;
}

int64_t close_handler(uint64_t fd, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5) {
    int rerrno = 0;
    int result = fat_close((int)fd, &rerrno);
    
    return result < 0 ? -rerrno : result;
}

int64_t read_handler(uint64_t fd, uint64_t buffer_ptr, uint64_t count, uint64_t unused1, uint64_t unused2, uint64_t unused3) {
    void* buffer = (void*)buffer_ptr;
    int rerrno = 0;
    
    int bytes_read = fat_read((int)fd, buffer, (size_t)count, &rerrno);
    
    return bytes_read < 0 ? -rerrno : bytes_read;
}

int64_t write_handler(uint64_t fd, uint64_t buffer_ptr, uint64_t count, uint64_t unused1, uint64_t unused2, uint64_t unused3) {
    const void* buffer = (const void*)buffer_ptr;
    int rerrno = 0;
    
    int bytes_written = fat_write((int)fd, buffer, (size_t)count, &rerrno);
    
    return bytes_written < 0 ? -rerrno : bytes_written;
}

int64_t lseek_handler(uint64_t fd, uint64_t offset, uint64_t whence, uint64_t unused1, uint64_t unused2, uint64_t unused3) {
    int rerrno = 0;
    
    int result = fat_lseek((int)fd, (int)offset, (int)whence, &rerrno);
    
    return result < 0 ? -rerrno : result;
}

int64_t mkdir_handler(uint64_t path_ptr, uint64_t mode, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4) {
    char* path = (char*)path_ptr;
    int rerrno = 0;
    
    int result = fat_mkdir(path, (int)mode, &rerrno);
    
    return result < 0 ? -rerrno : result;
}

int64_t unlink_handler(uint64_t path_ptr, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5) {
    char* path = (char*)path_ptr;
    int rerrno = 0;
    
    int result = fat_unlink(path, &rerrno);
    
    return result < 0 ? -rerrno : result;
}

/*
	VMM or memory management
*/

int64_t malloc_handler(uint64_t size, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5) {
    void* ptr = umalloc((size_t)size);
    return (int64_t)ptr;
}

int64_t free_handler(uint64_t ptr, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5) {
    ufree((void*)ptr);
    return 0; // Success!
}

int64_t realloc_handler(uint64_t ptr, uint64_t new_size, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4) {
    void* new_ptr = urealloc((void*)ptr, (size_t)new_size);
    return (int64_t)new_ptr;
}

int64_t calloc_handler(uint64_t num, uint64_t size, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4) {
    void* ptr = ucalloc((size_t)num, (size_t)size);
    return (int64_t)ptr;
}

/*
    PROCESS/THREAD INFORMATION HANDLERS
*/

int64_t getpid_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
	return getpid(/*empty*/);
}

int64_t gettid_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
	return gettid(/*empty and no idea why i am specifing it*/);
}

int64_t get_by_tid_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    uint32_t tid = (uint32_t)tid_arg;
    
    thread_t* thread = thread_get_by_tid(tid);
    
    if (thread == NULL) {
        return 0;
    }

    return (int64_t)thread; // probably danger!
}

int64_t thread_exists_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    uint32_t tid = (uint32_t)tid_arg;
    
    // Call the actual threading function
    bool exists = thread_exists(tid);

	/*
		This is bool. but as a int64
		We use binary digits 1 = true, 0 = false
	*/
    
    return exists ? 1 : 0;
}

int64_t get_state_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    uint32_t tid = (uint32_t)tid_arg;
    
    thread_state_t state = thread_get_state(tid);

    return (int64_t)state;
}

/*
	Multithreading functions
*/

int64_t thread_create_handler(uint64_t func_ptr, uint64_t arg_ptr, uint64_t unused5/*dangouros BECAUSE previlage for ring 0 mustbe not for userspace*/, uint64_t priority_arg, uint64_t owner_pid_arg, uint64_t unused6)
{
    (void)unused6; // unused shi
	(void)unused5;

    thread_func_t entry = (thread_func_t)func_ptr;
    void* arg = (void*)arg_ptr;
    int privilege = THREAD_RING3;
    int priority = (int)priority_arg;  
    uint32_t owner_pid = (uint32_t)owner_pid_arg;

	/*Invalid*/
    if (!entry) {
        return -1;
    }

	/*
		Now call
	*/
    thread_t* t = thread_create(entry, arg, privilege, priority, owner_pid);
    if (!t) {
        return -2;
    }
    return (int64_t)t->tid;
}

int64_t thread_execute_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused2; (void)unused3; (void)unused4; (void)unused5; (void)unused6;

    uint32_t tid = (uint32_t)tid_arg;
    thread_t* t = thread_get_by_tid(tid);
    if (!t) {
        return -1;
    }

    int rc = thread_execute(t);
    return (int64_t)rc;
}

int64_t thread_exit_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused1; (void)unused2; (void)unused3; (void)unused4; (void)unused5; (void)unused6;
	/*
		NO RETURN
	*/
    thread_exit();
}

int64_t thread_terminate_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused2; (void)unused3; (void)unused4; (void)unused5; (void)unused6;

    uint32_t tid = (uint32_t)tid_arg;
    int rc = thread_terminate(tid);
    return (int64_t)rc;
}

int64_t thread_block_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    return thread_block();
}

int64_t thread_unblock_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    /*
		I GUESS WE DONT need this
	*/
	return thread_unblock();
}

/*
	Process
*/

int64_t spawn_process_handler(uint64_t filename_ptr, uint64_t priority, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
	(void)unused3; (void)unused4; (void)unused5; (void)unused6;

    const char* filename = (const char*)filename_ptr;
    int privilege = THREAD_RING3;
	thread_priority_t thread_priority = priority;

    if (!filename) {
        return -1;
    }

    process_t* proc = spawn_process(filename, (thread_privilege_t)privilege, thread_priority);
    if (!proc) {
        return -2; // YOU FAILURE
    }
    return (int64_t)proc->pid;
}

/*
	HEALPER for the PID
	Needed here
*/
static inline process_t* find_process_by_pid(uint32_t pid) {
    extern process_t* process_table[MAX_PROCESSES];
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] && process_table[i]->pid == pid) {
            return process_table[i];
        }
    }
    return NULL;
}

int64_t execute_process_handler(uint64_t pid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused2; (void)unused3; (void)unused4; (void)unused5; (void)unused6;

    uint32_t pid = (uint32_t)pid_arg;
    process_t* proc = find_process_by_pid(pid);
    if (!proc) {
        return -1;
    }
    return (int64_t)execute_process(proc);
}

int64_t kill_process_handler(uint64_t pid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused2; (void)unused3; (void)unused4; (void)unused5; (void)unused6;

    uint32_t pid = (uint32_t)pid_arg;
    return (int64_t)kill_process(pid);
}

/*

	PIPES or IPC

*/

int64_t makepipe_handler(uint64_t name_ptr, uint64_t func_ptr, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused3; (void)unused4; (void)unused5; (void)unused6;

    const char* name = (const char*)name_ptr;
    pipe_t func = (pipe_t)func_ptr;

    makepipe(name, func);
    return 0;
}

int64_t getpipe_handler(uint64_t name_ptr, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused2; (void)unused3; (void)unused4; (void)unused5; (void)unused6;

    const char* name = (const char*)name_ptr;
    pipe_t func = getpipe(name);

    return (int64_t)func;
}

/*
	Some SYSTEM time functions
*/

int64_t time_now_ms_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused1; (void)unused2; (void)unused3;
    (void)unused4; (void)unused5; (void)unused6;

    return (int64_t)time_now_ms();
}

int64_t uptime_seconds_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused1; (void)unused2; (void)unused3;
    (void)unused4; (void)unused5; (void)unused6;

    return (int64_t)uptime_seconds();
}

int64_t uptime_minutes_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused1; (void)unused2; (void)unused3;
    (void)unused4; (void)unused5; (void)unused6;

    return (int64_t)uptime_minutes();
}

int64_t time_after_handler(uint64_t a, uint64_t b, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused3; (void)unused4; (void)unused5; (void)unused6;

    return (int64_t)time_after(a, b);
}

int64_t time_before_handler(uint64_t a, uint64_t b, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6)
{
    (void)unused3; (void)unused4; (void)unused5; (void)unused6;

    return (int64_t)time_before(a, b);
}

