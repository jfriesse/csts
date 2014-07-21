#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test corosync-keygen"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

auth_key_file="/etc/corosync/authkey"
auth_key_file_backup="$test_var_dir/authkey.orig"
auth_key_file_new="$test_var_dir/authkey.new"

exit_trap_end_cb() {
    if run "$nodes_ip" "[ -f $auth_key_file_backup ]";then
        run "$nodes_ip" "mv $auth_key_file_backup $auth_key_file"
    fi
}

# Backup old authkey
run "$nodes_ip" "rm -f $auth_key_file_backup"
if run "$nodes_ip" "[ -f $auth_key_file ]";then
    run "$nodes_ip" "cp $auth_key_file $auth_key_file_backup"
fi

# Try to create authkey
run "$nodes_ip" "corosync-keygen -l"
# Key should exists
run "$nodes_ip" "[ -f $auth_key_file ]"
run "$nodes_ip" "rm -f $auth_key_file"

# Try to create authkey with given name
run "$nodes_ip" "corosync-keygen -l -k $auth_key_file_new"
# Key should exists
run "$nodes_ip" "[ -f $auth_key_file_new ]"
run "$nodes_ip" "rm -f $auth_key_file_new"

# Try to write to unwritable (nonexisting) directory
run "$nodes_ip" "corosync-keygen -l -k /nonexistingdir/authkey.new" && exit 1 || true

exit 0
