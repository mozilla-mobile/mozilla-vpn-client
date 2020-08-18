TEMPLATE  = app
TARGET    = mozillavpn_wg

SOURCES += \
  config.c \
  curve25519.c \
  encoding.c \
  genkey.c \
  ipc.c \
  pubkey.c \
  set.c \
  setconf.c \
  show.c \
  showconf.c \
  terminal.c \
  wg.c

HEADERS += \
  config.h \
  containers.h

OBJECTS_DIR = .obj
MOC_DIR = .moc

QMAKE_CFLAGS += -Wall -Wextra
QMAKE_CFLAGS += -MMD -MP
QMAKE_CFLAGS += -DRUNSTATEDIR=\\\"/var/run\\\"

linux-g++ {
  INCLUDEPATH += uapi/linux
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
