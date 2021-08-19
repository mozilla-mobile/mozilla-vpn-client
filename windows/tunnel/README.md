# Embeddable WireGuard Tunnel Library

A simple yet effective way of leveraging the existing WireGuard codebase.
This is more or less the same thing as the
[embeddable-dll-service](https://git.zx2c4.com/wireguard-windows/about/embeddable-dll-service/README.md) and
[tunnel](https://git.zx2c4.com/wireguard-windows/tree/tunnel) modules from upstream
[wireguard-windows](https://git.zx2c4.com/wireguard-windows/about/).

## Building

To build the embeddable dll service, run `.\build.cmd` to produce `x64\tunnel.dll`.

## License

The contents of this directory are MIT-licensed. Files which have been modified
from their upstream versions will also list the Mozilla Foundation as a copyright
holder.

```text
Copyright (C) 2018-2021 WireGuard LLC. All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
```