#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")	
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
using namespace std;

#define DEFAULT_BUFLEN 512

int cnt = 0;
char userAdrr[40][256];
char userPort[40][256];

struct Product {
	char name[50];
	int amount;
};


Product *product = new Product[500];

HOSTENT *hostbuf = new HOSTENT[100];
HANDLE *handbuf = new HANDLE[100];
SOCKET *sockbuf = new SOCKET[100];


// макрос для печати количества активных пользователей 
#define PRINTNUSERS if (number_of_clients) printf("%d user online\n",number_of_clients);else printf("No User online\n");

// прототип функции, обслуживающий подключившихся пользователей
DWORD WINAPI ServClient(LPVOID clsocket);

// глобальная переменная – количество активных пользователей 
int number_of_clients = 0;

DWORD WINAPI myAccept(LPVOID mysocket);

int readln(SOCKET &clsocket, char *recvbuf, int recvbuflen){
	int result = 0;
	int k = 0;
	while (k<recvbuflen){
		result = recv(clsocket, recvbuf + k, recvbuflen - k, 0);
		if (result == SOCKET_ERROR){
			printf("Error at socket(): %ld\n", WSAGetLastError());
			return 0;
		}
		k = k + result;
	}
	return 1;
}

int main(){
	char *exitbuf = new char[DEFAULT_BUFLEN];
	int result;
	WSADATA wsaData;
	SOCKET mysocket = INVALID_SOCKET;
	SOCKET* pmysocket = new SOCKET;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != NO_ERROR) {
		printf("WSAStartup failed: %d\n", result);
		return 1;
	}
	// Создание сокета
	int domain = AF_INET; //для передачи с использованием стека протоколов TCP/IP
	int type = SOCK_STREAM; //для организации надёжного канала связи с установлением соединений
	int protocol = 0;	// default
	mysocket = socket(domain, type, protocol);
	printf("Listening starting... \n");
	printf("You can quit, show, kill, killall\n");
	if (mysocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Организация соединения
	sockaddr_in service;
	service.sin_family = AF_INET; // Коммуникационный домен
	service.sin_addr.s_addr = inet_addr("192.168.0.101");
	service.sin_port = htons(19604); // Номер порта
	//Привязывание сокета
	result = bind(mysocket, (SOCKADDR*)&service, sizeof(service));

	// Перевод в режим прослушивания
	// размер очереди – 25
	listen(mysocket, 25);

	// Вызов нового потока для обслуживания клиента
	DWORD thID;
	*pmysocket = mysocket;
	HANDLE h = CreateThread(NULL, NULL, myAccept, pmysocket, NULL, &thID);
	while (true){
		fgets(&exitbuf[0], DEFAULT_BUFLEN, stdin);
		if (!strcmp(&exitbuf[0], "quit\n")){
			printf("Exit...");
			break;

		}
		if (!strcmp(&exitbuf[0], "show\n")){
			for (int i = 0; i < number_of_clients; i++){
				printf("%d____%s:%s \n", i, userAdrr[i], userPort[i]);
			}

		}

		if (!strcmp(&exitbuf[0], "kill\n")){
			int k = 0;
			printf("Client ID: ");
			cin >> k;
			send(sockbuf[k], "kill from server.\n", DEFAULT_BUFLEN, 0);
			shutdown(sockbuf[k], SD_BOTH);
			closesocket(sockbuf[k]);

			// Корректный выход
			printf("Kill Client with id= %d\n", k);
			for (int i = k; i < number_of_clients - 1; i++){
				sockbuf[k] = sockbuf[k + 1];
				handbuf[k] = handbuf[k + 1];
				hostbuf[k] = hostbuf[k + 1];
			}
			WaitForSingleObject(&handbuf[k], INFINITE);
			number_of_clients--;
			PRINTNUSERS
		}
		
		if (!strcmp(&exitbuf[0], "killall\n")){
			int cnt = number_of_clients;
			for (int k = 0; k < cnt; k++)
			{
				send(sockbuf[k], "kill from server.\n", DEFAULT_BUFLEN, 0);
				shutdown(sockbuf[k], SD_BOTH);
				closesocket(sockbuf[k]);

				// Корректный выход
				printf("Kill Client with id= %d\n", k);
				for (int i = k; i < number_of_clients - 1; i++){
					sockbuf[k] = sockbuf[k + 1];
					handbuf[k] = handbuf[k + 1];
					hostbuf[k] = hostbuf[k + 1];
				}
				WaitForSingleObject(&handbuf[k], INFINITE);
				number_of_clients--;
				PRINTNUSERS
			}
		}
	}


	closesocket(mysocket);
	WaitForSingleObject(h, INFINITE);				
	
	// Закрытие сокета
	WSACleanup();
	system("pause");
	//closesocket(mysocket);	
	return 0;
}

