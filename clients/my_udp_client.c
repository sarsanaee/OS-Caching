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

int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
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

    /* get a message from the user */
    bzero(buf, BUFSIZE);
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
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x01;
    buf[6] = 0x00;
    buf[7] = 0x00;

    sprintf(buf+8, "get key1\r\n");
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, buf, 18, 0, &serveraddr, serverlen);
    printf("salam\n");
    if (n < 0) 
      error("ERROR in sendto");
    
    /* print the server's reply */
    printf("%s %d\n", buf, strlen(buf));
    n = recvfrom(sockfd, buf, 50, 0, &serveraddr, &serverlen);
    if (n < 0) 
      error("ERROR in recvfrom");
    printf("Echo from server: %50x %d\n", buf, n);
    int i =0;
    for(i = 0; i < n; i++)
	printf("%02x\n",buf[i]);
    return 0;
}

