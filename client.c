/*
 ** client.c -- a stream socket client demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

char* PORT;
int MAXDATASIZE = 100;
int failedToConnectTCP = 0;
int failedToConnectUDP = 0;
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
	if (argc != 3 || strcmp(argv[1], argv[2]) == 0) {
		puts("2 arguments must be given and they cannot be the same.");
		exit(1);
	}
	IP = argv[1];
	PORT = argv[2];
	pthread_t tcpThread, udpThread;
	pthread_create(&tcpThread, NULL, tcp, NULL);
	pthread_create(&udpThread, NULL, udp, NULL);
	pthread_join(&tcpThread, NULL);
	pthread_join(&udpThread, NULL);
	if (failedToConnectTCP + failedToConnectUDP == 2)
		puts("Failed to make TCP or UDP connection");
	return 0;
}
int tcp(void* none) {

	int MAXDATASIZE = 100;
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		failedToConnectTCP = 1;
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s,
			sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n", buf);

	close(sockfd);

	return 0;
}
int udp(void* SERVERPORT) {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(IP, SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

// loop through all the results and make a socket
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		failedToConnectUDP = 1;
		return 2;
	}
	int receivedStatus = 1;
	char* message = PORT;
	char* reponse;
	while(numbytes != 0){
	  scanf(">>", &message);
	  if ((numbytes = sendto(sockfd, message, strlen(message), 0, p->ai_addr,
			       p->ai_addrlen)) == -1) {
	    perror("talker: sendto");
	    exit(1);
	  }
	  if((numbytes = recvfrom(sockfd, &response, 100, 0, p->ai_addr, p->ai_addrlen)) != -1){
	    perror("talker: recvfrom");
	    exit(1);
	  }

	}

	freeaddrinfo(servinfo);
	close(sockfd);

	return 0;
}
