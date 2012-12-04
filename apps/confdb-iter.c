/*
 * Check find and iter for confdb
 */
#include <stdio.h>

#include <corosync/corotypes.h>
#include <corosync/confdb.h>
#include <assert.h>

#define OBJECTS 10
#define SUBOBJECTS 10
#define EQUAL_NAME_OBJS 3
#define EQUAL_NAME_SUBOBJS 4

hdb_handle_t *object_handle; //[EQUAL_NAME_OBJS * EQUAL_NAME_OBJS];
hdb_handle_t *subobject_handle; //[EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS];
hdb_handle_t *found_obj_handle; //[EQUAL_NAME_OBJS];
hdb_handle_t *found_subobj_handle; //[EQUAL_NAME_SUBOBJS];
int *obj_found; //[EQUAL_NAME_OBJS * EQUAL_NAME_OBJS];
int *subobj_found; //[EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS];
int *no_obj_found; //EQUAL_NAME_OBJS
int *no_subobj_found; //EQUAL_NAME_SUBOBJS

void
init_mem(void)
{
	object_handle = malloc(sizeof(hdb_handle_t) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS);
	assert(object_handle != NULL);
	memset(object_handle, 0, sizeof(hdb_handle_t) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS);
	subobject_handle = malloc(sizeof(hdb_handle_t) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS);
	assert(subobject_handle != NULL);
	memset(subobject_handle, 0, sizeof(hdb_handle_t) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS);

	found_obj_handle = malloc(sizeof(hdb_handle_t) * EQUAL_NAME_OBJS);
	assert(found_obj_handle != NULL);

	found_subobj_handle = malloc(sizeof(hdb_handle_t) * EQUAL_NAME_SUBOBJS);
	assert(found_subobj_handle != NULL);

	obj_found = malloc(sizeof(int) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS);
	assert(obj_found != NULL);

	subobj_found = malloc(sizeof(int) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS);
	assert(subobj_found != NULL);

	no_obj_found = malloc(sizeof(int) * EQUAL_NAME_OBJS);
	assert(no_obj_found != NULL);

	no_subobj_found = malloc(sizeof(int) * EQUAL_NAME_SUBOBJS);
	assert(no_subobj_found != NULL);
}

void
free_mem(void)
{
	free(object_handle);
	free(subobject_handle);
	free(found_obj_handle);
	free(found_subobj_handle);
	free(obj_found);
	free(subobj_found);
	free(no_subobj_found);
	free(no_obj_found);
}

void
destroy_objects(confdb_handle_t handle)
{
	int i;

	for (i = 0; i < EQUAL_NAME_OBJS * EQUAL_NAME_OBJS; i++) {
		assert(confdb_object_destroy(handle, object_handle[i]) == CS_OK);
	}
}

void
create_objects(confdb_handle_t handle)
{
	int i, j;
	char obj_name[255];
	int16_t i16;
	int32_t i32;

	for (i = 0; i < EQUAL_NAME_OBJS * EQUAL_NAME_OBJS; i++) {
		snprintf(obj_name, sizeof(obj_name), "testconfdb%u", i % EQUAL_NAME_OBJS);
		assert(confdb_object_create(handle, OBJECT_PARENT_HANDLE,
				obj_name, strlen(obj_name), &object_handle[i]) == CS_OK);

		for (j = 0; j < EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS; j++) {
			snprintf(obj_name, sizeof(obj_name), "subobj%u", j % EQUAL_NAME_SUBOBJS);
			assert(confdb_object_create(handle, object_handle[i],
					obj_name, strlen(obj_name),
					&subobject_handle[j + i * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS]) == CS_OK);
			i32 = i16 = j + i * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS;
			assert(confdb_key_create_typed(handle,
				subobject_handle[j + i * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS], "testkeyi16",
				&i16, sizeof(i16), CONFDB_VALUETYPE_INT16) == CS_OK);
			assert(confdb_key_create_typed(handle,
				subobject_handle[j + i * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS], "testkeyi32",
				&i32, sizeof(i32), CONFDB_VALUETYPE_INT32) == CS_OK);
		}
	}
}

