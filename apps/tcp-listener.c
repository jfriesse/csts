/*
 * Accept connections and print input to stdout in following format:
 * Basic_iso_data_time:Start
 * Basic_iso_data_time:Stop
 * Basic_iso_data_time:ip_addr:port:Message:Message_content
 * Basic_iso_data_time:ip_addr:port:Connect
 * Basic_iso_data_time:ip_addr:port:Close
 */

#include <sys/types.h>

#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_ACTIVE_CONNECTIONS		5000

struct client_info {
	int active;		/* Is client active? */
	int fd;			/* Client connection fd */
	char *ip_addr_str;	/* Client IP address (string) */
	uint16_t port;		/* Client port */
};

static struct client_info clients[MAX_ACTIVE_CONNECTIONS];
static struct pollfd poll_fds[MAX_ACTIVE_CONNECTIONS + 1];
static int server_fd;
static int exit_app = 0;

static void
print_basic_iso_datetime(FILE *out)
{
	time_t t;
	struct tm *tmp;
	char time_str[128];

	t = time(NULL);
	tmp = localtime(&t);
	if (tmp == NULL) {
		errx(1, "Can't get localtime");
	}

	if (strftime(time_str, sizeof(time_str), "%Y%m%dT%H%M%S", tmp) == 0) {
		errx(1, "Can't format time");
	}
	fprintf(out, "%s", time_str);
}

static void
print_client_ipaddr_port(FILE *out, int client_id)
{

	fprintf(out, "%s:%"PRIu16, clients[client_id].ip_addr_str, clients[client_id].port);
}

static void
print_connect(FILE *out, int client_id)
{

	print_basic_iso_datetime(out);
	fprintf(out, ":");
	print_client_ipaddr_port(out, client_id);
	fprintf(out, ":Connect\n");
}

static void
print_close(FILE *out, int client_id)
{

	print_basic_iso_datetime(out);
	fprintf(out, ":");
	print_client_ipaddr_port(out, client_id);
	fprintf(out, ":Close\n");
}

static void
print_msg_header(FILE *out, int client_id)
{

	print_basic_iso_datetime(out);
	fprintf(out, ":");
	print_client_ipaddr_port(out, client_id);
	fprintf(out, ":Message:");
}

static int
add_client(int fd, const char *ip_addr_str, uint16_t port)
{
	int i;

	for (i = 0; i < MAX_ACTIVE_CONNECTIONS; i++) {
		if (!clients[i].active) {
			clients[i].fd = fd;
			if ((clients[i].ip_addr_str = strdup(ip_addr_str)) == NULL) {
				errx(1, "add_client: Memory exchausted");
			}
			clients[i].port = port;
			clients[i].active = 1;

			print_connect(stdout, i);

			return (0);
		}
	}

	return (-1);
}

static int
client_fd_to_id(int fd)
{
	int i;

	for (i = 0; i < MAX_ACTIVE_CONNECTIONS; i++) {
		if (clients[i].fd == fd) {
			return (i);
		}
	}

	return (-1);
}

static void
del_client(int fd)
{
	int client_id;

	client_id = client_fd_to_id(fd);
	if (client_id == -1) {
		errx(1, "client_fd_to_id returned -1");
	}

	print_close(stdout, client_id);
	clients[client_id].active = 0;
}

static int
read_client_msg(int fd)
{
	int client_id;
	char buf[2];
	int i;
	char c;
	ssize_t bytes;
	int last_was_cr;
	int header_printed;

	header_printed = 0;

	client_id = client_fd_to_id(fd);
	if (client_id == -1) {
		errx(1, "client_fd_to_id returned -1");
	}

	while (1) {
		if (exit_app) {
			break;
		}

		bytes = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);

		if (bytes == 0) {
			return (-1);
		}

		if (bytes == -1) {
			if (errno == EINTR) {
				continue ;
			} else if (errno == EAGAIN) {
				break;
			} else if (errno == ECONNRESET) {
				return (-1);
			} else {
				err(1, "read_client_msg");
			}
		}

		for (i = 0; i < bytes; i++) {
			last_was_cr = 0;

			c = buf[i];

			if (!header_printed) {
				header_printed = 1;

				print_msg_header(stdout, client_id);
			}

			if (c >= 0x20 && c <= 0x7E) {
				fprintf(stdout, "%c", c);
			} else if (c == '\n') {
				fprintf(stdout, "\n");
				last_was_cr = 1;
				header_printed = 0;
			} else {
				fprintf(stdout, ".");
			}
		}
	}

	if (!last_was_cr) {
		printf("\n");
	}

	return (0);
}

