/*
 * Opens unix socket. Everything written into stdin is forwarded to
 * socket. Apps ends with end of file.
 */
#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUF_SIZE 4096

static int
create_client_socket(char *path)
{
	int s;
	socklen_t conn_len;
	struct sockaddr_un conn;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		err(1, "socket");
	}

	conn.sun_family = AF_UNIX;
	strcpy(conn.sun_path, path);
	conn_len = strlen(conn.sun_path) + sizeof(conn.sun_family);
	if (connect(s, (struct sockaddr *)&conn, conn_len) == -1) {
		err(1, "connect");
	}

	return (s);
}

static int
read_write_loop(FILE *input_file, int remote_sock)
{
	char buf[BUF_SIZE];
	size_t received;

	while ((received = fread(buf, 1, sizeof(buf), input_file)) > 0) {
		assert(send(remote_sock, buf, received, 0) == received);
	}

	return (0);
}

int
main(int argc, char *argv[])
{
	int sock;

	if (argc < 2) {
		errx(1, "Please enter socket parameter!");
	}

	signal(SIGPIPE, SIG_IGN);
	setlinebuf(stdout);
	setlinebuf(stdin);

	sock = create_client_socket(argv[1]);
	read_write_loop(stdin, sock);

	return (0);
}
