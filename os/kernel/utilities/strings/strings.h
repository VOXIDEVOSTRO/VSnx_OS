#ifndef STR_H
#define STR_H
/*
	Header files
*/
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#ifndef STR
#define STR
/*
    String length and copying functions
*/
int strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);

/*
    String comparison functions
*/
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
int strcasecmp(const char* str1, const char* str2);
int strncasecmp(const char* str1, const char* str2, size_t n);

/*
    String concatenation functions
*/
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);

/*
    String searching functions
*/
char* strchr(const char* str, int c);
char* strrchr(const char* str, int c);
char* strstr(const char* haystack, const char* needle);

/*
    Memory manipulation functions
*/
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memset(void* ptr, int value, size_t n);
int memcmp(const void* ptr1, const void* ptr2, size_t n);
void* memchr(const void* ptr, int value, size_t n);

/*
    String utility functions
*/
char* strdup(const char* str);
char* strtok(char* str, const char* delim);

/*
    String to number conversion functions
*/
int atoi(const char* str);
long atol(const char* str);
#endif
/*
    Additional constants
*/
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#endif