static int
create_listener(const char *port_str)
{
	struct addrinfo hints, *res, *res0;
	int sockfd;
	int error;
	const char *cause;
	int opt;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	error = getaddrinfo(NULL, port_str, &hints, &res0);
	if (error != 0) {
		errx(1, "%s", gai_strerror(error));
		return (-1);
	}

	for (res = res0; res != NULL; res = res->ai_next) {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) {
			sockfd = -1;
			cause = "socket";

			continue ;
		}

		opt = 1;

		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

#ifdef SO_REUSEPORT
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

		if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
			close(sockfd);
			sockfd = -1;
			cause = "bind";

			continue ;
		}

		listen(sockfd, 10);
		break;
	}

	freeaddrinfo(res0);

	if (sockfd == -1) {
		err(1, "%s", cause);
	}

	return (sockfd);
}

static void *
sockaddr_addr(const struct sockaddr *sa)
{
	switch (sa->sa_family) {
	case AF_INET:
		return &(((struct sockaddr_in*)sa)->sin_addr);
		break;
	case AF_INET6:
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
		break;
	default:
		errx(1, "sockaddr_addr: Unknown address family %u", sa->sa_family);
		/* NOTREACHED */
	}
}

static char *
sockaddr_to_str(const struct sockaddr *sa)
{
	static char res[INET6_ADDRSTRLEN];

	if (inet_ntop(sa->sa_family, sockaddr_addr(sa), res, sizeof(res)) == NULL) {
		err(1, "sockaddr_to_str");
	}

	return (res);
}

static uint16_t
sockaddr_port(const struct sockaddr *sa)
{
	switch (sa->sa_family) {
	case AF_INET:
		return (ntohs(((struct sockaddr_in*)sa)->sin_port));
		break;
	case AF_INET6:
		return (ntohs(((struct sockaddr_in6*)sa)->sin6_port));
		break;
	default:
		errx(1, "sockaddr_port: Unknown address family %u", sa->sa_family);
		/* NOTREACHED */
	}
}

static int
fill_poll_fds(void)
{
	int i;
	int no_fds;

	no_fds = 0;
	poll_fds[no_fds].events = POLLIN;
	poll_fds[no_fds].revents = 0;
	poll_fds[no_fds++].fd = server_fd;

	for (i = 0; i < MAX_ACTIVE_CONNECTIONS; i++) {
		if (clients[i].active) {
			poll_fds[no_fds].events = POLLIN;
			poll_fds[no_fds].revents = 0;
			poll_fds[no_fds++].fd = clients[i].fd;
		}
	}

	return (no_fds);
}

static void
exec_poll_event(int no_fds)
{
	int i;
	int conn_fd;
	struct sockaddr_storage client_addr;
	socklen_t sin_size;

	for (i = 0; i < no_fds; i++) {
		if (exit_app) {
			break;
		}

		if (poll_fds[i].revents & POLLIN) {
			if (i == 0) {
				sin_size = sizeof(client_addr);
				conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
				if (conn_fd == -1) {
					err(1, "accept");
				}

				if (add_client(conn_fd,
				    sockaddr_to_str((struct sockaddr *)&client_addr),
				    sockaddr_port((struct sockaddr *)&client_addr)) == -1) {
					close(conn_fd);
				}
			} else {
				if (read_client_msg(poll_fds[i].fd) == -1) {
					del_client(poll_fds[i].fd);
					close(poll_fds[i].fd);
				}
			}
		} else if (poll_fds[i].revents & (POLLERR|POLLHUP|POLLNVAL)) {
			del_client(poll_fds[i].fd);
			close(poll_fds[i].fd);
		}
	}
}

static void
sig_handler(int sig)
{

	exit_app = 1;
}

static void
register_signals(void)
{
	struct sigaction sa;

	sa.sa_handler = sig_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		err(1, "sigaction");
	}
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		err(1, "sigaction");
	}
}
int
main(int argc, char *argv[])
{
	int no_fds;
	int error;

	server_fd = create_listener("1500");

	register_signals();
	print_basic_iso_datetime(stdout);
	fprintf(stdout, ":Start\n");

	while (!exit_app) {
		no_fds = fill_poll_fds();

		error = poll(poll_fds, no_fds, INFTIM);
		if (error == -1) {
			if (errno == EINTR) {
				break ;
			} else {
				err(1, "poll");
			}
		}

		exec_poll_event(no_fds);
	}

	print_basic_iso_datetime(stdout);
	fprintf(stdout, ":Stop\n");

	return (0);
}
