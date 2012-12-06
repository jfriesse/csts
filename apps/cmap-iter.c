/*
 * Check iter for cmap
 */
#include <stdio.h>

#include <corosync/corotypes.h>
#include <corosync/cmap.h>
#include <assert.h>

static void
keys_create(cmap_handle_t handle)
{
	int8_t i8;
	uint8_t u8;
	int16_t i16;
	uint16_t u16;
	int32_t i32;
	uint32_t u32;
	int64_t i64;
	uint64_t u64;
	float flt;
	double dbl;
	char *str = "testkeystr";

	i8 = 98;
	u8 = 99;
	i16 = 1;
	u16 = 2;
	i32 = 3;
	u32 = 4;
	i64 = 5;
	u64 = 6;
	flt = 7.2;
	dbl = 8.3;

	assert(cmap_set(handle, "testobj.testkey", "one", strlen("one"), CMAP_VALUETYPE_BINARY) == CS_OK);
	assert(cmap_set_int8(handle, "testobj.testkeyi8", i8) == CS_OK);
	assert(cmap_set_int16(handle, "testobj.testkeyi16", i16) == CS_OK);
	assert(cmap_set_int32(handle, "testobj.testkeyi32", i32) == CS_OK);
	assert(cmap_set_int64(handle, "testobj.testkeyi64", i64) == CS_OK);
	assert(cmap_set_uint8(handle, "testobj.testkeyu8", u8) == CS_OK);
	assert(cmap_set_uint16(handle, "testobj.testkeyu16", u16) == CS_OK);
	assert(cmap_set_uint32(handle, "testobj.testkeyu32", u32) == CS_OK);
	assert(cmap_set_uint64(handle, "testobj.testkeyu64", u64) == CS_OK);
	assert(cmap_set_float(handle, "testobj.testkeyflt", flt) == CS_OK);
	assert(cmap_set_double(handle, "testobj.testkeydbl", dbl) == CS_OK);
	assert(cmap_set_string(handle, "testobj.testkeystr", str) == CS_OK);
}

static void
check_key(const char *key_name, size_t *value_len, cmap_value_types_t *type)
{
	size_t expected_value_len;
	cmap_value_types_t expected_type;

	expected_value_len = 0;

	if (strcmp(key_name, "testobj.testkey") == 0) {
		expected_value_len = 3;
		expected_type = CMAP_VALUETYPE_BINARY;
	}
	if (strcmp(key_name, "testobj.testkeyi8") == 0) {
		expected_value_len = sizeof(int8_t);
		expected_type = CMAP_VALUETYPE_INT8;
	}
	if (strcmp(key_name, "testobj.testkeyu8") == 0) {
		expected_value_len = sizeof(uint8_t);
		expected_type = CMAP_VALUETYPE_UINT8;
	}
	if (strcmp(key_name, "testobj.testkeyi16") == 0) {
		expected_value_len = sizeof(int16_t);
		expected_type = CMAP_VALUETYPE_INT16;
	}
	if (strcmp(key_name, "testobj.testkeyu16") == 0) {
		expected_value_len = sizeof(uint16_t);
		expected_type = CMAP_VALUETYPE_UINT16;
	}
	if (strcmp(key_name, "testobj.testkeyi32") == 0) {
		expected_value_len = sizeof(int32_t);
		expected_type = CMAP_VALUETYPE_INT32;
	}
	if (strcmp(key_name, "testobj.testkeyu32") == 0) {
		expected_value_len = sizeof(uint32_t);
		expected_type = CMAP_VALUETYPE_UINT32;
	}
	if (strcmp(key_name, "testobj.testkeyi64") == 0) {
		expected_value_len = sizeof(int64_t);
		expected_type = CMAP_VALUETYPE_INT64;
	}
	if (strcmp(key_name, "testobj.testkeyu64") == 0) {
		expected_value_len = sizeof(uint64_t);
		expected_type = CMAP_VALUETYPE_UINT64;
	}
	if (strcmp(key_name, "testobj.testkeyflt") == 0) {
		expected_value_len = sizeof(float);
		expected_type = CMAP_VALUETYPE_FLOAT;
	}
	if (strcmp(key_name, "testobj.testkeydbl") == 0) {
		expected_value_len = sizeof(double);
		expected_type = CMAP_VALUETYPE_DOUBLE;
	}
	if (strcmp(key_name, "testobj.testkeystr") == 0) {
		expected_value_len = strlen("testkeystr") + 1;
		expected_type = CMAP_VALUETYPE_STRING;
	}

	assert(expected_value_len != 0);
	if (value_len != NULL) {
		assert(*value_len == expected_value_len);
	}

	if (type != NULL) {
		assert(*type == expected_type);
	}
}

static void
keys_iter(cmap_handle_t handle)
{
	cmap_iter_handle_t iter_handle;
	char key_name[CMAP_KEYNAME_MAXLEN + 1];
	size_t value_len;
	cmap_value_types_t type;
	cs_error_t err;
	int count;

	assert(cmap_iter_init(handle, "testobj.", NULL) == CS_ERR_INVALID_PARAM);
	assert(cmap_iter_init(handle, NULL, NULL) == CS_ERR_INVALID_PARAM);

	assert(cmap_iter_init(handle, "testobj.", &iter_handle) == CS_OK);
	assert(cmap_iter_next(handle, iter_handle, NULL, NULL, NULL) == CS_ERR_INVALID_PARAM);
	assert(cmap_iter_next(handle, iter_handle, key_name, NULL, NULL) == CS_OK);
	check_key(key_name, NULL, NULL);
	assert(cmap_iter_finalize(handle, iter_handle) == CS_OK);
	assert(cmap_iter_finalize(handle, iter_handle) == CS_ERR_BAD_HANDLE);

	assert(cmap_iter_init(handle, "testobj.", &iter_handle) == CS_OK);
	assert(cmap_iter_next(handle, iter_handle, key_name, &value_len, NULL) == CS_OK);
	check_key(key_name, &value_len, NULL);
	assert(cmap_iter_finalize(handle, iter_handle) == CS_OK);

	assert(cmap_iter_init(handle, "testobj.", &iter_handle) == CS_OK);

	err = CS_OK;
	count = 0;

	while (err == CS_OK) {
		err = cmap_iter_next(handle, iter_handle, key_name, &value_len, &type);
		assert(err == CS_OK || err == CS_ERR_NO_SECTIONS);
		if (err == CS_ERR_NO_SECTIONS)
			break;

		check_key(key_name, &value_len, &type);
		count++;
	}
	assert(count == 12);
	assert(cmap_iter_finalize(handle, iter_handle) == CS_OK);

}

static void
keys_delete(cmap_handle_t handle)
{
	cmap_value_types_t type;

	assert(cmap_delete(handle, "testobj.testkey") == CS_OK);
	assert(cmap_get(handle, "testobj.testkey", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeyi8") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyi8", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeyu8") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyu8", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeyi16") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyi16", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeyu16") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyu16", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeyi32") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyi32", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeyu32") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyu32", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeyi64") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyi64", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeyu64") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyu64", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeyflt") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyflt", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeydbl") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeydbl", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeystr") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeystr", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
}

int
main(void)
{
	cmap_handle_t handle;

	printf("cmap-iter initialize\n");

	assert(cmap_initialize(&handle) == CS_OK);

	printf("cmap-iter run\n");
	keys_create(handle);
	keys_iter(handle);
	keys_delete(handle);

	printf("cmap-iter finish\n");

	return (0);
}
