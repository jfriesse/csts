/*
 * Copyright (c) 2019, Red Hat, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND RED HAT, INC. DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL RED HAT, INC. BE LIABLE
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Jan Friesse <jfriesse@redhat.com>
 */

/*
 * Main process creates multiple (NO_CHILDS) childs and waits for their exit.
 * Each child joins CPG group and starts processing CPG events. Each child is
 * identified by ID (my_id) which is set by main process in increasing order (so
 * value is 0..NO_CHILDS-1). When all childs join CPG group (ConfchgCallback variable
 * member_list_entries is equal to NO_CHILDS) then child with ID 0 sends CPG message
 * with random length (up-to MAX_MSG_LEN) and random content. When child with ID 1
 * receives message issued by child with ID 0, it sends another random message.
 * This process continues with child with ID 2, then with child 3, ... NO_CHILDS - 1 and
 * then overlaps back to 0. Each child compares received message by generating same
 * random numbers (rand_r function is used) stream as sender so sent_rand_seed and
 * received_rand_seed must be kept in sync. Keeping receied_rand_seed in sync is
 * simple, because it is updated every time when message is received.
 * sent_rand_seed is updated ether by sending message or by
 * calling adjust_sent_rand_seed function.
 *
 * This test checks following properties CPG:
 * - More than one client can join CPG group
 * - Each client eventually sees all other clients
 * - Each client receives messages sent by other clients [1]
 * - Message is not corrupted [1]
 * - Messages are delivered with FIFO quarantee [1]
 *
 * [1] - Ensured by equality of rand_r generated bytes where sender seed
 *       (sent_rand_seed) and receiver seed (received_rand_seed) are kept in
 *       sync.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <corosync/corotypes.h>
#include <corosync/cpg.h>

/*
 * Predefined test constants
 */
#define TEST_NODEID		1
#define TEST_GROUP_NAME		"smoke-api-test-cpg-group-mp"
#define MAX_RETRIES		30
#define MAX_MSG_LEN		(1024 * 128)

#define NO_CHILDS		3

#define MESSAGES_TO_SENT	(128 * NO_CHILDS)

/*
 * Useful macros
 */
#define cs_repeat(code) do {				\
	int cs_repeat_counter = 0;			\
	do {						\
		if ((code) == CS_ERR_TRY_AGAIN) {	\
			cs_repeat_counter++;		\
			poll(NULL, 0, 1000);		\
		} else {				\
			break;				\
		}					\
	} while (cs_repeat_counter < MAX_RETRIES);	\
} while (0)

#ifndef INFTIM
#define INFTIM			-1
#endif

#define ENTER() printf("[%ld] + %s:%u %s\n", (long signed int)getpid(), __FILE__,\
    __LINE__, __func__)
#define LEAVE() printf("[%ld] - %s:%u %s\n", (long signed int)getpid(), __FILE__,\
    __LINE__, __func__)

static int32_t my_id = -1;
static int full_membership_seen = 0;
static int sent_messages = 0;
static int test_cpg_loop_cont = 1;

/*
 * both rand seeds must be in sync
 */
static unsigned int sent_rand_seed = 0;
static unsigned int received_rand_seed = 0;
static int32_t expected_sender_id;
static uint32_t expected_msg_seq_no = 0;

struct test_msg {
	int32_t sender_id __attribute__ ((aligned (8)));
	uint32_t seq_no __attribute__ ((aligned (8)));
	uint32_t data_len __attribute__ ((aligned (8)));
	unsigned char data[0] __attribute__ ((aligned (8)));
};

static void
send_msg(cpg_handle_t cpg_handle)
{
	struct test_msg msg;
	unsigned char *data;
	uint32_t u32;
	struct iovec iov[2];
	cs_error_t cs_res;

	ENTER();

	msg.sender_id = my_id;
	msg.data_len = rand_r(&sent_rand_seed) % MAX_MSG_LEN;
	msg.seq_no = expected_msg_seq_no;

	data = malloc(sizeof(unsigned char) * msg.data_len);
	assert(data != NULL);

	for (u32 = 0; u32 < msg.data_len; u32++) {
		data[u32] = (unsigned char)rand_r(&sent_rand_seed);
	}

	iov[0].iov_base = (void *)&msg;
	iov[0].iov_len = sizeof(msg);
	iov[1].iov_base = (void *)data;
	iov[1].iov_len = msg.data_len;

	cs_res = cpg_mcast_joined(cpg_handle, CPG_TYPE_AGREED, iov, 2);
	assert(cs_res == CS_OK);

	free(data);

	sent_messages++;

	LEAVE();
}

static void
adjust_sent_rand_seed(void)
{
	uint32_t data_len;
	uint32_t u32;

	data_len = rand_r(&sent_rand_seed) % MAX_MSG_LEN;

	for (u32 = 0; u32 < data_len; u32++) {
		(void)rand_r(&sent_rand_seed);
	}
}

