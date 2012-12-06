/*
 * Check inc/dec for cmap
 */
#include <stdio.h>

#include <corosync/corotypes.h>
#include <corosync/cmap.h>
#include <assert.h>

static void
keys_delete(cmap_handle_t handle)
{
	cmap_value_types_t type;

	assert(cmap_delete(handle, "testobj.testkeyi8") == CS_OK);
	assert(cmap_delete(handle, "testobj.testkeyu8") == CS_OK);

	assert(cmap_delete(handle, "testobj.testkeyi16") == CS_OK);
	assert(cmap_delete(handle, "testobj.testkeyu16") == CS_OK);

	assert(cmap_delete(handle, "testobj.testkeyi32") == CS_OK);
	assert(cmap_delete(handle, "testobj.testkeyu32") == CS_OK);

	assert(cmap_delete(handle, "testobj.testkeyi64") == CS_OK);
	assert(cmap_delete(handle, "testobj.testkeyu64") == CS_OK);

	assert(cmap_delete(handle, "testobj.testkeyflt") == CS_OK);
	assert(cmap_delete(handle, "testobj.testkeydbl") == CS_OK);

	assert(cmap_delete(handle, "testobj.testkeystr") == CS_OK);
	assert(cmap_delete(handle, "testobj.testkeybin") == CS_OK);
}

