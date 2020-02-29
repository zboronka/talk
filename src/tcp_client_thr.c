#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFLEN 10000256
#define PORT   2830

int main(int argc, char** argv) {
	int bts[] = {1, 16, 64, 256, 1000};
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char *buf = malloc(BUFLEN * sizeof(char));
	char *rec_buf = malloc(BUFLEN * sizeof(char));
	int s;
	int rec_len;
	int offset = 0;
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

	printf("1K,16K,64K,256K,1M\n");

	for(int i = 0; i < 50; i++) {
		for(int j = 0; j < 5; j++) {
			sep = j < 4 ? ',' : '\n';
			memset(buf, 'p', bts[j] * 1000 * sizeof(char));
			strcpy(buf+((bts[j] * 1000)-4), "\r\n\r\n");
			send_t = clock();
			if(send(s, buf, strlen(buf), 0) < 0) {
				perror("send");
				close(s);
				exit(1);
			}
			while(rec_len = recv(s, rec_buf+offset, (BUFLEN-offset) * sizeof(char), 0) &&
			      !strstr(rec_buf, "\r\n\r\n")) {
				if(rec_len < 0) {
					perror("recv");
					break;
				}
				offset+=rec_len;
			}

			reply_t = clock();
			printf("%f%c", ((double) (reply_t - send_t)) / CLOCKS_PER_SEC, sep);

			offset=0;
			bzero(rec_buf, BUFLEN * sizeof(char));
		}
	}

	shutdown(s, SHUT_RDWR);

	free(buf);
	free(rec_buf);
}
