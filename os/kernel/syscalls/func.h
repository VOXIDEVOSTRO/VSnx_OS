/*
	Syscall.h and this will be use for predelared functions for easy use.
	Also as always: HEADER FILES!
*/
#include "syscall.h"
#include <stdbool.h>
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