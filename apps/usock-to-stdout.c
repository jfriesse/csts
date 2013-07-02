/*
 * Opens unix socket. Everything written into socket is forwarded to
 * stdout. Socket keeps listening until empty string is sent
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

int
create_listening_socket(char *path)
{
	int s;
	socklen_t bind_len;
	struct sockaddr_un local;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		err(1, "socket");
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, path);
	unlink(local.sun_path);
	bind_len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(s, (struct sockaddr *)&local, bind_len) == -1) {
		err(1, "bind");
	}

	if (listen(s, 2) == -1) {
		err(1, "listen");
	}

	return (s);
}

int
accept_conn(int server_sock)
{
	int c;
	socklen_t remote_len;
	struct sockaddr_un remote;

	remote_len = sizeof(remote);
	if ((c = accept(server_sock, (struct sockaddr *)&remote, &remote_len)) == -1) {
		err(1, "accept");
	}

	return (c);
}

int
read_write_loop(int client_sock, FILE *output_file)
{
	char buf[BUF_SIZE];
	ssize_t received;
	int first_input;

	first_input = 1;

	while ((received = recv(client_sock, buf, sizeof(buf), 0)) > 0) {
		assert(fwrite(buf, received, 1, output_file) == 1);
		first_input = 0;
	}

	if (first_input == 1) {
		return (-1);
	}

	return (0);
}

int
main(int argc, char *argv[])
{
	int server_sock;
	int client_sock;

	if (argc < 2) {
		errx(1, "Please enter socket parameter!");
	}

	signal(SIGPIPE, SIG_IGN);
	setlinebuf(stdout);

	server_sock = create_listening_socket(argv[1]);

	do {
		client_sock = accept_conn(server_sock);
	} while (read_write_loop(client_sock, stdout) != -1);

	return (0);
}
