#ifndef _CBUSGROUPS_H
#define _CBUSGROUPS_H

#define DEBUG

#if defined DEBUG
    #define DEBUG_PRINT(fmt, ...) do { printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)
#else
    #define DEBUG_PRINT(fmt, ...) /* Don't do anything in release builds */
#endif

void cbusSetLevel(char* group, int level);
void cbusSetGroup(char* str);
int cgateconnect(int argc, char *argv[]);
#endif
