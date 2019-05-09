#!/bin/bash

# Copyright (c) 2019, Red Hat, Inc.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND RED HAT, INC. DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL RED HAT, INC. BE LIABLE
# FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# Author: Jan Friesse <jfriesse@redhat.com>

# Home https://github.com/jfriesse/csts/tree/master/smoke

# -e is really important
set -xe
set -o pipefail

# Variables changing test behavior
MAX_REPEATS=60

# Start of the test (for journalctl)
JOURNAL_DATE_SINCE=$(date +"%F %T")

####################
# Helper functions #
####################

# service_start service
service_start() {
    # service service must be inactive
    systemctl is-active "$1" && exit 1 || true

    systemctl start "$1"

    systemctl is-active "$1"
}

# service_stop service
service_stop() {
    systemctl is-active "$1" || exit 1

    systemctl stop "$1"

    systemctl is-active "$1" && exit 1 || true
}

# wait_for_log_msg message
wait_for_log_msg() {
    cont=true
    repeats=0

    journalctl --since "$JOURNAL_DATE_SINCE" | cat

    while $cont;do
        if journalctl _SYSTEMD_UNIT=spausedd.service -o cat --since "$JOURNAL_DATE_SINCE" | grep "$1";then
            cont=false
        else
            sleep 1
            repeats=$((repeats+1))
            [ "$repeats" -le "$MAX_REPEATS" ]
        fi
    done
}

##################
# Test functions #
##################

test_spausedd_h() {
    # Check that spausedd binary exists and -h returns help text
    res=`spausedd -h || true`
    [ "$res" != "${res/usage/}" ]
}

test_spausedd_start() {
    service_start "spausedd"

    wait_for_log_msg 'Running main poll loop with maximum timeout .* and steal threshold .*%'
}

test_spausedd_stop() {
    service_stop "spausedd"

    wait_for_log_msg 'During .*s runtime spausedd was .*x not scheduled on time'
}

test_sig_stop() {
    spausedd_pid=$(systemctl show spausedd -p "MainPID")
    spausedd_pid=${spausedd_pid##*=}

    kill -STOP "$spausedd_pid"
    sleep 5
    kill -CONT "$spausedd_pid"

    wait_for_log_msg 'Not scheduled for .*s (threshold is .*s), steal time is '
}

########
# main #
########
test_spausedd_h

test_spausedd_start

test_sig_stop

test_spausedd_stop
