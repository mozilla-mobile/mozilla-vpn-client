# VPN Connection benchmarks

For showing the user information on their internet speed we need to be able to test their current connection. The test can be run successfully only when the VPN is on and the connection has a signal. Results of the connection test are shown on the **Connection Info Screen**.

## Benchmarks

In order to get information on the current connection the following benchmarks are written as `Task`s that are run sequentially.

### Benchmark: Ping

The value for ping is determined by listening to changes of ping latency using the `ConnectionHealth` class for a certain duration and calculating the average of all sent and received pings.

### Benchmark: Download

The value for download is determined by downloading data from a remote file server and calculating the amount of data that is being received while running the download.

### Speed

Depending on the results of the benchmarks we set a connection speed to indicate what the current connection is capable of.

* `SpeedSlow`: Connection speed is slow and should be potentially troubleshooted.
    * Switching server locations
    * Checking your internet connection
* `SpeedMedium`: Connection speed is good enough for basic internet usage.
    * Browsing the internet
    * Streaming video
    * Video conferencing
* `SpeedFast`: Connection speed is capable of handling usage that requires more bandwidth.
    * Streaming in 4K
    * High-speed downloads
    * Online gaming

## States

- `StateInitial`: Benchmarking did not start or was stopped without any errors.
- `StateRunning`: The current connection is being benchmarked.
- `StateReady`: All benchmarks were run successfully and the results are available.
- `StateError`: While benchmarking an unexpected error occurred.

## User flow

```mermaid
flowchart LR
    MainView(Main View)
    LoadingView(Loading view)
    ConnectionView(Connection info view)
    ErrorView(Error view)

    MainView -->|Start| LoadingView
    LoadingView -->|User aborted| MainView
    LoadingView -->|Backend error| ErrorView
    ErrorView -->|User dismissed| MainView
    ErrorView -->|User restarted| LoadingView
    LoadingView -->|Result| ConnectionView
```
