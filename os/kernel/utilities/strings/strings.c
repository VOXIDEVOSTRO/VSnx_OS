/*
	HEADER
*/
#include "strings.h"
#include "../../hal/mem/k_mem/k_mem.h" // Some using them
/*
	Main string functions
	adding all manual stuff
*/
int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    while (*dest) dest++;
    while ((*dest++ = *src++));
    return original_dest;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* original_dest = dest;
    while (*dest) dest++;
    while (n-- && (*dest++ = *src++));
    if (n == SIZE_MAX) *dest = '\0';
    return original_dest;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }
    return (c == '\0') ? (char*)str : NULL;
}

char* strrchr(const char* str, int c) {
    char* last = NULL;
    while (*str) {
        if (*str == c) last = (char*)str;
        str++;
    }
    return (c == '\0') ? (char*)str : last;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return NULL;
}

void* memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (n--) *d++ = *s++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

void* memset(void* ptr, int value, size_t n) {
    unsigned char* p = (unsigned char*)ptr;
    while (n--) *p++ = (unsigned char)value;
    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

void* memchr(const void* ptr, int value, size_t n) {
    const unsigned char* p = (const unsigned char*)ptr;
    while (n--) {
        if (*p == (unsigned char)value) {
            return (void*)p;
        }
        p++;
    }
    return NULL;
}

char* strdup(const char* str) {
    size_t len = strlen(str) + 1;
    char* copy = (char*)kmalloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

char* strtok(char* str, const char* delim) {
    static char* last = NULL;
    if (str) last = str;
    if (!last) return NULL;
    
    while (*last && strchr(delim, *last)) last++;
    if (!*last) return NULL;
    
    char* token = last;
    
    while (*last && !strchr(delim, *last)) last++;
    if (*last) {
        *last = '\0';
        last++;
    } else {
        last = NULL;
    }
    
    return token;
}

// Case-insensitive string comparison
int strcasecmp(const char* str1, const char* str2) {
    while (*str1 && *str2) {
        char c1 = (*str1 >= 'A' && *str1 <= 'Z') ? *str1 + 32 : *str1;
        char c2 = (*str2 >= 'A' && *str2 <= 'Z') ? *str2 + 32 : *str2;
        if (c1 != c2) return c1 - c2;
        str1++;
        str2++;
    }
    char c1 = (*str1 >= 'A' && *str1 <= 'Z') ? *str1 + 32 : *str1;
    char c2 = (*str2 >= 'A' && *str2 <= 'Z') ? *str2 + 32 : *str2;
    return c1 - c2;
}

int strncasecmp(const char* str1, const char* str2, size_t n) {
    while (n && *str1 && *str2) {
        char c1 = (*str1 >= 'A' && *str1 <= 'Z') ? *str1 + 32 : *str1;
        char c2 = (*str2 >= 'A' && *str2 <= 'Z') ? *str2 + 32 : *str2;
        if (c1 != c2) return c1 - c2;
        str1++;
        str2++;
        n--;
    }
    if (n == 0) return 0;
    char c1 = (*str1 >= 'A' && *str1 <= 'Z') ? *str1 + 32 : *str1;
    char c2 = (*str2 >= 'A' && *str2 <= 'Z') ? *str2 + 32 : *str2;
    return c1 - c2;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;
    
    // Handle sign avoiding large numbers
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

long atol(const char* str) {
    long result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;
    
    // Handle sign to avoid large numbers
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}
