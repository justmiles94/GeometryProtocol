//client
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <math.h>


int main(char[] args){

	int MESSAGE_LENGTH = 100;
	int success;
	struct addrinfo hints, *serverData;
	char* port, ip;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if(argc < 2) {
		fprintf(stderr, "Must supply IPaddress and port\n");
		exit(1);
	}
	//null in place of ip or port chooses the localhost and a random port
	if((success = getaddrinfo(&ip, &port, &hints, &serverData)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_stderror(success));
		exit(1);
	}
	struct addrinfo *index, *firstFullServerData;
	for(index = serverData; index != NULL; index = index->ai_next){
		if(serverData->ai_family != NULL && serverData->ai_socktype != NULL && serverData->ai_protocol != NULL){
			firstFullServerData = index;
		}
	}
	if(firstFullServerData == NULL){
		fprintf(stderr, "No valid IP Addresses found matching the information: %s", errno);
		exit(1);
	}
	int socketDescriptor = socket(serverData->ai_family, serverData->ai_socktype, serverData->ai_protocol);
	if(socketDescriptor == -1){
		fprintf(stderr, "Error creating the socket: %s", errno);
		exit(1);
	}
	freeaddrinfo(serverData);
	int AFFIRM = 1;
	if(setsockopt(serverData, SOL_SOCKET, SO_REUSEADDR, &AFFIRM, sizeof AFFIRM) == -1){
		fprintf("setsockopt error");
	}
	if(bind(socketDescriptor, serverData->ai_addr, serverData->addrlen)){
		fprintf(stderr, "Error binding socket to port: %s", errno);
		exit(1);
	}
	if(connect(socketDescriptor, serverData->ai_addr, serverData->ai_addrlen) == -1){
		fprintf(stderr, "Connection error: %s", errno);
		exit(1);
	}
	char* receivedMessage;
	int bytesReceived = readMessage(acceptedDescriptor, receivedMessage, MESSAGE_LENGTH);
	char* sentMessage = "Connection created!\n";
	int messageLen = strlen(sentMessage );
	int bytesSent = sendMessage(acceptedDescriptor, &sentMessage, messageLen);

	close(acceptedDescriptor);
	close(socketDescriptor);
}
int readMessage(int socketDescriptor, void *buffer, int bufferLen){
	int bytesReceived;
	if(bytesReceived = recv(socketDescriptor, buffer, bufferLen, 0) == -1){
		fprintf(stderr, "Error reading new message: %d", errno);
	}
	return bytesReceived;
}

int sendMessage(int socketDescriptor, char* message, int messageLen){
	int bytesSent;
	while(bytesSent < messageLen){
		if(bytesSent = send(acceptedDescriptor, message, messageLen, 0) != -1){
			fprintf(stderr, "Error sending message: %d", errno)
		}
	}
	return bytesSent;
}
