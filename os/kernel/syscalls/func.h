/*
	Syscall.h and this will be use for predelared functions for easy use.
	Also as always: HEADER FILES!
*/
#include "syscall.h"
#include "../ipc/ipc.h"
#include <stdbool.h>
/*
	prototypes
	Because of the compiler...
*/
/* ---- Print / Debug ---- */
static inline int print(const char *fmt, ...);

/* ---- Filesystem ---- */
static inline int open(const char *filename, int flags, int mode);
static inline int close(int fd);
static inline int read(int fd, void *buffer, size_t count);
static inline int write(int fd, const void *buffer, size_t count);
static inline int lseek(int fd, int offset, int whence);
static inline int mkdir(const char *path, int mode);
static inline int unlink(const char *path);

/* ---- Memory management PMM and VMM ---- */
static inline void* umalloc(size_t size);
static inline void  ufree(void* ptr);
static inline void* urealloc(void* ptr, size_t new_size);
static inline void* ucalloc(size_t num, size_t size);

/* ---- Process / Thread info ---- */
static inline int   getpid(void);
static inline int   gettid(void);
static inline void* get_by_tid(uint32_t tid);
static inline bool  thread_exists(uint32_t tid);
static inline int   get_state(uint32_t tid);

/* ---- Multithreading or THREADING matters on you ---- */
static inline int  t_create(void* entry, void* arg, int privilege, int priority, int owner_pid);
static inline int  t_execute(uint32_t tid);
static inline void t_exit(void);
static inline int  t_terminate(uint32_t tid);

/* ---- Spinlock / Sleep simple sleep ---- */
static inline void cpu_relax(void);
static inline void t_sleep(uint32_t milliseconds);
static inline int  t_block(void);
static inline int  t_unblock(void);

/* ---- Process/proc management ---- */
static inline int spawn_proc(const char* filename, int privilege);
static inline int execute_proc(int pid);
static inline int kill_proc(int pid);

/* ---- Pipes/IPC ---- */
static inline int    createpipe(const char* name, pipe_t func);
static inline pipe_t retrievepipe(const char* name);

/* ---- System time ---- */
static inline uint64_t uptime_milli(void);
static inline uint32_t uptime_sec(void);
static inline uint32_t uptime_min(void);
static inline int      time_aft(uint64_t a, uint64_t b);
static inline int      time_bef(uint64_t a, uint64_t b);
/*
	Print/Debug function for Ring 3 processes
*/
static inline int print(const char *fmt, ...) {
	return syscall(1, (uint64_t)fmt, 0, 0, 0, 0, 0);
}
/*
	Filesystem related
*/

// Some user constants here
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0040
#define O_TRUNC     0x0200
#define O_APPEND    0x0400
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

static inline int open(const char *filename, int flags, int mode) {
    return syscall(2, (uint64_t)filename, (uint64_t)flags, (uint64_t)mode, 0, 0, 0);
}

static inline int close(int fd) {
    return syscall(3, (uint64_t)fd, 0, 0, 0, 0, 0);
}

static inline int read(int fd, void *buffer, size_t count) {
    return syscall(4, (uint64_t)fd, (uint64_t)buffer, (uint64_t)count, 0, 0, 0);
}

static inline int write(int fd, const void *buffer, size_t count) {
    return syscall(5, (uint64_t)fd, (uint64_t)buffer, (uint64_t)count, 0, 0, 0);
}

static inline int lseek(int fd, int offset, int whence) {
    return syscall(6, (uint64_t)fd, (uint64_t)offset, (uint64_t)whence, 0, 0, 0);
}

static inline int mkdir(const char *path, int mode) {
    return syscall(7, (uint64_t)path, (uint64_t)mode, 0, 0, 0, 0);
}

static inline int unlink(const char *path) {
    return syscall(8, (uint64_t)path, 0, 0, 0, 0, 0);
}
/*
    Memory management operations
*/
static inline void* umalloc(size_t size) {
    return (void*)syscall(9, (uint64_t)size, 0, 0, 0, 0, 0);
}

static inline void ufree(void* ptr) {
    syscall(10, (uint64_t)ptr, 0, 0, 0, 0, 0);
}

