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

####################
# Helper functions #
####################
get_ip() {
    ip_res=$(ip route get 8.8.8.8)
    # Format is "8.8.8.8 via ROUTE_IPADDR dev DEV src IPADDR uid NUMBER"
    # Remove everything up to "src " and then everything after " "
    addr=${ip_res##*src }
    addr=${addr%% *}

    echo "$addr"
}

##################
# Test functions #
##################

test_omping_v() {
    # Check that omping -V prints version
    ver_res=`omping -V`
    [ "$ver_res" != "${ver_res/[0-9]*\.[0-9]*\.[0-9]/}" ]

    # Check that omping exists and prints usage
    res=`omping || true`
    [ "$res" != "${res/usage/}" ]
}

test_man_page() {
    man -w "omping"
}

test_local_ping() {
    omping_res_file=`mktemp`
    local_ip=$(get_ip)

    omping -c 5 "$local_ip" | tee "$omping_res_file"

    grep "$local_ip.*joined (S,G)" "$omping_res_file"
    grep "$local_ip.*given amount of query messages was sent" "$omping_res_file"
    grep "$local_ip.*unicast, xmt/rcv/%loss =.*min/avg/max/std-dev"  "$omping_res_file"
    grep "$local_ip.*multicast, xmt/rcv/%loss =.*min/avg/max/std-dev" "$omping_res_file"

    rm -f "$omping_res_file"
}

########
# main #
########
test_omping_v
test_man_page

test_local_ping
