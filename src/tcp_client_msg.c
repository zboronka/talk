#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFLEN 10000
#define PORT   2830

int main(int argc, char** argv) {
	int bts[] = {1024, 512, 256};
	int tts[] = {1024, 2048, 4096};
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char *buf = malloc(BUFLEN * sizeof(char));
	char rec_buf[16];
	int s;
	int snd_len = 1024; 
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

	printf("1024B,512B,256B\n");

	//for(int i = 0; i < 50; i++) {
		for(int j = 0; j < 3; j++) {
			sep = j < 2 ? ',' : '\n';
			memset(buf, 'p', bts[j] * sizeof(char));
			send_t = clock();
			for(int k = 0; k < tts[j]-1; k++) {
				if(send(s, buf, strlen(buf), 0) < 0) {
					perror("send");
					close(s);
					exit(1);
				}
			}
			strcpy(buf+((bts[j])-4), "\r\n\r\n");
			if(send(s, buf, strlen(buf), 0) < 0) {
				perror("send");
				close(s);
				exit(1);
			}
			if(rec_len = recv(s, rec_buf, sizeof(rec_buf), 0)) {
				if(rec_len < 0) {
					perror("recv");
					break;
				}
				reply_t = clock();
				printf("%f%c", ((double) (reply_t - send_t)) / CLOCKS_PER_SEC, sep);
			}

			bzero(buf, BUFLEN * sizeof(char));
		}
	//}

	shutdown(s, SHUT_RDWR);
}
