# Mozilla VPN - RCC-Merge

RCCMerge is a simple tool that unifies and compress a set of RCC binary files
for the VPN client.

### Format

The format of the output is the following:

HEADER - 4 bytes - "MVPN"

Then a sequence of the following blocks:

File name size: 4-bytes
File name: a string with length equal to the previous 4-bytes value.
File content size: 4-bytes
File content: a sequence of N bytes (N is equal to the previous 4-bytes value).
