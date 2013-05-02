/*
 * Track object in confdb/cmap for changes in multiple process and when callback is called,
 * eventually do no_burst changes. This will make a HUGE load to confdb.
 */

#include <stdio.h>

#include <corosync/corotypes.h>

#ifdef USE_CONFDB
#include <corosync/confdb.h>
#endif

#ifdef USE_CMAP
#include <corosync/cmap.h>
#endif

#include <assert.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <string.h>
#include <err.h>
#include <poll.h>

#ifdef USE_CMAP
#define cs_repeat(counter, max, code) do {      \
	code;                                   \
	if (result == CS_ERR_TRY_AGAIN) {       \
		counter++;                      \
		sleep(1);                       \
	} else {                                \
		break;                          \
	}                                       \
} while (counter < max)
#endif

static char my_key_uint[255];
static char my_key_str[255];
static int my_id;
static int expected_msgs_uint;
static int expected_msgs_str;
static int no_childs;
static int burst_count;
static int change_uint32;
static int change_str_len;
static pid_t *child_pids;
static int *child_pipes;
static int master_pipe;

static void
inc_str(char *str, int str_len)
{
	int i;

	for (i = 0; i < str_len; i++) {
		str[i] = ((str[i] - 'A' + 1) % ('Z' - 'A' + 1)) + 'A';
	}
}

#ifdef USE_CONFDB
static void
confdb_key_change_notify (
	confdb_handle_t handle,
	confdb_change_type_t change_type,
	hdb_handle_t parent_object_handle,
	hdb_handle_t object_handle,
	const void *object_name,
	size_t object_name_len,
	const void *key_name,
	size_t key_name_len,
	const void *key_value,
	size_t key_value_len)
{
	char str_val[255];
	uint32_t val;
	int i;

	if (key_name_len == strlen(my_key_uint) && memcmp(key_name, my_key_uint, key_name_len) == 0) {
		expected_msgs_uint--;

		if (expected_msgs_uint == 0) {
			for (i = 0; i < burst_count; i++) {
				confdb_key_increment(handle, parent_object_handle, my_key_uint, strlen(my_key_uint), &val);
			}
			expected_msgs_uint = burst_count;
		}
	}

	if (key_name_len == strlen(my_key_str) && memcmp(key_name, my_key_str, key_name_len) == 0) {
		expected_msgs_str--;

		if (expected_msgs_str == 0) {
			memcpy(str_val, key_value, key_value_len);
			str_val[key_value_len] = '\0';

			for (i = 0; i < burst_count; i++) {
				inc_str(str_val, key_value_len);
				confdb_key_replace(handle, parent_object_handle, my_key_str, strlen(my_key_str),
						NULL, 0, str_val, strlen(str_val));
			}
			expected_msgs_str = burst_count;
		}
	}
}
#endif

#ifdef USE_CMAP
static void cmap_notify(
	cmap_handle_t cmap_handle,
	cmap_track_handle_t cmap_track_handle,
	int32_t event,
	const char *key_name,
	struct cmap_notify_value new_value,
	struct cmap_notify_value old_value,
	void *user_data)
{
	char str_val[255];
	int i;

	if (strlen(key_name) == strlen(my_key_uint) && memcmp(key_name, my_key_uint, strlen(key_name)) == 0) {
		expected_msgs_uint--;

		if (expected_msgs_uint == 0) {
			for (i = 0; i < burst_count; i++) {
				cmap_inc(cmap_handle, key_name);
			}
			expected_msgs_uint = burst_count;
		}
	}

	if (strlen(key_name) == strlen(my_key_str) && memcmp(key_name, my_key_str, strlen(key_name)) == 0) {
		expected_msgs_str--;

		if (expected_msgs_str == 0) {
			memcpy(str_val, new_value.data, new_value.len);
			str_val[new_value.len - 1] = '\0';

			for (i = 0; i < burst_count; i++) {
				inc_str(str_val, new_value.len - 1);
				cmap_set_string(cmap_handle, my_key_str, str_val);
			}
			expected_msgs_str = burst_count;
		}
	}

}
#endif

