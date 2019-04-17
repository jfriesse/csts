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

# generate_corosync_conf crypto [token]
# crypto can be on or off
# when token is defined it is used for token timeout
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

cat << _EOF_
    totem {
        version: 2
        cluster_name: smoketestcluster
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