void
find_test(confdb_handle_t handle)
{
	hdb_handle_t obj_handle, subobj_handle;
	char obj_name[255],  subobj_name[255];
	int i, j, k, l, count, count2;
	cs_error_t err, err2;

	memset(found_obj_handle, 0, sizeof(hdb_handle_t) * EQUAL_NAME_OBJS);
	memset(found_subobj_handle, 0, sizeof(hdb_handle_t) * EQUAL_NAME_SUBOBJS);
	memset(obj_found, 0, sizeof(int) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS);
	memset(subobj_found, 0, sizeof(int) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS);

	for (i = 0; i < EQUAL_NAME_OBJS; i++) {
		snprintf(obj_name, sizeof(obj_name), "testconfdb%u", i);

		assert(confdb_object_find_start(handle, OBJECT_PARENT_HANDLE) == CS_OK);
		err = CS_OK;
		count = 0;

		while (err == CS_OK) {
			err = confdb_object_find(handle, OBJECT_PARENT_HANDLE, obj_name, strlen(obj_name), &obj_handle);
			assert(err == CS_OK || err == CS_ERR_ACCESS);
			if (err == CS_ERR_ACCESS)
				break;

			count++;
			assert(count <= EQUAL_NAME_OBJS);
			found_obj_handle[count - 1] = obj_handle;

			for (k = 0; k < EQUAL_NAME_OBJS * EQUAL_NAME_OBJS; k++) {
				if (object_handle[k] == obj_handle) {
					assert(++(obj_found[k]) == 1);
				}
			}

			for (j = 0; j < EQUAL_NAME_SUBOBJS; j++) {
				snprintf(subobj_name, sizeof(subobj_name), "subobj%u", j);
				assert(confdb_object_find_start(handle, obj_handle) == CS_OK);
				err2 = CS_OK;
				count2 = 0;

				while (err2 == CS_OK) {
					err2 = confdb_object_find(handle, obj_handle,
						subobj_name, strlen(subobj_name), &subobj_handle);
					assert(err2 == CS_OK || err2 == CS_ERR_ACCESS);
					if (err2 == CS_ERR_ACCESS)
						break;

					count2++;
					assert(count2 <= EQUAL_NAME_SUBOBJS);
					found_subobj_handle[count2 - 1] = subobj_handle;
					for (k = 0;
					    k < EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS;
					    k++) {
						if (subobject_handle[k] == subobj_handle) {
							assert(++(subobj_found[k]) == 1);
						}
					}
				}

				assert(count2 == EQUAL_NAME_SUBOBJS);
				for (k = 0; k < count2; k++) {
					for (l = k + 1; l < count2; l++) {
						assert(found_subobj_handle[k] != found_subobj_handle[l]);
					}
				}
			}
		}

		assert(count == EQUAL_NAME_OBJS);
		for (k = 0; k < count; k++) {
			for (l = k + 1; l < count; l++) {
				assert(found_obj_handle[k] != found_obj_handle[l]);
			}
		}
	}
}

