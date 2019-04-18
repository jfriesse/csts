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
 * Simple test of CPG callbacks. Program initially joins CPG group, waits for
 * cpg_confchg and cpg_totem_confchg callbacks and then starts loop of sending
 * and waiting for the message. Message has random length (up-to MAX_MSG_LEN)
 * and random content. rand_r is used for random number generating. Message
 * is checked on delivery by issuing same rand_r. Seed for rand_r are
 * stored in sent_rand_seed and received_rand_seed variables.
 *
 * This test checks following CPG properties:
 * - Process can join CPG group
 * - Both cpg_confchg and cpg_totem_confchg callbacks are received with
 *   only one node and only one process in there
 * - Messages are sent and received without any corruption
 */

#include <sys/types.h>
#include <sys/socket.h>

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
#define TEST_GROUP_NAME		"smoke-api-test-cpg-group"
#define MAX_RETRIES		30
#define MAX_MSG_LEN		(1024 * 128)
#define MESSAGES_TO_SENT	128

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

#define ENTER() printf("+ %s:%u %s\n", __FILE__, __LINE__, __func__)
#define LEAVE() printf("- %s:%u %s\n", __FILE__, __LINE__, __func__)

static int cpg_confchg_received = 0;
static int cpg_totem_confchg_received = 0;

/*
 * both rand seeds must be in sync
 */
static unsigned int sent_rand_seed = 0;
static unsigned int received_rand_seed = 0;
static uint32_t sent_msg_seq_no = 0;
static uint32_t received_msg_seq_no = 0;

struct test_msg {
	uint32_t seq_no __attribute__ ((aligned (8)));
	uint32_t data_len __attribute__ ((aligned (8)));
	unsigned char data[0] __attribute__ ((aligned (8)));
};

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
	assert(pid == getpid());

	assert(cpg_msg_len >= sizeof(struct test_msg));

	msg = (struct test_msg *)cpg_msg;
	assert(msg->seq_no == received_msg_seq_no);
	received_msg_seq_no++;

	assert(cpg_msg_len == msg->data_len + sizeof(struct test_msg));

	/*
	 * Message has expected length and content
	 */
	assert(msg->data_len == (uint32_t)(rand_r(&received_rand_seed) % MAX_MSG_LEN));

	for (u32 = 0; u32 < msg->data_len; u32++) {
		assert(msg->data[u32] == (unsigned char)rand_r(&received_rand_seed));
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

	cpg_totem_confchg_received = 1;

	LEAVE();
}

static void
ConfchgCallback(cpg_handle_t handle,
    const struct cpg_name *group_name,
    const struct cpg_address *member_list, size_t member_list_entries,
    const struct cpg_address *left_list, size_t left_list_entries,
    const struct cpg_address *joined_list, size_t joined_list_entries)
{

	ENTER();

	assert(group_name->length == strlen(TEST_GROUP_NAME));
	assert(memcmp(group_name->value, TEST_GROUP_NAME, strlen(TEST_GROUP_NAME)) == 0);
	assert(member_list_entries == 1);
	assert(member_list[0].nodeid == TEST_NODEID);
	assert(member_list[0].pid == getpid());

	cpg_confchg_received = 1;

	LEAVE();
}

static cpg_model_v1_data_t model_data = {
	.cpg_deliver_fn =	DeliverCallback,
	.cpg_confchg_fn =	ConfchgCallback,
	.cpg_totem_confchg_fn =	TotemConfchgCallback,
	.flags =		CPG_MODEL_V1_DELIVER_INITIAL_TOTEM_CONF,
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

	msg.data_len = rand_r(&sent_rand_seed) % MAX_MSG_LEN;
	msg.seq_no = sent_msg_seq_no;

	sent_msg_seq_no++;

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

	LEAVE();
}

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
	int state;
	int cont;

	ENTER();

	state = 0;

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

	cont = 1;

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

		switch (state) {
		case 0:
			/*
			 * Waiting for cpg_confchg_received and cpg_totem_confchg_received
			 */
			if (cpg_confchg_received && cpg_totem_confchg_received) {
				/*
				 * Send first message and wait for it in next state
				 */
				send_msg(cpg_handle);
				state = 1;
			}
			break;
		case 1:
			if (received_msg_seq_no >= MESSAGES_TO_SENT) {
				cont = 0;
			} else  if (received_msg_seq_no == sent_msg_seq_no) {
				/*
				 * Message delivered so sent new one and wait for it
				 */
				send_msg(cpg_handle);
			}
			break;
		}
	} while (cont);

	cs_repeat(cs_res = cpg_leave(cpg_handle, &group_name));
	assert(cs_res == CS_OK);

	cs_repeat(cs_res = cpg_finalize(cpg_handle));
	assert(cs_res == CS_OK);

	LEAVE();
}

int
main(void)
{

	ENTER();

	setlinebuf(stdout);

	test_cpg();

	LEAVE();

	return (0);
}
