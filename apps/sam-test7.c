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
#include <corosync/cmap.h>

static int sam_test7(void);

int
main(void)
{
    return (sam_test7());
}

static void *test7_thread (void *arg)
{
	/* Wait 5s */
	sleep (5);
	exit (0);
}

/*
 * Test quorum
 */
static int
sam_test7 (void) {
	cmap_handle_t cmap_handle;
	cs_error_t err;
	unsigned int instance_id;
	pthread_t kill_thread;
	char *str;
	uint32_t expected_votes;

	setlinebuf(stdout);
	err = cmap_initialize (&cmap_handle);
	if (err != CS_OK) {
		printf ("Could not initialize Cluster Map API instance error %d. Test skipped\n", err);
		return (1);
	}


	if (cmap_get_string(cmap_handle, "quorum.provider", &str) != CS_OK) {
		printf ("Could not get \"provider\" key: %d. Test skipped\n", err);
		return (1);
	}
        if (strcmp(str, "corosync_votequorum") != 0) {
		printf ("Provider is not corosync_votequorum. Test skipped\n");
		return (1);
        }
	free(str);

	if (cmap_get_uint32(cmap_handle, "quorum.expected_votes", &expected_votes) != CS_OK) {
		printf ("Could not get \"expected_votes\" key: %d. Test skipped\n", err);
		return (1);
	}

	if (expected_votes != 1) {
		printf ("Expected_votes is not 1. Test skipped\n");
		return (1);
	}

	/*
	 * Set to not quorate
	 */
	err = cmap_set_uint32(cmap_handle, "quorum.expected_votes", 2);
	if (err != CS_OK) {
		printf ("Can't set map key. Error %d\n", err);
		return (2);
	}

	printf ("%s: initialize\n", __FUNCTION__);
	err = sam_initialize (2000, SAM_RECOVERY_POLICY_QUORUM_RESTART);
	if (err != CS_OK) {
		fprintf (stderr, "Can't initialize SAM API. Error %d\n", err);
		return 2;
	}

	printf ("%s: register\n", __FUNCTION__);
	err = sam_register (&instance_id);
	if (err != CS_OK) {
		fprintf (stderr, "Can't register. Error %d\n", err);
		return 2;
	}

	if (instance_id == 1) {
		/*
		 * Sam start should block forever, but 10s for us should be enough
		 */
		pthread_create (&kill_thread, NULL, test7_thread, NULL);

		printf ("%s iid %d: start - should block forever (waiting 5s)\n", __FUNCTION__, instance_id);
		err = sam_start ();
		if (err != CS_OK) {
			fprintf (stderr, "Can't start hc. Error %d\n", err);
			return 2;
		}

		printf ("%s iid %d: wasn't killed\n", __FUNCTION__, instance_id);
		return (2);
	}

	if (instance_id == 2) {
		/*
		 * Set to quorate
		 */
		err = cmap_set_uint32(cmap_handle, "quorum.expected_votes", 1);
		if (err != CS_OK) {
			printf ("Can't set map key. Error %d\n", err);
			return (2);
		}

		printf ("%s iid %d: start\n", __FUNCTION__, instance_id);
		err = sam_start ();
		if (err != CS_OK) {
			fprintf (stderr, "Can't start hc. Error %d\n", err);
			return 2;
		}

		/*
		 * Set corosync unquorate
		 */
		err = cmap_set_uint32(cmap_handle, "quorum.expected_votes", 2);
		if (err != CS_OK) {
			printf ("Can't set map key. Error %d\n", err);
			return (2);
		}

		printf ("%s iid %d: sleep 3\n", __FUNCTION__, instance_id);
		sleep (3);

		printf ("%s iid %d: wasn't killed\n", __FUNCTION__, instance_id);
		return (2);
	}

	if (instance_id == 3) {
		return (0);
	}

	return (2);
}
