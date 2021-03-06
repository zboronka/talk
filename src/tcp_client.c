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

int main(int argc, char** argv) {
	int bts[] = {8, 64, 1024};
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char buf[BUFLEN], rec_buf[BUFLEN];
	int s;
	int rec_len;
	clock_t send_t, reply_t;
	char sep;

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

	bzero((char*)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char*)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(PORT);

	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	if(connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		perror("connect");
		close(s);
		exit(1);
	}

	printf("8B,64B,1024B\n");

	for(int i = 0; i < 50; i++) {
		for(int j = 0; j < 3; j++) {
			sep = j < 2 ? ',' : '\n';
			memset(buf, 'p', bts[j] * sizeof(char));
			buf[bts[j]] = '\0';
			send_t = clock();
			send(s, buf, strlen(buf), 0);
			if(rec_len = recv(s, rec_buf, sizeof(rec_buf), 0)) {
				reply_t = clock();
				printf("%f%c", ((double) (reply_t - send_t)) / CLOCKS_PER_SEC, sep);
			}
		}
	}

	shutdown(s, SHUT_RDWR);
}
