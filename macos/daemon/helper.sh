#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
#

set -e -o pipefail
shopt -s extglob
export LC_ALL=C

SELF="${BASH_SOURCE[0]}"
[[ $SELF == */* ]] || SELF="./$SELF"
SELF="$(cd "${SELF%/*}" && pwd -P)/${SELF##*/}"
export PATH="/usr/bin:/bin:/usr/sbin:/sbin:${SELF%/*}:$PATH"

WG_CONFIG=""
INTERFACE=""
ADDRESSES=( )
MTU=""
DNS=""
TABLE=""
CONFIG_FILE=""
PROGRAM="${0##*/}"
ARGS=( "$@" )

cmd() {
	echo "[#] $*" >&2
	"$@"
}

die() {
	echo "$PROGRAM: $*" >&2
	exit 1
}

[[ ${BASH_VERSINFO[0]} -ge 3 ]] || die "Version mismatch: bash ${BASH_VERSINFO[0]} detected, when bash 3+ required"

parse_options() {
	local interface_section=0 line key value stripped path v
	CONFIG_FILE="$1"
	[[ -e $CONFIG_FILE ]] || die "\`$CONFIG_FILE' does not exist"
	[[ $CONFIG_FILE =~ (^|/)([a-zA-Z0-9_=+.-]{1,15})\.conf$ ]] || die "The config file must be a valid interface name, followed by .conf"
	CONFIG_FILE="$(cd "${CONFIG_FILE%/*}" && pwd -P)/${CONFIG_FILE##*/}"
	INTERFACE="${BASH_REMATCH[2]}"
	shopt -s nocasematch
	while read -r line || [[ -n $line ]]; do
		stripped="${line%%\#*}"
		key="${stripped%%=*}"; key="${key##*([[:space:]])}"; key="${key%%*([[:space:]])}"
		value="${stripped#*=}"; value="${value##*([[:space:]])}"; value="${value%%*([[:space:]])}"
		[[ $key == "["* ]] && interface_section=0
		[[ $key == "[Interface]" ]] && interface_section=1
		if [[ $interface_section -eq 1 ]]; then
			case "$key" in
			Address) ADDRESSES+=( ${value//,/ } ); continue ;;
			MTU) MTU="$value"; continue ;;
			DNS) DNS="${value//,/ }"; continue ;;
			Table) TABLE="$value"; continue ;;
			esac
		fi
		WG_CONFIG+="$line"$'\n'
	done < "$CONFIG_FILE"
	shopt -u nocasematch
}

admin_check() {
	[[ $UID == 0 ]] || die "This script must be executed as admin"
}

get_real_interface() {
	local interface diff
	wg show interfaces >/dev/null
	[[ -f "/var/run/wireguard/$INTERFACE.name" ]] || return 1
	interface="$(< "/var/run/wireguard/$INTERFACE.name")"
	[[ -n $interface && -S "/var/run/wireguard/$interface.sock" ]] || return 1
	diff=$(( $(stat -f %m "/var/run/wireguard/$interface.sock" 2>/dev/null || echo 200) - $(stat -f %m "/var/run/wireguard/$INTERFACE.name" 2>/dev/null || echo 100) ))
	[[ $diff -ge 2 || $diff -le -2 ]] && return 1
	REAL_INTERFACE="$interface"
	echo "[+] Interface for $INTERFACE is $REAL_INTERFACE" >&2
	return 0
}

del_routes() {
	[[ -n $REAL_INTERFACE ]] || return 0
	local todelete=( ) destination gateway netif
	while read -r destination _ _ _ _ netif _; do
		[[ $netif == "$REAL_INTERFACE" ]] && todelete+=( "$destination" )
	done < <(netstat -nr -f inet)
	for destination in "${todelete[@]}"; do
		cmd route -q -n delete -inet "$destination" >/dev/null || true
	done
	todelete=( )
	while read -r destination gateway _ netif; do
		[[ $netif == "$REAL_INTERFACE" || ( $netif == lo* && $gateway == "$REAL_INTERFACE" ) ]] && todelete+=( "$destination" )
	done < <(netstat -nr -f inet6)
	for destination in "${todelete[@]}"; do
		cmd route -q -n delete -inet6 "$destination" >/dev/null || true
	done
	for destination in "${ENDPOINTS[@]}"; do
		if [[ $destination == *:* ]]; then
			cmd route -q -n delete -inet6 "$destination" >/dev/null || true
		else
			cmd route -q -n delete -inet "$destination" >/dev/null || true
		fi
	done
}