static void
DeliverCallback(cpg_handle_t handle,
    const struct cpg_name *groupName,
    uint32_t nodeid,
    uint32_t pid,
    void *cpg_msg,
    size_t cpg_msg_len)
{
	struct test_msg *msg;
	uint32_t u32;

	ENTER();

	assert(nodeid == TEST_NODEID);

	assert(cpg_msg_len >= sizeof(struct test_msg));

	msg = (struct test_msg *)cpg_msg;
	assert(msg->sender_id == expected_sender_id);

	expected_sender_id = (expected_sender_id + 1) % NO_CHILDS;

	assert(msg->seq_no == expected_msg_seq_no);
	expected_msg_seq_no++;

	assert(cpg_msg_len == msg->data_len + sizeof(struct test_msg));

	/*
	 * Message has expected length and content
	 */
	assert(msg->data_len == (uint32_t)(rand_r(&received_rand_seed) % MAX_MSG_LEN));

	for (u32 = 0; u32 < msg->data_len; u32++) {
		assert(msg->data[u32] == (unsigned char)rand_r(&received_rand_seed));
	}

	if (expected_msg_seq_no > MESSAGES_TO_SENT) {
		test_cpg_loop_cont = 0;
	}

	if (expected_sender_id == my_id) {
		send_msg(handle);
	} else {
		adjust_sent_rand_seed();
	}

	LEAVE();
}

static void
TotemConfchgCallback(cpg_handle_t handle,
    struct cpg_ring_id ring_id,
    uint32_t member_list_entries,
    const uint32_t *member_list)
{

	ENTER();

	assert(member_list_entries == 1);
	assert(member_list[0] == TEST_NODEID);

	LEAVE();
}

static void
ConfchgCallback(cpg_handle_t handle,
    const struct cpg_name *group_name,
    const struct cpg_address *member_list, size_t member_list_entries,
    const struct cpg_address *left_list, size_t left_list_entries,
    const struct cpg_address *joined_list, size_t joined_list_entries)
{
	int i;

	ENTER();

	assert(group_name->length == strlen(TEST_GROUP_NAME));
	assert(memcmp(group_name->value, TEST_GROUP_NAME, strlen(TEST_GROUP_NAME)) == 0);

	for (i = 0; i < member_list_entries; i++) {
		assert(member_list[i].nodeid == TEST_NODEID);
	}

	if (!full_membership_seen && member_list_entries == NO_CHILDS) {
		if (my_id == 0) {
			/*
			 * First child sends first message
			 */
			send_msg(handle);
		} else {
			adjust_sent_rand_seed();
		}

		full_membership_seen = 1;
	}

	LEAVE();
}

static cpg_model_v1_data_t model_data = {
	.cpg_deliver_fn =	DeliverCallback,
	.cpg_confchg_fn =	ConfchgCallback,
	.cpg_totem_confchg_fn =	TotemConfchgCallback,
	.flags =		CPG_MODEL_V1_DELIVER_INITIAL_TOTEM_CONF,
};


static void
test_cpg(void)
{
	cpg_handle_t cpg_handle;
	struct cpg_name group_name;
	cs_error_t cs_res;
	unsigned int local_nodeid;
	int cpg_fd;
	struct pollfd pfd;
	int poll_res;

	ENTER();

	strcpy(group_name.value, TEST_GROUP_NAME);
	group_name.length = strlen(TEST_GROUP_NAME);

	cs_repeat(cs_res = cpg_model_initialize(&cpg_handle, CPG_MODEL_V1,
	    (cpg_model_data_t *)&model_data, NULL));
	assert(cs_res == CS_OK);

	cs_repeat(cs_res = cpg_join(cpg_handle, &group_name));
	assert(cs_res == CS_OK);

	cs_repeat(cs_res = cpg_local_get(cpg_handle, &local_nodeid));
	assert(cs_res == CS_OK);
	assert(local_nodeid == TEST_NODEID);

	cs_repeat(cs_res = cpg_fd_get(cpg_handle, &cpg_fd));
	assert(cs_res == CS_OK);

	do {
		pfd.fd = cpg_fd;
		pfd.events = POLLIN;
		pfd.revents = 0;

		poll_res = poll(&pfd, 1, INFTIM);
		if (poll_res == -1) {
			perror("poll_res == -1");
		}

		assert(poll_res != 0);
		assert(pfd.revents & POLLIN);

		cs_repeat(cs_res = cpg_dispatch(cpg_handle, CS_DISPATCH_ALL));
		assert(cs_res == CS_OK);
	} while (test_cpg_loop_cont);

	cs_repeat(cs_res = cpg_leave(cpg_handle, &group_name));
	assert(cs_res == CS_OK);

	cs_repeat(cs_res = cpg_finalize(cpg_handle));
	assert(cs_res == CS_OK);

	LEAVE();
}

int
main(void)
{
	int i;
	pid_t fork_res;
	pid_t wait_res;
	int status;

	ENTER();

	setlinebuf(stdout);

	for (i = 0; i < NO_CHILDS; i++) {
		my_id = i;

		fork_res = fork();

		if (fork_res == -1) {
			perror("fork");
		}

		if (fork_res == 0) {
			/*
			 * Child
			 */
			break ;
		} else {
			/*
			 * Parent
			 */
			my_id = -1;
		}
	}

	if (fork_res == 0) {
		/*
		 * Child runs the test
		 */
		test_cpg();
	} else {
		/*
		 * Wait for childs
		 */
		for (i = 0; i < NO_CHILDS; i++) {
			wait_res = wait(&status);
			if (wait_res == -1) {
				perror("wait");
			}

			assert(WIFEXITED(status));
			assert(WEXITSTATUS(status) == 0);
		}
	}

	LEAVE();

	return (0);
}
