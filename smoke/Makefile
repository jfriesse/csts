C_APPS = corosync-api-test corosync-api-test-mp

all: corosync-runtest.sh corosync-qdevice-runtest.sh

corosync-api-test: corosync-api-test.c
	$(CC) -ggdb -Wall `pkg-config --cflags libcpg` $< `pkg-config --libs libcpg` -o $@

corosync-api-test-mp: corosync-api-test-mp.c
	$(CC) -ggdb -Wall `pkg-config --cflags libcpg` $< `pkg-config --libs libcpg` -o $@

corosync-runtest.sh: common.inc.sh corosync-tests.inc.sh $(C_APPS)
	cat common.inc.sh > $@

	# Store C files as a base64 encoded string piped via base64 -d for storing decoded file into /tmp
	for f in $(C_APPS);do \
	    echo "# $$f.c" >> $@; \
	    echo "echo \"`base64 $$f.c`\" | base64 -d | tee \"/tmp/$$f.c\"" >> $@; \
	    echo "# - $$f.c" >> $@; \
	done

	cat corosync-tests.inc.sh >> $@

corosync-qdevice-runtest.sh: common.inc.sh corosync-qdevice-tests.inc.sh
	cat common.inc.sh corosync-qdevice-tests.inc.sh > $@

clean:
	rm -f corosync-runtest.sh corosync-api-test corosync-api-test-mp