collect_gateways() {
	local destination gateway

	GATEWAY4=""
	while read -r destination gateway _; do
		[[ $destination == default ]] || continue
		GATEWAY4="$gateway"
		break
	done < <(netstat -nr -f inet)

	GATEWAY6=""
	while read -r destination gateway _; do
		[[ $destination == default ]] || continue
		GATEWAY6="$gateway"
		break
	done < <(netstat -nr -f inet6)
}

collect_endpoints() {
	ENDPOINTS=( )
	while read -r _ endpoint; do
		[[ $endpoint =~ ^\[?([a-z0-9:.]+)\]?:[0-9]+$ ]] || continue
		ENDPOINTS+=( "${BASH_REMATCH[1]}" )
	done < <(wg show "$REAL_INTERFACE" endpoints)
}

array_contains() {
        local oifs
        oifs=$IFS
        IFS=';'
        for word in $1; do
              [[ -n "$word" ]] || continue
              if [[ "$(echo "$word" | cut -d= -f1)" == "$2" ]]; then
                    IFS=$oifs
                    return 0
              fi
        done
        IFS=$oifs
        return 1
}

array_remove() {
        local oifs
        oifs=$IFS
        IFS=';'
        tmp=""
        for word in $1; do
              [[ -n "$word" ]] || continue
              if [[ "$(echo "$word" | cut -d= -f1)" == "$2" ]]; then
                    continue
              fi
              tmp="$tmp|$word"
        done
        IFS=$oifs
        echo $tmp | sed "s/|/;/g"
}

SERVICE_DNS=""
SERVICE_DNS_SEARCH=""
collect_new_service_dns() {
	local service get_response
	local found_services
        local oifs

	{ read -r _ && while read -r service; do
		[[ $service == "*"* ]] && service="${service:1}"
		found_services="$found_services;$service"
		array_contains "$SERVICE_DNS" "$service" && continue
		get_response="$(cmd networksetup -getdnsservers "$service" |tr '\n' '-')"
		[[ $get_response == *" "* ]] && get_response="Empty"
		[[ -n $get_response ]] && SERVICE_DNS="$SERVICE_DNS;$service=$get_response"
		get_response="$(cmd networksetup -getsearchdomains "$service")"
		[[ $get_response == *" "* ]] && get_response="Empty"
		[[ -n $get_response ]] && SERVICE_DNS_SEARCH="$SERVICE_DNS_SEARCH;$service=$get_response"
	done; } < <(networksetup -listallnetworkservices)

        oifs=$IFS
        IFS=';'
	for service_value in $SERVICE_DNS; do
                [[ -n "$service_value" ]] || continue
                service=$(echo $service_value | cut -d= -f1)
                if ! array_contains "$found_services" "$service"; then
			SERVICE_DNS=$(array_remove "$SERVICE_DNS" "$service")
			SERVICE_DNS_SEARCH=$(array_remove "$SERVICE_DNS_SEARCH" "$service")
                fi
	done
        IFS=$oifs
}

