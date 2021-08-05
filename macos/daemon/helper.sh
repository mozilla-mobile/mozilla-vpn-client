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

cmd_usage() {
	cat >&2 <<-_EOF
	Usage: $PROGRAM [ up | down ] [ CONFIG_FILE ]
	Usage: $PROGRAM [ cleanup ] [ DNS, DNS... ]
	_EOF
}

cmd_up() {
	local i
	get_real_interface || die "\`$INTERFACE' does not exist"
	[[ -n "$DNS" ]] && set_dns
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
