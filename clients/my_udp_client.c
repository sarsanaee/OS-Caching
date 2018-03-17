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

#define BUFSIZE 1024

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

void printMessage(char * buffer, int length)
{
    int i;

    puts("################# Message Content ->");

    for(i = 0; i < length; i++)
	printf("%02x ",buffer[i]);

    puts("#################");
    
    return;
}




void sendMessage(int sockfd, struct sockaddr_in serveraddr, char * buffer)
{
    int bytes_transmited;
    int serverlen;
    serverlen = sizeof(serveraddr);
    bytes_transmited = sendto(sockfd, buffer, strlen(buffer+8) + 8, 0, &serveraddr, serverlen);
    printf("salam\n");
    if (bytes_transmited < 0) 
      error("ERROR in sendto");
    /* print the server's reply */
    bytes_transmited = recvfrom(sockfd, buffer, 50, 0, &serveraddr, &serverlen);
    if (bytes_transmited < 0) 
      error("ERROR in recvfrom");

    printMessage(buffer, 50);
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





int main(int argc, char **argv) {
    int sockfd, portno, n;
    // int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    int j = 0;

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

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

    /*j = sprintf(buf, "%hu", 0x0);
    j += sprintf(buf+j, "%hu", 0x0000);
    j += sprintf(buf+j, "%hu", 0x0001);
    j += sprintf(buf+j, "%hu", 0x0000);
    
    j += sprintf(buf+j, "get key1\r\n");
    */
    //char * my_buf = malloc(sizeof(char) * 19);
    //sprintf( + 8, "get key1\r\n");

    /* send the message to the server */
    //j = sprintf(buf, "%02x", 0x00);
    //j += sprintf(buf+j, "%02x", 0x00);
    //j += sprintf(buf+j, "%02x", 0x01);
    //j += sprintf(buf+j, "%02x", 0x00);
    //printf("%d\n", j);
    
    
    
    // buf[0] = 0x00;
    // buf[1] = 0x00;
    // buf[2] = 0x00;
    // buf[3] = 0x00;
    // buf[4] = 0x00;
    // buf[5] = 0x01;
    // buf[6] = 0x00;
    // buf[7] = 0x00;

    // sprintf(buf+8, "get key1\r\n");



    set(buf, "key", "alireza");
    sendMessage(sockfd, serveraddr, buf);
    get(buf, "key");
    sendMessage(sockfd, serveraddr, buf);


}

