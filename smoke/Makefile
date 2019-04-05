all: corosync-runtest.sh

corosync-runtest.sh: common.inc.sh corosync-tests.inc.sh
	cat common.inc.sh > $@
	cat corosync-tests.inc.sh >> $@

clean:
	rm -f corosync-runtest.sh
