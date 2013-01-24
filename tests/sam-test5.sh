#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - event driven healtchecking"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

compile_app "$nodes_ip" "sam-test5" "-lsam"
run_app "$nodes_ip" 'sam-test5'

exit 0