static void
incdec(cmap_handle_t handle)
{
	int8_t i8, i8inc;
	uint8_t u8, u8inc;
	int16_t i16, i16inc;
	uint16_t u16, u16inc;
	int32_t i32, i32inc;
	uint32_t u32, u32inc;
	int64_t i64, i64inc;
	uint64_t u64, u64inc;
	float flt, fltinc;
	double dbl, dblinc;
	char *str = "testkeystr";
	char *strinc;
	char val[255];
	size_t len;
	int i;

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

	assert(cmap_set_uint8(handle, "testobj.testkeyu8", u8) == CS_OK);
	assert(cmap_set_uint16(handle, "testobj.testkeyu16", u16) == CS_OK);
	assert(cmap_set_uint32(handle, "testobj.testkeyu32", u32) == CS_OK);
	assert(cmap_set_uint64(handle, "testobj.testkeyu64", u64) == CS_OK);
	assert(cmap_set_int8(handle, "testobj.testkeyi8", i8) == CS_OK);
	assert(cmap_set_int16(handle, "testobj.testkeyi16", i16) == CS_OK);
	assert(cmap_set_int32(handle, "testobj.testkeyi32", i32) == CS_OK);
	assert(cmap_set_int64(handle, "testobj.testkeyi64", i64) == CS_OK);
	assert(cmap_set_float(handle, "testobj.testkeyflt", flt) == CS_OK);
	assert(cmap_set_double(handle, "testobj.testkeydbl", dbl) == CS_OK);
	assert(cmap_set_string(handle, "testobj.testkeystr", str) == CS_OK);
	assert(cmap_set(handle, "testobj.testkeybin", "one", 3, CMAP_VALUETYPE_BINARY) == CS_OK);

	assert(cmap_inc(handle, "nonexisting") == CS_ERR_NOT_EXIST);
	assert(cmap_dec(handle, "nonexisting") == CS_ERR_NOT_EXIST);
	assert(cmap_inc(handle, "testobj.testkeyflt") == CS_ERR_INVALID_PARAM);
	assert(cmap_get_float(handle, "testobj.testkeyflt", &fltinc) == CS_OK);
	assert(fltinc == flt);
	assert(cmap_inc(handle, "testobj.testkeydbl") == CS_ERR_INVALID_PARAM);
	assert(cmap_get_double(handle, "testobj.testkeydbl", &dblinc) == CS_OK);
	assert(dblinc == dbl);
	assert(cmap_inc(handle, "testobj.testkeystr") == CS_ERR_INVALID_PARAM);
	assert(cmap_get_string(handle, "testobj.testkeystr", &strinc) == CS_OK);
	assert(strlen(strinc) == strlen(str));
	assert(memcmp(str, strinc, strlen(str)) == 0);
	free(strinc);
	assert(cmap_inc(handle, "testobj.testkeybin") == CS_ERR_INVALID_PARAM);
	len = 3;
	assert(cmap_get(handle, "testobj.testkeybin", val, &len, NULL) == CS_OK);
	assert(len == 3);
	assert(memcmp(val, "one", len) == 0);

	assert(cmap_dec(handle, "testobj.testkeyflt") == CS_ERR_INVALID_PARAM);
	assert(cmap_get_float(handle, "testobj.testkeyflt", &fltinc) == CS_OK);
	assert(fltinc == flt);
	assert(cmap_dec(handle, "testobj.testkeydbl") == CS_ERR_INVALID_PARAM);
	assert(cmap_get_double(handle, "testobj.testkeydbl", &dblinc) == CS_OK);
	assert(dblinc == dbl);
	assert(cmap_dec(handle, "testobj.testkeystr") == CS_ERR_INVALID_PARAM);
	assert(cmap_get_string(handle, "testobj.testkeystr", &strinc) == CS_OK);
	assert(strlen(strinc) == strlen(str));
	assert(memcmp(str, strinc, strlen(str)) == 0);
	free(strinc);
	assert(cmap_inc(handle, "testobj.testkeybin") == CS_ERR_INVALID_PARAM);
	len = 3;
	assert(cmap_get(handle, "testobj.testkeybin", val, &len, NULL) == CS_OK);
	assert(len == 3);
	assert(memcmp(val, "one", len) == 0);
	assert(cmap_inc(handle, NULL) == CS_ERR_INVALID_PARAM);
	assert(cmap_dec(handle, NULL) == CS_ERR_INVALID_PARAM);

	for (i = 1; i < 99; i++) {
		assert(cmap_inc(handle, "testobj.testkeyi8") == CS_OK);
		assert(cmap_get_int8(handle, "testobj.testkeyi8", &i8inc) == CS_OK);
		assert((uint8_t)i8inc == (uint8_t)(i8 + i));
		assert(cmap_inc(handle, "testobj.testkeyi16") == CS_OK);
		assert(cmap_get_int16(handle, "testobj.testkeyi16", &i16inc) == CS_OK);
		assert(i16inc == i16 + i);
		assert(cmap_inc(handle, "testobj.testkeyi32") == CS_OK);
		assert(cmap_get_int32(handle, "testobj.testkeyi32", &i32inc) == CS_OK);
		assert(i32inc == i32 + i);
		assert(cmap_inc(handle, "testobj.testkeyi64") == CS_OK);
		assert(cmap_get_int64(handle, "testobj.testkeyi64", &i64inc) == CS_OK);
		assert(i64inc == i64 + i);

		assert(cmap_inc(handle, "testobj.testkeyu8") == CS_OK);
		assert(cmap_get_uint8(handle, "testobj.testkeyu8", &u8inc) == CS_OK);
		assert((uint8_t)u8inc == (uint8_t)(u8 + i));
		assert(cmap_inc(handle, "testobj.testkeyu16") == CS_OK);
		assert(cmap_get_uint16(handle, "testobj.testkeyu16", &u16inc) == CS_OK);
		assert(u16inc == u16 + i);
		assert(cmap_inc(handle, "testobj.testkeyu32") == CS_OK);
		assert(cmap_get_uint32(handle, "testobj.testkeyu32", &u32inc) == CS_OK);
		assert(u32inc == u32 + i);
		assert(cmap_inc(handle, "testobj.testkeyu64") == CS_OK);
		assert(cmap_get_uint64(handle, "testobj.testkeyu64", &u64inc) == CS_OK);
		assert(u64inc == u64 + i);
	}

	for (i = 97; i >= 0; i--) {
		assert(cmap_dec(handle, "testobj.testkeyi8") == CS_OK);
		assert(cmap_get_int8(handle, "testobj.testkeyi8", &i8inc) == CS_OK);
		assert((uint8_t)i8inc == (uint8_t)(i8 + i));
		assert(cmap_dec(handle, "testobj.testkeyi16") == CS_OK);
		assert(cmap_get_int16(handle, "testobj.testkeyi16", &i16inc) == CS_OK);
		assert(i16inc == i16 + i);
		assert(cmap_dec(handle, "testobj.testkeyi32") == CS_OK);
		assert(cmap_get_int32(handle, "testobj.testkeyi32", &i32inc) == CS_OK);
		assert(i32inc == i32 + i);
		assert(cmap_dec(handle, "testobj.testkeyi64") == CS_OK);
		assert(cmap_get_int64(handle, "testobj.testkeyi64", &i64inc) == CS_OK);
		assert(i64inc == i64 + i);

		assert(cmap_dec(handle, "testobj.testkeyu8") == CS_OK);
		assert(cmap_get_uint8(handle, "testobj.testkeyu8", &u8inc) == CS_OK);
		assert((uint8_t)u8inc == (uint8_t)(u8 + i));
		assert(cmap_dec(handle, "testobj.testkeyu16") == CS_OK);
		assert(cmap_get_uint16(handle, "testobj.testkeyu16", &u16inc) == CS_OK);
		assert(u16inc == u16 + i);
		assert(cmap_dec(handle, "testobj.testkeyu32") == CS_OK);
		assert(cmap_get_uint32(handle, "testobj.testkeyu32", &u32inc) == CS_OK);
		assert(u32inc == u32 + i);
		assert(cmap_dec(handle, "testobj.testkeyu64") == CS_OK);
		assert(cmap_get_uint64(handle, "testobj.testkeyu64", &u64inc) == CS_OK);
		assert(u64inc == u64 + i);
	}
}

int
main(void)
{
	cmap_handle_t handle;
	int i;

	printf("cmap-incdec initialize\n");

	assert(cmap_initialize(&handle) == CS_OK);

	printf("cmap-incdec run\n");
	for (i = 0; i < 10; i++) {
		incdec(handle);
		keys_delete(handle);
	}
	printf("cmap-incdec finish\n");

	return (0);
}
