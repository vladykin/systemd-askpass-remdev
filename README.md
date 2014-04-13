systemd-askpass-remdev
======================

This is a [systemd password agent][1] that reads password from removable drive,
e.g. USB thumb.


### How to use it

Get the sources and build them with `make`. [iniParser library][2] is required.
On Gentoo iniParser can be installed with `emerge -av1 dev-libs/iniparser`.

If the build succeeds, you'll get `systemd-ask-password-remdev` executable.
Copy it to some directory which is available during boot, e.g. `/usr/local/bin`.

Copy two provided systemd unit files (\*.path and \*.service) to `/etc/systemd/system` and edit the \*.service one:

* in the Requires line specify the device unit corresponding to your removable drive
* in the ExecStart line specify path to `systemd-ask-password-remdev`,   the removable device, its filesystem type and path to password file on the device

Create directory `/etc/systemd/system/sysinit.target.wants` and inside it create symlink to `systemd-ask-password-remdev.path`.


### Tested on

Gentoo Linux, GCC 4.7.3, systemd 208


  [1]: http://www.freedesktop.org/wiki/Software/systemd/PasswordAgents/
  [2]: http://ndevilla.free.fr/iniparser/
