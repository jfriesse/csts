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

static int sam_test5(void);

int
main(void)
{
    return (sam_test5());
}

static int test5_hc_cb_count = 0;

static int test5_hc_cb (void)
{
	cs_error_t res;
	printf ("%s %d\n", __FUNCTION__, ++test5_hc_cb_count);

	res = sam_data_store (&test5_hc_cb_count, sizeof (test5_hc_cb_count));
	if (res != CS_OK)
		return 1;

	if (test5_hc_cb_count > 10)
		return 1;

	return 0;
}
/*
 * Test event driven healtchecking.
 */
static int
sam_test5 (void)
{
	cs_error_t error;
	unsigned int instance_id;
	int hc_cb_count;

	setlinebuf(stdout);
	printf ("%s: initialize\n", __FUNCTION__);
	error = sam_initialize (100, SAM_RECOVERY_POLICY_RESTART);
	if (error != CS_OK) {
		fprintf (stderr, "Can't initialize SAM API. Error %d\n", error);
		return 1;
	}
	printf ("%s: register\n", __FUNCTION__);
	error = sam_register (&instance_id);
	if (error != CS_OK) {
		fprintf (stderr, "Can't register. Error %d\n", error);
		return 1;
	}

	if (instance_id == 1) {
		printf ("%s iid %d: hc callback register\n", __FUNCTION__, instance_id);
		error = sam_hc_callback_register (test5_hc_cb);
		if (error != CS_OK) {
			fprintf (stderr, "Can't register hc cb. Error %d\n", error);
			return 1;
		}


		printf ("%s iid %d: start\n", __FUNCTION__, instance_id);
		error = sam_start ();
		if (error != CS_OK) {
			fprintf (stderr, "Can't start hc. Error %d\n", error);
			return 1;
		}

		sleep (2);

		printf ("%s iid %d: Failed. Wasn't killed.\n", __FUNCTION__, instance_id);
		return 1;
	}

	if (instance_id == 2) {
		error = sam_data_restore (&hc_cb_count, sizeof (hc_cb_count));
		if (error != CS_OK) {
			fprintf (stderr, "sam_data_restore should return CS_OK. Error returned %d\n", error);
			return 1;
		}

		if (hc_cb_count != 11) {
			fprintf (stderr, "%s iid %d: Premature killed. hc_cb_count should be 11 and it is %d\n",
				__FUNCTION__, instance_id - 1, hc_cb_count);
			return 1;

		}
		return 0;
	}

	return 1;
}
