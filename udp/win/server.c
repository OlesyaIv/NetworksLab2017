#define _CRT_SECURE_NO_WARNING
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>
#include <locale.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include "windows.h"
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <direct.h>

#pragma comment (lib, "Ws2_32.lib")

#define PORT 1500

int main(int argc, char* argv[])
{

	SOCKET sockfd;
	sockaddr_in local_addr;

	char buff[1024];
	printf("UDP Server\n");

	unsigned int t;
	t = WSAStartup(0x202, (WSADATA *)&buff[0]);
	if (t)
	{
		printf("Error WSAStartup : %d\n", WSAGetLastError());
		return -1;
	}


	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		printf("Socket() error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (sockaddr *)&local_addr, sizeof(local_addr)))
	{
		printf("bind error: %d\n", WSAGetLastError());
		closesocket(sockfd);
		WSACleanup();
		return -1;
	}

	while (1){
		sockaddr_in client_addr;
		int client_addr_size = sizeof(client_addr);
		int bytes_recv = recvfrom(sockfd, &buff[0], sizeof(buff) - 1, 0,
			(sockaddr *)&client_addr, &client_addr_size);
		if (bytes_recv == SOCKET_ERROR)
			printf("recvfrom() error: %d\n", WSAGetLastError());

		HOSTENT *serv;
		serv = gethostbyaddr((char *)&client_addr.sin_addr, 4, AF_INET);
		buff[bytes_recv] = 0;
		printf("%s:%d new message: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), &buff[0]);

		const char* buff2 = "I got your message";

		sendto(sockfd, &buff2[0], sizeof(buff), 0,
			(sockaddr *)&client_addr, sizeof(client_addr));
	}
}