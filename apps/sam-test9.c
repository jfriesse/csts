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

static int sam_test9(pid_t pid, pid_t old_pid, int test_n);

int
main(void)
{
        pid_t pid, old_pid;
        int err;
        int stat;

        setlinebuf(stdout);
	pid = fork ();

	if (pid == -1) {
		fprintf (stderr, "Can't fork\n");
		return 2;
	}

	if (pid == 0) {
		err = sam_test9 (getpid (), 0, 1);
		sam_finalize ();
		return (err);
	}

	waitpid (pid, &stat, 0);
	old_pid = pid;

	if (WEXITSTATUS (stat) == 0) {
		pid = fork ();

		if (pid == -1) {
			fprintf (stderr, "Can't fork\n");
			return 2;
		}

		if (pid == 0) {
			err = sam_test9 (old_pid, 0, 2);
			sam_finalize ();
			return (err);
		}

		waitpid (pid, &stat, 0);
	}

	return (WEXITSTATUS (stat));
}

/*
 * Test cmap integration + restart policy
 */
static int
sam_test9 (pid_t pid, pid_t old_pid, int test_n) {
	cs_error_t err;
	cmap_handle_t cmap_handle;
	unsigned int instance_id;
	char *str;
	char key_name[CMAP_KEYNAME_MAXLEN];

	err = cmap_initialize (&cmap_handle);
	if (err != CS_OK) {
		printf ("Could not initialize Cluster Map API instance error %d. Test skipped\n", err);
		return (1);
	}

	printf ("%s test %d\n", __FUNCTION__, test_n);

	if (test_n == 1) {
		printf ("%s: initialize\n", __FUNCTION__);
		err = sam_initialize (2000, SAM_RECOVERY_POLICY_RESTART | SAM_RECOVERY_POLICY_CMAP);
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
		printf ("%s: iid %d\n", __FUNCTION__, instance_id);

		if (instance_id < 3) {
			snprintf(key_name, CMAP_KEYNAME_MAXLEN, "resources.process.%d.recovery", pid);
			err = cmap_get_string(cmap_handle, key_name, &str);
			if (err != CS_OK) {
				printf ("Could not get \"recovery\" key: %d.\n", err);
				return (2);
			}

			if (strcmp(str, "restart") != 0) {
				printf ("Recovery key \"%s\" is not \"restart\".\n", str);
				return (2);
			}
			free(str);

			snprintf(key_name, CMAP_KEYNAME_MAXLEN, "resources.process.%d.state", pid);
			err = cmap_get_string(cmap_handle, key_name, &str);
			if (err != CS_OK) {
				printf ("Could not get \"state\" key: %d.\n", err);
				return (2);
			}

			if (strcmp(str, "stopped") != 0) {
				printf ("State key is not \"stopped\".\n");
				return (2);
			}
			free(str);

			printf ("%s iid %d: start\n", __FUNCTION__, instance_id);
			err = sam_start ();
			if (err != CS_OK) {
				fprintf (stderr, "Can't start hc. Error %d\n", err);
				return 2;
			}

			err = cmap_get_string(cmap_handle, key_name, &str);
			if (err != CS_OK) {
				printf ("Could not get \"state\" key: %d.\n", err);
				return (2);
			}

			if (strcmp(str, "running") != 0) {
				printf ("State key is not \"running\".\n");
				return (2);
			}
			free(str);

			printf ("%s iid %d: waiting for kill\n", __FUNCTION__, instance_id);
			sleep (10);

			return (2);
		}

		if (instance_id == 3) {
			printf ("%s iid %d: mark failed\n", __FUNCTION__, instance_id);
			if (err != CS_OK) {
				fprintf (stderr, "Can't start hc. Error %d\n", err);
				return 2;
			}
			err = sam_mark_failed ();
			if (err != CS_OK) {
				fprintf (stderr, "Can't mark failed. Error %d\n", err);
				return 2;
			}

			sleep (10);

			return (2);
		}

		return (2);
	}

	if (test_n == 2) {
		printf ("%s Testing if status is failed\n", __FUNCTION__);

		/*
		 * Previous should be FAILED
		 */
		snprintf(key_name, CMAP_KEYNAME_MAXLEN, "resources.process.%d.state", pid);
		err = cmap_get_string(cmap_handle, key_name, &str);
		if (err != CS_OK) {
			printf ("Could not get \"state\" key: %d.\n", err);
			return (2);
		}

		if (strcmp(str, "failed") != 0) {
			printf ("State key is not \"failed\".\n");
			return (2);
		}
		free(str);

		return (0);
	}

	return (2);
}
