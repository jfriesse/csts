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

static int sam_test8(pid_t pid, pid_t old_pid, int test_n);

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
		err = sam_test8 (getpid (), 0, 1);
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
			err = sam_test8 (getpid (), old_pid, 2);
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
				err = sam_test8 (old_pid, 0, 3);
				sam_finalize ();
				return (err);
			}

			waitpid (pid, &stat, 0);
		}
	}

	return (WEXITSTATUS (stat));
}

/*
 * Test cmap integration + quit policy
 */
static int
sam_test8 (pid_t pid, pid_t old_pid, int test_n) {
	cmap_handle_t cmap_handle;
	cs_error_t err;
	uint64_t tstamp1, tstamp2;
	int32_t msec_diff;
	unsigned int instance_id;
	char key_name[CMAP_KEYNAME_MAXLEN];
	char *str;

	err = cmap_initialize (&cmap_handle);
	if (err != CS_OK) {
		printf ("Could not initialize Cluster Map API instance error %d. Test skipped\n", err);
		return (1);
	}

	printf ("%s test %d\n", __FUNCTION__, test_n);

	if (test_n == 2) {
		/*
		 * Object should not exist
		 */
		printf ("%s Testing if object exists (it shouldn't)\n", __FUNCTION__);

		snprintf(key_name, CMAP_KEYNAME_MAXLEN, "resources.process.%d.state", pid);
		err = cmap_get_string(cmap_handle, key_name, &str);
		if (err == CS_OK) {
			printf ("Could find key \"%s\": %d.\n", key_name, err);
			free(str);
			return (2);
		}
	}

	if (test_n == 1 || test_n == 2) {
		printf ("%s: initialize\n", __FUNCTION__);
		err = sam_initialize (2000, SAM_RECOVERY_POLICY_QUIT | SAM_RECOVERY_POLICY_CMAP);
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

		snprintf(key_name, CMAP_KEYNAME_MAXLEN, "resources.process.%d.recovery", pid);
		err = cmap_get_string(cmap_handle, key_name, &str);
		if (err != CS_OK) {
			printf ("Could not get \"recovery\" key: %d.\n", err);
			return (2);
		}

		if (strcmp(str, "quit") != 0) {
			printf ("Recovery key \"%s\" is not \"quit\".\n", key_name);
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

		printf ("%s iid %d: stop\n", __FUNCTION__, instance_id);
		err = sam_stop ();
		if (err != CS_OK) {
			fprintf (stderr, "Can't stop hc. Error %d\n", err);
			return 2;
		}

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

		printf ("%s iid %d: sleeping 5\n", __FUNCTION__, instance_id);
		sleep (5);

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

		printf ("%s iid %d: start 2\n", __FUNCTION__, instance_id);
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

		if (test_n == 2) {
			printf ("%s iid %d: sleeping 5. Should be killed\n", __FUNCTION__, instance_id);
			sleep (5);

			return (2);
		} else {
			printf ("%s iid %d: Test HC\n", __FUNCTION__, instance_id);
			err = sam_hc_send ();
			if (err != CS_OK) {
				fprintf (stderr, "Can't send hc. Error %d\n", err);
				return 2;
			}

			snprintf(key_name, CMAP_KEYNAME_MAXLEN, "resources.process.%d.last_updated", pid);
			err = cmap_get_uint64(cmap_handle, key_name, &tstamp1);
			if (err != CS_OK) {
				printf ("Could not get \"last_updated\" key: %d.\n", err);
				return (2);
			}
			printf ("%s iid %d: Sleep 1\n", __FUNCTION__, instance_id);
			sleep (1);
			err = sam_hc_send ();
			if (err != CS_OK) {
				fprintf (stderr, "Can't send hc. Error %d\n", err);
				return 2;
			}
			sleep (1);
			err = cmap_get_uint64(cmap_handle, key_name, &tstamp2);
			if (err != CS_OK) {
				printf ("Could not get \"last_updated\" key: %d.\n", err);
				return (2);
			}
			msec_diff = (tstamp2 - tstamp1)/CS_TIME_NS_IN_MSEC;

			if (msec_diff < 500 || msec_diff > 2000) {
				printf ("Difference %d is not within <500, 2000> interval.\n", msec_diff);
				return (2);
			}

			printf ("%s iid %d: stop 2\n", __FUNCTION__, instance_id);
			err = sam_stop ();
			if (err != CS_OK) {
				fprintf (stderr, "Can't stop hc. Error %d\n", err);
				return 2;
			}

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

			printf ("%s iid %d: exiting\n", __FUNCTION__, instance_id);
			return (0);
		}
	}

	if (test_n == 3) {
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

		return (0);
	}

	return (2);
}
