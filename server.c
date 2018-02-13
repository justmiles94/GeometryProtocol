//server
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <math.h>

int MESSAGE_LENGTH = 100;
int success, bytesReceived;
struct addrinfo hints, *serverData;
char *receivedMessage;
struct sockaddr_storage stored;

int main(int argc, char* argv[]) {

	if(argc < 3) {
		fprintf(stderr, "Must supply IPaddress and port\n");
		exit(1);
	}
	char* udp = argv[1];
	char* tcp = argv[2];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	int acceptedAddrSize = sizeof stored;

	//server setup
	puts(tcp);
	int socketDescriptor = getConnectedSocketDescriptor(tcp);

//	//accept connection
//	int acceptedDescriptor = accept(socketDescriptor, (struct sockaddr *) &stored, &acceptedAddrSize);
//
//	//send message
//	sendMessage(acceptedDescriptor, "Connection created!\n");
//
//	//read message
//	bytesReceived = readMessage(acceptedDescriptor, receivedMessage, MESSAGE_LENGTH);
//
//	//print message
//	puts(&receivedMessage);
//	puts(receivedMessage);

	//close connection
//	close(acceptedDescriptor);
	close(socketDescriptor);
}
int getConnectedSocketDescriptor(char* tcp){
	//null in place of ip or port chooses the localhost and a random port
	puts(tcp);
	if((success = getaddrinfo(NULL, tcp, &hints, &serverData)) != 0) {
		fprintf(stderr, "GETADDRINFO: %s\n", strerror(success));
		exit(1);
	}
	struct addrinfo *index, *firstFullServerData;
	for(index = serverData; index != NULL; index = index->ai_next){
		if(serverData->ai_family != NULL && serverData->ai_socktype != NULL && serverData->ai_protocol != NULL){
			firstFullServerData = index;
		}
	}
	freeaddrinfo(index);

	if(firstFullServerData == NULL){
		fprintf(stderr, "NULLADDRINFO: %s\n", strerror(errno));
		exit(1);
	}
	int socketDescriptor = socket(serverData->ai_family, serverData->ai_socktype, serverData->ai_protocol);
	if(socketDescriptor == -1){
		fprintf(stderr, "SOCKET: %s\n", strerror(errno));
		exit(1);
	}
	freeaddrinfo(serverData);
	int AFFIRM = 1;
	if(setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &AFFIRM, sizeof AFFIRM) == -1){
		fprintf(stderr, "SETSOCK will continue: %s\n", strerror(errno));
	}
	if(bind(socketDescriptor, serverData->ai_addr, serverData->ai_addrlen)){
		fprintf(stderr, "BIND: %s\n", strerror(errno));
		exit(1);
	}
	if(listen(socketDescriptor, 10) != -1){
		fprintf(stderr, "Error listening to socket: %d", errno);
	}
	return socketDescriptor;
}
int readMessage(int socketDescriptor, void *buffer, int bufferLen){
	int bytesReceived;
	if(bytesReceived = recv(socketDescriptor, buffer, bufferLen, 0) == -1){
		fprintf(stderr, "Error reading new message: %d", errno);
	}
	return bytesReceived;
}
int sendMessage(int socketDescriptor, char* message){
	int messageLen = strlen(message);
	int bytesSent;
	while(bytesSent < messageLen){
		if(bytesSent = send(socketDescriptor, message, messageLen, 0) != -1){
			fprintf(stderr, "Error sending message: %d", errno);
		}
	}
	return bytesSent;
}

double calculateCircleArea(double radius){
	return M_PI*pow(radius, 2);
}
double calculateCircleCircumference(double area){
	return M_PI*2*sqrt(area/M_PI);
}
double calculateSphereVolume(double radius){
	return 4*M_PI*pow(radius, 3)/3;
}
double calculateSphereRadius(double area){
	return sqrt(area/M_PI)/2;
}
double calculateCylinderSurface(double radius, double height){
	return 2*M_PI*radius*height + 2*M_PI*pow(radius, 2);
}
double calculateCylinerHeight(double volume, double radius){
	return volume/(M_PI*pow(radius, 2));
}
