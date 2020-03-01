#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define BUFLEN 2048
#define PORT   2830

void die(char *s) {
	perror(s);
	exit(1);
}

int main(int argc, char **argv) {
	int bts[] = {1024, 512, 256};
	int tts[] = {1024, 2048, 4096};
	struct hostent *hp;
	struct sockaddr_in si_other;
	int s, i, slen=sizeof(si_other);

	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	char buf[BUFLEN];
	char drop[] = "drop";
	char message[BUFLEN];
	char *host, sep;
	clock_t send_t, reply_t;

	if(argc>=2) {
		host = argv[1];
	} else {
		host = "localhost";
	}

	hp = gethostbyname(host);
	if(!hp) {
		fprintf(stderr, "unknown host: %s\n", host);
		exit(1);
	}

	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		die("socket");
	}

	if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		die("setsockopt");
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	bcopy(hp->h_addr, (char*)&si_other.sin_addr, hp->h_length);
	
	printf("1024B,512B,256B\n");

	for(int i = 0; i < 50; i++) {
		for(int j = 0; j < 3; j++) {
			sep = j < 2 ? ',' : '\n';
			memset(message, 'p', bts[j] * sizeof(char));
			message[bts[j]] = '\0';
			send_t = clock();
			for(int k = 0; k < tts[j]; k++) {
				if(sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == -1) {
					die("sendto");
				}
			}

			if(recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1) {
				if(errno == EAGAIN) {
					if(sendto(s, drop, strlen(drop), 0, (struct sockaddr *) &si_other, slen) == -1) {
						die("sendto");
					}
					j--;
					continue;
				} else {
					die("recvfrom");
				}
			}
			reply_t = clock();
			printf("%f%c", ((double) (reply_t - send_t)) / CLOCKS_PER_SEC, sep);
		}
	}
}
