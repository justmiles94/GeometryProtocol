/*
 ** server.c -- a stream socket server demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>

double calculateCircleArea(double radius) {
	return M_PI * pow(radius, 2);
}
double calculateCircleCircumference(double area) {
	return M_PI * 2 * sqrt(area / M_PI);
}
double calculateSphereVolume(double radius) {
	return 4 * M_PI * pow(radius, 3) / 3;
}
double calculateSphereRadius(double area) {
	return sqrt(area / M_PI) / 2;
}
double calculateCylinderSurface(double radius, double height) {
	return 2 * M_PI * radius * height + 2 * M_PI * pow(radius, 2);
}
double calculateCylinerHeight(double volume, double radius) {
	return volume / (M_PI * pow(radius, 2));
}
void sigchld_handler(int s) {
	(void) s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0)
		;

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}
int udp(void* MYPORT) {
	int MAXBUFLEN = 100;
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, (char*) MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	char *childIp = "";
	close(sockfd); // child doesn't need the listener
	int state = 0;
	while (state == 0) {
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		if (buf != NULL) {
			state = parseMessage(&buf, &childIp, state);
		}else{
			sprintf(&buf, "200 BYE %s(TCP)", childIp);
			state = 0;
		}
		if ((numbytes = sendto(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
	}
	printf("listener: got packet from %s\n",
			inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *) &their_addr), s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);

	close(sockfd);

	return 0;
}
int tcp(void* PORT) {
	int BACKLOG = 10;
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, (char*) PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while (1) { // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			char *childIp = s;
			close(sockfd); // child doesn't need the listener
			char* message;
			int state = 0;
			while (state == 0) {
				if (recv(new_fd, &message, 100, 0) == -1)
					perror("recv");
				if (message != NULL) {
					state = parseMessage(&message, &childIp, state);
				}else{
					sprintf(&message, "200 BYE %s(TCP)", childIp);
					state = 0;
				}
				if (send(new_fd, &message, 100, 0) == -1)
					perror("send");
			}
			close(new_fd);
			exit(0);
		}
		close(new_fd); // parent doesn't need this
	}

	return 0;
}
enum SHAPE { CYLINDER = 6, CIRCLE = 7, SPHERE = 8 };
int parseMessage(char* message, char* IP, int shape) {
	char* value;
	if ((value = strsep(&message, " ")) == "HELO") {
		sprintf(&message, "HELO %s(TCP)", IP);
		return shape;
	} else if ((value = strsep(&message, " ")) == "HELO") {
		sprintf(&message,
				"Help can be found be referncing the server documentation");
		return shape;
	} else if (strcmp(&message, "CYLINDER") == 0) {
		return CYLINDER;
	} else if (strcmp(&message, "CIRCLE") == 0) {
		return CIRCLE;
	} else if (strcmp(&message, "SPHERE") == 0) {
		return SPHERE;
	} else {
		char* cmd;
		double params[2] = {-1, -1};
		int count = 0;
		char* value;
		while ((value = strsep(&message, " ")) != NULL) {
			switch (count) {
			case 0:
				cmd = &value;
				break;
			case 1:
				params[0] = atof(&value);
				break;
			case 2:
				params[1] = atof(&value);
			}
		}
		if(params[1] == -1){
			if(shape == CIRCLE){//circle
				if(strcmp(&cmd, "AREA") == 0 ){
					sprintf(&message, "250 %f", calculateCircleArea(params[0]));
				}else if(strcmp(&cmd, "CIRC") == 0){
					sprintf(&message, "250 %f", calculateCircleCircumference(params[0]));
				}else{
					sprintf(&message, "Syntax error command unrecognized");
				}
			}else if(shape == SPHERE){//sphere
				if(strcmp(&cmd, "VOL") == 0){
					sprintf(&message, "250 %f", calculateSphereVolume(params[0]));
				}else if(strcmp(&cmd, "RAD") == 0){
					sprintf(&message, "250 %f", calculateSphereRadius(params[0]));
				}else{
					sprintf(&message, "Syntax error command unrecognized");
				}
			}else{
				sprintf(&message, "Syntax error command unrecognized");
			}
		}else{
			if(shape == CYLINDER){//cylinder
				if(strcmp(&cmd, "AREA") == 0){
					sprintf(&message, "250 %f", calculateCylinderSurface(params[0], params[1]));
				}else if(strcmp(&cmd, "HGT") == 0){
					sprintf(&message, "250 %f", calculateCylinerHeight(params[0], params[1]));
				}else{
					sprintf(&message, "Syntax error command unrecognized");
				}
			}else{
				sprintf(&message, "Syntax error command unrecognized");
			}
		}
	}
	sprintf(&message, "Syntax error: unknown");
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc != 3 || strcmp(argv[1], argv[2]) == 0) {
		puts("2 arguments must be given and they cannot be the same.");
		exit(1);
	}
	pthread_t tcpThread, udpThread;
	pthread_create(&tcpThread, NULL, tcp, argv[1]);
	pthread_create(&udpThread, NULL, udp, argv[2]);
	pthread_join(&tcpThread, NULL);
	pthread_join(&udpThread, NULL);
	return 0;
}
