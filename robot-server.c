/*
*****************************************************************************************
*
*        		===============================================
*           		Rapid Rescuer (RR) Theme (eYRC 2019-20)
*        		===============================================
*
*  This script is to implement Task 1B of Rapid Rescuer (RR) Theme (eYRC 2019-20).
*  
*  This software is made available on an "AS IS WHERE IS BASIS".
*  Licensee/end user indemnifies and will keep e-Yantra indemnified from
*  any and all claim(s) that emanate from the use of the Software or 
*  breach of the terms of this agreement.
*  
*  e-Yantra - An MHRD project under National Mission on Education using ICT (NMEICT)
*
*****************************************************************************************
*/

/*
* Team ID:			[ Team-ID ]
* Author List:		[ Names of team members worked on this file separated by Comma: Name1, Name2, ... ]
* Filename:			robot-server.c
* Functions:		socket_create, receive_from_send_to_client
* 					[ Comma separated list of functions in this file ]
* Global variables:	SERVER_PORT, RX_BUFFER_SIZE, TX_BUFFER_SIZE, MAXCHAR,
* 					dest_addr, source_addr, rx_buffer, tx_buffer,
* 					ipv4_addr_str, ipv4_addr_str_client, listen_sock, line_data, input_fp, output_fp
* 					[ List of global variables defined in this file ]
*/


// Include necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>


// Constants defined
#define SERVER_PORT 3333
#define RX_BUFFER_SIZE 1024
#define TX_BUFFER_SIZE 1024

#define MAXCHAR 1000				// max characters to read from txt file

// Global variables
struct sockaddr_in dest_addr;
struct sockaddr_in source_addr;

char rx_buffer[RX_BUFFER_SIZE];		// buffer to store data from client
char tx_buffer[RX_BUFFER_SIZE];		// buffer to store data to be sent to client

char ipv4_addr_str[128];			// buffer to store IPv4 addresses as string
char ipv4_addr_str_client[128];		// buffer to store IPv4 addresses as string

int listen_sock;

char line_data[MAXCHAR];

FILE *input_fp, *output_fp;