void
iter_test(confdb_handle_t handle)
{
	hdb_handle_t obj_handle, subobj_handle;
	char obj_name[255], subobj_name[255];
	char key_value[256];
	char key_name[256];
	int i, j, k, count;
	cs_error_t err, err2, err3;
	size_t obj_name_len;
	size_t subobj_name_len;
	size_t len;
	confdb_value_types_t type;
	int16_t i16;
	int32_t i32;

	memset(found_obj_handle, 0, sizeof(hdb_handle_t) * EQUAL_NAME_OBJS);
	memset(found_subobj_handle, 0, sizeof(hdb_handle_t) * EQUAL_NAME_SUBOBJS);
	memset(obj_found, 0, sizeof(int) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS);
	memset(subobj_found, 0, sizeof(int) * EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS);
	memset(no_obj_found, 0, sizeof(int) * EQUAL_NAME_OBJS);

	assert(confdb_object_iter_start(handle, OBJECT_PARENT_HANDLE) == CS_OK);

	err = CS_OK;

	while (err == CS_OK) {
		err = confdb_object_iter(handle, OBJECT_PARENT_HANDLE, &obj_handle, &obj_name, &obj_name_len);
		assert(err == CS_OK || err == CS_ERR_ACCESS);
		if (err == CS_ERR_ACCESS)
			break;

		obj_name[obj_name_len] = 0;
		if (strncmp(obj_name, "testconfdb", strlen("testconfdb")) != 0)
			continue ;

		sscanf(obj_name, "testconfdb%u", &i);
		no_obj_found[i]++;
		assert(no_obj_found[i] <= EQUAL_NAME_OBJS);

		memset(no_subobj_found, 0, sizeof(int) * EQUAL_NAME_SUBOBJS);
		assert(confdb_object_iter_start(handle, obj_handle) == CS_OK);

		for (k = 0; k < EQUAL_NAME_OBJS * EQUAL_NAME_OBJS; k++) {
			if (object_handle[k] == obj_handle) {
				assert(++(obj_found[k]) == 1);
			}
		}

		err2 = CS_OK;

		while (err2 == CS_OK) {
			err2 = confdb_object_iter(handle, obj_handle, &subobj_handle, &subobj_name, &subobj_name_len);
			assert(err2 == CS_OK || err2 == CS_ERR_ACCESS);
			if (err2 == CS_ERR_ACCESS)
				break;

			subobj_name[subobj_name_len] = 0;
			assert(strncmp(subobj_name, "subobj", strlen("subobj")) == 0);

			sscanf(subobj_name, "subobj%u", &j);
			no_subobj_found[j]++;
			assert(no_subobj_found[i] <= EQUAL_NAME_SUBOBJS);

			for (k = 0; k < EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS; k++) {
				if (subobject_handle[k] == subobj_handle) {
					assert(++(subobj_found[k]) == 1);
				}
			}

			i32 = i16 = (((no_obj_found[i] - 1) * EQUAL_NAME_OBJS) + i)
			    * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS + ((no_subobj_found[j] - 1) * EQUAL_NAME_SUBOBJS + j);
			assert(confdb_key_get_typed(handle, subobj_handle, "testkeyi16", &key_value, &len, &type) == CS_OK);
			assert(len == sizeof(i16));
			assert(memcmp(key_value, &i16, sizeof(i16)) == 0);

			assert(confdb_key_get_typed(handle, subobj_handle, "testkeyi32", &key_value, &len, &type) == CS_OK);
			assert(len == sizeof(i32));
			assert(memcmp(key_value, &i32, sizeof(i32)) == 0);

			assert(confdb_key_iter_start(handle, subobj_handle) == CS_OK);

			err3 = CS_OK;
			count = 0;

			while (err3 == CS_OK) {
				err3 = confdb_key_iter_typed(handle, subobj_handle, key_name, key_value, &len, &type);
				assert(err3 == CS_OK || err3 == CS_ERR_ACCESS);
				if (err3 == CS_ERR_ACCESS)
					break;

				if (strcmp(key_name, "testkeyi16") == 0) {
					assert(len == sizeof(i16));
					assert(memcmp(key_value, &i16, sizeof(i16)) == 0);
				} else if (strcmp(key_name, "testkeyi32") == 0) {
					assert(len == sizeof(i32));
					assert(memcmp(key_value, &i32, sizeof(i32)) == 0);
				} else {
					assert(key_name == NULL);
				}
				count++;
			}
			assert(count == 2);
		}
	}

	for (k = 0; k < EQUAL_NAME_OBJS * EQUAL_NAME_OBJS; k++) {
		assert(obj_found[k] == 1);
	}

	for (k = 0; k < EQUAL_NAME_OBJS * EQUAL_NAME_OBJS * EQUAL_NAME_SUBOBJS * EQUAL_NAME_SUBOBJS; k++) {
		assert(subobj_found[k] == 1);
	}
}

int
main(void)
{
	confdb_handle_t handle;
	confdb_callbacks_t callbacks;
	int i;

	printf("confdb-iter initialize\n");

	memset(&callbacks, 0, sizeof(callbacks));
	init_mem();

	assert(confdb_initialize(&handle, &callbacks) == CS_OK);
	printf("confdb-iter run\n");
	for (i = 0; i < 100; i++) {
		create_objects(handle);
		find_test(handle);
		iter_test(handle);
		destroy_objects(handle);
	}

	printf("confdb-iter finish\n");
	free_mem();

	return (0);
}
