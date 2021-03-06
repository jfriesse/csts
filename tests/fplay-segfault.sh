#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that replaying randomly corrupted fdata always ends and doesn't segfault"
test_corover_undefined_enabled=false

. inc/common.sh
. inc/cpg-load.sh

fplay_app="corosync-fplay"
output_file="/var/lib/corosync/fdata"
if [ "$corosync_version" == "needle" ];then
    fplay_app="qb-blackbox"
    fplay_params="$output_file"
fi

exit_trap_end_cb() {
    run "$nodes_ip" "killall -9 $fplay_app || true"
}

# Generate valid fdata file
configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"
cpg_load_prepare "$nodes_ip"
cpg_load_start "$nodes_ip" "1000"
sleep 10
cpg_load_stop "$nodes_ip"
run "$nodes_ip" "corosync-blackbox &>/dev/null"
stop_corosync "$nodes_ip"
run "$nodes_ip" "cat /var/lib/corosync/fdata > /var/lib/corosync/fdata-correct"

# Run actual test
compile_app "$nodes_ip" "file-change-bytes"

run "$nodes_ip" "cd ~/csts-apps; \
    for ((i=0; i<1024; i++));do \
        ./file-change-bytes -i /var/lib/corosync/fdata-correct -o $output_file -n 1024; \
        $fplay_app $fplay_params &>/dev/null; \
        [ "'"$?"'" -gt 127 ] && exit 1 || true; \
    done"

run "$nodes_ip" "rm -f $output_file"

exit 0
