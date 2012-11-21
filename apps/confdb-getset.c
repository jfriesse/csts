/*
 * Check set / get / replace and delete for confdb
 */
#include <stdio.h>

#include <corosync/corotypes.h>
#include <corosync/confdb.h>
#include <assert.h>

static void
keys_create(confdb_handle_t handle, hdb_handle_t object_handle)
{
	int16_t i16;
	uint16_t u16;
	int32_t i32;
	uint32_t u32;
	int64_t i64;
	uint64_t u64;
	float flt;
	double dbl;
	char *str = "testkeystr";

	i16 = 1;
	u16 = 2;
	i32 = 3;
	u32 = 4;
	i64 = 5;
	u64 = 6;
	flt = 7.2;
	dbl = 8.3;

	assert(confdb_key_create (handle, object_handle, "testkey",
			strlen("testkey"), "one", strlen("one")) == CS_OK);
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
	assert(confdb_key_create_typed(handle, object_handle, "testkeyany",
			str, strlen(str), CONFDB_VALUETYPE_ANY) == CS_OK);
}

static void
check_keys_value(confdb_handle_t handle, hdb_handle_t object_handle)
{
	char key_value[256];
	void *key_value2;
	size_t len;
	int16_t i16;
	uint16_t u16;
	int32_t i32;
	uint32_t u32;
	int64_t i64;
	uint64_t u64;
	float flt;
	double dbl;
	char *str = "testkeystr";
	confdb_value_types_t type;

	i16 = 1;
	u16 = 2;
	i32 = 3;
	u32 = 4;
	i64 = 5;
	u64 = 6;
	flt = 7.2;
	dbl = 8.3;

	assert(confdb_key_get(handle, object_handle, "testkey", strlen("testkey"), &key_value, &len) == CS_OK);
	assert(len == 3);
	assert(strncmp(key_value, "one", len) == 0);

	assert(confdb_key_get(handle, object_handle, "testkeyi16", strlen("testkeyi16"), &key_value, &len) == CS_OK);
	assert(len == sizeof(i16));
	assert(memcmp(key_value, &i16, sizeof(i16)) == 0);
	assert(confdb_key_get(handle, object_handle, "testkeyi32", strlen("testkeyi32"), &key_value, &len) == CS_OK);
	assert(len == sizeof(i32));
	assert(memcmp(key_value, &i32, sizeof(i32)) == 0);
	assert(confdb_key_get(handle, object_handle, "testkeyi64", strlen("testkeyi64"), &key_value, &len) == CS_OK);
	assert(len == sizeof(i64));
	assert(memcmp(key_value, &i64, sizeof(i64)) == 0);
	assert(confdb_key_get(handle, object_handle, "testkeyu16", strlen("testkeyu16"), &key_value, &len) == CS_OK);
	assert(len == sizeof(u16));
	assert(memcmp(key_value, &u16, sizeof(u16)) == 0);
	assert(confdb_key_get(handle, object_handle, "testkeyu32", strlen("testkeyu32"), &key_value, &len) == CS_OK);
	assert(len == sizeof(u32));
	assert(memcmp(key_value, &u32, sizeof(u32)) == 0);
	assert(confdb_key_get(handle, object_handle, "testkeyu64", strlen("testkeyu64"), &key_value, &len) == CS_OK);
	assert(len == sizeof(u64));
	assert(memcmp(key_value, &u64, sizeof(u64)) == 0);
	assert(confdb_key_get(handle, object_handle, "testkeyflt", strlen("testkeyflt"), &key_value, &len) == CS_OK);
	assert(len == sizeof(flt));
	assert(memcmp(key_value, &flt, sizeof(flt)) == 0);
	assert(confdb_key_get(handle, object_handle, "testkeydbl", strlen("testkeydbl"), &key_value, &len) == CS_OK);
	assert(len == sizeof(dbl));
	assert(memcmp(key_value, &dbl, sizeof(dbl)) == 0);
	assert(confdb_key_get(handle, object_handle, "testkeystr", strlen("testkeystr"), &key_value, &len) == CS_OK);
	assert(len == strlen(str));
	assert(memcmp(key_value, str, len) == 0);
	assert(confdb_key_get(handle, object_handle, "testkeyany", strlen("testkeyany"), &key_value, &len) == CS_OK);
	assert(len == strlen(str));
	assert(memcmp(key_value, str, len) == 0);

	assert(confdb_key_get_typed(handle, object_handle, "testkey", &key_value, &len, &type) == CS_OK);
	assert(type == CONFDB_VALUETYPE_ANY);
	assert(len == 3);
	assert(strncmp(key_value, "one", len) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeyi16", &key_value, &len, &type) == CS_OK);
	assert(type == CONFDB_VALUETYPE_INT16);
	assert(len == sizeof(i16));
	assert(memcmp(key_value, &i16, sizeof(i16)) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeyi32", &key_value, &len, &type) == CS_OK);
	assert(len == sizeof(i32));
	assert(memcmp(key_value, &i32, sizeof(i32)) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeyi64", &key_value, &len, &type) == CS_OK);
	assert(len == sizeof(i64));
	assert(memcmp(key_value, &i64, sizeof(i64)) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeyu16", &key_value, &len, &type) == CS_OK);
	assert(len == sizeof(u16));
	assert(memcmp(key_value, &u16, sizeof(u16)) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeyu32", &key_value, &len, &type) == CS_OK);
	assert(len == sizeof(u32));
	assert(memcmp(key_value, &u32, sizeof(u32)) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeyu64", &key_value, &len, &type) == CS_OK);
	assert(len == sizeof(u64));
	assert(memcmp(key_value, &u64, sizeof(u64)) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeyflt", &key_value, &len, &type) == CS_OK);
	assert(len == sizeof(flt));
	assert(memcmp(key_value, &flt, sizeof(flt)) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeydbl", &key_value, &len, &type) == CS_OK);
	assert(len == sizeof(dbl));
	assert(memcmp(key_value, &dbl, sizeof(dbl)) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeystr", &key_value, &len, &type) == CS_OK);
	assert(len == strlen(str));
	assert(memcmp(key_value, str, len) == 0);
	assert(confdb_key_get_typed(handle, object_handle, "testkeyany", &key_value, &len, &type) == CS_OK);
	assert(len == strlen(str));
	assert(memcmp(key_value, str, len) == 0);

	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkey", &key_value2, &len, &type) == CS_OK);
	assert(type == CONFDB_VALUETYPE_ANY);
	assert(len == 3);
	assert(strncmp(key_value2, "one", len) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeyi16", &key_value2, &len, &type) == CS_OK);
	assert(type == CONFDB_VALUETYPE_INT16);
	assert(len == sizeof(i16));
	assert(memcmp(key_value2, &i16, sizeof(i16)) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeyi32", &key_value2, &len, &type) == CS_OK);
	assert(len == sizeof(i32));
	assert(memcmp(key_value2, &i32, sizeof(i32)) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeyi64", &key_value2, &len, &type) == CS_OK);
	assert(len == sizeof(i64));
	assert(memcmp(key_value2, &i64, sizeof(i64)) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeyu16", &key_value2, &len, &type) == CS_OK);
	assert(len == sizeof(u16));
	assert(memcmp(key_value2, &u16, sizeof(u16)) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeyu32", &key_value2, &len, &type) == CS_OK);
	assert(len == sizeof(u32));
	assert(memcmp(key_value2, &u32, sizeof(u32)) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeyu64", &key_value2, &len, &type) == CS_OK);
	assert(len == sizeof(u64));
	assert(memcmp(key_value2, &u64, sizeof(u64)) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeyflt", &key_value2, &len, &type) == CS_OK);
	assert(len == sizeof(flt));
	assert(memcmp(key_value2, &flt, sizeof(flt)) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeydbl", &key_value2, &len, &type) == CS_OK);
	assert(len == sizeof(dbl));
	assert(memcmp(key_value2, &dbl, sizeof(dbl)) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeystr", &key_value2, &len, &type) == CS_OK);
	assert(len == strlen(str));
	assert(memcmp(key_value2, str, len) == 0);
	free(key_value2);
	key_value2 = NULL;
	assert(confdb_key_get_typed2(handle, object_handle, "testkeyany", &key_value2, &len, &type) == CS_OK);
	assert(len == strlen(str));
	assert(memcmp(key_value2, str, len) == 0);
	free(key_value2);
}

static void
key_replace(confdb_handle_t handle, hdb_handle_t object_handle)
{
	char key_value[256];
	size_t len;

	assert(confdb_key_replace(handle, object_handle, "testkey", strlen("testkey"),
			NULL, 0, "something", strlen("something")) == CS_OK);

	assert(confdb_key_get(handle, object_handle, "testkey", strlen("testkey"), &key_value, &len) == CS_OK);
	assert(len == strlen("something"));
	assert(strncmp(key_value, "something", len) == 0);

	assert(confdb_key_replace(handle, object_handle, "testkey", strlen("testkey"),
			NULL, 0, "two", strlen("two")) == CS_OK);

	assert(confdb_key_get(handle, object_handle, "testkey", strlen("testkey"), &key_value, &len) == CS_OK);
	assert(len == strlen("two"));
	assert(strncmp(key_value, "two", len) == 0);
}

static void
keys_delete(confdb_handle_t handle, hdb_handle_t object_handle)
{
	char key_value[256];
	size_t len;

	assert(confdb_key_delete(handle, object_handle, "testkey", strlen("testkey"), NULL, 0) == CS_OK);
	assert(confdb_key_get(handle, object_handle, "testkey", strlen("testkey"), &key_value, &len) == CS_ERR_ACCESS);
	assert(confdb_key_delete(handle, object_handle, "testkeyi16", strlen("testkeyi16"), NULL, 0) == CS_OK);
	assert(confdb_key_get(handle, object_handle, "testkeyi16", strlen("testkeyi16"), &key_value, &len) == CS_ERR_ACCESS);
	assert(confdb_key_delete(handle, object_handle, "testkeyi32", strlen("testkeyi32"), NULL, 0) == CS_OK);
	assert(confdb_key_get(handle, object_handle, "testkeyi32", strlen("testkeyi32"), &key_value, &len) == CS_ERR_ACCESS);
	assert(confdb_key_delete(handle, object_handle, "testkeyi64", strlen("testkeyi64"), NULL, 0) == CS_OK);
	assert(confdb_key_get(handle, object_handle, "testkeyi64", strlen("testkeyi64"), &key_value, &len) == CS_ERR_ACCESS);

	assert(confdb_key_delete(handle, object_handle, "testkey", strlen("testkey"), NULL, 0) == CS_ERR_ACCESS);
	assert(confdb_key_get(handle, object_handle, "testkey", strlen("testkey"), &key_value, &len) == CS_ERR_ACCESS);

	assert(confdb_key_delete(handle, object_handle, "testkeyu16", strlen("testkeyu16"), NULL, 0) == CS_OK);
	assert(confdb_key_get(handle, object_handle, "testkeyu16", strlen("testkeyu16"), &key_value, &len) == CS_ERR_ACCESS);
	assert(confdb_key_delete(handle, object_handle, "testkeyu32", strlen("testkeyu32"), NULL, 0) == CS_OK);
	assert(confdb_key_get(handle, object_handle, "testkeyu32", strlen("testkeyu32"), &key_value, &len) == CS_ERR_ACCESS);
	assert(confdb_key_delete(handle, object_handle, "testkeyu64", strlen("testkeyu64"), NULL, 0) == CS_OK);
	assert(confdb_key_get(handle, object_handle, "testkeyu64", strlen("testkeyu64"), &key_value, &len) == CS_ERR_ACCESS);

	assert(confdb_key_delete(handle, object_handle, "testkeyu16", strlen("testkeyu16"), NULL, 0) == CS_ERR_ACCESS);

	assert(confdb_key_delete(handle, object_handle, "testkeyflt", strlen("testkeyflt"), NULL, 0) == CS_OK);
	assert(confdb_key_get(handle, object_handle, "testkeyflt", strlen("testkeyflt"), &key_value, &len) == CS_ERR_ACCESS);
	assert(confdb_key_delete(handle, object_handle, "testkeydbl", strlen("testkeydbl"), NULL, 0) == CS_OK);
	assert(confdb_key_get(handle, object_handle, "testkeydbl", strlen("testkeydbl"), &key_value, &len) == CS_ERR_ACCESS);
}

int
main(void)
{
	confdb_handle_t handle;
	confdb_callbacks_t callbacks;
	hdb_handle_t object_handle;

	printf("confdb-getset initialize\n");

	memset(&callbacks, 0, sizeof(callbacks));

	assert(confdb_initialize(&handle, &callbacks) == CS_OK);

	assert(confdb_object_create(handle, OBJECT_PARENT_HANDLE,
			"testconfdb", strlen("testconfdb"), &object_handle) == CS_OK);

	printf("confdb-getset run\n");
	keys_create(handle, object_handle);
	check_keys_value(handle, object_handle);
	key_replace(handle, object_handle);
	keys_delete(handle, object_handle);
	assert(confdb_object_destroy(handle, object_handle) == CS_OK);

	printf("confdb-getset finish\n");

	return (0);
}
