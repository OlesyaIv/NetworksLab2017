#define _CRT_SECURE_NO_WARNING
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#pragma comment (lib, "Ws2_32.lib")

#define PORT 1500
#define SERVERADDR "127.0.0.1"

int main(int argc, char* argv[])
{
	SOCKET sockfd;
	char buff[10 * 1024];
	HOSTENT *hst;
	sockaddr_in dest_addr;

	printf("UDP Client\n");

	unsigned int t;
	t = WSAStartup(0x202, (WSADATA *)&buff[0]);
	if (t !=0 )
	{
		printf("WSAStartup error: %d\n", WSAGetLastError());
		return -1;
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == INVALID_SOCKET)
	{
		printf("socket() error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else
	{
		if (hst = gethostbyname(SERVERADDR))
			dest_addr.sin_addr.s_addr = ((unsigned long **)hst->h_addr_list)[0][0];
		else
		{
			printf("Unknown host: %d\n", WSAGetLastError());
			closesocket(sockfd);
			WSACleanup();
			return -1;
		}
	}
	while (1)
	{
		printf("\nMessage TO Server:");
		fgets(&buff[0], sizeof(buff) - 1, stdin);

		if (!strcmp(&buff[0], "quit\n")) break;

		sendto(sockfd, &buff[0], strlen(&buff[0]), 0, (sockaddr *)&dest_addr, sizeof(dest_addr));

		sockaddr_in server_addr;
		int server_addr_size = sizeof(server_addr);

		int n = recvfrom(sockfd, &buff[0], sizeof(buff) - 1, 0, (sockaddr *)&server_addr, &server_addr_size);

		if (n == SOCKET_ERROR)
		{
			printf("recvfrom() error: %d\n", WSAGetLastError());
			closesocket(sockfd);
			WSACleanup();
			return -1;
		}

		buff[n] = 0;

		printf("\nMessage FROM Server:%s", &buff[0]);
	} 

	closesocket(sockfd);
	WSACleanup();
	return 0;
}