set_endpoint_direct_route() {
	local old_endpoints endpoint old_gateway4 old_gateway6 remove_all_old=0 added=( )
	old_endpoints=( "${ENDPOINTS[@]}" )
	old_gateway4="$GATEWAY4"
	old_gateway6="$GATEWAY6"
	collect_gateways
	collect_endpoints

	[[ $old_gateway4 != "$GATEWAY4" || $old_gateway6 != "$GATEWAY6" ]] && remove_all_old=1

	if [[ $remove_all_old -eq 1 ]]; then
		for endpoint in "${ENDPOINTS[@]}"; do
			[[ " ${old_endpoints[*]} " == *" $endpoint "* ]] || old_endpoints+=( "$endpoint" )
		done
	fi

	for endpoint in "${old_endpoints[@]}"; do
		[[ $remove_all_old -eq 0 && " ${ENDPOINTS[*]} " == *" $endpoint "* ]] && continue
		if [[ $endpoint == *:* && $AUTO_ROUTE6 -eq 1 ]]; then
			cmd route -q -n delete -inet6 "$endpoint" >/dev/null 2>&1 || true
		elif [[ $AUTO_ROUTE4 -eq 1 ]]; then
			cmd route -q -n delete -inet "$endpoint" >/dev/null 2>&1 || true
		fi
	done

	for endpoint in "${ENDPOINTS[@]}"; do
		if [[ $remove_all_old -eq 0 && " ${old_endpoints[*]} " == *" $endpoint "* ]]; then
			added+=( "$endpoint" )
			continue
		fi
		if [[ $endpoint == *:* && $AUTO_ROUTE6 -eq 1 ]]; then
			if [[ -n $GATEWAY6 ]]; then
				cmd route -q -n add -inet6 "$endpoint" -gateway "$GATEWAY6" >/dev/null || true
			else
				# Prevent routing loop
				cmd route -q -n add -inet6 "$endpoint" ::1 -blackhole >/dev/null || true
			fi
			added+=( "$endpoint" )
		elif [[ $AUTO_ROUTE4 -eq 1 ]]; then
			if [[ -n $GATEWAY4 ]]; then
				cmd route -q -n add -inet "$endpoint" -gateway "$GATEWAY4" >/dev/null || true
			else
				# Prevent routing loop
				cmd route -q -n add -inet "$endpoint" 127.0.0.1 -blackhole >/dev/null || true
			fi
			added+=( "$endpoint" )
		fi
	done
	ENDPOINTS=( "${added[@]}" )
}

set_dns() {
	collect_new_service_dns
	local service response
        local oifs ooifs

        oifs=$IFS
        IFS=';'
	for service_value in $SERVICE_DNS; do
                [[ -n "$service_value" ]] || continue
                service=$(echo $service_value | cut -d= -f1)

                ooifs=$IFS
                IFS=' '
		while read -r response; do
			[[ $response == *Error* ]] && echo "$response" >&2
		done < <(
			cmd networksetup -setdnsservers "$service" $DNS
		        cmd networksetup -setsearchdomains "$service" Empty
		)
                IFS=$ooifs
	done
        IFS=$oifs
}

array_value() {
        local oifs
        oifs=$IFS
        IFS=';'
        for word in $1; do
              [[ -n "$word" ]] || continue
              if [[ "$(echo "$word" | cut -d= -f1)" == "$2" ]]; then
                    IFS=$oifs
                    echo $(echo "$word" | cut -d= -f2)
                    return
              fi
        done
        IFS=$oifs
}

del_dns() {
	local service response oifs

        oifs=$IFS
        IFS=';'
	for service_value in $SERVICE_DNS; do
                [[ -n "$service_value" ]] || continue
                service=$(echo $service_value | cut -d= -f1)
                value=$(echo $service_value | cut -d= -f2 | sed "s/-/ /g")
                IFS=' '
		while read -r response; do
			[[ $response == *Error* ]] && echo "$response" >&2
		done < <(
			cmd networksetup -setdnsservers "$service" $value || true
			cmd networksetup -setsearchdomains "$service" "$(array_value "$SERVICE_DNS_SEARCH" "$service")" || true
		)
                IFS=';'
	done
        IFS=$oifs
}

monitor_daemon() {
	echo "[+] Backgrounding route monitor" >&2
	(trap 'del_routes; del_dns; exit 0' INT TERM EXIT
	exec >/dev/null 2>&1
	local event pid=$BASHPID
	[[ -n "$DNS" ]] && trap set_dns ALRM
	# TODO: this should also check to see if the endpoint actually changes
	# in response to incoming packets, and then call set_endpoint_direct_route
	# then too. That function should be able to gracefully cleanup if the
	# endpoints change.
	while read -r event; do
		[[ $event == RTM_* ]] || continue
		ifconfig "$REAL_INTERFACE" >/dev/null 2>&1 || break
		[[ $AUTO_ROUTE4 -eq 1 || $AUTO_ROUTE6 -eq 1 ]] && set_endpoint_direct_route
		[[ -z $MTU ]] && set_mtu
		if [[ -n "$DNS" ]]; then
			set_dns
			sleep 2 && kill -ALRM $pid 2>/dev/null &
		fi
	done < <(route -n monitor)) &
	disown
}

