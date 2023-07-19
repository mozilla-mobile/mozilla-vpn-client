- Status:
- Date: 2023-06-07
- Author: [@brizental](https://github.com/brizental)
- RFC PR: [#7112](https://github.com/mozilla-mobile/mozilla-vpn-client/pull/7112)
- Implementation GitHub issue: TODO

## Problem Statement

### The two processes: Main Application and Daemon

The Mozilla VPN application is split in two processes: the main application and the daemon
(a background process). The main application is where the user interface runs and the
daemon is what manages the VPN configuration and tunnel.

> **Note**: The daemon process may also be referred to as "network extension" on iOS
> or "broker" in multiple platforms. Throughout this document the term "daemon" will be used.

#### The lifetime of each process varies between the supported platforms

On Android the daemon process starts alongside the main application process once the application is
launched and remains active until killed, regardless on whether the VPN is turned on or not.
The main application process is only active while the application is in the foreground.
iOS is similar, but the daemon process is only active while the VPN is turned ON.

On Desktop environments the main application process is active while the application is active,
independent of the application being foregrounded or backgrounded. Due to OS specific mechanisms
such as [App Nap](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/power_efficiency_guidelines_osx/AppNap.html)
this process may go dormant intermittently after the application is backgrounded for a long time.

#### The daemon process may be started independently from the main application on mobile platforms

Both the operational system and the Mozilla VPN application expose features that allow the user
to turn on the VPN without interacting with the main application. On mobile environments, when
the VPN is turned on through any of these methods, the main application process is never launched
while the daemon process is.

### The issue with mobile session based telemetry

When analyzing incoming telemetry from the Mozilla VPN application, it's a recurring requirement
to be able to partition the incoming data by session. **A VPN session starts when the VPN is turned
on through some user action and ends once it is turned off through some user action.**

That is easily addressed on desktop environments, by having the main application manage a session_id
metric and add that to all session pings. Data from a given session all have the same session_id
and can be joined using that identifier.

On mobile environments, due to the lack of guarantees on the main application process being alive
throughout most or any of the VPN session, having the main application manage the session_id is unreliable.
This identifier needs to be rotated on every session start/end and the main application process may
not be running at these times.

Moreover, on mobile platforms, the fact that the main application process is killed whenever
the application goes to the background makes the main application telemetry insufficient for collecting
all the telemetry required to understand the health of the Mozilla VPN application and its usage patterns.
To address this issue, mobile daemons have a separate instance of Glean, used to collect telemetry
while the application is backgrounded. In order to get a full picture of a mobile user's session,
joining the daemon data with the main application is necessary. However, the telemetry clients in
each process are independent so manually sharing an identifier is required for such joins.

On [VPN session pings - Tech Spec](https://mozilla-hub.atlassian.net/wiki/spaces/SECPRV/pages/184419319/VPN+session+pings+-+Tech+spec)
an installation_id metric is introduced that would allows joins between daemon and main application
data. That still does not make it possible to join specific sessions i.e. we are able to infer that
a given daemon session is coming from the same installation as a main application session, but we
cannot reliably or easily infer that a given daemon session is the same as a given main application session.

## Document Purpose

Propose a solution to reliably join mobile session data between daemon and main application telemetry.

## Requirements

The main transport for session related data are the session pings, namely
the [`vpnsession`](https://dictionary.telemetry.mozilla.org/apps/mozilla_vpn/pings/vpnsession) ping
and the [`daemonsession`](https://dictionary.telemetry.mozilla.org/apps/mozilla_vpn/pings/daemonsession) ping.

It is a requirement on the proposed solution, that the metrics included in each ping are all
recorded during the session related to the shared session id in it i.e. the ping does not contain
any metric from a previous session.

## Proposed Solution

A new metric will be introduced to mobile platforms: `shared_session_id`. This metric will be the
primary session identifier metric for Mozilla VPN mobile telemetry. This metric will have lifetime
`User`, with this lifetime, once a metric is set it can only be overwritten but never cleared from
the telemetry storage.

The daemon process will be responsible for managing this metric. The daemon is the preferred
place for this, because it is _guaranteed_ to be running throughout the whole VPN session.

The main application will communicate with the daemon in order to get the correct value for
this metric whenever necessary.

The scheduling of the `vpnsession` and `daemonsession` pings will be tweaked to meet
the requirements of isolating session data per ping.

### On the daemon

There are three important triggers that require action on the shared session id and
the `daemonsession` ping on the daemon process.

#### On session start

Once a new session is started,

1. The `daemonsession` ping will be submitted with reason `flush`, in order to flush out
   any lingering data before starting collection telemetry related to the new session.
2. The `shared_session_id` is generated and recorded.
3. The value recorded to telemetry is also persisted on the daemon's local storage.
4. Session metrics may be recorded from this point on.
5. Session is started.

The `flush` ping submitted on step one is expected to be empty. The submission of it right
before the start of a new session is a safety measure to guarantee no lingering data makes it
to the actual session ping.

#### On session end

Once an ongoing session is ended,

1. The `daemonsession` ping is submitted with the reason `daemon_end`.
2. The daemon's local storage for the `shared_session_id` is cleared.
3. The `shared_session_id` metric is set to a known value. Due to its `User` lifetime,
   this metric cannot be cleared from storage ever. To indicate a period of inactivity,
   we will leverage a known UUID.

#### On daemon process start

The session end trigger may not be hit, if the daemon process is force killed. Therefore,
whenever the daemon process is started the local storage will be searched for a lingering
`shared_session_id` value. That is a clear signal that the session end trigger was not hit
and that there may be lingering data from a previous session on the Glean storage.

When that is the case,

1. The lingering `shared_session_id` value is recorded to Glean.
2. The `daemonsession` ping is submitted with the reason `flush_init`.

### On the main application

The same three triggers which are important for the daemon and the `daemonsession` ping,
are also important for the main application and the `vpnsession` ping.

#### On session start

If the main application is active during a session start it will,

1. Signal to the daemon that a new session is to be started.
2. It will then ask the daemon for the value of the shared session id it created.
3. If the previous two communications were successful
   1. The `vpnsession` ping will be submitted with reason `flush`.
   2. The value of the retrieved `shared_session_id` is recorded.
   3. Other session metrics may be recorded from this point.
   4. If the communication with the daemon fails, an error metric is recorded.

> **Note**: If the retrieved session id is the known inactive session id,
> this is considered an unsuccessful retrieval and the error path -- i.e. step 4 of the above steps -- is taken.

#### On session end

If the main application is active during a session end it will,

1. Submit the session ping with reason `end`.
2. Set `shared_session_id` metric is set to a known value. The same known value used for inactive
   sessions on the `daemonsession` ping.

#### On application start

When the main application is started, just like in the daemon, there may be some lingering data
from a previous session in storage. On the main application, the session_id is not directly managed,
so there is no way to know if there is lingering data or not.

In this case, the main application will submit the `vpnsession` ping with reason `flush_init` on
_every_ initialization. In case there is no lingering data, this is not really an issue. The ping
will be sent with only the currently stored session id value.

After sending the `flush_init` ping, the VPN will query the daemon for a session id.
If a session id is received, that means there is an ongoing VPN session. In that case, the
retrieved session id is recorded to the `shared_session_id` metric. If nothing is received from
the daemon, it is assumed there is no ongoing VPN session and the `shared_session_id` metric is
set to the known value, to indicate inactivity.

### Validation

In order to validate this feature after release, these questions need to be answered:

- Are we getting too much data assigned to the inactive session id?
  If the design is completely air tight i.e. all collected data is related to an active session id,
  there should be no data on inactive session flush pings. If there is too much data on these pings,
  that would signal a possible bug on the design or on the implementation.
- Does every session coming from the daemon have a `start` and `end` or `flush_init` pings?
  One of the core assumptions in this proposal is that the daemon process is always capable of
  capturing the whole VPN session. As such it should always have these pings in a given session.
- Are there too many error metrics being recorded due to unsuccessful retrieval of the session id
  from the daemon?
  Another core assumption here is that, if the daemon has an ongoing session the main application will,
  most of the time, not run into any issues retrieving the shared_session_id from it. If there are too
  many errors in communication that would mean that assumption was incorrect or there is a bug in the
  implementation.

## Considered Alternatives

Following are the other considered alternatives to address the same issue as the proposed solution
and the reasoning on why these were ultimately not the chosen solution.

### Collect all data on the daemon

If all data were collected on the daemon, instead of having the two Glean architecture that is
currently implemented that would not also fix the issue of joining daemon data with main application
data, simply by not having main application data at all.

The issue with this approach, is that most of the telemetry collected on the Mozilla VPN application
is in the main application. Button clicks, configuration, performance metrics and so on. If there were
only a Glean instance of the daemon process, every time an event that requires telemetry happened on
the main application that would need to be passed through to the daemon. Not only does that significantly
increase the overhead of adding any new telemetry to the application, but it is also not possible
for cases such as iOS where the daemon process is only alive while a session is active.

### Share the management burdens of the shared session id

The first iteration of the proposed solution, included the main application storing and managing
the `shared_session_id` and passing it along to the daemon when possible. That can be read on
[7e20c2c933fc868b3ea50d6eb35651b904628658](https://github.com/mozilla-mobile/mozilla-vpn-client/tree/7e20c2c933fc868b3ea50d6eb35651b904628658).
Ultimately, that is an overly complex way to achieve the same end goal as the current proposal.
