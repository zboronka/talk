#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFLEN 10000
#define PORT   2830
#define MAX_PENDING  5

int main() {
	struct sockaddr_in sin;
	char *buf = malloc(BUFLEN * sizeof(char));
	int buf_len, addr_len;
	int s, new_s, opt = 1;
	int offset = 0;

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);

	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	if((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
		perror("bind");
		exit(1);
	}

	if(listen(s, MAX_PENDING) < 0) {
		perror("listen");
		exit(1);
	}

	while(1) {
		if((new_s = accept(s, (struct sockaddr *)&sin, &addr_len)) < 0) {
			perror("accept");
			exit(1);
		}
		while(buf_len = recv(new_s, buf, BUFLEN * sizeof(char), 0)) {
			if(!strstr(buf, "\r\n\r\n")) {
				if(buf_len < 0) {
					perror("recv");
					break;
				}
			} else {
				send(new_s, "ACKNWLGE", 8, MSG_NOSIGNAL);
				bzero(buf, BUFLEN * sizeof(char));
			}
		}
		close(new_s);
	}

	free(buf);
}
