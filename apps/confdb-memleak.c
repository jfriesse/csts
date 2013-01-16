/*
 * Create many objects in confdb
 */
#include <stdio.h>

#include <corosync/corotypes.h>
#include <corosync/confdb.h>
#include <assert.h>
#include <string.h>

static void
keys_create(confdb_handle_t handle, hdb_handle_t object_handle, int oper)
{
	int c1, c2, c3, c4;
	char obj_name[16];
	uint32_t u32;
	hdb_handle_t new_obj_handle;
	char str[128];
	int i;

	for (i = 0; i < sizeof(str); i++) {
		str[i] = 'a' + i % ('z' - 'a');
	}
	str[sizeof(str) - 1] = 0;

	u32 = 0;
	memset(obj_name, 0, sizeof(obj_name));

	obj_name[0] = '0' + oper;

	for (c1 = 'a'; c1 <= 'z'; c1++) {
		obj_name[1] = c1;
		for (c2 = 'a'; c2 <= 'z'; c2++) {
			obj_name[2] = c2;
			for (c3 = 'a'; c3 <= 'z'; c3++) {
				obj_name[3] = c3;
				for (c4 = 'a'; c4 <= 'g'; c4++) {
					obj_name[4] = c4;
					u32++;
					assert(confdb_object_create(handle, object_handle,
						obj_name, strlen(obj_name), &new_obj_handle) == CS_OK);

					if (u32 % 2 == 0) {
						assert(confdb_key_create_typed(handle, new_obj_handle, "value",
								&u32, sizeof(u32), CONFDB_VALUETYPE_UINT32) == CS_OK);
					} else {
						i = str[sizeof(str) - 2];
						memmove(str + 1, str, sizeof(str) - 2);
						str[0] = i;
						assert(confdb_key_create_typed(handle, new_obj_handle, "value",
							str, strlen(str), CONFDB_VALUETYPE_STRING) == CS_OK);
					}
				}
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	confdb_handle_t handle;
	confdb_callbacks_t callbacks;
	hdb_handle_t object_handle;
	int oper;
	int delete = 0;

	if (argc != 2 || (strcmp(argv[1], "-1") != 0 && strcmp(argv[1], "-2") != 0 &&
	    strcmp(argv[1], "-1d") != 0 && strcmp(argv[1], "-2d") != 0)) {
		printf("Usage: %s [-1|-2|-d]\n", argv[0]);
		printf("  -1d    First iteration (create many objects + delete)\n");
		printf("  -2d    Second iteration (create many objects with different prefix + delete)\n");
		printf("  -1    First iteration (create many objects no delete)\n");
		printf("  -2    Second iteration (create many objects with different prefix no delete)\n");

		return (0);
	}
	if (strcmp(argv[1], "-1d") == 0) {
		oper = 1;
		delete = 1;
	}
	if (strcmp(argv[1], "-2d") == 0) {
		oper = 2;
		delete = 1;
	}
	if (strcmp(argv[1], "-1") == 0) {
		oper = 1;
	}
	if (strcmp(argv[1], "-2") == 0) {
		oper = 2;
	}

	printf("confdb-memleak initialize\n");

	memset(&callbacks, 0, sizeof(callbacks));

	assert(confdb_initialize(&handle, &callbacks) == CS_OK);

	assert(confdb_object_create(handle, OBJECT_PARENT_HANDLE,
			"testconfdb", strlen("testconfdb"), &object_handle) == CS_OK);
	printf("confdb-memleak keys_create\n");
	keys_create(handle, object_handle, oper);

	printf("confdb-memleak finish\n");
	if (delete) {
		assert(confdb_object_destroy(handle, object_handle) == CS_OK);
	}

	return (0);
}
