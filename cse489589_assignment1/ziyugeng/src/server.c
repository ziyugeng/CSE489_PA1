/**
* @server
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
* This file contains the server init and main while loop for tha application.
* Uses the select() API to multiplex between network I/O and STDIN.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256
#define ubit "ziyugeng"
#define google_ip "8.8.8.8"
#define google_port "53"
#define login "login successful"
#include "../include/client.h"

// step1: Create a UDP socket (this is not used for sending or receiving anything!)
// step2: connect the socket to some outside IP/port pair (I’d recommend 8.8.8.8:53 which is Google’s public DNS server). At this point, the kernel will give your UDP socket an IP which basically the first interface on a path to Google’s DNS server, and a port which is an random ephemeral port. Note that no packet is sent at all! The connect() command on an unbounded UDP socket forces the kernel to look up some information on the local routing table.
// step3: Call getsockname() on the dummy UDP socket to get the IP out. For the name, you can just use gethostname() for simplicity. If you are more careful, you should try gethostbyaddr() on the IP address that getsockname() returns to get a consistent name/IP pair. (gethostbyaddr() does a reverse DNS look up.)
// step4: Close the dummy UDP socket.
// Cite from https://ubmnc.wordpress.com/2010/09/22/on-getting-the-ip-name-of-a-machine-for-chatty/, which are provided in handout
// The get_ip function follows the https://stackoverflow.com/questions/25879280/getting-my-own-ip-address-by-connecting-using-udp-socket, and modified based on it.

// Reference
// 1. On getting the IP & name of a machine for chatty. SUNY Buffalo Networking Course’s Blog. (2010, October 22). https://ubmnc.wordpress.com/2010/09/22/on-getting-the-ip-name-of-a-machine-for-chatty/ 

// 2. EricEric2, & user207421user207421306k4444 gold badges307307 silver badges484484 bronze badges. (2014, September 18). Getting my own IP address by Connect()ing using UDP socket?. Stack Overflow. https://stackoverflow.com/questions/25879280/getting-my-own-ip-address-by-connecting-using-udp-socket 

char* get_ip() {
    int sockfd;
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen = sizeof(remoteaddr);
    char remoteIP[INET6_ADDRSTRLEN];
    char *ip_addr;

    struct addrinfo hints, *ai, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    // step1
    if (getaddrinfo("8.8.8.8", "53", &hints, &ai) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("UDP: socket");
            continue;
        }
        // step2
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("UDP: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "UDP: failed to bind socket\n");
        exit(EXIT_FAILURE);
    }

    // step3
    if (getsockname(sockfd, (struct sockaddr*)&remoteaddr, &addrlen) == -1) {
        perror("getsockname");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (remoteaddr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&remoteaddr;
        inet_ntop(AF_INET, &s->sin_addr, remoteIP, sizeof remoteIP);
    }
    else { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&remoteaddr;
        inet_ntop(AF_INET6, &s->sin6_addr, remoteIP, sizeof remoteIP);
    }
    printf("IP_ADDRESS: %s\n", remoteIP);

    freeaddrinfo(ai);

    // step4
    close(sockfd);

    ip_addr = strdup(remoteIP);
    if (ip_addr == NULL) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    return ip_addr;
}


void start_server(char *port_str) {
    int server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
	struct sockaddr_in client_addr;
	struct addrinfo hints, *res;
	fd_set master_list, watch_list;

	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
    	hints.ai_family = AF_INET;
    	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_flags = AI_PASSIVE;

	/* Fill up address structures */
	if (getaddrinfo(NULL, port_str, &hints, &res) != 0)
		perror("getaddrinfo failed");
	
	/* Socket */
	server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(server_socket < 0)
		perror("Cannot create socket");
	
	/* Bind */
	if(bind(server_socket, res->ai_addr, res->ai_addrlen) < 0 )
		perror("Bind failed");

	freeaddrinfo(res);
	
	/* Listen */
	if(listen(server_socket, BACKLOG) < 0)
		perror("Unable to listen on port");
	
	/* ---------------------------------------------------------------------------- */
	
	/* Zero select FD sets */
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	
	/* Register the listening socket */
	FD_SET(server_socket, &master_list);
	/* Register STDIN */
	FD_SET(STDIN, &master_list);
	
	head_socket = server_socket;
    
    while (TRUE) {
		memcpy(&watch_list, &master_list, sizeof(master_list));
		
		printf("\n[PA1-Server@CSE489/589]$ ");
		fflush(stdout);
		
		/* select() system call. This will BLOCK */
		selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
		if(selret < 0)
			perror("select failed.");
		
		/* Check if we have sockets/STDIN to process */
		if(selret > 0){
			/* Loop through socket descriptors to check which ones are ready */
			for(sock_index=0; sock_index<=head_socket; sock_index+=1){
				
				if(FD_ISSET(sock_index, &watch_list)){
					
					/* Check if new command on STDIN */
					if (sock_index == STDIN){
						char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);
						
						memset(cmd, '\0', CMD_SIZE);
						if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
							exit(-1);
						
						printf("\nI got: %s\n", cmd);
						cmd[strcspn(cmd, "\n")] = 0; // remove \n
						
						//Process PA1 commands here ...

						if(strcmp(cmd, "AUTHOR") == 0) { // AUTHOR
							cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
							cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", ubit);
							cse4589_print_and_log("[AUTHOR:END]\n");
							} 
						else if (strcmp(cmd, "IP") == 0){ // IP
						char* ip_addr = get_ip();
							cse4589_print_and_log("[IP:SUCCESS]\n");
							cse4589_print_and_log("IP:%s\n", ip_addr);
							cse4589_print_and_log("[IP:END]\n");
						}
						else if (strcmp(cmd, "PORT") == 0){ //PORT
							cse4589_print_and_log("[PORT:SUCCESS]\n");
							cse4589_print_and_log("PORT:%s\n", port_str);
							cse4589_print_and_log("[PORT:END]\n");
						}
						else {
							cse4589_print_and_log("[%s:ERROR]\n", cmd);
							cse4589_print_and_log("[%s:END]\n", cmd);
							}
						
						free(cmd);
					}
					/* Check if new client is requesting connection */
					else if(sock_index == server_socket){
						caddr_len = sizeof(client_addr);
						fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
						if(fdaccept < 0)
							perror("Accept failed.");
						
						printf("\nRemote Host connected!\n");                        
						
						/* Add to watched socket list */
						FD_SET(fdaccept, &master_list);
						if(fdaccept > head_socket) head_socket = fdaccept;
					}
					/* Read from existing clients */
					else{
						/* Initialize buffer to receieve response */
						char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
						memset(buffer, '\0', BUFFER_SIZE);
						
						if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
							close(sock_index);
							printf("Remote Host terminated connection!\n");
							
							/* Remove from watched list */
							FD_CLR(sock_index, &master_list);
						}
						else {
							//Process incoming data from existing clients here ...
							buffer[strcspn(buffer, "\n")] = 0; // remove \n
							char *login_cmd = strtok(buffer, " "); 
							if(strcmp(buffer, "AUTHOR") == 0) { // AUTHOR
							cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
							cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", ubit);
							cse4589_print_and_log("[AUTHOR:END]\n");
							} 
						else if (strcmp(buffer, "IP") == 0){ // IP
				            char* ip_addr = get_ip();
							cse4589_print_and_log("[IP:SUCCESS]\n");
							cse4589_print_and_log("IP:%s\n", ip_addr);
							cse4589_print_and_log("[IP:END]\n");
						}
						else if (strcmp(buffer, "PORT") == 0){ //PORT
							cse4589_print_and_log("[PORT:SUCCESS]\n");
							cse4589_print_and_log("PORT:%s\n", port_str);
							cse4589_print_and_log("[PORT:END]\n");
						}
						
						else if (strcmp(login_cmd , "LOGIN") == 0){ //LOGIN
							char client_ip[INET_ADDRSTRLEN];
							inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
							int client_port = ntohs(client_addr.sin_port);
							cse4589_print_and_log("[LOGIN:SUCCESS]\n");
							cse4589_print_and_log("IP: %s, Port: %d\n", client_ip, client_port);
							cse4589_print_and_log("[LOGIN:END]\n");
						}
						else if(strcmp(buffer , "EXIT") == 0){
							// still need to remove client info
							close(server_socket);
							cse4589_print_and_log("[EXIT:SUCCESS]\n");
							cse4589_print_and_log("[EXIT:END]\n");
						}
						else {
							cse4589_print_and_log("[%s:ERROR]\n", buffer);
							cse4589_print_and_log("[%s:END]\n", buffer);
							}

							printf("\nClient sent me: %s\n", buffer);
							printf("ECHOing it back to the remote host ... ");
							if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
								printf("Done!\n");
							fflush(stdout);
						}
						
						free(buffer);
					}
				}
			}
		}
	}
    close(server_socket);
}
// The above code refers to the pa1_demo_code-servert.c


