# VPN Connection info test

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