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

# test_corosync_start crypto
# crypto can be on or off
test_corosync_start() {
    generate_corosync_conf "$1" | tee "$COROSYNC_CONF"

    systemctl start corosync
}

test_corosync_stop() {
    systemctl stop corosync
}

test_corosync_quorumtool() {
    quorumtool_res_file=`mktemp`
    # This is already fixed in upstream db38e3958c4f88d5d06e8f7c83d6d90334d9fbd2
    (corosync-quorumtool -ips || true) | tee "$quorumtool_res_file"

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

test_corosync_api() {
	cflags=$(pkg-config --cflags libcpg)
	libs=$(pkg-config --libs libcpg)

	gcc -ggdb -Wall $cflags "/tmp/corosync-api-test.c" \
	    $libs -o "/tmp/corosync-api-test"

	/tmp/corosync-api-test
}

########
# main #
########
if [ -z "$PREFIX" ];then
    echo "PREFIX not defined. Do not run *.inc.sh directly"
    exit 1
fi

test_corosync_v
test_corosync_keygen

for crypto in "off" "on";do
    test_corosync_start "$crypto"
    test_corosync_quorumtool
    test_corosync_cmapctl
    test_corosync_api
    test_corosync_stop
done
