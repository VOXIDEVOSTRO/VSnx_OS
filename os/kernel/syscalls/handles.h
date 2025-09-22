#ifndef HANDLES_H
#define HANDLES_H
/*
	header files
*/
#include "syscall.h"
/*
	print handler (debugging)
*/
int64_t printf_handler(uint64_t format_ptr, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
/*
	FAT32 Filesystem operations
*/
int64_t open_handler(uint64_t filename_ptr, uint64_t flags, uint64_t mode, uint64_t unused1, uint64_t unused2, uint64_t unused3);
int64_t close_handler(uint64_t fd, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5);
int64_t read_handler(uint64_t fd, uint64_t buffer_ptr, uint64_t count, uint64_t unused1, uint64_t unused2, uint64_t unused3);
int64_t write_handler(uint64_t fd, uint64_t buffer_ptr, uint64_t count, uint64_t unused1, uint64_t unused2, uint64_t unused3);
int64_t lseek_handler(uint64_t fd, uint64_t offset, uint64_t whence, uint64_t unused1, uint64_t unused2, uint64_t unused3);
int64_t mkdir_handler(uint64_t path_ptr, uint64_t mode, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4);
int64_t unlink_handler(uint64_t path_ptr, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5);
/*
	Memory management or simple stuff from VMM
*/
int64_t malloc_handler(uint64_t size, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5);
int64_t free_handler(uint64_t ptr, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5);
int64_t realloc_handler(uint64_t ptr, uint64_t new_size, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4);
int64_t calloc_handler(uint64_t num, uint64_t size, uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4);
/*
	Threading information
*/
int64_t getpid_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t gettid_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t get_by_tid_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t thread_exists_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t get_state_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
/*
	Multithreading
*/
int64_t thread_create_handler(uint64_t func_ptr, uint64_t arg_ptr, uint64_t privilege_arg, uint64_t priority_arg, uint64_t owner_pid_arg, uint64_t unused6);
int64_t thread_execute_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t thread_exit_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t thread_terminate_handler(uint64_t tid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t thread_block_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t thread_unblock_handler(uint64_t tid, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
/*
	Process
*/
int64_t spawn_process_handler(uint64_t filename_ptr, uint64_t priority, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t execute_process_handler(uint64_t pid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t kill_process_handler(uint64_t pid_arg, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
/*
	IPC Pipes
*/
int64_t makepipe_handler(uint64_t name_ptr, uint64_t func_ptr, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t getpipe_handler(uint64_t name_ptr, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
/*
	SYSTEM time
*/
int64_t time_now_ms_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t uptime_seconds_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t uptime_minutes_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t time_after_handler(uint64_t a, uint64_t b, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t time_before_handler(uint64_t a, uint64_t b, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
#endif
