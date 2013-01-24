#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - warn signal set"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

compile_app "$nodes_ip" "sam-test6" "-lsam"
run_app "$nodes_ip" 'sam-test6'

exit 0