static int
create_childs(void)
{
	pid_t pid;
	int i;
	int pipe_fd[2];

	assert((child_pids = malloc(no_childs * sizeof(pid_t))) != NULL);
	assert((child_pipes = malloc(no_childs * sizeof(int))) != NULL);

	for (i = 1; i <= no_childs; i++) {
		assert(pipe(pipe_fd) == 0);

		pid = fork();
		assert(pid >= 0);
		if (pid == 0) {
			master_pipe = pipe_fd[1];
			close(pipe_fd[0]);

			return (i);
		}

		child_pids[i - 1] = pid;

		child_pipes[i - 1] = pipe_fd[0];
		close(pipe_fd[1]);
	}

	return (0);
}


static void
usage(void) {

	printf("Usage: [-u] [-s str_len] [-c no_childs] [-n burst count]\n");

	printf(" -u             Change uint enabled\n");
        printf(" -s str_len     Change string with str_len length\n");
	printf(" -c no_childs   Number of childs to fork (number of corosync IPC connections) (default 16)\n");
	printf(" -n burst_count Number of changes in burst (default 64)\n");

	exit(1);
}

static void
sigint_handler_parent(int sig)
{
	int i, j;
	struct pollfd *pfds;
	int nfds;
	int res;

	assert((pfds = malloc(no_childs * sizeof(*pfds))) != NULL);

	do {
		nfds = 0;
		for (i = 0; i < no_childs; i++) {
			if (child_pipes[i] != 0) {
				kill(child_pids[i], SIGINT);

				pfds[nfds].events = 0;
				pfds[nfds].revents = 0;
				pfds[nfds++].fd = child_pipes[i];
			}
		}

		res = poll(pfds, nfds, 100);

		for (j = 0; j < res; j++) {
			for (i = 0; i < no_childs; i++) {
				if (child_pipes[i] == pfds[j].fd) {
					if (pfds[j].revents & (POLLERR | POLLHUP | POLLNVAL)) {
						child_pipes[i] = 0;
					}
				}
			}
		}
	} while (nfds > 0);
}

static void
sigint_handler_child(int sig)
{

	exit(0);
}

