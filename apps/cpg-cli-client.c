/*
 * Basic cpg client
 */

/*
 * Output is:
 * Basic_iso_date_time:ConfchgCallback:CPGCONFCHGGROUP:no_joined_nodes,no_left_nodes,no_members:(join_node_1 join_pid1)[,(join_node_2 join_pid2)...]:(leave_node1 leave_pid1)[,(leave_node2 leave_pid2)...]:(member_node1 member_pid1)[,(member_node2 member_pid2)
 * Basic_iso_date_time:TotemConfchgCallback:ring_id_node.ring_id_seq:(member_node1 member_pid1)[,(member_node2 member_pid2)
 * Basic_iso_data_time:Sending:RAND:seq_no:length:chsum
 * Basic_iso_date_time:Sending:STR:length:chsum:text_msg
 * Basic_iso_date_time:Arrived:(node_id pid):RAND:seq_no:length:chsum
 * Basic_iso_date_time:Arrived:(node_id pid):STR:length:chsum:text_msg
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <err.h>
#include <signal.h>

#include <corosync/corotypes.h>
#include <corosync/cpg.h>

#define MSG_TYPE_STR	1
#define MSG_TYPE_RANDOM	2

#define MAX_MSG_LEN	50000

#ifndef INFTIM
#define INFTIM		-1
#endif

static int wait_for_msg = 0;
static uint64_t wait_for_msg_seqno = 0;

struct my_msg {
	char msg_type;
	uint64_t seq_no __attribute__ ((aligned (8)));
	uint16_t chsum __attribute__ ((aligned (8)));
	uint32_t data_len __attribute__ ((aligned (8)));
	unsigned char data[0] __attribute__ ((aligned (8)));
};

static void
print_basic_iso_datetime(FILE *out)
{
	time_t t;
	struct tm *tmp;
	char time_str[128];

	t = time(NULL);
	tmp = localtime(&t);
	if (tmp == NULL) {
		exit (3);
	}

	if (strftime(time_str, sizeof(time_str), "%Y%m%dT%H%M%S", tmp) == 0) {
		exit (3);
	}
	fprintf(out, "%s", time_str);
}

static void
print_cpgname(const struct cpg_name *name)
{
	int i;

	for (i = 0; i < name->length; i++) {
		printf("%c", name->value[i]);
	}
}

static uint32_t
compute_chsum(const unsigned char *data, uint32_t data_len)
{
	unsigned int i;
	unsigned int checksum = 0;

	for (i = 0; i < data_len; i++) {
		if (checksum & 1)
			checksum |= 0x10000;
		checksum = ((checksum >> 1) + (unsigned char)data[i]) & 0xffff;
	}

	return (checksum);
}

static void
fill_data(unsigned char *data, uint32_t data_len)
{
	int j;

	for (j = 0; j < data_len; j++) {
                data[j] = (unsigned char)rand();
	}
}

static void
send_msg(cpg_handle_t handle, char msg_type, uint64_t seq_no, uint32_t max_msg_len, char *msg_text)
{
	struct iovec iov[2];
	static unsigned char data[MAX_MSG_LEN];
	struct my_msg msg;
	cs_error_t result;

	msg.msg_type = msg_type;
	msg.seq_no = seq_no;

	switch (msg_type) {
	case MSG_TYPE_STR:
		msg.data_len = strlen(msg_text) + 1;
		memcpy(data, msg_text, msg.data_len);
		data[msg.data_len - 1] = '\0';
		break;
	case MSG_TYPE_RANDOM:
		msg.data_len = rand() % max_msg_len;
		fill_data(data, msg.data_len);
		break;
	default:
		errx(1, "Unknown msg_type");
	}

	msg.chsum = compute_chsum(data, msg.data_len);

	iov[0].iov_base = (void *)&msg;
	iov[0].iov_len = sizeof(msg);
	iov[1].iov_base = (void *)data;
	iov[1].iov_len = msg.data_len;

	print_basic_iso_datetime(stdout);
	fprintf(stdout, ":Sending:");
	switch (msg_type) {
	case MSG_TYPE_STR:
		fprintf(stdout, "STR:%"PRIu32":%04x:%s\n", msg.data_len, msg.chsum, (char *)data);
		break;
	case MSG_TYPE_RANDOM:
		fprintf(stdout, "RAND:%"PRIu64":", msg.seq_no);
		fprintf(stdout, "%"PRIu32":%04x\n", msg.data_len, msg.chsum);
		break;
	}
	result = cpg_mcast_joined (handle, CPG_TYPE_AGREED, iov, 2);
	if (result != CS_OK) {
		errx(1, "cpg_mcast_joined result %u != CS_OK", result);
	}
}

static void
send_str_msg(cpg_handle_t handle, char *msg_text)
{

	send_msg(handle, MSG_TYPE_STR, 0, 0, msg_text);
}

static void
send_random_msg(cpg_handle_t handle, uint64_t seq_no, uint32_t max_msg_len)
{

	send_msg(handle, MSG_TYPE_RANDOM, seq_no, max_msg_len, NULL);
}

static void
ConfchgCallback(cpg_handle_t handle,
    const struct cpg_name *groupName,
    const struct cpg_address *member_list, size_t member_list_entries,
    const struct cpg_address *left_list, size_t left_list_entries,
    const struct cpg_address *joined_list, size_t joined_list_entries)
{
	int i;

	print_basic_iso_datetime(stdout);
	printf(":ConfchgCallback:");
	print_cpgname(groupName);
	printf(":");
	printf("%zu,%zu,%zu:", joined_list_entries, left_list_entries, member_list_entries);

	for (i=0; i < joined_list_entries; i++) {
		if (i != 0) {
			printf(",");
		}
		printf("(%x %x)", joined_list[i].nodeid, joined_list[i].pid);
	}
	printf(":");

	for (i=0; i < left_list_entries; i++) {
		if (i != 0) {
			printf(",");
		}
		printf("(%x %x)", left_list[i].nodeid, left_list[i].pid);
	}
	printf(":");

	for (i=0; i < member_list_entries; i++) {
		if (i != 0) {
			printf(",");
		}
		printf("(%x %x)", member_list[i].nodeid, member_list[i].pid);
	}
	printf ("\n");
}

static void
TotemConfchgCallback(cpg_handle_t handle,
    struct cpg_ring_id ring_id,
    uint32_t member_list_entries,
    const uint32_t *member_list)
{
	int i;

	print_basic_iso_datetime(stdout);
	printf(":TotemConfchgCallback:%x.%"PRIx64, ring_id.nodeid, ring_id.seq);
	printf(":");

	for (i=0; i < member_list_entries; i++) {
		if (i != 0) {
			printf(",");
		}
		printf("%x", member_list[i]);
	}
	printf ("\n");
}

static void
print_error_msg_start(uint32_t nodeid, uint32_t pid)
{

	print_basic_iso_datetime(stderr);
	fprintf(stderr, ":Error:(%"PRIx32" %"PRIx32"):", nodeid, pid);
}

static void
DeliverCallback(cpg_handle_t handle,
    const struct cpg_name *groupName,
    uint32_t nodeid,
    uint32_t pid,
    void *cpg_msg,
    size_t cpg_msg_len)
{
	struct my_msg *msg = (struct my_msg *)cpg_msg;
	uint32_t chsum, expected_chsum;

	print_basic_iso_datetime(stdout);
	fprintf(stdout, ":Arrived:(%"PRIx32" %"PRIx32"):", nodeid, pid);
	switch (msg->msg_type) {
	case MSG_TYPE_STR:
		fprintf(stdout, "STR:%"PRIu32":%04x:%s\n", msg->data_len, msg->chsum, (char *)msg->data);
		break;
	case MSG_TYPE_RANDOM:
		fprintf(stdout, "RAND:%"PRIu64":", msg->seq_no);
		fprintf(stdout, "%"PRIu32":%04x\n", msg->data_len, msg->chsum);

		if (msg->seq_no == wait_for_msg_seqno) {
			wait_for_msg_seqno = 0;
			wait_for_msg = 0;
		}
		break;
	default:
		errx(1, "Unknown message type");
		/* NOTREACHED */
	}

	if ((msg->data_len + sizeof(*msg)) != cpg_msg_len) {
		print_error_msg_start(nodeid, pid);
		fprintf(stderr, "Incorrect message length %"PRIu32"+%zu != %zu\n", msg->data_len, sizeof(*msg), cpg_msg_len);
		exit(2);
	}

	chsum = msg->chsum;
	expected_chsum = compute_chsum(msg->data, msg->data_len);
	if (chsum != expected_chsum) {
		print_error_msg_start(nodeid, pid);
		fprintf(stderr, "Incorrect message chsum %04x != %04x\n", chsum, expected_chsum);
		exit(2);
	}
}

