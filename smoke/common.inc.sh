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


# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# !!! Script overwrites corosync.conf, authkey and qdevice/qnetd certificates !!!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

# Home https://github.com/jfriesse/csts/tree/master/smoke

# -e is really important
set -xe
set -o pipefail

# Variables changing test behavior
PREFIX="/"

COROSYNC_SYSCONFD="${PREFIX}etc/corosync"
COROSYNC_CONF="${COROSYNC_SYSCONFD}/corosync.conf"
COROSYNC_AUTHKEY="${COROSYNC_SYSCONFD}/authkey"
COROSYNC_CLUSTER_NAME="smoketestcluster"

TOKEN_TIMEOUT=1000

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

# generate_corosync_conf crypto [token] [qdevice]
# crypto can be on or off
# when token is defined it is used for token timeout
# when qdevice is set to on qdevice section is created and second node is added
generate_corosync_conf() {
    case "$1" in
    "on")
        cipher="aes256"
        hash="sha256"
        ;;
    "off")
        cipher="none"
        hash="none"
        ;;
    *)
        # Unknown crypto
        exit 1
    esac

    token=$TOKEN_TIMEOUT
    if [ ! -z "$2" ];then
        token="$2"
    fi
    qdevice="$3"
    true_command=`which true`

cat << _EOF_
    totem {
        version: 2
        cluster_name: $COROSYNC_CLUSTER_NAME
        transport: knet
        crypto_cipher: $cipher
        crypto_hash: $hash
        token: $token
    }

    logging {
        to_logfile: yes
        logfile: /var/log/cluster/corosync.log
        to_syslog: yes
    }

    quorum {
        provider: corosync_votequorum
_EOF_

    if [ "$qdevice" == "on" ];then
cat << _EOF_
        device {
            votes: 1
            model: net
            net {
                host: $LOCAL_IP
                algorithm: ffsplit
            }
            heuristics {
                mode: sync
                exec_true: $true_command
            }
        }
_EOF_
    fi

cat << _EOF_
    }

    nodelist {
        node {
            nodeid: 1
            ring0_addr: $LOCAL_IP
        }
_EOF_

    if [ "$qdevice" == "on" ];then
cat << _EOF_
        node {
            nodeid: 2
            ring0_addr: 192.0.2.2
        }
_EOF_
    fi

cat << _EOF_
    }
_EOF_
}

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

######################
# Computed variables #
######################
LOCAL_IP=$(get_ip)

##################
# C test sources #
##################

# Test sources are encoded as a base64 string and piped to base64 to store them in /tmp
