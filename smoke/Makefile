all: corosync-runtest.sh

corosync-api-test: corosync-api-test.c
	$(CC) -ggdb -Wall `pkg-config --cflags libcpg` $< `pkg-config --libs libcpg` -o $@

corosync-runtest.sh: common.inc.sh corosync-tests.inc.sh corosync-api-test
	cat common.inc.sh > $@

	echo "# corosync-api-test.c" >> $@
	echo "echo \"`base64 corosync-api-test.c`\" | base64 -d | tee \"/tmp/corosync-api-test.c\"" >> $@
	echo "# - corosync-api-test.c" >> $@

	cat corosync-tests.inc.sh >> $@

clean:
	rm -f corosync-runtest.sh corosync-api-test