static cpg_model_v1_data_t model_data = {
	.cpg_deliver_fn =            DeliverCallback,
	.cpg_confchg_fn =            ConfchgCallback,
	.cpg_totem_confchg_fn =      TotemConfchgCallback,
	.flags =                     CPG_MODEL_V1_DELIVER_INITIAL_TOTEM_CONF,
};

static void sigintr_handler(int signum) __attribute__((noreturn));

static void
sigintr_handler(int signum) {
	exit (0);
}

static void
usage() {

	printf("Usage: [-n cpg_group_name]\n");
	printf(" -n         Name of cpg group to join\n");

	exit(1);
}

static void
init_rand(void) {
	FILE *f;
	unsigned int init_v;

	init_v = time(NULL) + getpid();

	f = fopen("/dev/urandom", "rb");
	if (f != NULL) {
		fread(&init_v, sizeof(init_v), 1, f);
		fclose(f);
	}
	srand(init_v);
}

static char *
read_token(char *s, char *token)
{
	int i;
	signed int state;
	char ch;
	int token_pos;

	state = 0;
	i = 0;
	token_pos = 0;

	while (state >= 0) {
		ch = s[i];

		switch (state) {
		case 0:
			if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
				state = 0;
			} else if (ch == '\0') {
				state = -1;
			} else {
				token[token_pos] = ch;
				token_pos++;
				state = 1;
			}
			break;
		case 1:
			if (ch == ' ' || ch == '\t' || ch == '\0' || ch == '\n' || ch == '\r') {
				state = 2;
			} else {
				token[token_pos] = ch;
				token_pos++;
			}
			break;
		case 2:
			if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
				state = 2;
			} else {
				state = -1;
				i--;
			}
		}

		i++;
	}

	token[token_pos] = 0;

	return s + i;
}

