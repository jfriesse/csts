/*
 * Check inc/dec for confdb
 */
#include <stdio.h>

#include <corosync/corotypes.h>
#include <corosync/confdb.h>
#include <assert.h>

static void
incdec(confdb_handle_t handle, hdb_handle_t object_handle)
{
	int16_t i16, i16inc;
	uint16_t u16, u16inc;
	int32_t i32, i32inc;
	uint32_t u32, u32inc;
	int64_t i64, i64inc;
	uint64_t u64, u64inc;
	float flt;
	double dbl;
	char *str = "testkeystr";
	unsigned int res;
	char key_value[256];
	size_t len;
	int i;

	i16 = 1;
	u16 = 2;
	i32 = 3;
	u32 = 4;
	i64 = 5;
	u64 = 6;
	flt = 7.2;
	dbl = 8.3;

	assert(confdb_key_create_typed(handle, object_handle, "testkeyi16",
			&i16, sizeof(i16), CONFDB_VALUETYPE_INT16) == CS_OK);
	assert(confdb_key_create_typed(handle, object_handle, "testkeyi32",
			&i32, sizeof(i32), CONFDB_VALUETYPE_INT32) == CS_OK);
	assert(confdb_key_create_typed(handle, object_handle, "testkeyi64",
			&i64, sizeof(i64), CONFDB_VALUETYPE_INT64) == CS_OK);
	assert(confdb_key_create_typed(handle, object_handle, "testkeyu16",
			&u16, sizeof(u16), CONFDB_VALUETYPE_UINT16) == CS_OK);
	assert(confdb_key_create_typed(handle, object_handle, "testkeyu32",
			&u32, sizeof(u32), CONFDB_VALUETYPE_UINT32) == CS_OK);
	assert(confdb_key_create_typed(handle, object_handle, "testkeyu64",
			&u64, sizeof(u64), CONFDB_VALUETYPE_UINT64) == CS_OK);
	assert(confdb_key_create_typed(handle, object_handle, "testkeyflt",
			&flt, sizeof(flt), CONFDB_VALUETYPE_FLOAT) == CS_OK);
	assert(confdb_key_create_typed(handle, object_handle, "testkeydbl",
			&dbl, sizeof(dbl), CONFDB_VALUETYPE_DOUBLE) == CS_OK);
	assert(confdb_key_create_typed(handle, object_handle, "testkeystr",
			str, strlen(str), CONFDB_VALUETYPE_STRING) == CS_OK);

	assert(confdb_key_increment(handle, object_handle, "testkeyflt", strlen("testkeyflt"), &res) == CS_ERR_ACCESS);
	assert(confdb_key_increment(handle, object_handle, "testkeydbl", strlen("testkeydbl"), &res) == CS_ERR_ACCESS);
	assert(confdb_key_increment(handle, object_handle, "testkeystr", strlen("testkeystr"), &res) == CS_ERR_ACCESS);

	for (i = 1; i < 99; i++) {
		i16inc = i16 + i;
		i32inc = i32 + i;
		i64inc = i64 + i;
		u16inc = u16 + i;
		u32inc = u32 + i;
		u64inc = u64 + i;

		assert(confdb_key_increment(handle, object_handle, "testkeyi16", strlen("testkeyi16"), &res) == CS_OK);
		assert(res == i16inc);
		assert(confdb_key_get(handle, object_handle, "testkeyi16", strlen("testkeyi16"), &key_value, &len) == CS_OK);
		assert(len == sizeof(i16));
		assert(memcmp(key_value, &i16inc, sizeof(i16inc)) == 0);

		assert(confdb_key_increment(handle, object_handle, "testkeyi32", strlen("testkeyi32"), &res) == CS_OK);
		assert(res == i32inc);
		assert(confdb_key_get(handle, object_handle, "testkeyi32", strlen("testkeyi32"), &key_value, &len) == CS_OK);
		assert(len == sizeof(i32));
		assert(memcmp(key_value, &i32inc, sizeof(i32inc)) == 0);

		assert(confdb_key_increment(handle, object_handle, "testkeyi64", strlen("testkeyi64"), &res) == CS_OK);
		assert(res == i64inc);
		assert(confdb_key_get(handle, object_handle, "testkeyi64", strlen("testkeyi64"), &key_value, &len) == CS_OK);
		assert(len == sizeof(i64));
		assert(memcmp(key_value, &i64inc, sizeof(i64inc)) == 0);

		assert(confdb_key_increment(handle, object_handle, "testkeyu16", strlen("testkeyu16"), &res) == CS_OK);
		assert(res == u16inc);
		assert(confdb_key_get(handle, object_handle, "testkeyu16", strlen("testkeyu16"), &key_value, &len) == CS_OK);
		assert(len == sizeof(u16));
		assert(memcmp(key_value, &u16inc, sizeof(u16inc)) == 0);

		assert(confdb_key_increment(handle, object_handle, "testkeyu32", strlen("testkeyu32"), &res) == CS_OK);
		assert(res == u32inc);
		assert(confdb_key_get(handle, object_handle, "testkeyu32", strlen("testkeyu32"), &key_value, &len) == CS_OK);
		assert(len == sizeof(u32));
		assert(memcmp(key_value, &u32inc, sizeof(u32inc)) == 0);

		assert(confdb_key_increment(handle, object_handle, "testkeyu64", strlen("testkeyu64"), &res) == CS_OK);
		assert(res == u64inc);
		assert(confdb_key_get(handle, object_handle, "testkeyu64", strlen("testkeyu64"), &key_value, &len) == CS_OK);
		assert(len == sizeof(u64));
		assert(memcmp(key_value, &u64inc, sizeof(u64inc)) == 0);
	}

	for (i = 97; i >= 0; i--) {
		i16inc = i16 + i;
		i32inc = i32 + i;
		i64inc = i64 + i;
		u16inc = u16 + i;
		u32inc = u32 + i;
		u64inc = u64 + i;

		assert(confdb_key_decrement(handle, object_handle, "testkeyi16", strlen("testkeyi16"), &res) == CS_OK);
		assert(res == i16inc);
		assert(confdb_key_get(handle, object_handle, "testkeyi16", strlen("testkeyi16"), &key_value, &len) == CS_OK);
		assert(len == sizeof(i16));
		assert(memcmp(key_value, &i16inc, sizeof(i16inc)) == 0);

		assert(confdb_key_decrement(handle, object_handle, "testkeyi32", strlen("testkeyi32"), &res) == CS_OK);
		assert(res == i32inc);
		assert(confdb_key_get(handle, object_handle, "testkeyi32", strlen("testkeyi32"), &key_value, &len) == CS_OK);
		assert(len == sizeof(i32));
		assert(memcmp(key_value, &i32inc, sizeof(i32inc)) == 0);

		assert(confdb_key_decrement(handle, object_handle, "testkeyi64", strlen("testkeyi64"), &res) == CS_OK);
		assert(res == i64inc);
		assert(confdb_key_get(handle, object_handle, "testkeyi64", strlen("testkeyi64"), &key_value, &len) == CS_OK);
		assert(len == sizeof(i64));
		assert(memcmp(key_value, &i64inc, sizeof(i64inc)) == 0);

		assert(confdb_key_decrement(handle, object_handle, "testkeyu16", strlen("testkeyu16"), &res) == CS_OK);
		assert(res == u16inc);
		assert(confdb_key_get(handle, object_handle, "testkeyu16", strlen("testkeyu16"), &key_value, &len) == CS_OK);
		assert(len == sizeof(u16));
		assert(memcmp(key_value, &u16inc, sizeof(u16inc)) == 0);

		assert(confdb_key_decrement(handle, object_handle, "testkeyu32", strlen("testkeyu32"), &res) == CS_OK);
		assert(res == u32inc);
		assert(confdb_key_get(handle, object_handle, "testkeyu32", strlen("testkeyu32"), &key_value, &len) == CS_OK);
		assert(len == sizeof(u32));
		assert(memcmp(key_value, &u32inc, sizeof(u32inc)) == 0);

		assert(confdb_key_decrement(handle, object_handle, "testkeyu64", strlen("testkeyu64"), &res) == CS_OK);
		assert(res == u64inc);
		assert(confdb_key_get(handle, object_handle, "testkeyu64", strlen("testkeyu64"), &key_value, &len) == CS_OK);
		assert(len == sizeof(u64));
		assert(memcmp(key_value, &u64inc, sizeof(u64inc)) == 0);
	}

}

int
main(void)
{
	confdb_handle_t handle;
	confdb_callbacks_t callbacks;
	hdb_handle_t object_handle;
	int i;

	printf("confdb-incdec initialize\n");

	memset(&callbacks, 0, sizeof(callbacks));

	assert(confdb_initialize(&handle, &callbacks) == CS_OK);

	printf("confdb-incdec run\n");
	for (i = 0; i < 10; i++) {
		assert(confdb_object_create(handle, OBJECT_PARENT_HANDLE,
				"testconfdb", strlen("testconfdb"), &object_handle) == CS_OK);

		incdec(handle, object_handle);
		assert(confdb_object_destroy(handle, object_handle) == CS_OK);
	}

	printf("confdb-incdec finish\n");

	return (0);
}
