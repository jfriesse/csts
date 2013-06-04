#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that sending INT signal before full start results in exit"
test_max_runtime=60

. inc/common.sh

exit_trap_start_cb() {
    run "$nodes_ip" "rm -f /tmp/corosync-conf-fifo"
    run "$nodes_ip" "killall -9 corosync || true; rm -f /var/run/corosync.pid"
}

configure_corosync "$nodes_ip"

run "$nodes_ip" "rm -f /tmp/corosync-conf-fifo; mkfifo /tmp/corosync-conf-fifo"

COROSYNC_MAIN_CONFIG_FILE=/tmp/corosync-conf-fifo run "$nodes_ip" "COROSYNC_MAIN_CONFIG_FILE=/tmp/corosync-conf-fifo corosync" &
pid="$!"

# Wait until corosync is started
while ! run "$nodes_ip" "pgrep corosync" >/dev/null;do
    sleep 1
done

# Send INT signal BEFORE reading config
run "$nodes_ip" "killall -INT corosync"

# Make sure corosync is still there
sleep 1
run "$nodes_ip" "pgrep corosync" >/dev/null || exit 0

# Feed config file
run "$nodes_ip" "cat /etc/corosync/corosync.conf > /tmp/corosync-conf-fifo"

# Finish initialization of corosync
wait $pid

# Wait for exit
no_retries=0
while run "$nodes_ip" "pgrep corosync" >/dev/null && [ $no_retries -lt 20 ];do
    sleep 1
    no_retries=$(($no_retries + 1))
done

[ "$no_retries" -ge 20 ] && exit 1 || true

exit 0
