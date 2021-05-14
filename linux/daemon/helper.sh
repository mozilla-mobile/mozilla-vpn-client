#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
#

echo "Running mozillavpn forked copy of wg-quick script."

set -e -o pipefail
shopt -s extglob
export LC_ALL=C

SELF="$(readlink -f "${BASH_SOURCE[0]}")"
export PATH="${SELF%/*}:$PATH"

WG_CONFIG=""
INTERFACE=""
DNS=( )
CONFIG_FILE=""

die() {
	echo "MozillaVPN WGQuick Fatal Error: $*" >&2
	exit 1
}

parse_options() {
	local interface_section=0 line key value stripped v
	CONFIG_FILE="$1"
	[[ $CONFIG_FILE =~ ^[a-zA-Z0-9_=+.-]{1,15}$ ]] && CONFIG_FILE="/etc/wireguard/$CONFIG_FILE.conf"
	[[ -e $CONFIG_FILE ]] || die "\`$CONFIG_FILE' does not exist"
	[[ $CONFIG_FILE =~ (^|/)([a-zA-Z0-9_=+.-]{1,15})\.conf$ ]] || die "The config file must be a valid interface name, followed by .conf"
	CONFIG_FILE="$(readlink -f "$CONFIG_FILE")"
	((($(stat -c '0%#a' "$CONFIG_FILE") & $(stat -c '0%#a' "${CONFIG_FILE%/*}") & 0007) == 0)) || echo "Warning: \`$CONFIG_FILE' is world accessible" >&2
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
			DNS) for v in ${value//,/ }; do
				[[ $v =~ (^[0-9.]+$)|(^.*:.*$) ]] && DNS+=( $v )
			done; continue ;;
			esac
		fi
		WG_CONFIG+="$line"$'\n'
	done < "$CONFIG_FILE"
	shopt -u nocasematch
}

del_if() {
	local table
	[[ $HAVE_SET_DNS -eq 0 ]] || unset_dns
}

resolvconf_iface_prefix() {
	[[ -f /etc/resolvconf/interface-order ]] || return 0
	local iface
	while read -r iface; do
		[[ $iface =~ ^([A-Za-z0-9-]+)\*$ ]] || continue
		echo "${BASH_REMATCH[1]}." && return 0
	done < /etc/resolvconf/interface-order
}

HAVE_SET_DNS=0
set_dns() {
	# Iterate through values of DNS array and pipe "nameserver" string to 
	# resolvconf command which adds nameserver rows for each dns entry to /etc/resolv.conf
	# (maybe amongst other things)
	#printf 'nameserver %s\n' "${DNS[@]}" | /bin/bash resolvconf -a "$(resolvconf_iface_prefix)$INTERFACE" -m 0 -x
	HAVE_SET_DNS=1
}

unset_dns() {
	#/bin/bash resolvconf -d "$(resolvconf_iface_prefix)$INTERFACE" -f
	echo "Nothing to see here"
}

cmd_up() {
	local i
	trap 'del_if; exit' INT TERM EXIT
	set_dns
	trap - INT TERM EXIT
}

cmd_down() {
	del_if
	unset_dns || true
}


if [[ $# -eq 2 && $1 == up ]]; then
	parse_options "$2"
	cmd_up
elif [[ $# -eq 2 && $1 == down ]]; then
	parse_options "$2"
	cmd_down
else
	exit 1
fi

exit 0
