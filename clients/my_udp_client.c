/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include "skbuff.h"

#define BUFSIZE 2000000
#define MESSCHUNK 10000

static volatile bool keepRunning = 1;

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

void intHandler(int dummy) {
    keepRunning = 0;
}

float nextTime(float rateParameter)
{
    float temp = -logf(1.0f - (float) random() / (RAND_MAX)) / rateParameter;
    return temp;
}

void printMessage(char * buffer, int length)
{
    int i;

    puts("################# Message Content ->");

    for(i = 0; i < length; i++)
    //if(buffer[i] != "\0")
	printf("%02x ",buffer[i]);

    puts("#################");
    
    return;
}


char * setPacketHeader(char * header, int totalPackets, int sequenceNumber)
{
    header[0] = 0x00;
    header[1] = 0x00;
    header[2] = 0x00;
    // buffer[3] = 0x00;
    header[3] = sequenceNumber;
    header[4] = 0x00;
    // buffer[5] = 0x01;
    header[5] = totalPackets;
    header[6] = 0x00;
    header[7] = 0x00;
}




int SendMessage(int sockfd, struct sockaddr_in serveraddr, Data * data, int timetoSleep)
{
    int bytes_transmited;
    int serverlen;
    serverlen = sizeof(serveraddr);
    struct timeval time_before, time_next;
    int diff = 0;
    int chunk;
    char * index_buffer = data->buffer;

    data->touched = false;

    chunk = data->bufferSize < MESSCHUNK ? data->bufferSize : MESSCHUNK;
    usleep(timetoSleep);
    while(data->bufferSize  > 0)
    {
        UpdateHeader(data);
        //ShowHeader(data->memcachedHeader);
        gettimeofday (&time_before, NULL);
        bytes_transmited = sendto(sockfd, data->buffer, chunk, 0, &serveraddr, serverlen);
        gettimeofday(&time_next, NULL);
        diff = (time_next.tv_sec - time_before.tv_sec)*1000000 + (time_next.tv_usec - time_before.tv_usec);

        if(bytes_transmited < 1)
            break;
        data->buffer = data->buffer + bytes_transmited;
        data->bufferSize  = data->bufferSize  - bytes_transmited;
        chunk = data->bufferSize < MESSCHUNK ? data->bufferSize : MESSCHUNK;
    }
    //bytes_transmited = sendto(sockfd, buffer, size, 0, &serveraddr, serverlen);


    if (bytes_transmited < 0)
    {
        error("ERROR in sendto");
        return 1;
    }
    //printf("####################################\n");
    data->buffer = index_buffer;
    gettimeofday (&time_before, NULL);
    bytes_transmited = recvfrom(sockfd, data->buffer, BUFSIZE, 0, &serveraddr, &serverlen);
    gettimeofday(&time_next, NULL);
    diff = diff + (time_next.tv_sec - time_before.tv_sec)*1000000 + (time_next.tv_usec - time_before.tv_usec);
    printf("%d\n", diff);
    if (bytes_transmited < 0) 
    {
        error("ERROR in recvfrom");
        return 1;
    }

    return 0;
    
    //printMessage(buffer, 20);

    
}

int get(char * buffer,
        char * key, int sockfd, 
        struct sockaddr_in serveraddr, 
        int timetoSleep)
{
    int result;

    Data * newPacket = (Data *) malloc(sizeof(Data));

    //clearing the buffer
    bzero(buffer, BUFSIZE);



    sprintf(buffer+8, "get %s\r\n", key);

    SerializeNewPacket(newPacket, buffer);

    result = SendMessage(sockfd, serveraddr, newPacket, timetoSleep);
       
    free(newPacket->memcachedHeader);
    free(newPacket);

    return result;
}


int set(char * buffer, 
        char * key, 
        char * value, 
        int sockfd, 
        struct sockaddr_in serveraddr, 
        int timetoSleep)
{
    //clearing the buffer
    bzero(buffer, BUFSIZE);

    printf("%s %d %d\n", value, strlen(key), strlen(value));
    sprintf(buffer+8, "set %s %d %d %d \r\n%s\r\n", key, 0, 0, strlen(value), value);
    return strlen(buffer+8) + 8;
}


void SerializeNewPacket(Data * data, char * buffer)
{
    char * header = (char *) malloc(sizeof(char) * 8);

    data->sequenceNumber = 0;
    data->buffer = (char *) buffer;
    data->bufferSize = strlen(buffer + 8) + 8;
    data->totalPackets = ((data->bufferSize + 30) / MESSCHUNK) + 1;
    data->bufferSize = data->bufferSize + 8 * (data->totalPackets);
    NewHeaderSetup(data->totalPackets, header); 
    data->memcachedHeader = header;
    memcpy(data->buffer, data->memcachedHeader, 8);

}

int SetBySize(char * buffer, 
        char * key, 
        size_t valueSize,
        int sockfd, 
        struct sockaddr_in serveraddr, 
        int timetoSleep)
{

    int result; 
    //making the message
    Data * newPacket = (Data *) malloc(sizeof(Data));

    char * value = malloc(valueSize * sizeof(char)); //in Bytes
    memset(value, 1, valueSize);

    //clearing the buffer
    bzero(buffer, BUFSIZE);
    
    sprintf(buffer + 8, "set %s %d %d %d \r\n%s\r\n", key, 0, 0, valueSize, value);

    SerializeNewPacket(newPacket, buffer);

    result = SendMessage(sockfd, serveraddr, newPacket, timetoSleep);
    
    free(value);
    free(newPacket->memcachedHeader);
    free(newPacket);
    
    
    return result;
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent * server;
    char * hostname;
    char buf[BUFSIZE];
    int wait;
    static int maximumRequestsToSend;
    static int requestCounter = 0;
    static int rate;
    int transmissionResult = 0;
    size_t packetSize = 0;
    

    /* check command line arguments */
    if (argc != 6) {
       fprintf(stderr,"usage: %s <hostname> <port> <rate> <maximumRequestsToSend> <packetSize>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
    rate = atoi(argv[3]);
    maximumRequestsToSend = atoi(argv[4]);
    packetSize = atoi(argv[5]);

    /* Signal Handling */

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
  	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    bzero(buf, BUFSIZE);
    // serverlen = sizeof(serveraddr);


    /* get a message from the user */
    // printf("Please enter msg: ");
    // fgets(buf, BUFSIZE, stdin);

    // set(buf, "key", "alireza");
    // sendMessage(sockfd, serveraddr, buf);
    // get(buf, "key");
    // sendMessage(sockfd, serveraddr, buf);
    wait = nextTime(rate) * 1000000;

    transmissionResult = SetBySize(buf, "testing", packetSize, sockfd, serveraddr, wait);
    // = sendMessage(sockfd, serveraddr, buf, wait);
    while(keepRunning && !transmissionResult && requestCounter < maximumRequestsToSend)
    {
        wait = nextTime(rate) * 1000000;
        transmissionResult = get(buf, "testing", sockfd, serveraddr, wait);
        requestCounter = requestCounter + 1;
    }

    return 0;

}

