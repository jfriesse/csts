/*
 * Copyright (c) 2009-2011 Red Hat, Inc.
 *
 * All rights reserved.
 *
 * Author: Jan Friesse (jfriesse@redhat.com)
 *
 * This software licensed under BSD license, the text of which follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the Red Hat, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <limits.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <corosync/corotypes.h>
#include <corosync/sam.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

static int sam_test4(void);

int
main(void)
{
    return (sam_test4());
}

/*
 * Test sam_data_store, sam_data_restore and sam_data_getsize
 */
static int
sam_test4 (void)
{
	size_t size;
	cs_error_t err;
	int i;
	unsigned int instance_id;
	char saved_data[128];
	char saved_data2[128];

	setlinebuf(stdout);
	printf ("%s: sam_data_getsize 1\n", __FUNCTION__);
	err = sam_data_getsize (&size);
	if (err != CS_ERR_BAD_HANDLE) {
		fprintf (stderr, "Test should return CS_ERR_BAD_HANDLE. Error returned %d\n", err);
		return 1;
	}

	printf ("%s: sam_data_getsize 2\n", __FUNCTION__);
	err = sam_data_getsize (NULL);
	if (err != CS_ERR_INVALID_PARAM) {
		fprintf (stderr, "Test should return CS_ERR_INVALID_PARAM. Error returned %d\n", err);
		return 1;
	}

	printf ("%s: sam_data_store 1\n", __FUNCTION__);
	err = sam_data_store (NULL, 0);
	if (err != CS_ERR_BAD_HANDLE) {
		fprintf (stderr, "Test should return CS_ERR_BAD_HANDLE. Error returned %d\n", err);
		return 1;
	}

	printf ("%s: sam_data_restore 1\n", __FUNCTION__);
	err = sam_data_restore (saved_data, sizeof (saved_data));
	if (err != CS_ERR_BAD_HANDLE) {
		fprintf (stderr, "Test should return CS_ERR_BAD_HANDLE. Error returned %d\n", err);
		return 1;
	}

	printf ("%s: sam_initialize\n", __FUNCTION__);
	err = sam_initialize (0, SAM_RECOVERY_POLICY_RESTART);
	if (err != CS_OK) {
		fprintf (stderr, "Can't initialize SAM API. Error %d\n", err);
		return 1;
	}

	printf ("%s: sam_data_getsize 3\n", __FUNCTION__);
	err = sam_data_getsize (&size);
	if (err != CS_OK) {
		fprintf (stderr, "Test should return CS_ERR_BAD_HANDLE. Error returned %d\n", err);
		return 1;
	}
	if (size != 0) {
		fprintf (stderr, "Test should return size of 0. Returned %zx\n", size);
		return 1;
	}

	printf ("%s: sam_data_restore 2\n", __FUNCTION__);
	err = sam_data_restore (NULL, sizeof (saved_data));
	if (err != CS_ERR_INVALID_PARAM) {
		fprintf (stderr, "Test should return CS_ERR_INVALID_PARAM. Error returned %d\n", err);
		return 1;
	}

	/*
	 * Store some real data
	 */
	for (i = 0; i < sizeof (saved_data); i++) {
		saved_data[i] = (char)(i + 5);
	}

	printf ("%s: sam_data_store 2\n", __FUNCTION__);
	err = sam_data_store (saved_data, sizeof (saved_data));
	if (err != CS_OK) {
		fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
		return 1;
	}

	printf ("%s: sam_data_getsize 4\n", __FUNCTION__);
	err = sam_data_getsize (&size);
	if (err != CS_OK) {
		fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
		return 1;
	}
	if (size != sizeof (saved_data)) {
		fprintf (stderr, "Test should return size of 0. Returned %zx\n", size);
		return 1;
	}

	printf ("%s: sam_data_restore 3\n", __FUNCTION__);
	err = sam_data_restore (saved_data2, sizeof (saved_data2) - 1);
	if (err != CS_ERR_INVALID_PARAM) {
		fprintf (stderr, "Test should return CS_ERR_INVALID_PARAM. Error returned %d\n", err);
		return 1;
	}

	printf ("%s: sam_data_restore 4\n", __FUNCTION__);
	err = sam_data_restore (saved_data2, sizeof (saved_data2));
	if (err != CS_OK) {
		fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
		return 1;
	}

	if (memcmp (saved_data, saved_data2, sizeof (saved_data2)) != 0) {
		fprintf (stderr, "Retored data are not same\n");
		return 1;
	}

	memset (saved_data2, 0, sizeof (saved_data2));

	printf ("%s: sam_data_store 3\n", __FUNCTION__);
	err = sam_data_store (NULL, 1);
	if (err != CS_OK) {
		fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
		return 1;
	}

	printf ("%s: sam_data_getsize 5\n", __FUNCTION__);
	err = sam_data_getsize (&size);
	if (err != CS_OK) {
		fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
		return 1;
	}
	if (size != 0) {
		fprintf (stderr, "Test should return size of 0. Returned %zx\n", size);
		return 1;
	}

	printf ("%s: sam_data_store 4\n", __FUNCTION__);
	err = sam_data_store (saved_data, sizeof (saved_data));
	if (err != CS_OK) {
		fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
		return 1;
	}

	printf ("%s: register\n", __FUNCTION__);
	err = sam_register (&instance_id);
	if (err != CS_OK) {
		fprintf (stderr, "Can't register. Error %d\n", err);
		return 1;
	}

	if (instance_id == 1) {
		printf ("%s iid %d: sam_start\n", __FUNCTION__, instance_id);
		err = sam_start ();
		if (err != CS_OK) {
			fprintf (stderr, "Can't start hc. Error %d\n", err);
			return 1;
		}

		printf ("%s iid %d: sam_data_getsize 6\n", __FUNCTION__, instance_id);
		err = sam_data_getsize (&size);
		if (err != CS_OK) {
			fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
			return 1;
		}
		if (size != sizeof (saved_data2)) {
			fprintf (stderr, "Test should return size of 0. Returned %zx\n", size);
			return 1;
		}

		printf ("%s iid %d: sam_data_restore 5\n", __FUNCTION__, instance_id);
		err = sam_data_restore (saved_data2, sizeof (saved_data2));
		if (err != CS_OK) {
			fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
			return 1;
		}

		if (memcmp (saved_data, saved_data2, sizeof (saved_data2)) != 0) {
			fprintf (stderr, "Retored data are not same\n");
			return 1;
		}

		for (i = 0; i < sizeof (saved_data); i++) {
			saved_data[i] = (char)(i - 5);
		}

		printf ("%s iid %d: sam_data_store 5\n", __FUNCTION__, instance_id);
		err = sam_data_store (saved_data, sizeof (saved_data) - 7);
		if (err != CS_OK) {
			fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
			return 1;
		}

		exit (1);
	}

	if (instance_id == 2) {
		printf ("%s iid %d: sam_start\n", __FUNCTION__, instance_id);
		err = sam_start ();
		if (err != CS_OK) {
			fprintf (stderr, "Can't start hc. Error %d\n", err);
			return 1;
		}

		printf ("%s iid %d: sam_data_getsize 7\n", __FUNCTION__, instance_id);
		err = sam_data_getsize (&size);
		if (err != CS_OK) {
			fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
			return 1;
		}
		if (size != sizeof (saved_data2) - 7) {
			fprintf (stderr, "Test should return size of 0. Returned %zx\n", size);
			return 1;
		}

		printf ("%s iid %d: sam_data_restore 6\n", __FUNCTION__, instance_id);
		err = sam_data_restore (saved_data2, sizeof (saved_data2));
		if (err != CS_OK) {
			fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
			return 1;
		}

		for (i = 0; i < sizeof (saved_data); i++) {
			saved_data[i] = (char)(i - 5);
		}

		if (memcmp (saved_data, saved_data2, sizeof (saved_data2) - 7) != 0) {
			fprintf (stderr, "Retored data are not same\n");
			return 1;
		}

		printf ("%s iid %d: sam_data_store 6\n", __FUNCTION__, instance_id);
		err = sam_data_store (NULL, 0);
		if (err != CS_OK) {
			fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
			return 1;
		}

		exit (1);
	}

	if (instance_id == 3) {
		printf ("%s iid %d: sam_data_getsize 8\n", __FUNCTION__, instance_id);
		err = sam_data_getsize (&size);
		if (err != CS_OK) {
			fprintf (stderr, "Test should return CS_OK. Error returned %d\n", err);
			return 1;
		}
		if (size != 0) {
			fprintf (stderr, "Test should return size of 0. Returned %zx\n", size);
			return 1;
		}
	}

	return (0);
}
