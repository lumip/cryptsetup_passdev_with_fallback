# cryptsetup passdev with password prompt fallback

A small keyscript combining passdev and askpass functionalities for systemd-based cryptsetup systems:
Will try to read a keyfile from a block device using passdev. After a timeout, if no
keyfile could be found, falls back to a password prompt using askpass.

## Installation

- Compile `passdev_with_fallback.c` and copy resulting binary to `/lib/cryptsetup/scripts/`.
- Copy `cryptpassdev_fallback` to `/usr/share/initramfs-tools/hooks/`.

## Usage

Point keyscript in `/etc/crypttab` entry to `/lib/cryptsetup/scripts/passdev_with_fallback` and provide
a string of the following format as keyfile argument:

`<delimiter><passdev argument><delimiter><askpass prompt>`

where
- `<delimiter>` is a single character used to separate arguments to passdev and askpass,
- `<passdev argument>` is the argument to passdev of form `<device>:<keyfile path on device>:<timeout>`,
- `<askpass prompt>` is the prompt displayed by askpass.

Example: `_/dev/sda1:keys/keyfile:5_password:`

Example for crypttab entry:
```
nvme0n1p1_crypt UUID=XXXX _/dev/disk/by-uuid/YYYY:/keyfile:1_Enter\040password\040for\040nvme0n1p1: luks,discard,keyscript=passdev_with_fallback,tries=3
```