static void
rtrim(char *s)
{
	int i;

	for (i = strlen(s) - 1; i >= 0; i--) {
		if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') {
			s[i] = 0;
		} else {
			break;
		}
	}
}

static int
process_cli_cmd(FILE *f, cpg_handle_t handle)
{
	char input_buf[512];
	char token[512];
	char *s;
	uint64_t seq_no;
	uint32_t max_msg_len;

	if (fgets(input_buf, sizeof(input_buf), f) == NULL) {
		return (-1);
	}

	s = read_token(input_buf, token);
	if (strcmp(token, "exit") == 0) {
		return (-1);
	} else if (strcmp(token, "sendstr") == 0) {
		rtrim(s);
		send_str_msg(handle, s);
	} else if (strcmp(token, "sendrand") == 0) {
		s = read_token(s, token);
		if (strcmp(token, "") == 0) {
			fprintf(stderr, "sendrand expects number\n");

			return (0);
		}
		seq_no = atoll(token);

		s = read_token(s, token);
		if (strcmp(token, "") == 0) {
			fprintf(stderr, "sendrand expects number\n");

			return (0);
		}
		max_msg_len = atol(token);

		if (max_msg_len < 3) {
			fprintf(stderr, "sendrand expects max_msg_len longer then 3\n");

			return (0);
		}

		send_random_msg(handle, seq_no, max_msg_len);
	} else if (strcmp(token, "sync") == 0) {
		wait_for_msg_seqno = 0xFFEEDDCC;

		s = read_token(s, token);
		if (strcmp(token, "") != 0) {
			wait_for_msg_seqno = atoll(token);
		}

		send_random_msg(handle, wait_for_msg_seqno, 16);
		wait_for_msg = 1;
	} else if (strcmp(token, "") == 0) {
	} else {
		fprintf(stderr, "Unknown command %s\n", token);
	}

	return (0);
}

int
main(int argc, char *argv[]) {
	cpg_handle_t handle;
	int select_fd;
	int result;
	struct cpg_name group_name;
	char ch;
	char *cpg_group_name_str;
	struct pollfd pfd[2];

	cpg_group_name_str = "CPGCLICLIENT";

	while ((ch = getopt(argc, argv, "hn:")) != -1) {
		switch (ch) {
		case 'n':
			cpg_group_name_str = optarg;
			break;
		case 'h':
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	}

	strcpy(group_name.value, cpg_group_name_str);
	group_name.length = strlen(group_name.value);

	result = cpg_model_initialize (&handle, CPG_MODEL_V1, (cpg_model_data_t *)&model_data, NULL);
	if (result != CS_OK) {
		fprintf(stderr, "Could not initialize Cluster Process Group API instance error %d\n", result);
		exit (1);
	}

	result = cpg_join(handle, &group_name);
	if (result != CS_OK) {
		fprintf(stderr, "Could not join process group, error %d\n", result);
		exit (1);
	}

	setlinebuf(stdout);

	init_rand();
	signal(SIGPIPE, SIG_IGN);

	cpg_fd_get(handle, &select_fd);
	do {
		pfd[0].fd = select_fd;
		pfd[0].events = POLLIN;
		pfd[0].revents = 0;

		pfd[1].fd = STDIN_FILENO;
		pfd[1].events = POLLIN;
		pfd[1].revents = 0;

		result = poll(pfd, ((wait_for_msg) ? 1 : 2), INFTIM);
		if (result == -1) {
			err(1, "poll\n");
		}

		if (pfd[0].revents & (POLLIN|POLLHUP)) {
			if ((result = cpg_dispatch (handle, CS_DISPATCH_ALL)) != CS_OK) {
				if (result == CS_ERR_LIBRARY) {
					fprintf(stderr, "Dispatch error %d\n", result);
					exit(0);
				}

				if (result != CS_ERR_TRY_AGAIN) {
					fprintf(stderr, "Dispatch error %d\n", result);
					exit(1);
				}
			}
		}

		if (pfd[1].revents & (POLLIN|POLLHUP)) {
			if (process_cli_cmd(stdin, handle) == -1) {
				exit(0);
			}
		}
	} while (result);

	return (0);
}
