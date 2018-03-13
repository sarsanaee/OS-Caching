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
#include <unistd.h>
#include <time.h>
#include <math.h>



#define BUFSIZE 1024
#define MAXREQ 1000


float nextTime(float rateParameter)
{
    float temp = -logf(1.0f - (float) random() / (RAND_MAX)) / rateParameter;
    return temp;
}

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
    char buf[BUFSIZE] = "6666666666666666666666666666666666666666666666gggggggggggggggggggggggggggggggggggggg";
    //char buf[BUFSIZE] = "66666";
    char buf_rec[BUFSIZE];
    struct timeval time_before, time_next;
    int diff;
    int now;
    int sent_requests = 0;
	

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

	while(sent_requests < MAXREQ)
	{
	    now = nextTime(200) * 1000000;
	    usleep(now);
	    /* get a message from the user */
	    //bzero(buf_rec, BUFSIZE);
	    //printf("Please enter msg: ");
	    //fgets(buf, BUFSIZE, stdin);

	    /* send the message to the server */
	    serverlen = sizeof(serveraddr);
	    gettimeofday (&time_before, NULL);
	    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
	    if (n < 0) 
		 error("ERROR in sendto");
	    
	    /* print the server's reply */
	    //printf("stop\n");
	    n = recvfrom(sockfd, buf_rec, strlen(buf_rec), 0, &serveraddr, &serverlen);
         gettimeofday(&time_next, NULL);
	    int diff;
    	    diff = (time_next.tv_sec - time_before.tv_sec)*1000000 + (time_next.tv_usec - time_before.tv_usec);
    	    printf("%d\n", diff);
	    if (n < 0) 
		 error("ERROR in recvfrom");
	    //printf("Echo from server: %s\n", buf_rec);
		sent_requests++;

	}
    return 0;
}
