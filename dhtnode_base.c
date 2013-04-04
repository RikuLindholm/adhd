/* You can use this file as a starting point for your C code. */

/* Reading the documentation for select and for TCP/IP is strongly advised;
   see e.g. man pages:
   select(2)
   select_tut(2)
   tcp(7)
   ip(7) */

#include "dhtpackettypes.h"
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#define EXAMPLE_TCP_PORT 9876
#define MAX_CONNECTIONS 5

void die(char *reason) {
    fprintf(stderr, "Fatal error: %s\n", reason);
    exit(1);
}

int create_listen_socket() {
    int fd;
    int t;

    struct sockaddr_in a;

    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_family = AF_INET;
    a.sin_port = htons(EXAMPLE_TCP_PORT);

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        die(strerror(errno));

    t = bind(fd, (struct sockaddr *)(&a), sizeof(struct sockaddr_in));
    if (t == -1)
        die(strerror(errno));

    t = listen(fd, MAX_CONNECTIONS);
    if (t == -1)
        die(strerror(errno));        

    return fd;
}

int main(void) {
    fd_set rfds;
    int retval;
    int running = 1;
    int listensock = create_listen_socket();

    while(running) {
  FD_ZERO(&rfds);
	FD_SET(0, &rfds); /* Standard input */
	FD_SET(listensock, &rfds);     

	retval = select(listensock + 1, &rfds, NULL, NULL, NULL);

	if (retval == -1)
	    die("select failed");
	else if (retval) {
	    if (FD_ISSET(0, &rfds))
		running = 0; /* Exit when console input available */
	    if (FD_ISSET(listensock, &rfds)) {
		struct sockaddr_in tempaddr;
		unsigned int addrlen = 0;
		int tempfd = accept(listensock, (struct sockaddr *)&tempaddr,
				    &addrlen);
		write(tempfd, "Hello!\n", 7); /* Answer politely then close. */
		close(tempfd);
	    }
	}
    }

    close(listensock);

    return 0;
}
