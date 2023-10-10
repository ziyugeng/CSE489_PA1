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

// step1: Create a UDP socket (this is not used for sending or receiving anything!)
// step2: connect the socket to some outside IP/port pair (I’d recommend 8.8.8.8:53 which is Google’s public DNS server). At this point, the kernel will give your UDP socket an IP which basically the first interface on a path to Google’s DNS server, and a port which is an random ephemeral port. Note that no packet is sent at all! The connect() command on an unbounded UDP socket forces the kernel to look up some information on the local routing table.
// step3: Call getsockname() on the dummy UDP socket to get the IP out. For the name, you can just use gethostname() for simplicity. If you are more careful, you should try gethostbyaddr() on the IP address that getsockname() returns to get a consistent name/IP pair. (gethostbyaddr() does a reverse DNS look up.)
// step4: Close the dummy UDP socket.
// cite from https://ubmnc.wordpress.com/2010/09/22/on-getting-the-ip-name-of-a-machine-for-chatty/, which are provided in handout
char* get_ip() {
    char *ip = malloc(INET_ADDRSTRLEN);

    // Step 1
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket < 0) {
        perror("Cannot create socket");
        free(ip);
        return NULL;
    }

    // Step 2
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(google_ip);
    server_addr.sin_port = htons(google_port);
    
    if(connect(udp_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(udp_socket);
        free(ip);
        return NULL;
    }

    // Step 3
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    if(getsockname(udp_socket, (struct sockaddr*)&local_addr, &addr_len) < 0) {
        perror("Getsockname failed");
        close(udp_socket);
        free(ip);
        return NULL;
    }

    if(inet_ntop(AF_INET, &(local_addr.sin_addr), ip, INET_ADDRSTRLEN) == NULL) {
        perror("Error converting IP to string");
        close(udp_socket);
        free(ip);
        return NULL;
    }

    // Step 4
    close(udp_socket);
    return ip;
}


void start_client(char *server_ip, char *server_port)
{
	int server;
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
		
		/* Initialize buffer to receive response */
		char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
		memset(buffer, '\0', BUFFER_SIZE);
		
		if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
			printf("Server responded: %s", buffer);
			fflush(stdout);
		}

		if(strcmp(msg, "AUTHOR") == 0) { // AUTHOR
							cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
							cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", ubit);
							cse4589_print_and_log("[AUTHOR:END]\n");
							} 
						else if (strcmp(msg, "IP") == 0){ // IP
						char* ip_addr = get_ip();
							cse4589_print_and_log("[IP:SUCCESS]\n");
							cse4589_print_and_log("IP:%s\n", ip_addr);
							cse4589_print_and_log("[IP:END]\n");
						}
						else if (strcmp(msg, "PORT") == 0){ //PORT
							cse4589_print_and_log("[PORT:SUCCESS]\n");
							cse4589_print_and_log("PORT:%s\n", server_port);
							cse4589_print_and_log("[PORT:END]\n");
						}
						else {
							cse4589_print_and_log("[%s:ERROR]\n", msg);
							cse4589_print_and_log("[%s:END]\n", msg);
							}
		
		free(msg);
		free(buffer);
	}
}

int connect_to_host(char *server_ip, char* server_port)
{
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