#ifndef SKBUFF_H
#define SKBUFF_H

#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct skbuff 
{
    
    bool touched;
    size_t sequenceNumber;
    size_t totalPackets;
    size_t bufferSize;
    char * memcachedHeader;
    char * buffer;

} Data;

void NewHeaderSetup(size_t totalPackets, char * header)
{
    header[0] = 0x00;  
    header[1] = 0x00;
    header[2] = 0x00;
    header[3] = 0x00;
    header[4] = 0x00;
    header[5] = totalPackets;
    header[6] = 0x00;
    header[7] = 0x00;
}

void ShowHeader(char * mheader)
{
    int i;
    puts("UDP header:");
    for(i = 0; i < 8; i++)
    {
        printf(" %2x ", mheader[i]);
    }
    puts("\nheader done");
}

void ShowSkbuff(Data * data)
{
    printf("touched: %d\nsequenceNumber %d\ntotalPackets %d\nbufferSize %d\n", 
    data->touched,
    data->sequenceNumber,
    data->totalPackets,
    data->bufferSize);
}

void showHeaderBuffer(Data * data)
{
    int i;
    puts("Buffer header:");
    for(i = 0; i < 10; i++)
    {
        printf(" %2x ", data->buffer[i]);
    }
    puts("\nBuffer done");
}

void UpdateHeader(Data * data)
{
    if(data->touched)
    {
        data->buffer -= 8;
        data->memcachedHeader[3]++;
        //data->memcachedHeader[1]++;
        memcpy(data->buffer, data->memcachedHeader, 8);
        return;
    }
    data->touched = true;
}

#endif