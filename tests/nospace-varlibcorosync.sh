#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that full /var/lib/corosync doesn't cause problem"
test_corover_undefined_enabled=false

. inc/common.sh
. inc/cpg-load.sh

nospace_image_file_size=1000
nospace_image_mount_point="/var/lib/corosync"

. inc/nospace.sh


configure_corosync "$nodes_ip"

nospace_init "$nodes_ip"

start_corosync "$nodes_ip"

# Test that corosync works when started with enough space to store ringid
cpg_load_prepare "$nodes_ip"
cpg_load_one_shot "$nodes_ip" "1000"

# ... even after fill
nospace_fill "$nodes_ip"
cpg_load_one_shot "$nodes_ip" "1000"

# Blackbox shouldn't work
if [ "$corosync_version" == "needle" ];then
    run "$nodes_ip" corosync-blackbox && true || true
fi

if [ "$corosync_version" == "flatiron" ];then
    run "$nodes_ip" corosync-blackbox && exit 1 || true
fi

cat_corosync_log "$nodes_ip" | grep "Can't store blackbox file:"

stop_corosync "$nodes_ip"

# Now fill again and try start
nospace_fini "$nodes_ip"
nospace_init "$nodes_ip"

nospace_fill "$nodes_ip"

# Start should fail
start_corosync "$nodes_ip" && exit 1 || true

run "$nodes_ip" "killall -9 corosync || true; rm -f /var/run/corosync.pid"

exit 0
