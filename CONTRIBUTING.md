# Contributing

Discussions happen in [#mozilla-vpn:mozilla.org](https://chat.mozilla.org/#/room/#mozilla-vpn:mozilla.org) Matrix Channel.

1. [Issues marked as `good-first-bug`](https://github.com/mozilla-mobile/mozilla-vpn-client/labels/good%20first%20issue) are self-contained enough that a contributor should be able to work on them.
2. Issues are considered not assigned, until there is a PR linked to them.

## CPP Notes

We have the includes following these guidelines:
- first the internal headers
- after an empty line, the external headers
- each block (internal and external headers) alphabetic sorted
- if there is a corresponding header for the cpp class, that goes on top. e.g. for file wgutils.cpp, wgutils.h (even if not alphabetic sorted)
