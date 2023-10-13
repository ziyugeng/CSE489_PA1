// server.h
#ifndef SERVER_H
#define SERVER_H

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256

void start_server(char *port_str);
char* get_ip();

#endif
