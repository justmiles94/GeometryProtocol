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

char *PORT, *IP;
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

int tcp() {

	int MAXDATASIZE = 100;
	int sockfd, numbytes = 1;
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

	sprintf(&buf, "HELO %s", IP);
	while (numbytes != 0) {
		if (numbytes = send(sockfd, buf, strlen(buf), 0) == -1) {
			perror("client: send");
			exit(1);
		}
		if (numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0) == -1) {
			perror("client: recv");
			exit(1);
		}
		buf[numbytes] = '\0';
		printf("%s\n", &buf);
		if (numbytes != 0) {
			scanf("%s", &buf);
		}
	}
	close(sockfd);

	return 0;
}
int udp(char* SERVERPORT) {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes = 1;

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
	char* response;
	sprintf(&response, "HELO %s (UDP)", IP);
	while (numbytes != 0) {
		if ((numbytes = sendto(sockfd, &response, strlen(response), 0, p->ai_addr,
				p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
		if ((numbytes = recvfrom(sockfd, &response, MAXDATASIZE - 1, 0,
				p->ai_addr, p->ai_addrlen)) != -1) {
			perror("talker: recvfrom");
			exit(1);
		}
		response[numbytes] = '\0';
		printf("%s\n", &response);
		if (numbytes != 0) {
			scanf("%s", &response);
		}
	}

	freeaddrinfo(servinfo);
	close(sockfd);

	return 0;
}
int main(int argc, char *argv[]) {
	if (argc != 3 || strcmp(argv[1], argv[2]) == 0) {
		puts("2 arguments must be given and they cannot be the same.");
		exit(1);
	}
	IP = argv[1];
	PORT = argv[2];
	failedToConnectTCP = tcp();
	failedToConnectUDP = udp(PORT);
	if (failedToConnectTCP + failedToConnectUDP == 2)
		puts("Failed to make TCP or UDP connection");
	return 0;
}
