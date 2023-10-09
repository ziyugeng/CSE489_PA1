/**
 * @ubitname_assignment1
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
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"
#include "../include/client.h"

void start_server(char* port);
void start_client(char *server_ip, char *server_port);


/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	if(argc != 3) {
        fprintf(stderr, "Usage: %s <s/c> <port_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	// 3.3 Running your program
	// s - server, c - client, port number
	int port = atoi(argv[2]);
	if (argv[1][0] == 's'){
		start_server(argv[2]);
	}
	else if(argv[1][0] == 'c'){
		start_client("127.0.0.1", argv[2]);
	}
	else{
		printf("Error: Please use 's' for server and 'c' for client.\n");
	}
	return 0;
}



