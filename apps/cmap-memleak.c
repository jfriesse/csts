/*
 * Create many objects in cmap
 */
#include <stdio.h>

#include <corosync/corotypes.h>
#include <corosync/cmap.h>
#include <assert.h>
#include <string.h>

static void
keys_create(cmap_handle_t handle, int oper)
{
	int c1, c2, c3, c4;
	char obj_name[16];
	uint32_t u32;
	char str[128];
	int i;
	char key_name[CMAP_KEYNAME_MAXLEN + 1];

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
					sprintf(key_name, "testobj.%s.value", obj_name);
					if (u32 % 2 == 0) {
						assert(cmap_set(handle, key_name, &u32, sizeof(u32),
						    CMAP_VALUETYPE_UINT32) == CS_OK);
					} else {
						i = str[sizeof(str) - 2];
						memmove(str + 1, str, sizeof(str) - 2);
						str[0] = i;
						assert(cmap_set_string(handle, key_name, str) == CS_OK);
					}
				}
			}
		}
	}
}

static void
keys_delete(cmap_handle_t handle)
{
	cmap_iter_handle_t iter_handle;
	cs_error_t err = CS_OK;
	char key_name[CMAP_KEYNAME_MAXLEN + 1];

	assert(cmap_iter_init(handle, "testobj.", &iter_handle) == CS_OK);
	while (err == CS_OK) {
		err = cmap_iter_next(handle, iter_handle, key_name, NULL, NULL);
		assert(err == CS_OK || err == CS_ERR_NO_SECTIONS);
		if (err == CS_ERR_NO_SECTIONS)
			break;
		assert(cmap_delete(handle, key_name) == CS_OK);
	}
}

int
main(int argc, char *argv[])
{
	cmap_handle_t handle;
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

	printf("cmap-memleak initialize\n");

	assert(cmap_initialize(&handle) == CS_OK);

	printf("cmap-memleak keys_create\n");
	keys_create(handle, oper);

	printf("cmap-memleak finish\n");
	if (delete) {
		keys_delete(handle);
	}

	return (0);
}
