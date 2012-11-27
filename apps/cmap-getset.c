/*
 * Check set / get / replace and delete for confdb
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
	assert(cmap_set(handle, "testobj.testkeyi8_2", &i8, sizeof(i8), CMAP_VALUETYPE_INT8) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeyi16_2", &i16, sizeof(i16), CMAP_VALUETYPE_INT16) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeyi32_2", &i32, sizeof(i32), CMAP_VALUETYPE_INT32) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeyi64_2", &i64, sizeof(i64), CMAP_VALUETYPE_INT64) == CS_OK);

	assert(cmap_set_uint8(handle, "testobj.testkeyu8", u8) == CS_OK);
	assert(cmap_set_uint16(handle, "testobj.testkeyu16", u16) == CS_OK);
	assert(cmap_set_uint32(handle, "testobj.testkeyu32", u32) == CS_OK);
	assert(cmap_set_uint64(handle, "testobj.testkeyu64", u64) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeyu8_2", &u8, sizeof(u8), CMAP_VALUETYPE_UINT8) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeyu16_2", &u16, sizeof(u16), CMAP_VALUETYPE_UINT16) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeyu32_2", &u32, sizeof(u32), CMAP_VALUETYPE_UINT32) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeyu64_2", &u64, sizeof(u64), CMAP_VALUETYPE_UINT64) == CS_OK);
	assert(cmap_set_float(handle, "testobj.testkeyflt", flt) == CS_OK);
	assert(cmap_set_double(handle, "testobj.testkeydbl", dbl) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeyflt_2", &flt, sizeof(flt), CMAP_VALUETYPE_FLOAT) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeydbl_2", &dbl, sizeof(dbl), CMAP_VALUETYPE_DOUBLE) == CS_OK);

	assert(cmap_set_string(handle, "testobj.str", str) == CS_OK);
	assert(cmap_set(handle, "testobj.str_2", str, strlen(str), CMAP_VALUETYPE_STRING) == CS_OK);

	assert(cmap_set_int8(handle, "tes tobj.testkeyu8", u8) == CS_ERR_NAME_TOO_LONG);
	assert(cmap_set_int8(handle, "testtesttest.testtesttesttesttesttesttesttesttesttesttesttest"
				"testtesttest.testtesttesttesttesttesttesttesttesttesttesttest" 
				"testtesttest.testtesttesttesttesttesttesttesttesttesttesttest" 
				"testtesttest.testtesttesttesttesttesttesttesttesttesttesttest" 
				"testtesttest.testtesttesttesttesttesttesttesttesttesttesttest" 
				"testtesttest.testtesttesttesttesttesttesttesttesttesttesttest" 
				"testtesttest.testtesttesttesttesttesttesttesttesttesttesttest" 
				"testtesttest.testtesttesttesttesttesttesttesttesttesttesttest", 
				u8) == CS_ERR_NAME_TOO_LONG);
	assert(cmap_set_int8(handle, NULL, u8) == CS_ERR_INVALID_PARAM);
	assert(cmap_set(handle, NULL, "one", strlen("one"), CMAP_VALUETYPE_BINARY) == CS_ERR_INVALID_PARAM);
	assert(cmap_set(handle, "", "one", strlen("one"), CMAP_VALUETYPE_BINARY) == CS_ERR_NAME_TOO_LONG);
	assert(cmap_set(handle, "testobj.fail", NULL, 0, CMAP_VALUETYPE_BINARY) == CS_ERR_INVALID_PARAM);
	assert(cmap_set(handle, "testobj.fail", NULL, 2, CMAP_VALUETYPE_BINARY) == CS_ERR_INVALID_PARAM);
}

static void
check_keys_value(cmap_handle_t handle)
{
	char key_value[256];
	char *key_value2;
	size_t len;
	int16_t i16;
	uint16_t u16;
	int32_t i32;
	uint32_t u32;
	int64_t i64;
	uint64_t u64;
	int8_t i8;
	uint8_t u8;
	float flt;
	double dbl;
	cmap_value_types_t type;
	char *str = "testkeystr";

	len = 0;
	assert(cmap_get(handle, "testobj.testkey", NULL, &len, &type) == CS_OK);
	assert(len == 3);
	assert(type == CMAP_VALUETYPE_BINARY);
	assert(cmap_get(handle, "testobj.testkey", key_value, &len, &type) == CS_OK);
	assert(strncmp(key_value, "one", len) == 0);
	len = 2;
	assert(cmap_get(handle, "testobj.testkey", key_value, &len, &type) == CS_ERR_INVALID_PARAM);

	assert(cmap_get_int8(handle, "testobj.testkeyi8_2", &i8) == CS_OK);
	assert(i8 == 98);
	assert(cmap_get_int8(handle, "testobj.testkeyi16_2", &i8) == CS_ERR_INVALID_PARAM);
	assert(cmap_get_uint8(handle, "testobj.testkeyu8_2", &u8) == CS_OK);
	assert(u8 == 99);

	assert(cmap_get_int16(handle, "testobj.testkeyi16_2", &i16) == CS_OK);
	assert(i16 == 1);
	assert(cmap_get_uint16(handle, "testobj.testkeyu16_2", &u16) == CS_OK);
	assert(u16 == 2);

	assert(cmap_get_int32(handle, "testobj.testkeyi32_2", &i32) == CS_OK);
	assert(i32 == 3);
	assert(cmap_get_uint32(handle, "testobj.testkeyu32_2", &u32) == CS_OK);
	assert(u32 == 4);

	assert(cmap_get_int64(handle, "testobj.testkeyi64_2", &i64) == CS_OK);
	assert(i64 == 5);
	assert(cmap_get_uint64(handle, "testobj.testkeyu64_2", &u64) == CS_OK);
	assert(u64 == 6);

	assert(cmap_get_float(handle, "testobj.testkeyflt_2", &flt) == CS_OK);
	assert(flt == 7.2f);
	assert(cmap_get_double(handle, "testobj.testkeydbl_2", &dbl) == CS_OK);
	assert(dbl == 8.3);

	len = sizeof(i8);
	assert(cmap_get(handle, "testobj.testkeyi8", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(i8));
	assert(type == CMAP_VALUETYPE_INT8);
	assert(memcmp(key_value, &i8, sizeof(i8)) == 0);

	len = sizeof(u8);
	assert(cmap_get(handle, "testobj.testkeyu8", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(u8));
	assert(type == CMAP_VALUETYPE_UINT8);
	assert(memcmp(key_value, &u8, sizeof(u8)) == 0);

	len = sizeof(i16);
	assert(cmap_get(handle, "testobj.testkeyi16", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(i16));
	assert(type == CMAP_VALUETYPE_INT16);
	assert(memcmp(key_value, &i16, sizeof(i16)) == 0);

	len = sizeof(u16);
	assert(cmap_get(handle, "testobj.testkeyu16", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(u16));
	assert(type == CMAP_VALUETYPE_UINT16);
	assert(memcmp(key_value, &u16, sizeof(u16)) == 0);

	len = sizeof(i32);
	assert(cmap_get(handle, "testobj.testkeyi32", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(i32));
	assert(type == CMAP_VALUETYPE_INT32);
	assert(memcmp(key_value, &i32, sizeof(i32)) == 0);

	len = sizeof(u32);
	assert(cmap_get(handle, "testobj.testkeyu32", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(u32));
	assert(type == CMAP_VALUETYPE_UINT32);
	assert(memcmp(key_value, &u32, sizeof(u32)) == 0);

	len = sizeof(i64);
	assert(cmap_get(handle, "testobj.testkeyi64", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(i64));
	assert(type == CMAP_VALUETYPE_INT64);
	assert(memcmp(key_value, &i64, sizeof(i64)) == 0);

	len = sizeof(u64);
	assert(cmap_get(handle, "testobj.testkeyu64", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(u64));
	assert(type == CMAP_VALUETYPE_UINT64);
	assert(memcmp(key_value, &u64, sizeof(u64)) == 0);

	len = sizeof(flt);
	assert(cmap_get(handle, "testobj.testkeyflt", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(flt));
	assert(type == CMAP_VALUETYPE_FLOAT);
	assert(memcmp(key_value, &flt, sizeof(flt)) == 0);

	len = sizeof(dbl);
	assert(cmap_get(handle, "testobj.testkeydbl", key_value, &len, &type) == CS_OK);
	assert(len == sizeof(dbl));
	assert(type == CMAP_VALUETYPE_DOUBLE);
	assert(memcmp(key_value, &dbl, sizeof(dbl)) == 0);

	len = strlen(str) + 1;
	assert(cmap_get(handle, "testobj.str", key_value, &len, &type) == CS_OK);
	assert(len == strlen(str) + 1);
	assert(type == CMAP_VALUETYPE_STRING);
	assert(memcmp(key_value, str, strlen(str) + 1) == 0);

	assert(cmap_get_string(handle, "testobj.str_2", &key_value2) == CS_OK);
	assert(strlen(key_value2) == strlen(str));
	assert(memcmp(key_value2, str, strlen(str)) == 0);	
	free(key_value2);

	assert(cmap_get_int8(handle, "testobj.testk", &i8) == CS_ERR_NOT_EXIST);

	assert(cmap_get(handle, NULL, NULL, NULL, NULL) == CS_ERR_INVALID_PARAM);
	assert(cmap_get(handle, "testobj.testkey", NULL, NULL, &type) == CS_OK);
	assert(type == CMAP_VALUETYPE_BINARY);
	assert(cmap_get(handle, "testobj.testkey", NULL, &len, NULL) == CS_OK);
	assert(len == 3);
	assert(cmap_get(handle, "testobj.testkey", key_value, NULL, NULL) == CS_ERR_INVALID_PARAM);
	assert(cmap_get(handle, "testobj.testkey", key_value, NULL, &type) == CS_ERR_INVALID_PARAM);
	len = 2;
	assert(cmap_get(handle, "testobj.testkey", key_value, &len, NULL) == CS_ERR_INVALID_PARAM);
}

static void
key_replace(cmap_handle_t handle)
{
	char key_value[256];
	size_t len;
	cmap_value_types_t type;

	assert(cmap_set(handle, "testobj.testkey", "something", strlen("something"), CMAP_VALUETYPE_BINARY) == CS_OK);
	len = 0;
	assert(cmap_get(handle, "testobj.testkey", NULL, &len, &type) == CS_OK);
	assert(len == strlen("something"));
	assert(type == CMAP_VALUETYPE_BINARY);
	assert(cmap_get(handle, "testobj.testkey", key_value, &len, &type) == CS_OK);
	assert(strncmp(key_value, "something", len) == 0);

	assert(cmap_set(handle, "testobj.testkey", "two", strlen("two"), CMAP_VALUETYPE_BINARY) == CS_OK);
	len = 0;
	assert(cmap_get(handle, "testobj.testkey", NULL, &len, &type) == CS_OK);
	assert(len == strlen("two"));
	assert(type == CMAP_VALUETYPE_BINARY);
	assert(cmap_get(handle, "testobj.testkey", key_value, &len, &type) == CS_OK);
	assert(strncmp(key_value, "two", len) == 0);
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

	assert(cmap_delete(handle, "testobj.testkeyi8_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyi8_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeyu8_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyu8_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeyi16_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyi16_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeyu16_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyu16_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeyi32_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyi32_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeyu32_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyu32_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeyi64_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyi64_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeyu64_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyu64_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.testkeyflt_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeyflt_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.testkeydbl_2") == CS_OK);
	assert(cmap_get(handle, "testobj.testkeydbl_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, "testobj.str_2") == CS_OK);
	assert(cmap_get(handle, "testobj.str_2", NULL, NULL, &type) == CS_ERR_NOT_EXIST);
	assert(cmap_delete(handle, "testobj.str") == CS_OK);
	assert(cmap_get(handle, "testobj.str", NULL, NULL, &type) == CS_ERR_NOT_EXIST);

	assert(cmap_delete(handle, NULL) == CS_ERR_INVALID_PARAM);
}

int
main(void)
{
	cmap_handle_t handle;
	const void *con_res;

	printf("cmap-getset initialize\n");

	assert(cmap_initialize(&handle) == CS_OK);
	assert(cmap_context_set(handle, keys_delete) == CS_OK);

	printf("cmap-getset run\n");
	keys_create(handle);
	check_keys_value(handle);
	key_replace(handle);
	keys_delete(handle);
	assert(cmap_context_get(handle, &con_res) == CS_OK);
	assert(con_res == keys_delete);

	printf("cmap-getset finish\n");

	return (0);
}
