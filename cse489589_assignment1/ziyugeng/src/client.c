/**
* @client
* @author  Ziyu Geng <ziyugeng@buffalo.edu>, Tianrui Tu <ttu4@buffalo.edu>
* @version 1.0
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details at
* http://www.gnu.org/copyleft/gpl.html
*
* @section DESCRIPTION
*
* This file contains the client.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>

#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define google_ip "8.8.8.8"
#define google_port "53"
#define ubit "ziyugeng"
#define login "login successful"

int connect_to_host(char *server_ip, char *server_port);
void start_client(char *server_ip, char *server_port);
#include "../include/server.h"

typedef struct {
	int list_id;
	char* hostname[100];
    char* ip[16];
    char* port;
} client;



void start_client(char *server_ip, char *server_port)
{
	int server;
	int logedin = 0;
	client client_info;
	client_info.list_id = 0;
	strncpy(client_info.hostname, "your_hostname", sizeof(client_info.hostname)); 

	server = connect_to_host(server_ip, server_port);
	
	while(TRUE){
		printf("\n[PA1-Client@CSE489/589]$ ");
		fflush(stdout);
		
		char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
		memset(msg, '\0', MSG_SIZE);
		if(fgets(msg, MSG_SIZE-1, stdin) == NULL)
			exit(-1);
		
		printf("I got: %s(size:%zu chars)\n", msg, strlen(msg));
		
		printf("\nSENDing it to the remote server ... ");
		if(send(server, msg, strlen(msg), 0) == strlen(msg))
			printf("Done!\n");
		fflush(stdout);
		char* ip_addr = get_ip();

		msg[strcspn(msg, "\n")] = 0;
		if(strcmp(msg, "AUTHOR") == 0) {
			cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
			cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", ubit);
		    cse4589_print_and_log("[AUTHOR:END]\n");
			}
		else if (strcmp(msg, "IP") == 0){
			
			cse4589_print_and_log("[IP:SUCCESS]\n");
			cse4589_print_and_log("IP:%s\n", ip_addr);
			cse4589_print_and_log("[IP:END]\n");
			}
		else if (strcmp(msg, "PORT") == 0){ //PORT
			cse4589_print_and_log("[PORT:SUCCESS]\n");
			cse4589_print_and_log("PORT:%s\n", server_port);
			cse4589_print_and_log("[PORT:END]\n");
			}
		else if (strcmp(msg, "LOGIN") == 0){
			strncpy(client_info.ip, ip_addr, sizeof(client_info.ip));
			strncpy(client_info.port, server_port, sizeof(client_info.port));
			int bytes_sent = send(server, (void*)&client_info, sizeof(client), 0);
			if(bytes_sent < 0) {
				perror("Error sending client info to server");
				}
			logedin = 1; 
				
		}
		else {
			cse4589_print_and_log("[%s:ERROR]\n", msg);
			cse4589_print_and_log("[%s:END]\n", msg);
			}
		
		/* Initialize buffer to receive response */
		char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
		memset(buffer, '\0', BUFFER_SIZE);
		
		if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
			printf("Server responded: %s", buffer);
			fflush(stdout);
		}

		
		free(msg);
		free(buffer);
	}
}

int connect_to_host(char *server_ip, char* server_port){
	int fdsocket;
	struct addrinfo hints, *res;

	/* Set up hints structure */	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	/* Fill up address structures */	
	if (getaddrinfo(server_ip, server_port, &hints, &res) != 0)
		perror("getaddrinfo failed");

	/* Socket */
	fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(fdsocket < 0)
		perror("Failed to create socket");
	
	/* Connect */
	if(connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0)
		perror("Connect failed");
	
	freeaddrinfo(res);

	return fdsocket;
}


// The above code refers to the pa1_demo_code-client.c