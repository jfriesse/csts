/*
 * Display confchg events
 */

/*
 * Output is:
 * ConfchgCallback:CPGCONFCHGGROUP:(join_node_1 join_pid1)[,(join_node_2 join_pid2)...]:(leave_node1 leave_pid1)[,(leave_node2 leave_pid2)...]:(member_node1 member_pid1)[,(member_node2 member_pid2)
 * TotemConfchgCallback:ring_id_node.ring_id_seq:(member_node1 member_pid1)[,(member_node2 member_pid2)
 * VIEW:(no_nodes:NE - Not equal):cpg_node1[,cpg_node2]:totem_node1[,totem_node2]:
 *
 * Example:
 * ConfchgCallback:CPGCONFCHGGROUP:(6b26220a 1dc2)::(6b26220a 1dc2)
 * VIEW:NE:6b26220a:
 * TotemConfchgCallback:6b26220a.a00000000010f98:6b26220a,6c26220a
 * VIEW:2:6b26220a,6c26220a:6b26220a,6c26220a
 */

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

#define MAX_NODES 255

#define ITEM_CPG	0
#define ITEM_TOTEM	1

struct view {
	int totem_up;
	int cpg_up;
	int nodeid;
};
static struct view nodes[MAX_NODES];

static void change_node_state(int nodeid, int up, int item)
{
	int i;

	for (i = 0; i < MAX_NODES; i++) {
		if (nodeid == nodes[i].nodeid) {
			switch (item) {
			case ITEM_CPG:
				nodes[i].cpg_up = up;
				break;
			case ITEM_TOTEM:
				nodes[i].totem_up = up;
				break;
			default:
				exit(2);
			}
			return ;
		}
	}

	for (i = 0; i < MAX_NODES; i++) {
		if (nodes[i].nodeid == 0) {
			nodes[i].nodeid = nodeid;
			change_node_state(nodeid, up, item);
			return ;
		}
	}
}

static void reset_nodes_state(int item)
{
	int i;

	for (i = 0;i < MAX_NODES; i++) {
		switch (item) {
		case ITEM_CPG:
			nodes[i].cpg_up = 0;
			break;
		case ITEM_TOTEM:
			nodes[i].totem_up = 0;
			break;
		default:
			exit(2);
		}
	}
}

static void display_nodes_view(void) {
	int i;
	int no_nodes = 0;
	int equal = 1;
	static char cpg_str[4096];
	static char totem_str[4096];
	static char nodeid_str[256];

	cpg_str[0] = 0;
	totem_str[0] = 0;

        printf("VIEW:");
	for (i = 0;i < MAX_NODES; i++) {
		if (nodes[i].nodeid != 0) {
			if (nodes[i].cpg_up == 1 || nodes[i].totem_up == 1) {
				if (nodes[i].cpg_up == 1 && nodes[i].totem_up == 1) {
					no_nodes++;
				} else {
					equal = 0;
				}

				sprintf(nodeid_str, "%x", nodes[i].nodeid);

				if (nodes[i].cpg_up == 1) {
					if (*cpg_str) {
						strcat(cpg_str, ",");
					}
					strcat(cpg_str, nodeid_str);
				}
				if (nodes[i].totem_up == 1) {
					if (*totem_str) {
						strcat(totem_str, ",");
					}
					strcat(totem_str, nodeid_str);
				}
			}
		}
	}

	if (!equal) {
		printf("NE:");
	} else {
		printf("%u:", no_nodes);
	}

	printf("%s:%s\n", cpg_str, totem_str);
}

static void print_cpgname (const struct cpg_name *name)
{
	int i;

	for (i = 0; i < name->length; i++) {
		printf("%c", name->value[i]);
	}
}

static void ConfchgCallback (
	cpg_handle_t handle,
	const struct cpg_name *groupName,
	const struct cpg_address *member_list, size_t member_list_entries,
	const struct cpg_address *left_list, size_t left_list_entries,
	const struct cpg_address *joined_list, size_t joined_list_entries)
{
	int i;

	printf("ConfchgCallback:");
	print_cpgname(groupName);
	printf(":");
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

	reset_nodes_state(ITEM_CPG);
	for (i=0; i < member_list_entries; i++) {
		if (i != 0) {
			printf(",");
		}
		printf("(%x %x)", member_list[i].nodeid, member_list[i].pid);
		change_node_state(member_list[i].nodeid, 1, ITEM_CPG);
	}
	printf ("\n");
	display_nodes_view();
}

static void TotemConfchgCallback (
	cpg_handle_t handle,
        struct cpg_ring_id ring_id,
        uint32_t member_list_entries,
        const uint32_t *member_list)
{
	int i;

	printf("TotemConfchgCallback:%x.%"PRIx64, ring_id.nodeid, ring_id.seq);
	printf(":");

	reset_nodes_state(ITEM_TOTEM);
	for (i=0; i < member_list_entries; i++) {
		if (i != 0) {
			printf(",");
		}
		printf("%x", member_list[i]);
		change_node_state(member_list[i], 1, ITEM_TOTEM);
	}
	printf ("\n");
	display_nodes_view();
}

static cpg_model_v1_data_t model_data = {
	.cpg_deliver_fn =            NULL,
	.cpg_confchg_fn =            ConfchgCallback,
	.cpg_totem_confchg_fn =      TotemConfchgCallback,
	.flags =                     CPG_MODEL_V1_DELIVER_INITIAL_TOTEM_CONF,
};

static void sigintr_handler (int signum) __attribute__((noreturn));
static void sigintr_handler (int signum) {
	exit (0);
}

int main (int argc, char *argv[]) {
	cpg_handle_t handle;
	fd_set read_fds;
	int select_fd;
	int result;
	int i;
	struct cpg_name group_name;

	for (i = 0; i < MAX_NODES; i++) {
		memset(&nodes[0], 0, sizeof(struct view));
	}

	strcpy(group_name.value, "CPGCONFCHGGROUP");
	group_name.length = strlen(group_name.value);

	result = cpg_model_initialize (&handle, CPG_MODEL_V1, (cpg_model_data_t *)&model_data, NULL);
	if (result != CS_OK) {
		printf ("Could not initialize Cluster Process Group API instance error %d\n", result);
		exit (1);
	}

	result = cpg_join(handle, &group_name);
	if (result != CS_OK) {
		printf ("Could not join process group, error %d\n", result);
		exit (1);
	}

	setlinebuf(stdout);

	FD_ZERO (&read_fds);
	cpg_fd_get(handle, &select_fd);
	do {
		FD_SET (select_fd, &read_fds);
		result = select (select_fd + 1, &read_fds, 0, 0, 0);
		if (result == -1) {
			perror ("select\n");
		}
		if (FD_ISSET (select_fd, &read_fds)) {
			if (cpg_dispatch (handle, CS_DISPATCH_ALL) != CS_OK)
				exit(1);
		}
	} while (result);

	return (0);
}
