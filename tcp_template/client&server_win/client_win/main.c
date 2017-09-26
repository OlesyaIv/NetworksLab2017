#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <netdb.h>
//#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

    /* Function redn */

int readn(int s, char* buf, int buflen, int f) {

    int result = 0;
    int recBytes = 0;

    while(recBytes < buflen) {

        result = read(s, buf + recBytes, buflen - recBytes);
//	result = recv(s, buf + recBytes, buflen, f);
        
	if (result < 1) {
        	printf("Error: NOT reading");
		exit(1);
        }

        recBytes += result;
    }
    
    return recBytes;
}


    /* Function main */
int main(int argc, char *argv[]) {
    int sockfd, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2),&wsaData!=0)){
	printf("ERROR: startup failed");
	return 1;
    }

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    memset((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, server-> h_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR: NOT connecting");
        exit(1);
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Please, enter the message: ");
    memset(buffer,0, 256);
    fgets(buffer, 255, stdin);

    /* Send message to the server */
    n = send(sockfd, buffer, strlen(buffer), 0);

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    /* Now read server response */
    memset(buffer, 256);
    n = recv(sockfd, buffer, 255, 0);

    if (n < 0) {
        perror("ERROR: NOT reading from socket");
        exit(1);
    }

    /* Closing socket */
    shutdown(sockfd, SHUT_RDWR);
    closesocket(sockfd);
    WSACleanup();

    printf("%s\n", buffer);
    return 0;
}