DWORD WINAPI myAccept(LPVOID mysocket)
{

	SOCKET new_sock;
	new_sock = *((SOCKET *)mysocket);
	// отслеживать поступление входящих соединений
	SOCKET clsocket = INVALID_SOCKET;

	sockaddr_in client_addr; // адрес клиента (заполняется системой)
	// функции accept необходимо передать размер структуры
	int client_addr_size = sizeof(client_addr);

	while (true){
		clsocket = accept(new_sock, (sockaddr *)&client_addr, &client_addr_size);
		if (clsocket == -1) {
			break;
		}
		sockbuf[number_of_clients] = clsocket;
		number_of_clients++; // увеличиваем счетчик подключившихся клиентов

		// пытаемся получить имя хоста
		HOSTENT *hst;
		hst = gethostbyaddr((char *)&client_addr.sin_addr.s_addr, 4, AF_INET);
		//hostbuf[number_of_clients - 1] = *((HOSTENT *)hst);
		strcpy(userAdrr[number_of_clients - 1], inet_ntoa(client_addr.sin_addr));
		_itoa(ntohs(client_addr.sin_port), userPort[number_of_clients - 1], 10);
		// вывод сведений о клиенте
		printf("%s:%d new connect!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		PRINTNUSERS
		// Вызов нового потока для обслужbвания клиента
		DWORD thID;
		HANDLE h = CreateThread(NULL, NULL, ServClient, &clsocket, NULL, &thID);
		handbuf[number_of_clients - 1] = h;
	}

	for (int i = 0; i<number_of_clients; i++)
	{
		shutdown(sockbuf[i], SD_BOTH);
		closesocket(sockbuf[i]);

	}
	DWORD thID;
	WaitForMultipleObjects(number_of_clients, &handbuf[0], true, INFINITE);

	return 0;
}


// обсуживает очередного подключившегося клиента независимо от остальных
DWORD WINAPI ServClient(LPVOID clsocket)
{
	SOCKET new_sock;
	int result;
	new_sock = ((SOCKET *)clsocket)[0];

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	char *sendbuf = "You connect to server!\n";
	char *sendid;

	bool flag = false;
	//Передача и приём данных по протоколу TCP
	readln(new_sock, recvbuf, sizeof(recvbuf));
	char k[16];
	sprintf(k, "%d", cnt);
	sendid = k;
	send(new_sock, sendid, DEFAULT_BUFLEN, 0);
	send(new_sock, sendbuf, DEFAULT_BUFLEN, 0);

	do {

		result = readln(new_sock, recvbuf, sizeof(recvbuf));

		if (!strcmp(&recvbuf[0], "exit\n")){
			readln(new_sock, recvbuf, sizeof(recvbuf));
			int id = atoi(recvbuf);
			shutdown(new_sock, SD_BOTH);
			closesocket(new_sock);
			for (int i = id; i<number_of_clients - 1; i++){
				sockbuf[id] = sockbuf[id + 1];
				handbuf[id] = handbuf[id + 1];
				hostbuf[id] = hostbuf[id + 1];
			}
			WaitForSingleObject(&handbuf[id], INFINITE);
			number_of_clients--;
			PRINTNUSERS
				break;
		}
		if (result > 0){
			printf("bytes received: %d\n", result);
			printf("message from client: %s\n", recvbuf);

			if (!strcmp(&recvbuf[0], "show\n")){
				//Sleep(30000);
				if (cnt != 0){
					char k[16];
					sprintf(k, "%d", cnt);
					sendbuf = k;
					send(new_sock, sendbuf, DEFAULT_BUFLEN, 0);
					for (int i = 0; i<cnt; i++){
						char str[100];
						char buf[16]; // need a buffer for that

						sprintf(buf, "%d", i);

						const char* p = buf;
						strcpy(str, "ID_");

						strcat(str, p);
						strcat(str, " Name: ");
						strcat(str, (char *)product[i].name);
						strcat(str, " Amount: ");

						sprintf(buf, "%d", product[i].amount);
						p = buf;
						strcat(str, p);
						strcat(str, "\n");
						sendbuf = str;

						send(new_sock, sendbuf, DEFAULT_BUFLEN, 0);
					}
					send(new_sock, "All list\n", DEFAULT_BUFLEN, 0);
				}
				else {
					send(new_sock, "0", DEFAULT_BUFLEN, 0);
					send(new_sock, "No product\n", DEFAULT_BUFLEN, 0);
				}

			}
			else if (!strcmp(&recvbuf[0], "add\n")){

				readln(new_sock, recvbuf, sizeof(recvbuf));
				bool flags = false;
				int tmp = 0;
				for (int i = 0; i<cnt; i++){
					if (!strcmp(&recvbuf[0], product[i].name)){
						flags = true;
						tmp = i;
					}
				}
				if (flags == false){
					strcpy(product[cnt].name, recvbuf);
					printf("Name: %s\n", recvbuf);

					readln(new_sock, recvbuf, sizeof(recvbuf));
					product[cnt].amount = atoi(recvbuf);
					printf("Amount: %s\n", recvbuf);
					send(new_sock, "Successfully. New product added.\n", DEFAULT_BUFLEN, 0);
					cnt++;
				}
				else{
					readln(new_sock, recvbuf, sizeof(recvbuf));
					product[tmp].amount = product[tmp].amount + atoi(recvbuf);
					send(new_sock, "Successfully. You update.\n", DEFAULT_BUFLEN, 0);
				}

			}
			else if (!strcmp(&recvbuf[0], "buy\n")){
				if (cnt == 0){
					readln(new_sock, recvbuf, sizeof(recvbuf));
					readln(new_sock, recvbuf, sizeof(recvbuf));
					send(new_sock, "Nothing products!\n", DEFAULT_BUFLEN, 0);
				}
				else{
					readln(new_sock, recvbuf, sizeof(recvbuf));
					int id = atoi(recvbuf);
					readln(new_sock, recvbuf, sizeof(recvbuf));
					int test_amount = atoi(recvbuf);
					if (id>cnt || id<0){
						send(new_sock, "This's number of id not exist!\n", DEFAULT_BUFLEN, 0);
					}
					else{

						if (test_amount>product[id].amount){
							char str[100];
							strcpy(str, "Failed buy. Max amount ");

							char buf[16]; 
							sprintf(buf, "%d", product[id].amount);
							const char* p = buf;

							strcat(str, p);
							strcat(str, ". Try again.\n");
							send(new_sock, str, DEFAULT_BUFLEN, 0);
						}
						else{
							char str[100];
							strcpy(str, "You buy: ");
							strcat(str, (char *)product[id].name);
							strcat(str, " Amount: ");

							char buf[16];
							sprintf(buf, "%d", test_amount);
							const char* p = buf;

							strcat(str, p);
							strcat(str, "\n");
							send(new_sock, str, DEFAULT_BUFLEN, 0);

							product[id].amount = product[id].amount - test_amount;
							if (product[id].amount == 0){
								for (id; id<cnt - 1; id++){
									strcpy(product[id].name, (char *)product[id + 1].name);
									product[id].amount = product[id + 1].amount;
									cnt--;
								}
							}
						}
					}
				}
			}
			else
				send(new_sock, sendbuf, DEFAULT_BUFLEN, 0);
		}
		else if (result == 0){
			printf("Connection closed\n");
			break;
		}
		else{
			printf("recv failed: %d\n", WSAGetLastError());
			break;
		}

	} while (1);
	return 0;
}