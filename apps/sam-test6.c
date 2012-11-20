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

static int sam_test6(void);

int
main(void)
{
    return (sam_test6());
}

static int test6_sig_delivered = 0;

static void test6_signal (int sig) {
	cs_error_t error;

	printf ("%s\n", __FUNCTION__);
	test6_sig_delivered++;

	if ((error = sam_data_store (&test6_sig_delivered, sizeof (test6_sig_delivered))) != CS_OK) {
		fprintf (stderr, "Can't store data! Error : %d\n", error);
	}
}

/*
 * Test warn signal set.
 */
static int
sam_test6 (void) {
	cs_error_t error;
	unsigned int instance_id;
	int test6_sig_del;

	setlinebuf(stdout);
	printf ("%s: initialize\n", __FUNCTION__);
	error = sam_initialize (2000, SAM_RECOVERY_POLICY_RESTART);
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
		error = sam_warn_signal_set (SIGUSR1);
		if (error != CS_OK) {
			fprintf (stderr, "Can't set warn signal. Error %d\n", error);
			return 1;
		}

		signal (SIGUSR1, test6_signal);

		printf ("%s iid %d: start\n", __FUNCTION__, instance_id);
		error = sam_start ();
		if (error != CS_OK) {
			fprintf (stderr, "Can't start hc. Error %d\n", error);
			return 1;
		}

		printf ("%s iid %d: sleep 1\n", __FUNCTION__, instance_id);
		sleep (1);

		printf ("%s iid %d: hc send\n", __FUNCTION__, instance_id);
		error = sam_hc_send ();
		if (error != CS_OK) {
			fprintf (stderr, "Can't send hc. Error %d\n", error);
			return 1;
		}


		printf ("%s iid %d: wait for delivery of signal\n", __FUNCTION__, instance_id);
		while (!test6_sig_delivered) {
			sleep (1);
		}

		printf ("%s iid %d: wait for real kill\n", __FUNCTION__, instance_id);

		sleep (3);

		printf ("%s iid %d: wasn't killed\n", __FUNCTION__, instance_id);
		return (1);
	}

	if (instance_id == 2) {
		error = sam_data_restore (&test6_sig_del, sizeof (test6_sig_del));
		if (error != CS_OK) {
			fprintf (stderr, "Can't restore data. Error %d\n", error);
			return 1;
		}

		if (test6_sig_del != 1) {
			fprintf (stderr, "Previous test failed. Signal was not delivered\n");
			return 1;
		}

		error = sam_warn_signal_set (SIGKILL);
		if (error != CS_OK) {
			fprintf (stderr, "Can't set warn signal. Error %d\n", error);
			return 1;
		}

		signal (SIGUSR1, test6_signal);

		printf ("%s iid %d: start\n", __FUNCTION__, instance_id);
		error = sam_start ();
		if (error != CS_OK) {
			fprintf (stderr, "Can't start hc. Error %d\n", error);
			return 1;
		}

		printf ("%s iid %d: sleep 1\n", __FUNCTION__, instance_id);
		sleep (1);

		printf ("%s iid %d: hc send\n", __FUNCTION__, instance_id);
		error = sam_hc_send ();
		if (error != CS_OK) {
			fprintf (stderr, "Can't send hc. Error %d\n", error);
			return 1;
		}


		printf ("%s iid %d: wait for delivery of signal\n", __FUNCTION__, instance_id);
		while (!test6_sig_delivered) {
			sleep (1);
		}

		printf ("%s iid %d: wasn't killed\n", __FUNCTION__, instance_id);
		return (1);
	}

	if (instance_id == 3) {
		error = sam_data_restore (&test6_sig_del, sizeof (test6_sig_del));
		if (error != CS_OK) {
			fprintf (stderr, "Can't restore data. Error %d\n", error);
			return 1;
		}

		if (test6_sig_del != 1) {
			fprintf (stderr, "Previous test failed. Signal WAS delivered\n");
			return 1;
		}

		return (0);
	}

	return 1;
}
