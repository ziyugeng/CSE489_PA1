/**
* @server
* @author  Swetank Kumar Saha <swetankk@buffalo.edu>, Shivang Aggarwal <shivanga@buffalo.edu>
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
#include <string.h>
#include <unistd.h>

#define BACKLOG 5
#define BUFFER_SIZE 256

void start_server(char *port_str) {
    int server_socket, new_socket, max_fd, activity, i, n;
    struct sockaddr_in server_addr;
    struct sockaddr_storage server_storage;
    socklen_t addr_size;
    char buffer[BUFFER_SIZE];
    fd_set master_fds, read_fds;

    // Convert port string to integer
    int port = atoi(port_str);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    // Configure settings in address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

    // Bind socket to address struct
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen on the socket
    if (listen(server_socket, BACKLOG) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    // Clear the master and temp sets
    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);
    
    // Add server socket to set
    FD_SET(server_socket, &master_fds);
    max_fd = server_socket; // so far, it's this one

    printf("Server is listening on port: %d\n", port);
    
    while (1) {
        // Copy master FD set to read FD set for select()
        read_fds = master_fds;
        
        // Wait for activity on any of the sockets
        if ((activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL)) < 0) {
            perror("select failed");
            exit(EXIT_FAILURE);
        }
        
        // Check each socket for activity
        for (i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) { // if activity
                if (i == server_socket) {
                    // New connection on the listening socket
                    addr_size = sizeof server_storage;
                    new_socket = accept(server_socket, (struct sockaddr *) &server_storage, &addr_size);
                    
                    if (new_socket == -1) {
                        perror("Accept failed");
                    } else {
                        FD_SET(new_socket, &master_fds); // add new FD to master set
                        if (new_socket > max_fd) { // keep track of the max FD
                            max_fd = new_socket;
                        }
                        printf("New connection from socket FD %d\n", new_socket);
                    }
                } else {
                    // Data from an existing client
                    if ((n = recv(i, buffer, BUFFER_SIZE, 0)) <= 0) {
                        // Got error or connection closed by client
                        if (n == 0) {
                            printf("Socket FD %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master_fds); // remove from master set
                    } else {
                        // We got some data from a client
                        buffer[n] = '\0';
                        printf("Received message from FD %d: %s\n", i, buffer);
            
                        // For simplicity, let's just echo it back
                        if (send(i, buffer, n, 0) != n) {
                            perror("send");
                        }
                    }
                }
            }
        }
    }
    // Close the socket before we finish
    close(server_socket);
}


