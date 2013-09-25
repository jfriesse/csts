#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that full /dev/shm doesn't cause segfault on corosync exec"

. inc/common.sh

exit_trap_end_cb() {
    if run "$nodes_ip" mount | grep "$test_var_dir/devshm.img";then
	run "$nodes_ip" umount "$test_var_dir/devshm.img"
    fi
}

configure_corosync "$nodes_ip"
# Create /dev/shm loop image
run "$nodes_ip" dd if=/dev/zero of="$test_var_dir/devshm.img" bs=512 count=1024
run "$nodes_ip" mkfs.ext2 -F "$test_var_dir/devshm.img"
run "$nodes_ip" mount "$test_var_dir/devshm.img" "/dev/shm" -o loop

# Corosync will never start correctly (probe_command need connection)
start_corosync "$nodes_ip" && res=$? || res=$?
[ "$res" -gt 127 ] && exit 1 || true

exit 0
