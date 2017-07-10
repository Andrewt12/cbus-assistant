
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "libcgate.h"

#define PROJECT "HOME"
#define NET 254
#define APP 56

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

static int32_t sockfd;
static CbusGroups* groups;

char * server_filename = "/tmp/socket-server";

int levenshtein(char *s1, char *s2) {
    unsigned int s1len, s2len, x, y, lastdiag, olddiag;
    s1len = strlen(s1);
    s2len = strlen(s2);
    unsigned int column[s1len+1];
    for (y = 1; y <= s1len; y++)
        column[y] = y;
    for (x = 1; x <= s2len; x++) {
        column[0] = x;
        for (y = 1, lastdiag = x-1; y <= s1len; y++) {
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y-1] + 1, lastdiag + (s1[y-1] == s2[x-1] ? 0 : 1));
            lastdiag = olddiag;
        }
    }
    return(column[s1len]);
}

int32_t findgroup(char *input)
{

    char group[100];
    int32_t i,j;
    int32_t lastmatch = 100, thismatch;
    int32_t bestmatch;

    i = j = 0;
    do{
       j = 0;
        memset(group,'\0',100);
        while(groups[i].TagName[j]){
            group[j] = tolower(groups[i].TagName[j]);
            j++;
        }
        thismatch = levenshtein(input,group);
        DEBUG_PRINT("Compared %s & %s and got %d\n",input,group,thismatch);
        if(thismatch < lastmatch){
            bestmatch = i;
            lastmatch = thismatch;
        }
        i++;
    }while(groups[i-1].index != -1);
    DEBUG_PRINT("Bestmatch TagName = %s : Address = %d\n",groups[bestmatch].TagName, groups[bestmatch].Address);
    if(lastmatch <= 5)
        return(groups[bestmatch].Address);
    else
        return -1;
}

int cgateconnect(int argc, char *argv[])
{
    int32_t portno,i;


    if (argc < 4) {
       fprintf(stderr,"usage %s cbus ip cbus port server port\n", argv[0]);
       exit(1);
    }

    portno = atoi(argv[2]);
    if((sockfd = cgate_connect(argv[1],portno, (int8_t*)PROJECT, NET)) <= 0){
        perror("Can not connect to C-Gate Server");
        exit(1);
    }
    DEBUG_PRINT("Connected to C-Gate\n");

    groups = cgate_parsedb(sockfd, NET, APP);

    i=0;
    do{
        DEBUG_PRINT("Application %s : %d : %d\n", groups[i].TagName, groups[i].Address, groups[i].index);
        //if(groups[i].index == -1)
        //    break;
        i++;
    }while(groups[i-1].index != -1);
    return 0;
}

void cbusSetLevel(char* group, int level)
{
    int n;
    uint8_t intlevel = (level * 255) / 100;
    if((n = findgroup(group)) != -1)
        cgate_set_ramp(sockfd, NET,APP, (uint8_t)n, intlevel, 4);

}

void cbusSetGroup(char* str)
{
    char *sptr;
    char *group;
    int n,len = 0;
    uint8_t value;

    if((group = malloc(strlen(str))) == NULL){
        perror("malloc error\n");
        return;
    }
    printf("strlen str %ld\n",strlen(str));
    sptr = strtok (str," ");
    while (sptr != NULL)
    {
        if(!strncmp(sptr, "on",2)){
            value = 255;
            DEBUG_PRINT("Group On\n");
        }else if(!strncmp(sptr, "off",3)){
            value = 0;
            DEBUG_PRINT("Group Off\n");
        }
        else if(!strcmp(sptr, "the"))
            DEBUG_PRINT("got the, do nothing\n");
        else if(!strncmp(sptr, "light",3))
            DEBUG_PRINT("got light, do nothing\n");
        else{
            printf ("* %s\n",sptr);
            strncpy(group+len,sptr,strlen(sptr));
            len += strlen(sptr);
        }
        sptr = strtok (NULL, " ,.-");
    }
    group[len] = '\0';
    DEBUG_PRINT("Formatted group = %s\n",group);
    if((n = findgroup(group)) != -1)
        cgate_set_group(sockfd,NET,APP, n,value);
}
