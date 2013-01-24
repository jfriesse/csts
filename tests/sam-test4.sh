#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - sam_data_store, sam_data_restore and sam_data_getsize"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

compile_app "$nodes_ip" "sam-test4" "-lsam"
run_app "$nodes_ip" 'sam-test4'

exit 0
