/*
 * Creates upto MAX_NO_CLIENTS forks to fit into /dev/shm and loops
 * cpg_model_initialize and cpg_finalize for maximum MAX_RUNTIME
 * seconds. When one of call falls, error is returned ($? != 0).
 */
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <time.h>

#include <corosync/corotypes.h>
#include <corosync/cpg.h>

#define MAX_NO_CLIENTS			4096
/*
 * Maximum runtime is 3 minutes
 */
#define MAX_RUNTIME			(3 * 60)

cpg_handle_t handles[MAX_NO_CLIENTS];

static cpg_model_v1_data_t model_data = {
	.cpg_deliver_fn =            NULL,
	.cpg_confchg_fn =            NULL,
	.cpg_totem_confchg_fn =      NULL,
	.flags =                     0,
};

int
count_maximum_no_clients(void)
{
	cs_error_t res;
	int i, j;

	i = 0;

	while (i < MAX_NO_CLIENTS) {
		res = cpg_model_initialize(&handles[i], CPG_MODEL_V1, (cpg_model_data_t *)&model_data, NULL);
		if (res != CS_OK) {
			break;
		} else if (res == CS_ERR_TRY_AGAIN) {
			/*
			 * Try again
			 */
		} else {
			i++;
		}
	}

	for (j = 0; j < i; j++) {
		assert(cpg_finalize(handles[j]) == CS_OK);
	}

	return (i);
}

int
main(int argc, char *argv[])
{
	cs_error_t res;
	int i;
	int no_clients;
	cpg_handle_t handle;
	int status;
	struct timeval tv;
	time_t sec_start;
	int ret;

	no_clients = count_maximum_no_clients();
	if (no_clients < 10) {
		errx(1, "Can't initialize enough cpg clients");
	}

	gettimeofday(&tv, NULL);
	sec_start = tv.tv_sec;

	srand(tv.tv_sec);

	while (1) {
		for (i = 0; i < no_clients; i++) {
			if (fork() == 0) {
				srand(getpid() + i);

				poll(NULL, 0, rand() % 20);

				res = cpg_model_initialize(&handle, CPG_MODEL_V1, (cpg_model_data_t *)&model_data, NULL);
				if (res != CS_OK && res != CS_ERR_TRY_AGAIN) {
					errx(2, "Error %u", res);
				}
				if (res == CS_OK) {
					assert(cpg_finalize(handle) == CS_OK);
				}

				return (0);
			}
		}

		ret = 0;
		for (i = 0; i < no_clients; i++) {
			wait(&status);

			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) != 0) {
					ret = WEXITSTATUS(status);
				}
			} else {
					ret = 3;
			}
		}

		if (ret != 0) {
			return (ret);
		}

		gettimeofday(&tv, NULL);
		if (abs(tv.tv_sec - sec_start) > MAX_RUNTIME) {
			break ;
		}
	}

	return (0);
}
