#include "stdio.h"
#include "netinet/in.h"
#include "iostream"
#include "arpa/inet.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"

#define SERVERIP "192.168.0.101"

#define DEFAULT_BUFLEN 512

int readn(int socket, char *message, int length) {
    int result = 0;
    int k=0;
    while (k < length) {
        result = recv(socket, message+k, length-k, 0);
        if (result ==0){
            printf("Error at socket");
            system("pause");
            exit(2);
        }

        k = k+result;
    }
    return 1;
}

int main() {
    int sock;
    sockaddr_in addr{};
    char *sendbuf=new char [DEFAULT_BUFLEN];
    char *getIP=new char [14];
    char *id= new char[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];
    int result;

    // задали сокет
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Can't create socket");
        system("pause");
        return -1;
    }

    // задали адрес
    addr.sin_family = AF_INET;
    addr.sin_port = htons(19604);
    addr.sin_addr.s_addr = inet_addr(SERVERIP);

    // установление соединения с сервером со стороны клиента
    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        printf("Сonnection error");
        system("pause");
        return -2;
    }

    if(send(sock, "New client connect", DEFAULT_BUFLEN, 0)==-1){
        printf("sent failed with error");
        close(sock);
        system("pause");
        return 1;
    }

    printf("Client started..\n");
    printf("===============================================\n");
    printf("Hello! This is shop and you can...\n");
    printf("===============================================\n");
    printf("| ADD NAME AMOUNT                             |\n");
    printf("| BUY NAME AMOUNT                             |\n");
    printf("| SHOW                                        |\n");
    printf("| EXIT                                        |\n");
    printf("===============================================\n");
    readn(sock, recvbuf, sizeof(recvbuf));
	strcpy( id, recvbuf);

    while (1) {
        readn(sock, recvbuf, sizeof(recvbuf));


        printf("Message FROM Shop:%s", recvbuf);
        printf("Message TO Shop:");

        fgets(&sendbuf[0], DEFAULT_BUFLEN, stdin);
        // отправляем message на сервер
        send(sock, sendbuf, DEFAULT_BUFLEN, 0);

        if (!strcmp(&sendbuf[0],"add\n")){
		 printf("Name product: ");
		 fgets(&sendbuf[0],DEFAULT_BUFLEN,stdin);
		 send(sock,sendbuf,DEFAULT_BUFLEN,0);
		 printf("Amount product: ");
		 fgets(&sendbuf[0],DEFAULT_BUFLEN,stdin);
		 send(sock,sendbuf,DEFAULT_BUFLEN,0);
      }
	  if (!strcmp(&sendbuf[0],"show\n")){
		 readn(sock, recvbuf, sizeof(recvbuf));
		 int count = atoi(recvbuf);
		 for(int i=0; i<count; i++){
				readn(sock, recvbuf, sizeof(recvbuf));
				printf("%s",recvbuf);
		 }

      }
	if (!strcmp(&sendbuf[0],"buy\n")){
		 printf("Input ID product: ");
		 fgets(&sendbuf[0],DEFAULT_BUFLEN,stdin);
		 send(sock,sendbuf,DEFAULT_BUFLEN,0);
		 printf("Input amount product: ");
		 fgets(&sendbuf[0],DEFAULT_BUFLEN,stdin);
		 send(sock,sendbuf,DEFAULT_BUFLEN,0);
      }
        if(!strcmp(&sendbuf[0], "exit\n")){
            printf("Exit...");
            send(sock,id,DEFAULT_BUFLEN,0);
            close(sock);
            system("pause");
            return 0;
        }
    }
    close(sock);

    return 0;
}