int
main(int argc, char *argv[])
{
#ifdef USE_CONFDB
	confdb_handle_t handle;
	confdb_callbacks_t callbacks;
	hdb_handle_t object_handle;
#endif
#ifdef USE_CMAP
	cmap_handle_t handle;
	cmap_track_handle_t track_handle;
	int retries;
	cs_error_t result;
#endif
	uint32_t u32;
	int status;
	int i;
	cs_error_t res;
	char str_val[255];
	int ch;
	char *ep;

	change_uint32 = 0;
	change_str_len = 0;
	no_childs = 16;
	burst_count = 64;

	while ((ch = getopt(argc, argv, "hus:c:n:")) != -1) {
		switch (ch) {
		case 'u':
			change_uint32 = 1;
			break;
		case 's':
			change_str_len = strtol(optarg, &ep, 10);
			if (change_str_len <= 0 || *ep != '\0') {
				warnx("illegal number, -s argument -- %s", optarg);
				usage();
			}
			break;
		case 'c':
			no_childs = strtol(optarg, &ep, 10);
			if (no_childs <= 0 || *ep != '\0') {
				warnx("illegal number, -c argument -- %s", optarg);
				usage();
			}
			break;
		case 'n':
			burst_count = strtol(optarg, &ep, 10);
			if (burst_count <= 0 || *ep != '\0') {
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

	signal(SIGPIPE, SIG_IGN);

	setlinebuf(stdout);

#ifdef USE_CONFDB
	memset(&callbacks, 0, sizeof(callbacks));
	assert(confdb_initialize(&handle, &callbacks) == CS_OK);
	assert(confdb_object_create(handle, OBJECT_PARENT_HANDLE,
		"testconfdb", strlen("testconfdb"), &object_handle) == CS_OK);
	assert(confdb_finalize(handle) == CS_OK);
#endif

	my_id = create_childs();
	u32 = my_id;

#ifdef USE_CONFDB
	snprintf(my_key_uint, sizeof(my_key_uint), "testkeyu32id%u", my_id);

	snprintf(my_key_str, sizeof(my_key_str), "testkeystrid%u", my_id);
#endif

#ifdef USE_CMAP
	snprintf(my_key_uint, sizeof(my_key_uint), "testconfdb.testkeyu32id%u", my_id);

	snprintf(my_key_str, sizeof(my_key_str), "testconfdb.testkeystrid%u", my_id);
#endif

	for (i = 0; i < change_str_len; i++) {
		str_val[i] = ((my_id + i) % ('Z' - 'A' + 1)) + 'A';
	}
	str_val[i] = '\0';

	if (my_id > 0) {
#ifdef USE_CONFDB
		memset(&callbacks, 0, sizeof(callbacks));
		callbacks.confdb_key_change_notify_fn = confdb_key_change_notify;

		assert(confdb_initialize(&handle, &callbacks) == CS_OK);
#endif

#ifdef USE_CMAP
		retries = 0;
		cs_repeat(retries, 30, result = cmap_initialize(&handle));
		assert(result == CS_OK);
#endif
		if (change_uint32) {
#ifdef USE_CONFDB
			assert(confdb_key_create_typed(handle, object_handle, my_key_uint,
				&u32, sizeof(u32), CONFDB_VALUETYPE_UINT32) == CS_OK);
#endif
#ifdef USE_CMAP
			assert(cmap_set_uint32(handle, my_key_uint, u32) == CS_OK);
#endif
		}

		if (change_str_len > 0) {
#ifdef USE_CONFDB
			assert(confdb_key_create_typed(handle, object_handle, my_key_str,
				str_val, strlen(str_val), CONFDB_VALUETYPE_STRING) == CS_OK);
#endif
#ifdef USE_CMAP
			assert(cmap_set_string(handle, my_key_str, str_val) == CS_OK);
#endif
		}
	} else {
		/*
		 * "Wait" for other processes to initialize
		 */
		poll(NULL, 0, 1000);

		printf("Initialization finished\n");
	}

	if (my_id > 0) {
		signal(SIGINT, sigint_handler_child);

#ifdef USE_CONFDB
		assert(confdb_track_changes(handle, object_handle, OBJECT_KEY_REPLACED) == CS_OK);
#endif

#ifdef USE_CMAP
		assert(cmap_track_add(handle, "testconfdb.", CMAP_TRACK_MODIFY | CMAP_TRACK_PREFIX,
			cmap_notify, NULL, &track_handle) == CS_OK);
#endif

		if (change_uint32) {
#ifdef USE_CONFDB
			assert(confdb_key_increment(handle, object_handle, my_key_uint,
						strlen(my_key_uint), &u32) == CS_OK);
#endif
#ifdef USE_CMAP
			assert(cmap_inc(handle, my_key_uint) == CS_OK);
#endif
			expected_msgs_uint = 1;
		}

		if (change_str_len > 0) {
			inc_str(str_val, change_str_len);
#ifdef USE_CONFDB
			assert(confdb_key_replace(handle, object_handle, my_key_str, strlen(my_key_str), NULL, 0,
					str_val, strlen(str_val)) == CS_OK);
#endif
#ifdef USE_CMAP
			assert(cmap_set_string(handle, my_key_str, str_val) == CS_OK);
#endif
			expected_msgs_str = 1;
		}

		/*
		 * Give other processes a little time to initialize
		 */
		poll(NULL, 0, 250);

		do {
#ifdef USE_CONFDB
			res = confdb_dispatch(handle, CS_DISPATCH_BLOCKING);
#endif
#ifdef USE_CMAP
			res = cmap_dispatch(handle, CS_DISPATCH_BLOCKING);
#endif
		} while (res == CS_OK || res == CS_ERR_TRY_AGAIN);
	} else {
		signal(SIGINT, sigint_handler_parent);

		for (i = 0; i < no_childs; i++) {
			wait(&status);
		}

#ifdef USE_CONFDB
		confdb_object_destroy(handle, object_handle);
#endif
		printf("Finalization finished\n");
	}

	return (0);
}
