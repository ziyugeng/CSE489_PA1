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

int connect_to_host(char *server_ip, char *server_port);
void start_client(char *server_ip, char *server_port);
#include "../include/server.h"
char* get_ip();


int isnum(char* str) { // check all part is number
    while (*str) {
        if (*str < '0' || *str > '9') {
            return 0;
        }
        str++;
    }
    return 1;
}

int valid_port(char* port) {
    int num;

    if (port && isnum(port)) { 
        num = atoi(port);
        if (num >= 0 && num <= 65535) { //get the range from 10.13 OH)
            return 1;
        }
    }
    return 0;
}

int valid_ip(char *ip) {
    char *first_part, *second_part, *third_part, *forth_part;

    char *ipCopy = strdup(ip);

    first_part = strtok(ipCopy, ".");
    second_part = strtok(NULL, ".");
    third_part = strtok(NULL, ".");
    forth_part = strtok(NULL, ".");

    if (!first_part || !second_part || !third_part || !forth_part) {
        free(ipCopy);
        return 0;
    }

	if (!isnum(first_part) || !isnum(second_part) || !isnum(third_part) || !isnum(forth_part)) {
        free(ipCopy);
        return 0;
    }

    int a = atoi(first_part);
    int b = atoi(second_part);
    int c = atoi(third_part);
    int d = atoi(forth_part);

    free(ipCopy);

    if ((a >= 0 && a <= 255) && (b >= 0 && b <= 255) && (c >= 0 && c <= 255) && (d >= 0 && d <= 255)) {
        return 1; //get the range from 10.13 OH)
    }
    else {
        return 0;
    }
}

void start_client(char *server_ip, char *server_port)
{
	int server;
	// server = connect_to_host(server_ip, server_port);
	
	
	while(TRUE){
		printf("\n[PA1-Client@CSE489/589]$ ");
		fflush(stdout);
		
		char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
		memset(msg, '\0', MSG_SIZE);
		if(fgets(msg, MSG_SIZE-1, stdin) == NULL)
			exit(-1);

		msg[strcspn(msg, "\n")] = 0; // Removing newline character

        char *login_cmd = strtok(msg, " "); 
		if(strcmp(login_cmd, "LOGIN") == 0) {
			char *new_ip = strtok(NULL, " ");
			char *new_port = strtok(NULL, " ");
			
		    if (valid_ip(new_ip) && valid_port(new_port)) { 
                server_ip = strdup(new_ip);
                server_port = strdup(new_port); 

                server = connect_to_host(server_ip, server_port);
                cse4589_print_and_log("[LOGIN:SUCCESS]\n");
            } else {
                cse4589_print_and_log("[LOGIN:ERROR IP or PORT]\n");
                cse4589_print_and_log("[LOGIN:END]\n");
            }
		}

        char* ip_addr = get_ip();
		printf("I got: %s(size:%zu chars)\n", msg, strlen(msg));
		
		printf("\nSENDing it to the remote server ... ");
		if(send(server, msg, strlen(msg), 0) == strlen(msg))
			printf("Done!\n");
		fflush(stdout);

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
		// else if (strcmp(msg, "EXIT") == 0){
		// 	close(server); 
		// 	cse4589_print_and_log("[EXIT:SUCCESS]\n");
		// 	cse4589_print_and_log("[EXIT:END]\n");
		// 	exit(0);  
		// }
		else {
			cse4589_print_and_log("[%s:ERROR]\n", msg);
			cse4589_print_and_log("[%s:END]\n", msg);
			}
		
		/* Initialize buffer to receive response */
		char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
		memset(buffer, '\0', BUFFER_SIZE);

		buffer[strcspn(buffer, "\n")] = 0;
		
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