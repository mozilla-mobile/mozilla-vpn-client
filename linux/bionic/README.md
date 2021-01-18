# Steps to create QT packages for bionic

From _this_ directory, run:

```
$ docker build -t mozillavpnbionic .
```

Now start the container with type mozillavpnbionic and run the script /tmp/qt_ppa.sh:

```
$ docker run -it mozillavpnbionic /tmp/qt_ppa.sh
```

Finally, copy all the files in /tmp/qt_ppa_final from the container to launchpad.
