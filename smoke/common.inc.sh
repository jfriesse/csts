#!/bin/bash

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# !!! Script overwrites corosync.conf and authkey !!!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

# Home https://github.com/jfriesse/csts/tree/master/smoke

set -xe
set -o pipefail

# Variables changing test behavior
PREFIX="/"

COROSYNC_SYSCONFD="${PREFIX}etc/corosync"
COROSYNC_CONF="${COROSYNC_SYSCONFD}/corosync.conf"
COROSYNC_AUTHKEY="${COROSYNC_SYSCONFD}/authkey"

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

# generate_corosync_conf crypto
# crypto can be on or off
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

cat << _EOF_
    totem {
        version: 2
        cluster_name: smoketestcluster
        transport: knet
        crypto_cipher: $cipher
        crypto_hash: $hash
    }

    logging {
        to_logfile: yes
        logfile: /var/log/cluster/corosync.log
        to_syslog: yes
    }

    quorum {
        provider: corosync_votequorum
    }

    nodelist {
        node {
            nodeid: 1
            ring0_addr: $LOCAL_IP
        }
    }
_EOF_
}

######################
# Computed variables #
######################
LOCAL_IP=$(get_ip)

##################
# C test sources #
##################
