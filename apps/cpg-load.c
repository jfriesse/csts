/*
 * Generate maximum cpg load
 * Output (stdout):
 * Basic_iso_date_time:(nodeid pid):msg_seq_no:msg_data_length:checksum
 * Basic_iso_date_time:Sending:msg_seq_no:msg_data_length:checksum:cpg_mcast_error
 * (stderr):
 * Basic_iso_date_time:(nodeid pid):msg_seq_no:Message
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <err.h>

#include <corosync/corotypes.h>
#include <corosync/cpg.h>

#define MAX_MSG_LEN     50000

#define cs_repeat(counter, max, code) do {	\
	code;					\
	if (result == CS_ERR_TRY_AGAIN) {	\
		counter++;			\
		sleep(1);			\
	} else {				\
		break;				\
	}					\
} while (counter < max)

struct my_msg {
    uint64_t seq_no __attribute__ ((aligned (8)));
    uint16_t chsum __attribute__ ((aligned (8)));
    uint32_t data_len __attribute__ ((aligned (8)));
    unsigned char data[0] __attribute__ ((aligned (8)));
};

static uint64_t last_msg;
static uint64_t last_expected;
static int quiet = 0;
static uint32_t local_nodeid;

static void print_basic_iso_datetime(FILE *out)
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

uint32_t compute_chsum(const unsigned char *data, uint32_t data_len)
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

void fill_data(unsigned char *data, uint32_t data_len)
{
	int j;
	unsigned char val;
	unsigned char step;

	val = (unsigned char)rand();
	step = (unsigned char)rand();

	for (j = 0; j < data_len; j++) {
		data[j] = val;
		val += step;
	}
}

void generate_msg(struct my_msg *msg, unsigned char *data, uint32_t data_len, int refill_data)
{

        msg->seq_no = last_msg;
	msg->data_len = rand() % data_len;

	if (refill_data) {
		fill_data(data, data_len);
	}

	msg->chsum = compute_chsum(data, msg->data_len);
}

cs_error_t send_msg(cpg_handle_t handle, int refill_data)
{
	struct iovec iov[2];
	static unsigned char data[MAX_MSG_LEN];
	struct my_msg msg;
	cs_error_t result;

	generate_msg(&msg, data, sizeof(data), refill_data);
	iov[0].iov_base = (void *)&msg;
	iov[0].iov_len = sizeof(msg);
	iov[1].iov_base = (void *)data;
	iov[1].iov_len = msg.data_len;

	if (!quiet) {
		print_basic_iso_datetime(stdout);
		fprintf(stdout, ":Sending:%"PRIu64":", msg.seq_no);
		fprintf(stdout, "%"PRIu32":%u", msg.data_len, msg.chsum);
	}
	result = cpg_mcast_joined (handle, CPG_TYPE_AGREED, iov, 2);
	if (!quiet) {
		printf(":%u\n", result);
	}

	return (result);
}

void send_msgs(cpg_handle_t handle, int no_msgs)
{
	int i;
	int refill_data;
	cs_error_t err;

	refill_data = 1;

	for (i = 0; i < no_msgs; i++) {
		err = send_msg(handle, refill_data);
		refill_data = 0;
		if (err == CS_OK) {
			last_msg++;
			refill_data = 1;
		}
	}
}

static void DeliverCallback (
	cpg_handle_t handle,
	const struct cpg_name *groupName,
	uint32_t nodeid,
	uint32_t pid,
	void *cpg_msg,
	size_t cpg_msg_len)
{
	struct my_msg *msg = (struct my_msg *)cpg_msg;

	if (!quiet) {
		print_basic_iso_datetime(stdout);
		fprintf(stdout, ":(%"PRIx32" %"PRIx32"):%"PRIu64":", nodeid, pid, msg->seq_no);
		fprintf(stdout, "%"PRIu32":%u\n", msg->data_len, msg->chsum);
	}

	if (nodeid == local_nodeid && pid == getpid()) {
		if (last_expected == msg->seq_no) {
			last_expected++;
		} else {
			print_basic_iso_datetime(stderr);
			fprintf(stderr, ":(%"PRIx32" %"PRIx32"):%"PRIu64":", nodeid, pid, msg->seq_no);
			fprintf(stderr, "Incorrect msg seq %"PRIu64" != %"PRIu64"\n", msg->seq_no, last_expected);
		}
	}

	if ((msg->data_len + sizeof(*msg)) != cpg_msg_len) {
		print_basic_iso_datetime(stderr);
		fprintf(stderr, ":(%"PRIx32" %"PRIx32"):%"PRIu64":", nodeid, pid, msg->seq_no);
		fprintf(stderr, "Incorrect message length %"PRIu32"+%zu != %zu\n", msg->data_len, sizeof(*msg), cpg_msg_len);
		return ;
	}

	if (msg->chsum != compute_chsum(msg->data, msg->data_len)) {
		print_basic_iso_datetime(stderr);
		fprintf(stderr, ":(%"PRIx32" %"PRIx32"):%"PRIu64":", nodeid, pid, msg->seq_no);
		fprintf(stderr, "Incorrect message chsum %u != %u\n", msg->chsum, compute_chsum(msg->data, msg->data_len));
		return ;
	}

}

static cpg_model_v1_data_t model_data = {
	.cpg_deliver_fn =            DeliverCallback,
	.cpg_confchg_fn =            NULL,
	.cpg_totem_confchg_fn =      NULL,
};

static void sigintr_handler (int signum) __attribute__((noreturn));
static void sigintr_handler (int signum) {
	exit (0);
}

static void usage(void)
{
	printf("Usage: [-q] [-n num]\n");
	printf(" -q         Quiet mode\n");
	printf(" -n num     Number of messages in one burst\n");

	exit(1);
}

int main (int argc, char *argv[]) {
	cpg_handle_t handle;
	int select_fd;
	int result;
	struct cpg_name group_name;
	char *ep;
	long num = 1;
	int ch;
	struct pollfd pfd;
	int retries;

	last_msg = 0;
	last_expected = 0;

	while ((ch = getopt(argc, argv, "qhn:")) != -1) {
		switch (ch) {
		case 'q':
			quiet = 1;
			break;
		case 'n':
			num = strtol(optarg, &ep, 10);
			if (num <= 0 || *ep != '\0') {
				warnx("illegal number, -n argument -- %s", optarg);
				usage();
			}
			break;
		case 'h':
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	}

	strcpy(group_name.value, "CPGLOAD");
	group_name.length = strlen(group_name.value);

	result = cpg_model_initialize (&handle, CPG_MODEL_V1, (cpg_model_data_t *)&model_data, NULL);
	if (result != CS_OK) {
		fprintf (stderr, "Could not initialize Cluster Process Group API instance error %d\n", result);
		exit (1);
	}

	retries = 0;
	cs_repeat(retries, 30, result = cpg_join(handle, &group_name));
	if (result != CS_OK) {
		fprintf (stderr, "Could not join process group, error %d\n", result);
		exit (1);
	}

	retries = 0;
	cs_repeat(retries, 30, result = cpg_local_get (handle, &local_nodeid));
	if (result != CS_OK) {
		fprintf (stderr, "Could not get local node id\n");
		exit (1);
	}

	setlinebuf(stdout);

	cpg_fd_get(handle, &select_fd);

	do {
		pfd.fd = select_fd;
		pfd.events = POLLIN;
		pfd.revents = 0;

		result = poll(&pfd, 1, 0);

		if (result == -1) {
			perror("poll\n");
		}

		if (last_expected == last_msg) {
			send_msgs(handle, num);
		}

		if (result == 1 && pfd.revents & POLLIN) {
			if ((result = cpg_dispatch (handle, CS_DISPATCH_ALL)) != CS_OK) {
				if (result == CS_ERR_LIBRARY) {
					exit(0);
				}
				if (result != CS_ERR_TRY_AGAIN) {
					fprintf (stderr, "Dispatch error %d\n", result);
					exit(1);
				}
			}
		}
	} while (result != -1);

	return (0);
}
