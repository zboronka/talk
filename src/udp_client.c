#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFLEN 2048
#define PORT   2830

void die(char *s) {
	perror(s);
	exit(1);
}

int main(int argc, char **argv) {
	int bts[] = {8, 64, 1024};
	struct hostent *hp;
	struct sockaddr_in si_other;
	int s, i, slen=sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	char *host, sep;
	clock_t send_t, reply_t;

	if(argc>=2) {
		host = argv[1];
	} else {
		fprintf(stderr, "usage: udp_client host\n");
		exit(1);
	}

	hp = gethostbyname(host);
	if(!hp) {
		fprintf(stderr, "unknown host: %s\n", host);
		exit(1);
	}

	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		die("socket");
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	bcopy(hp->h_addr, (char*)&si_other.sin_addr, hp->h_length);
	
	printf("8B,64B,1024B");

	for(int i = 0; i < 50; i++) {
		for(int j = 0; j < 3; j++) {
			sep = j < 2 ? ',' : '\n';
			memset(message, 'p', bts[j] * sizeof(char));
			message[bts[j]] = '\0';
			send_t = clock();
			if(sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == -1) {
				die("sendto");
			}
			bzero(buf, BUFLEN);
			if(recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1) {
				die("recvfrom");
			}
			reply_t = clock();
			printf("%f\n", ((double) (reply_t - send_t)) / CLOCKS_PER_SEC);
			//printf("%s\n", buf);
		}
	}
}
