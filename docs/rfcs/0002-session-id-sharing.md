- Status:
- Date: 2023-06-07
- Author: [@brizental](https://github.com/brizental)
- RFC PR: [#TODO](https://github.com/mozilla-mobile/mozilla-vpn-client/pull/TODO)
- Implementation GitHub issue: TODO

## Problem Statement

The VPN application is divided in two processes: the main application and the daemon. The main application is where the user interface runs and the daemon is what manages the VPN configuration and tunnel.

In desktop environments, even while backgrounded the main application process is still running, but in mobile environments it is not. Due to this caveat, the mobile daemon processes have a separate instance of Glean to be able to collect telemetry about the connection while the application is in the background.

This architecture poses a problem for analyzers. It is not possible to easily join the data from the same session between daemon and main application. The session ids are different due to these being two different Glean instances.

For more information on this topic, refer to [Mozilla VPN Telemetry Refactor](https://docs.google.com/document/d/1jyNZ_g_cUpZZsEr2hYwnwZgkxlvqTmoFUJKp_LAOPts/edit#heading=h.xararkvsnpss).

## Proposed Solution

> **Note** Honestly, I think for this proposed solution it's just easier to look at the pseudo-code in Appendix 1.

A new metric will be added to both the daemonsession and the session pings: shared_session_id. This metric will have lifetime ping i.e. it is cleared from storage whenever a ping containing it is submitted. Thus it needs to be set again every time a new session/daemonsession ping is submitted.

Both daemon and main application will store the value of this metric in their local storages for synchronization purposes. More about this in the sections below.

A new reason will be added to the session pings “flush_init”. As better described in the sections below, the processes will be able to identify that either the VPN tunnel or main application did not gracefully hit the deactivation signal i.e. were force killed. The “flush_init” ping will be used to flush any data remaining from the previous session.

There will be four key triggers that will handle the shared_session_id: initialization, on VPN activation, ping timer and on VPN deactivation.

### ON VPN ACTIVATION

#### MAIN APPLICATION

In case the VPN is started from the main application, a session id will be created and shared through the activation signal with the daemon. The generated UUID will be stored in the main application local storage and the session ping will be submitted.

#### DAEMON

If the VPN is being activated through the main application, a session id will have been passed through the activation signal. Otherwise the Daemon will generate this value itself. The UUID will be stored in the daemon local storage and the session ping will be submitted.

### ON PING TIMER

#### MAIN APPLICATION

The session id is retrieved from local storage and recorded. The session ping is submitted.

#### DAEMON

The session id is retrieved from local storage and recorded. The session ping is submitted.

### ON VPN DEACTIVATION

The deactivation signal may not be hit in both the main application and the daemon, because both processes may be force killed. The local storage that contains the session id will be cleared after submitting the final session ping. This will be a flag at initialization that the final session ping was not submitted and there may be lingering data from a previous session in storage that needs to be submitted.

#### MAIN APPLICATION

The session id is retrieved from local storage and recorded. The session ping is submitted. Finally local storage for the session id is cleared.

#### DAEMON

The session id is retrieved from local storage and recorded. The session ping is submitted. Finally local storage for the session id is cleared.

### ON INITIALIZATION

#### MAIN APPLICATION

The session id is retrieved from local storage. If it is not empty, there are four possibilities:

1. The same session from a previous app run is still ongoing. No action is required
2. Another session is ongoing in which case a flush ping is required to data from a previous session and start recording data for the new session
3. No session is ongoing but there is a lingering session id indicating that there is still lingering data from a previous session in storage and a flush ping is required
4. No session is ongoing and no lingering session id is in storage. No action is required

#### DAEMON

On the daemon there are only two possibilities:

1. No session is ongoing but there is a lingering session id indicating that there is still lingering data from a previous session in storage and a flush ping is required
2. No session is ongoing and no lingering session id is in storage. No action is required

## APPENDIX

### APPENDIX 1: PSEUDO CODE

#### Main application

```cpp
MainApp::init() {
  String lingeringSessionId = LocalStorage::get(SESSION_ID_STORAGE_KEY);
  // If this is not empty, the VPN was not deactivated from the app.
  if (!lingeringSessionId.isEmpty()) {
    if (Daemon::vpnIsOn()) {
      try {
        String sessionId = Daemon::getSessionId();
        // Check if we are still in the same session
        if (sessionId != lingeringSessionId) {
          // Flush any lingering data from a previous session
          pings::main_app::session.submit(INIT_FLUSH_REASON);

          metrics::main_app::sessionId.set(sessionId);
          // Record session::* context metrics e.g. app_exclusions.

          LocalStorage::set(SESSION_ID_STORAGE_KEY, sessionId);
        }

        // Schedule the timer.
        MainApp::schedulePingTimer();
      } catch {
        // Note: If communication with the daemon doesn't work,
        // let's assume the VPN is off. This is very unlikely, given we
        // have just communicated with the daemon through Daemon::vpnIsOn();

        // Flush any lingering data from a previous session
        pings::main_app::session.submit(INIT_FLUSH_REASON);
        // Clear the sessionId storage
        LocalStorage::clear(SESSION_ID_STORAGE_KEY);
      }
    } else {
      // Flush any lingering data from a previous session
      pings::main_app::session.submit(INIT_FLUSH_REASON);
      // Clear the sessionId storage
      LocalStorage::clear(SESSION_ID_STORAGE_KEY);
    }
  }
}

MainApp::onActivate() {
  // Let's flush any lingering data for before activating the VPN.
  // We want the active session pings to only include data from the current session.
  //
  // This ping should be always empty. This is a way to monitor if it isn't
  pings::main_app::session.submit(FLUSH_REASON);

  String sessionId = generateUUID();
  try {
    Daemon::activate(config, sessionId);
    // Note: session::* metrics should only be recorded from now on,
    // so that flush pings don't contain unexpected data.

    metrics::main_app::sessionId.set(sessionId);
    // Record session::* context metrics e.g. app_exclusions.

    LocalStorage::set(SESSION_ID_STORAGE_KEY, sessionId);

    // Submit the start ping
    pings::main_app::session.submit(START_REASON);
  } catch {
    // TODO: Record error.
    LocalStorage::clear(SESSION_ID_STORAGE_KEY);
  }
}

MainApp::onPingTimer() {
  String sessionId = LocalStorage::get(SESSION_ID_STORAGE_KEY);
  metrics::main_app::sessionId.set(sessionId);

  // Submit the timer ping
  pings::main_app::session.submit(TIMER_REASON);
}

MainApp::onDeactivate() {
  // Stop the timer.
  MainApp::stopPingTimer();

  String sessionId = LocalStorage::get(SESSION_ID_STORAGE_KEY);
  metrics::main_app::sessionId.set(sessionId);

  // Submit the stop ping
  pings::main_app::session.submit(TIMER_REASON);
  LocalStorage::clear(SESSION_ID_STORAGE_KEY);
}
```

#### Daemon

```cpp
Daemon::init() {
  String lingeringSessionId = LocalStorage::get(SESSION_ID_STORAGE_KEY);

  // If this is not empty, the tunnel was force killed
  if (!lingeringSessionId.isEmpty()) {
    metrics::daemon::sessionId.set(lingeringSessionId);
    // Flush lingering data
    pings::daemonsession.submit(INIT_FLUSH_REASON);
    LocalStorage::clear(SESSION_ID_STORAGE_KEY);
  }
}

Daemon::onActivate(Config config, String sessionId = "") {
  // Let's flush any lingering data for before activating the VPN.
  // We want the active session pings to only include data from the current session.
  //
  // This ping should be always empty. This is a way to monitor if it isn't
  pings::daemonsession.submit(FLUSH_REASON);

  auto sessionId = sessionId;
  if (sessionId.isEmpty()) {
    logger.info() << "Starting VPN from somewhere other than the main app";
    sessionId = metrics::daemon::sessionId.generateAndSet();
  } else {
    metrics::daemon::sessionId.set(sessionId);
  }
  LocalStorage::set(SESSION_ID_STORAGE_KEY, sessionId);

  // Submit the start ping
  pings::daemonsession.submit(START_REASON);
}

Daemon::onPingTimer() {
  String sessionId = LocalStorage::get(SESSION_ID_STORAGE_KEY);
  metrics::daemon::sessionId.set(sessionId);

  // Submit the timer ping
  pings::daemonsession.submit(TIMER_REASON);
}

Daemon::onDeactivate() {
  String sessionId = LocalStorage::get(SESSION_ID_STORAGE_KEY);
  metrics::daemon::sessionId.set(sessionId);

  // Submit the stop ping
  pings::daemonsession.submit(STOP_REASON);

  // Clear the local storage
  LocalStorage::clear(SESSION_ID_STORAGE_KEY);
}

```
