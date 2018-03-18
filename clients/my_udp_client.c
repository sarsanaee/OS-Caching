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

#define BUFSIZE 2000000

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




int sendMessage(int sockfd, struct sockaddr_in serveraddr, char * buffer, int timetoSleep)
{
    int bytes_transmited;
    int serverlen;
    serverlen = sizeof(serveraddr);
    struct timeval time_before, time_next;
    int diff;
    
    usleep(timetoSleep);
    gettimeofday (&time_before, NULL);
    bytes_transmited = sendto(sockfd, buffer, strlen(buffer+8) + 8, 0, &serveraddr, serverlen);
    if (bytes_transmited < 0)
    {
        error("ERROR in sendto");
        return 1;
    }
    bytes_transmited = recvfrom(sockfd, buffer, BUFSIZE, 0, &serveraddr, &serverlen);
    gettimeofday(&time_next, NULL);
    diff = (time_next.tv_sec - time_before.tv_sec)*1000000 + (time_next.tv_usec - time_before.tv_usec);
    printf("%d\n", diff);
    if (bytes_transmited < 0) 
    {
        error("ERROR in recvfrom");
        return 1;
    }
    return 0;
    
    //printMessage(buffer, 20);

    
}

void get(char * buffer ,char * key)
{
    //clearing the buffer
    bzero(buffer, BUFSIZE);

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x00;
    buffer[7] = 0x00;

    sprintf(buffer+8, "get %s\r\n", key);
}


int set(char * buffer, char * key, char * value)
{
    //clearing the buffer
    bzero(buffer, BUFSIZE);

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    printf("%s %d %d\n", value, strlen(key), strlen(value));
    sprintf(buffer+8, "set %s %d %d %d \r\n%s\r\n", key, 0, 0, strlen(value), value);
    puts("after");
    return strlen(buffer+8) + 8;
}

int setBySize(char * buffer, char * key, size_t valueSize)
{

    //making the message
    char * value = malloc(valueSize * sizeof(char)); //in Bytes
    memset(value, 1, valueSize);

    //clearing the buffer
    bzero(buffer, BUFSIZE);

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    buffer[5] = 0x01;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    //printf("%s %d %d\n", value, strlen(key), strlen(value));
    sprintf(buffer+8, "set %s %d %d %d \r\n%s\r\n", key, 0, 0, strlen(value), value);
    puts("after");
    return strlen(buffer+8) + 8;
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
    

    /* check command line arguments */
    if (argc != 5) {
       fprintf(stderr,"usage: %s <hostname> <port> <rate> <maximumRequestsToSend>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
    rate = atoi(argv[3]);
    maximumRequestsToSend = atoi(argv[4]);

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

    setBySize(buf, "testing", 100);
    transmissionResult = sendMessage(sockfd, serveraddr, buf, wait);

    while(keepRunning && !transmissionResult && requestCounter < maximumRequestsToSend)
    {
        wait = nextTime(rate) * 1000000;

        get(buf, "testing");
        transmissionResult = sendMessage(sockfd, serveraddr, buf, wait);
        requestCounter = requestCounter + 1;
    }

    return 0;

}

