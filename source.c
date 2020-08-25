#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h> //definitions for internet protocols
//#include <netdb.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h> //socket() syscall and necessary defs
#include <sys/types.h>

#define outPORT 80	//the port on which the ipv6 client will connect to the server (HTTP)
#define inPORT 22023 //the port on which the ipv4 client will connect
#define ADDRESS "2a00:1a80:2000:50:4552:494f:4e0a:50" //address of the ipv6 server

int callServer(){
	FILE *outputF; //output file
	int conn_fd, ret;
	char *message, server_reply[2000];
	struct sockaddr_in6 server_addr;

	server_addr.sin6_family = AF_INET6; //specifies that is IPV6 family
	server_addr.sin6_port = htons(outPORT); //assigns port 80; htons convers value to computer-friendly
	
	ret = inet_pton(AF_INET6, ADDRESS, &server_addr.sin6_addr); //converts to binary
	if(ret!=1) { // 1->successful conversion; 0->address doesn't contain a valid network; -1->address family is wrong
		if(ret==-1)
			perror("inte_pton");
		printf("Failed to convert address %d !\n", ADDRESS); //informs user
		exit(0);
	}

	printf("CONNECTING to %s on port : %d \n", ADDRESS, outPORT);
	conn_fd = socket(AF_INET6, SOCK_STREAM, 0); //atempt to create socket
	if(conn_fd == -1){
		perror("ERROR when creating socket!"); //to know error is socket related
		exit(0);
	}

	ret = connect(conn_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)); //establish connection through socket conn_fd
	if(ret == -1){
		perror("Can not connect!");
		exit(0);
	}
	else printf("Connection established!\n");

	message = "GET / HTTP/1.0\r\n\r\n"; //the command to send to the server
	if(send(conn_fd, message, strlen(message),0)<0) { //checks to see if the request was sent successfully
		puts("Send failed!");
		exit(0);
	}

	puts("Request sent \n");
	outputF = fopen("output.txt", "w+"); //opens file output.txt in read+write mode

	do {
		ret = recv(conn_fd, server_reply,2000,0);
		if(ret > 0) //client is connected
			fprintf(outputF, server_reply);
		else if(ret == 0) //client disconnects
			puts("Connection closed");
		else {
			puts("Failed to receive");
			exit(0);
		}
	}while(ret > 0);	//the connection remains open and listens for commands until the client disconnects
	fclose(outputF);	//close the output file

	ret = shutdown(conn_fd, SHUT_RDWR);	//socket shutdown
	if(ret == -1) {
		perror("ERROR when shutting down!");
		exit(0);
	}

	ret = close(conn_fd);	//closes all socket related files
	if(ret == -1) {
		perror("ERROR when closing files!");
		return -1;
	}
	return 0;
}

int main() {
	char buffer1[256], buffer2[256];
	int server;
	//int opt=1;
	struct sockaddr_in my_addr, peer_addr;

	server = socket(AF_INET, SOCK_STREAM, 0); //socket creation
	if(server < 0)
		puts("ERROR when creating server!\n");
	else puts("Server created\n");

	my_addr.sin_family = AF_INET;	//ipv4 family
	my_addr.sin_addr.s_addr = INADDR_ANY;	//specifies the source ip
	my_addr.sin_port = htons(inPORT);		//specifies the port on which the requests will arrive

	if(bind(server, (struct sockaddr*) &my_addr, sizeof(my_addr)) == 0) //binds the socket to an address
		puts("Binded successfully\n");
	else puts("Unable to bind!\n");

	if(listen(server,3) == 0)
		puts("Listening ...\n");
	else puts("Unable to listen!\n");

	socklen_t addr_size;
	addr_size = sizeof(struct sockaddr_in);

	int acc = accept(server, (struct sockaddr*) &peer_addr, &addr_size);
	puts("Connection established\n");

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(peer_addr.sin_addr), ip, INET_ADDRSTRLEN); //converts from binary to text
	printf("Connection established with IP : %s and SOURCE-PORT : %d, DESTINATION-PORT : %d \n",ip,ntohs(peer_addr.sin_port), ntohs(my_addr.sin_port));
	
	if(recv==0)	//if client disconnects
		exit(0);
	while(recv!=0) {	//if client is connectet
		recv(acc, buffer2, 256, 0);	//receives from client
		printf("Client : %s\n", buffer2);	//prints the command
		if(!strcmp(buffer2, "16#")) {	//if the client sends the command 16
			callServer();	//connects to ipv6 server and sends the GET HTTP request
			strcpy(buffer1, "Done\n");
		}
		else strcpy(buffer1, "Command not implemented\n");	//
		send(acc, buffer1, 256, 0);
	}

	close(acc);
	shutdown(acc,2);	//connection shutdown
	return 0;
	}