static inline void* urealloc(void* ptr, size_t new_size) {
    return (void*)syscall(11, (uint64_t)ptr, (uint64_t)new_size, 0, 0, 0, 0);
}

static inline void* ucalloc(size_t num, size_t size) {
    return (void*)syscall(12, (uint64_t)num, (uint64_t)size, 0, 0, 0, 0);
}
/*
	proc/threading information
*/

static inline int getpid(void) {
	return syscall(13, 0, 0, 0, 0, 0, 0);
}

static inline int gettid(void) {
    return syscall(14, 0, 0, 0, 0, 0, 0);
}

static inline void* get_by_tid(uint32_t tid) {
    int64_t result = syscall(15, tid, 0, 0, 0, 0, 0);
    return (void*)result;
}

static inline bool thread_exists(uint32_t tid) {
    int64_t result = syscall(16, tid, 0, 0, 0, 0, 0);
    return result == 1;
}

static inline int get_state(uint32_t tid) {
    int64_t result = syscall(17, tid, 0, 0, 0, 0, 0);
    return (int)result;
}

/*
	Multithreading
*/

static inline int t_create(void* entry, void* arg, int privilege, int priority, int owner_pid) { 
	return syscall(18, (uint64_t)entry, (uint64_t)arg, (uint64_t)privilege, (uint64_t)priority, (uint64_t)owner_pid, 0); 
}

static inline int t_execute(uint32_t tid) { 
	return syscall(19, (uint64_t)tid, 0, 0, 0, 0, 0); 
}

static inline void t_exit(void) { 
	syscall(20, 0, 0, 0, 0, 0, 0); 
}

static inline int t_terminate(uint32_t tid) { 
	return syscall(21, (uint64_t)tid, 0, 0, 0, 0, 0); 
}

/*
	Simple CPU spinlock or relax?
*/
static inline void cpu_relax(void) {
#if defined(__x86_64__) || defined(__i386__)
    __asm__ volatile("pause");
#elif defined(__aarch64__) || defined(__arm__)
    __asm__ volatile("yield");
#endif
}

/* Busy-wait sleep using uptime_milli() syscall */
static inline void t_sleep(uint32_t milliseconds) {
    if (milliseconds == 0) return;
	/*
		Use our timers
	*/
    uint64_t start  = uptime_milli();
    uint64_t target = start + (uint64_t)milliseconds;
	/*
		Wait
	*/
    while (time_bef(uptime_milli(), target)) {
        cpu_relax();
    }
}


static inline int t_block(void) {
    return syscall(28, 0, 0, 0, 0, 0, 0);
}

static inline int t_unblock(void) {
    return syscall(29, 0, 0, 0, 0, 0, 0);
}

/*
	proc/process
*/

static inline int spawn_proc(const char* filename, int privilege) {
    return (int)syscall(22, (uint64_t)filename, (uint64_t)privilege, 0, 0, 0, 0);
}

static inline int execute_proc(int pid) {
    return (int)syscall(23, (uint64_t)pid, 0, 0, 0, 0, 0);
}

static inline int kill_proc(int pid) {
    return (int)syscall(24, (uint64_t)pid, 0, 0, 0, 0, 0);
}

/*
	Pipes function
*/

static inline int createpipe(const char* name, pipe_t func) {
    return (int)syscall(25, (uint64_t)name, (uint64_t)func, 0, 0, 0, 0);
}

static inline pipe_t retrievepipe(const char* name) {
    return (pipe_t)syscall(26, (uint64_t)name, 0, 0, 0, 0, 0);
}

/*
	System time functions
*/

static inline uint64_t uptime_milli(void) {
    return (uint64_t)syscall(30, 0,0,0,0,0,0);
}

static inline uint32_t uptime_sec(void) {
    return (uint32_t)syscall(31, 0,0,0,0,0,0);
}

static inline uint32_t uptime_min(void) {
    return (uint32_t)syscall(32, 0,0,0,0,0,0);
}

static inline int time_aft(uint64_t a, uint64_t b) {
    return (int)syscall(33, a, b, 0,0,0,0);
}

static inline int time_bef(uint64_t a, uint64_t b) {
    return (int)syscall(34, a, b, 0,0,0,0);
}
