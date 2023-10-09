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
#define ubit "ziyugeng and ttu4"
#define google_ip "8.8.8.8"
#define google_port "53"

// step1: Create a UDP socket (this is not used for sending or receiving anything!)
// step2: connect the socket to some outside IP/port pair (I’d recommend 8.8.8.8:53 which is Google’s public DNS server). At this point, the kernel will give your UDP socket an IP which basically the first interface on a path to Google’s DNS server, and a port which is an random ephemeral port. Note that no packet is sent at all! The connect() command on an unbounded UDP socket forces the kernel to look up some information on the local routing table.
// step3: Call getsockname() on the dummy UDP socket to get the IP out. For the name, you can just use gethostname() for simplicity. If you are more careful, you should try gethostbyaddr() on the IP address that getsockname() returns to get a consistent name/IP pair. (gethostbyaddr() does a reverse DNS look up.)
// step4: Close the dummy UDP socket.
// cite from https://ubmnc.wordpress.com/2010/09/22/on-getting-the-ip-name-of-a-machine-for-chatty/, which are provided in handout

char* external_ip() {
    int udp_socket;
    char *ip = malloc(INET_ADDRSTRLEN);
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if(!ip) {
        perror("Malloc failed");
        return NULL;
    }

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket < 0) {
        free(ip);
        return NULL;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(google_ip);
    server_addr.sin_port = htons(google_port);

    // if(connect(udp_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    //     perror("Connect failed");
    //     close(udp_socket);
    //     free(ip);
    //     return NULL;
    // }

    // if(getsockname(udp_socket, (struct sockaddr*)&client_addr, &addr_len) < 0) {
    //     perror("Getsockname failed");
    //     close(udp_socket);
    //     free(ip);
    //     return NULL;
    // }

    // if(inet_ntop(AF_INET, &(client_addr.sin_addr), ip, INET_ADDRSTRLEN) == NULL) {
    //     perror("Error converting IP to string");
    //     close(udp_socket);
    //     free(ip);
    //     return NULL;
    // }

    close(udp_socket);
    return ip;
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
						cmd[strcspn(cmd, "\n")] = 0;
						
						//Process PA1 commands here ...

						if(strcmp(cmd, "AUTHOR") == 0) { // AUTHOR
							cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
							cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", ubit);
							cse4589_print_and_log("[AUTHOR:END]\n");
							} 
						else if (strcmp(cmd, "IP") == 0){ // IP
						char* ip_addr = external_ip();
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


