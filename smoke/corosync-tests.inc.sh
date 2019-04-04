##################
# Test functions #
##################
test_corosync_v() {
    # Check corosync -v exits without error and
    # result version contain numbers in path format [0-9]*.[0-9]*.[0-9]
    ver_res=$(corosync -v)
    [ "$ver_res" != "${ver_res/[0-9]*\.[0-9]*\.[0-9]/}" ]
}

test_corosync_keygen() {
    rm -f "$COROSYNC_AUTHKEY"
    corosync-keygen

    # File exists
    [ -f "$COROSYNC_AUTHKEY" ]

    # ... and is not empty (wc format is "SIZE FILENAME" - remove FILENAME part)
    wc_res=$(wc -c "$COROSYNC_AUTHKEY")
    [ "${wc_res%% *}" -gt 0 ]
}

test_corosync_start() {
    generate_corosync_conf | tee "$COROSYNC_CONF"

    systemctl start corosync
}

test_corosync_stop() {
    systemctl stop corosync
}

test_corosync_quorumtool() {
    quorumtool_res_file=`mktemp`
    corosync-quorumtool -ips | tee "$quorumtool_res_file"

    # Ensure this is single node cluster
    grep -qi '^Nodes:.*1$' "$quorumtool_res_file"
    # Current node id is 1
    grep -qi '^Node ID:.*1$' "$quorumtool_res_file"
    # Is quorate (libquorum)
    grep -qi '^Quorate:.*Yes$' "$quorumtool_res_file"
    # Quorum is 1
    grep -qi '^Quorum:.*1' "$quorumtool_res_file"
    # Is quorate (libvotequorum)
    grep -qi '^Flags:.*Quorate' "$quorumtool_res_file"

    rm -f "$quorumtool_res_file"
}

test_corosync_cmapctl() {
    cmapctl_res_file=`mktemp`
    corosync-cmapctl | tee "$cmapctl_res_file"
    # cluster_name key exists in full output
    grep -qi '^totem.cluster_name (str) = smoketestcluster$' "$cmapctl_res_file"

    corosync-cmapctl "totem." | tee "$cmapctl_res_file"
    # cluster_name key exists in reduced output
    grep -qi '^totem.cluster_name (str) = smoketestcluster$' "$cmapctl_res_file"

    # cluster_name key should be returned
    corosync-cmapctl -g "totem.cluster_name" | tee "$cmapctl_res_file"
    grep -qi '^totem.cluster_name (str) = smoketestcluster$' "$cmapctl_res_file"

    rm -f "$cmapctl_res_file"
}

########
# main #
########
test_corosync_v
test_corosync_keygen
test_corosync_start
test_corosync_quorumtool
test_corosync_cmapctl
test_corosync_stop
