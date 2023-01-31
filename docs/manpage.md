% mozillavpn(1)

# NAME
`mozillavpn` - A fast, secure and easy to use VPN. Built by the makers of Firefox.

# SYNOPSIS
mozillavpn [-h | \-\-help] [-v | \-\-version] command [command-options]

# DESCRIPTION
**mozillavpn** is the command-line tool for the Mozilla VPN. Most users would
typically interact with the VPN using the graphical user interface, which is
launched when no arguments are given. Providing this tool with additional
commands can provide other mechanisms to control the VPN.

# OPTIONS
**-h**, **\-\-help**
: Displays a friendly help message.

**-v**, **\-\-version**
: Displays the software version.

# COMMANDS
Unless the **-h**, **\-\-help**, **-v**, or **\-\-version** options are given,
one of the commands below must be present.

## ui
Launch the graphical user interface. This is the default behavior when no
options or commands are given. This command also accepts the following options:

**-h**, **\-\-help**
: Display a friendly help message describing the ui command.

**-m**, **\-\-minimized**
: Start the UI in a minimized state.

**-s**, **\-\-start-at-boot**
: Start at boot (if configured).

**-t**, **\-\-testing**
: Launch the UI configured for the staging environment.

## activate
Attempt to start the VPN and connect to the currently selected server.

## deactivate
Deactivate the VPN and return to an idle state.

## login
Begin the user authentication flow.

## logout
Log out of the current user and discard the authentication state.

## select
Select an exit server to use for the next VPN connection attempt.

## servers
Show a list of the available VPN servers. This command also accepts the
following options:

**-h**, **\-\-help**
: Display a friendly help message describing the status command.

**-v**, **\-\-verbose**
: Display additional information about each server.

**-c**, **\-\-cache**
: Fetch status from local cache only (no network access).

## status
Report the current status of the VPN. This command also accepts the following
options:

**-h**, **\-\-help**
: Display a friendly help message describing the status command.

**-c**, **\-\-cache**
: Fetch status from local cache only (no network access).

## linuxdaemon
Start the backend daemon to drive the VPN tunnel.

## wgconf
Export the current configuration as a wireguard configuration file.

# EXIT VALUES
**0**
: Success

**1**
: Failure
