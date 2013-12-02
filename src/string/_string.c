#include "_string.h"

int diff_strcmp(const char *s1, const char *s2) {
    while(*s1 == *s2) {
        if(*s1 == '\0')
            return 0;
        s1++, s2++;
    }
    return *s1 - *s2;
}



char diff_tolower(char c) {
    if(c >= 'A' && c <= 'Z')
        c += 32;
    return c;
}



int diff_strcasecmp(const char *s1, const char *s2) {
    char s1_c = diff_tolower(*s1), s2_c = diff_tolower(*s2);

    while(s1_c == s2_c) {
        if(s1_c == '\0')
            return 0;
        s1++, s2++;
        s1_c = diff_tolower(*s1), s2_c = diff_tolower(*s2);
    }
    return s1_c - s2_c;
}



size_t diff_strlen(const char *s) {
    size_t size = 0;
    while(*s != '\0')
        size++, s++;
    return size;
}



char* diff_strcpy(char *dest, const char *src) {

    while(*src != '\0') {
        *dest = *src;
        dest++, src++;
    }
    *dest = '\0';

    return dest;
}


char* diff_strchr(const char *s, int c) {
    while(*s != '\0') {
        if((int)*s == c)
            return (char*)s;
        s++;
    }
    return NULL;
}