/*
* Function Name:	socket_create
* Inputs:			dest_addr [ structure type for destination address ]
* 					source_addr [ structure type for source address ]
* Outputs: 			my_sock [ socket value, if connection is properly created ]
* Purpose: 			the function creates the socket connection with the server
* Example call: 	int sock = socket_create(dest_addr, source_addr);
*/
int socket_create(struct sockaddr_in dest_addr, struct sockaddr_in source_addr){

	int addr_family;
	int ip_protocol;

	dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(SERVER_PORT);
	addr_family = AF_INET;
	ip_protocol = IPPROTO_IP;

	int my_sock;

	printf("Self IP: %s\n", inet_ntoa(dest_addr.sin_addr));

	if((my_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == -1)
	{
		perror("Socket can't be created.");
		exit(0);
	}
	else
	{
		printf("Socket created\n");
	}

	if( bind(my_sock,(struct sockaddr *)&dest_addr , sizeof(dest_addr)) < 0)
	{
		puts("bind failed");
		exit(0);
	}
	else
	{
		printf("Socket binded, port %d\n", SERVER_PORT);
	}

	listen(my_sock, 3);
	puts("Socket listening");

	return my_sock;
}


/*
* Function Name:	receive_from_send_to_client
* Inputs:			sock [ socket value, if connection is properly created ]
* Outputs: 			None
* Purpose: 			the function receives the data from server and updates the 'rx_buffer'
*					variable with it, sends the obstacle position based on obstacle_pos.txt
*					file and sends this information to the client in the provided format.
* Example call: 	receive_from_send_to_client(sock);
*/
int receive_from_send_to_client(int sock){

	FILE *fp, *fptr;
	fp = fopen("obstacle_pos.txt", "r");
	fptr = fopen("obstacle_pos.txt", "r");
	int new_socket;
	int count_val = 0;
	int count = 0;
	char ch;
	char ptr, test_eof;

	int c = sizeof(struct sockaddr_in);
	new_socket = accept(sock, (struct sockaddr *)&dest_addr, (socklen_t*)&c);

	if (new_socket < 0){

		perror("accept failed");
		exit(0);
	}
	puts("Socket accepted");

	int received_data = 1;
	int flag = 0;                       //if maze no. found in file mark it 1.
	int file_end = 0;                   //if maze no. not found in file mark it 1.

	while(received_data){

		printf("\n_______________________________\n");

		flag = 0;
		file_end = 0;
		count_val = 0;
		int count_pos = 1;

		received_data = recv(new_socket, rx_buffer, RX_BUFFER_SIZE, 0);
		rx_buffer[received_data] = '\0';

		if(received_data){

			printf("Received %d bytes from %s:\n", received_data - 2, inet_ntoa(dest_addr.sin_addr));
			printf("Data received = %s", rx_buffer);
			fseek(fp, 0, SEEK_SET);

			while(1){                      //find the line which contain obstacle coordinate for corresponding maze

				if(count == 1){            

					break;
				}

				ch = fgetc(fp);
				count_pos++;
				if(ch == rx_buffer[0]){

					ch = fgetc(fp);
					if(ch == ':'){

						fseek(fptr, count_pos + 1, SEEK_SET);
						flag = 1;
					}
				}
				else if(flag == 1){

					count = 1;
					break;
				}
				else if(ch == EOF){

					file_end = 1;
					break;
				}
			}

			if(file_end == 1){             //if maze has no obstacle

				tx_buffer[0] = '@';
				tx_buffer[1] = '$';
				tx_buffer[2] = '@';
				tx_buffer[3] = '\0';
				count_val = 3;
			}

			while(1){

				if(file_end == 1){

					break;
				}
				ptr = fgetc(fptr);
				if(ptr == '('){

					count_val = 0;
					tx_buffer[count_val] = '@';
					count_val++;

					while(ptr != ')'){

						tx_buffer[count_val] = ptr;
						count_val++;
						ptr = fgetc(fptr);
					}
					tx_buffer[count_val++] = ')';
					tx_buffer[count_val++] = '@';
					tx_buffer[count_val++] = '\0';
					break;
				}	

				else if(ptr == '\n'){

					tx_buffer[0] = '@';
					tx_buffer[1] = '$';
					tx_buffer[2] = '@';
					tx_buffer[3] = '\0';
					count_val = 3;
					count = 0;
					break;
				}
			}
			
			write(new_socket, tx_buffer, count_val+1);

			printf("\n%d bytes of data sent = %s", count_val, tx_buffer);

		}
		else{

			printf("\n_______________________________\n");
			close(new_socket);
			close(sock);
			printf("Connection closed\n");
			printf("Shutting down socket.\n");
			fclose(fp);
			fclose(fptr);
		}
	}
	
	return 0;

}


/*
* Function Name:	main()
* Inputs:			None
* Outputs: 			None
* Purpose: 			the function solves Task 1B problem statement by making call to
* 					functions socket_create() and receive_from_send_to_client()
*/
int main() {
	
    char *input_file_name = "obstacle_pos.txt";
	char *output_file_name = "data_from_client.txt";

	// Create socket and accept connection from client
	int sock = socket_create(dest_addr, source_addr);

	input_fp = fopen(input_file_name, "r");

	if (input_fp == NULL){
		printf("Could not open file %s\n",input_file_name);
		return 1;
	}

	fgets(line_data, MAXCHAR, input_fp);

	output_fp = fopen(output_file_name, "w");

	if (output_fp == NULL){
		printf("Could not open file %s\n",output_file_name);
		return 1;
	}

	while (1) {

		// Receive and send data from client and get the new shortest path
		receive_from_send_to_client(sock);

		// NOTE: YOU ARE NOT ALLOWED TO MAKE ANY CHANGE HERE
		fputs(rx_buffer, output_fp);
		fputs("\n", output_fp);

	}

	return 0;
}

