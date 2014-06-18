#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

# It's expected to have #token and #consensus in config file.
generate_corosync_conf_cb() {
    sed 's/^[ \t]*debug: .*$/debug: on/' | \
        sed '/version:/a\\t#token: 1000\n\t#consensus: 1200\n\t#token_coefficient: 650'
}

receive_current_token_timeout() {
    cat_corosync_log "$nodes_ip" | grep "Token Timeout ([0-9]* ms)" | tail -1 |
      sed 's/^.*Token Timeout (\([0-9]*\) ms).*$/\1/'
}

receive_current_consensus_timeout() {
    cat_corosync_log "$nodes_ip" | grep "consensus ([0-9]* ms)" | tail -1 |
      sed 's/^.*consensus (\([0-9]*\) ms).*$/\1/'
}

# change_tc_value item value cmap|config
# Change token or consensus value ether by using cmap or config.
# No reload is called.
# value can be ether value (number), empty (no change) or del (means value is deleted)
# item can be anything totem.$item, but tested only for totem.token, totem.consensus and
# totem.token_coefficient
change_tc_value() {
    local item="$1"
    local value="$2"
    local method="$3"

    case "$method" in
    "cmap")
        if [ "$value" == "del" ];then
            cmap_del "$nodes_ip" "totem.$item" || return $?
        fi

	if [ "$value" != "" ] && [ "$value" != "del" ];then
            cmap_set "$nodes_ip" "totem.$item" "u32" "$value" || return $?
	fi

        true

        ;;
    "config")
        if [ "$value" == "del" ];then
            tmp_file=`mktemp`
            get_current_corosync_config "$nodes_ip" > "$tmp_file"
            sed -i 's/\t'$item':/\t#'$item':/' "$tmp_file"
            cat "$tmp_file" | store_corosync_config "$nodes_ip"
            rm -f "$tmp_file"
        fi

	if [ "$value" != "" ] && [ "$value" != "del" ];then
            tmp_file=`mktemp`
            get_current_corosync_config "$nodes_ip" > "$tmp_file"
            sed -i 's/#'$item':/'$item':/' "$tmp_file"
            sed -i 's/'$item':.*$/'$item': '$value'/' "$tmp_file"
            cat "$tmp_file" | store_corosync_config "$nodes_ip"
            rm -f "$tmp_file"
	fi

        true
        ;;
    *)
        echo "Unknown method $method"

        false
    esac

}

# change_token_and_consensus token consensus cmap|config
# token and consensus can be ether value (number), empty (no change)
# or del (means value is deleted)
# Reload is called for config change
change_token_and_consensus() {
    local token="$1"
    local consensus="$2"
    local method="$3"

    change_tc_value "token" "$token" "$method" || return $?
    change_tc_value "consensus" "$consensus" "$method" || return $?

    if [ "$method" == "config" ];then
        run "$nodes_ip" "corosync-cfgtool -R"
    fi
}
