#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <corosync/corotypes.h>
#include <corosync/cpg.h>


static void DeliverCallback (
        cpg_handle_t handle,
	const struct cpg_name *groupName,
        uint32_t nodeid,
        uint32_t pid,
        void *msg,
        size_t msg_len)
{
}

static void ConfchgCallback (
	cpg_handle_t handle,
	const struct cpg_name *groupName,
	const struct cpg_address *member_list, size_t member_list_entries,
	const struct cpg_address *left_list, size_t left_list_entries,
	const struct cpg_address *joined_list, size_t joined_list_entries)
{
}

static void TotemConfchgCallback (
	cpg_handle_t handle,
	struct cpg_ring_id ring_id,
	uint32_t member_list_entries,
	const uint32_t *member_list)
{
}

static cpg_model_v1_data_t model_data = {
	.cpg_deliver_fn =            DeliverCallback,
	.cpg_confchg_fn =            ConfchgCallback,
	.cpg_totem_confchg_fn =      TotemConfchgCallback,
	.flags =                     CPG_MODEL_V1_DELIVER_INITIAL_TOTEM_CONF,
};

static struct cpg_name group_name;

int main (int argc, char *argv[]) {
	cpg_handle_t handle;
	int result;
	int i;
	struct iovec iov;
	int no_msgs = 300000;
	char str[255];

	strcpy(group_name.value, "GROUP");
	group_name.length = 6;

	result = cpg_model_initialize (&handle, CPG_MODEL_V1, (cpg_model_data_t *)&model_data, NULL);
	if (result != CS_OK) {
		printf ("Could not initialize Cluster Process Group API instance error %d\n", result);
		exit (1);
	}

	do {
	    result = cpg_join(handle, &group_name);
	    if (result != CS_OK && result != CS_ERR_TRY_AGAIN) {
		printf ("Could not join process group, error %d\n", result);
		exit (1);
	    }
	} while (result != CS_OK);


	for (i = 1; i < no_msgs; i++) {
		snprintf(str, sizeof(str), "longstrlongstrlongstrlongstrlongstrlongstrlongstrlongstrlongstrlongstr"
		    "longstrlongstrlongstr %u", i);
		iov.iov_base = str;
		iov.iov_len = strlen(iov.iov_base) + 1;
		result = cpg_mcast_joined(handle, CPG_TYPE_AGREED, &iov, 1);

		if (result == CS_ERR_TRY_AGAIN) i--;

		if (((i * 700) % no_msgs) == 0) {
			printf(".");
			fflush(stdout);
			if (((i * 10) % no_msgs) == 0) {
				printf(" %u %%\n", ((i * 100) / no_msgs));
			}
		}
	}

	printf(" %u %%\n", 100);
	fflush(stdout);
	if ((argc > 1) && (strcmp(argv[1], "-w") == 0)) {
		do {
			printf("Enter EXIT to exit\n");
			fflush(stdout);
			if (fgets(str, sizeof(str), stdin)) ; /* Ignore result */
		} while (strncmp(str, "EXIT", 4) != 0);
	}
	result = cpg_finalize (handle);
	return (0);
}
