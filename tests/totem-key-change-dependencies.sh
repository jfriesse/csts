#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that totem key changes properly handles dependencies"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh
. inc/token-set.sh

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

for mode in cmap config cmap config;do
    # First make sure that receive of token and consensus really works
    # and test token setting
    change_token_and_consensus "1000" "2000" $mode
    consensus=`receive_current_consensus_timeout`
    [ "$consensus" == "2000" ]
    token=`receive_current_token_timeout`
    [ "$token" == "1000" ]

    # Put consensus to original state
    change_token_and_consensus "" "del" $mode
    consensus=`receive_current_consensus_timeout`
    [ "$consensus" != "2000" ] && [ "$consensus" != "" ]

    # Change token timeout. Consensus should be recomputed
    change_token_and_consensus "2000" "" $mode
    token_new=`receive_current_token_timeout`
    consensus_new=`receive_current_consensus_timeout`
    [ "$token_new" == "2000" ]
    [ "$consensus_new" != "$consensus" ]

    # Set consensus timeout and then token. Consensus must not be recomputed
    change_token_and_consensus "" "2000" $mode
    change_token_and_consensus "1000" "" $mode
    token_new=`receive_current_token_timeout`
    consensus_new=`receive_current_consensus_timeout`
    [ "$token_new" == "1000" ]
    [ "$consensus_new" == "2000" ]

    # Now delete consensus and consensus_new should be recomputed
    change_token_and_consensus "" "del" $mode
    consensus_new=`receive_current_consensus_timeout`
    [ "$consensus_new" == "$consensus" ]

    # Now reset token and consensus
    change_token_and_consensus "1000" "del" $mode
    token_new=`receive_current_token_timeout`
    consensus_new=`receive_current_consensus_timeout`
    [ "$token_new" == "1000" ]
    [ "$consensus_new" == "$consensus" ]
done

stop_corosync "$nodes_ip"

exit 0