add_route() {
	[[ $TABLE != off ]] || return 0

	local family=inet
	[[ $1 == *:* ]] && family=inet6

	if [[ $1 == */0 && ( -z $TABLE || $TABLE == auto ) ]]; then
		if [[ $1 == *:* ]]; then
			AUTO_ROUTE6=1
			cmd route -q -n add -inet6 ::/1 -interface "$REAL_INTERFACE" >/dev/null
			cmd route -q -n add -inet6 8000::/1 -interface "$REAL_INTERFACE" >/dev/null
		else
			AUTO_ROUTE4=1
			cmd route -q -n add -inet 0.0.0.0/1 -interface "$REAL_INTERFACE" >/dev/null
			cmd route -q -n add -inet 128.0.0.0/1 -interface "$REAL_INTERFACE" >/dev/null
		fi
	else
		[[ $TABLE == main || $TABLE == auto || -z $TABLE ]] || die "Darwin only supports TABLE=auto|main|off"
		cmd route -q -n add -$family "$1" -interface "$REAL_INTERFACE" >/dev/null

	fi
}

cmd_usage() {
	cat >&2 <<-_EOF
	Usage: $PROGRAM [ up | down ] [ CONFIG_FILE ]
	Usage: $PROGRAM [ cleanup ] [ DNS, DNS... ]
	_EOF
}

cmd_up() {
	local i
	get_real_interface || die "\`$INTERFACE' does not exist"
	trap 'del_routes; exit' INT TERM EXIT
	for i in $(while read -r _ i; do for i in $i; do [[ $i =~ ^[0-9a-z:.]+/[0-9]+$ ]] && echo "$i"; done; done < <(wg show "$REAL_INTERFACE" allowed-ips) | sort -nr -k 2 -t /); do
		add_route "$i"
	done
	[[ $AUTO_ROUTE4 -eq 1 || $AUTO_ROUTE6 -eq 1 ]] && set_endpoint_direct_route
	[[ -n "$DNS" ]] && set_dns
	monitor_daemon
	trap - INT TERM EXIT
}

cmd_down() {
	if ! get_real_interface || [[ " $(wg show interfaces) " != *" $REAL_INTERFACE "* ]]; then
		die "\`$INTERFACE' is not a WireGuard interface"
	fi
}

cmd_cleanup() {
  INTERFACE="$1"

  { read -r _ && while read -r service; do
    [[ $service == "*"* ]] && service="${service:1}"
    get_response="$(cmd networksetup -getdnsservers "$service" |tr '\n' '-')"
    [[ $get_response == *" "* ]] && continue

    get_response="$(echo $get_response | tr '-' '\n')"
    echo "$get_response" | grep "$2" &>/dev/null || continue
    get_response="$(echo "$get_response" | grep -v "$2")"

    if [[ "$3" ]]; then
      get_response="$(echo "$get_response" | grep -v "$3" || echo "")"
    fi

    if [[ $get_response ]]; then
      cmd networksetup -setdnsservers "$service" $get_response
    else
      cmd networksetup -setdnsservers "$service" "Empty"
    fi
  done; } < <(networksetup -listallnetworkservices)
}

# ~~ function override insertion point ~~

if [[ $# -eq 1 && ( $1 == --help || $1 == -h || $1 == help ) ]]; then
	cmd_usage
elif [[ $# -eq 2 && $1 == up ]]; then
	admin_check
	parse_options "$2"
	cmd_up
elif [[ $# -eq 2 && $1 == down ]]; then
	admin_check
	parse_options "$2"
	cmd_down
elif [[ $1 == cleanup ]]; then
	admin_check
	cmd_cleanup "$2" "$3" "$4"
else
	cmd_usage
	exit 1
fi

exit 0
