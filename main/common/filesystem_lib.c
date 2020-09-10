#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_file(char* haysack, char* buff)
{
    char* dup = strdup(haysack);
    char* marker = NULL;
    char* lastworking = NULL;
    
    marker = strstr(dup, "/");
    while(marker != NULL)
    {
        lastworking = marker+1;
        marker = strstr(marker+1, "/");
    }
    strcpy(buff, lastworking);
    free(dup); 
    return 0;
}

int get_path(char* haysack, char* buff)
{
    char* dup = strdup(haysack);
    char* marker = NULL;
    char* lastworking = NULL;
    
    marker = strstr(dup, "/");
    while(marker != NULL)
    {
        lastworking = marker;
        marker = strstr(marker+1, "/");
    }
    *lastworking = 0;
    strcpy(buff, dup);
    free(dup); 
    return 0;
}

int get_url(char* haysack, char* buff)
{
    char* dup = strdup(haysack);
    char* marker = NULL;
    char* lastworking = NULL;
    
    marker = strstr(dup, ":");
    while(marker != NULL)
    {
        lastworking = marker;
        marker = strstr(marker+1, ":");
    }
    *lastworking = 0;

    marker = strstr(dup, "/");
    while(marker != NULL)
    {
        lastworking = marker;
        marker = strstr(marker+1, "/");
    }
    lastworking++;

    strcpy(buff, lastworking);
    free(dup); 
    return 0;
}

int get_ip(char* haysack, char* buff)
{
    int ret = -1;
    char* dup = strdup(haysack);
    char* marker = NULL;
    char* lastworking = NULL;

    marker = strstr(dup, "://");
    if (marker != NULL) {
        lastworking = marker + 3;

        marker = strstr(lastworking, ":");
        if (marker != NULL) {
            *marker = 0;
            strcpy(buff, lastworking);
        }
    }

    free(dup);
    return ret;
}

int get_port(char* haysack, char* buff)
{
    char* dup = strdup(haysack);
    char* marker = NULL;
    char* lastworking = NULL;
    
    marker = strstr(dup, ":");
    while(marker != NULL)
    {
        lastworking = marker;
        marker = strstr(marker+1, ":");
    }
    if(lastworking == NULL) return -1;
    *lastworking = 0;
    lastworking++;
    //printf("lastworking [%s]\n", lastworking);
    marker = strstr(lastworking, "/");
    if(marker == NULL) return -1;
    *marker = 0;
    //printf("lastworking [%s]\n", lastworking);

    strcpy(buff, lastworking);
    free(dup); 
    return 0;
}

int get_filepath(char* haysack, char* buff)
{
    char* dup = strdup(haysack);
    char* marker = NULL;
    char* lastworking = NULL;
    
    marker = strstr(dup, ":");
    while(marker != NULL)
    {
        lastworking = marker;
        marker = strstr(marker+1, ":");
    }
    if(lastworking == NULL) return -1;
    lastworking++;
    marker = strstr(lastworking, "/");
    if(marker == NULL) return -1;

    strcpy(buff, marker);
    free(dup); 
    return 0;
}
