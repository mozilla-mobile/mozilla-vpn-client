/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "startupnotifywatcher.h"
#include "leakdetector.h"
#include "logger.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <xcb/xcb.h>

constexpr const char* XDISPLAY_PRIMARY_SOCKET = "/tmp/.X11-unix/X0";

namespace {
Logger logger(LOG_LINUX, "StartupNotifyWatcher");
}

StartupNotifyWatcher::StartupNotifyWatcher(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(StartupNotifyWatcher);
  logger.log() << "StartupNotifyWatcher created.";

  /* Acquire the effective UID of the primary X display */
  uid_t realuid = getuid();
  struct stat st;
  if (stat(XDISPLAY_PRIMARY_SOCKET, &st) != 0) {
    logger.log() << "Unable to state X display:" << strerror(errno);
    return;
  }
  if (st.st_uid != realuid) {
    if (seteuid(st.st_uid) != 0) {
      logger.log() << "Failed to set EUID for X display:" << strerror(errno);
      st.st_uid = realuid;
    }
  }

  /* Open the primary X display socket. */
  struct sockaddr_un addr;
  socklen_t len = sizeof(int);
  int bufsize;

  strcpy(addr.sun_path, XDISPLAY_PRIMARY_SOCKET);
  addr.sun_family = AF_UNIX;
  m_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (m_socket < 0) {
    logger.log() << "Failed to open socket:" << strerror(errno);
    return;
  }
  if (getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &bufsize, &len) == 0) {
    if (bufsize < (64 * 1024)) {
      bufsize = 64 * 1024;
      setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &bufsize, len);
    }
  }
  if (::connect(m_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    logger.log() << "Failed to connect to socket:" << strerror(errno);
    return;
  }

  if (st.st_uid != realuid) {
    if (seteuid(realuid) != 0) {
      logger.log() << "Failed to restore UID:" << strerror(errno);
      return;
    }
  }

  /* Connext to the X display server. */
  xconn = xcb_connect_to_fd(m_socket, NULL);
  if (xconn == NULL) {
    logger.log() << "Failed to create X connection";
    return;
  }
  int err = xcb_connection_has_error(xconn);
  if (err != 0) {
    logger.log() << "X connection failed with code:" << err;
    return;
  }

  /* Configure all screens to send notification events. */
  xcb_screen_iterator_t iter;
  iter = xcb_setup_roots_iterator(xcb_get_setup(xconn));
  while (iter.rem) {
    const uint32_t mask[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_change_window_attributes(xconn, iter.data->root, XCB_CW_EVENT_MASK,
                                 mask);
    xcb_screen_next(&iter);
  }

  /* Load the atoms to decode startup notification events.  */
  xatom_begin = xatomLookup(xconn, "_NET_STARTUP_INFO_BEGIN");
  xatom_info = xatomLookup(xconn, "_NET_STARTUP_INFO");
  xatom_id = xatomLookup(xconn, "_NET_STARTUP_ID");
  xatom_utf8 = xatomLookup(xconn, "UTF8_STRING");

  /* Start processing as though we got a corrupt message. */
  m_corrupt = true;

  /* Listen for client events */
  m_notifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);
  connect(m_notifier,
          SIGNAL(activated(QSocketDescriptor, QSocketNotifier::Type)),
          SLOT(xeventReady()));
}

StartupNotifyWatcher::~StartupNotifyWatcher() {
  MVPN_COUNT_DTOR(StartupNotifyWatcher);
  xcb_disconnect(xconn);
  if (m_socket >= 0) {
    close(m_socket);
  }
}

xcb_atom_t StartupNotifyWatcher::xatomLookup(xcb_connection_t* c,
                                             const char* string) {
  xcb_intern_atom_cookie_t cookie =
      xcb_intern_atom(c, false, strlen(string), string);
  xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(c, cookie, NULL);
  xcb_atom_t atom = 0;
  if (reply) {
    atom = reply->atom;
    free(reply);
  }
  return atom;
}

void StartupNotifyWatcher::xeventStartupData(
    const xcb_client_message_event_t* msg) {
  /* Inspect for valid startup sequences. */
  if ((msg->type == xatom_begin) && (memcmp(msg->data.data8, "new:", 4) == 0)) {
    m_corrupt = false;
    m_buffer.clear();
    m_values.clear();
  } else if (m_corrupt) {
    return;
  }

  const char* data = (const char*)msg->data.data8;
  const char* end = (const char*)memchr(data, '\0', sizeof(msg->data));
  if (!end) {
    m_buffer.append(data, sizeof(msg->data));
    return;
  }
  m_buffer.append(data, end - data);
  QString message = QString::fromUtf8(m_buffer);

  /* Get the startup notification type */
  int length = message.count();
  int index = message.indexOf(':');
  if (index <= 0) {
    m_corrupt = true;
    return;
  }
  QString type = message.left(index);
  index++;

  /* Parse message name/value pairs. */
  while (index < length) {
    int eqindex = message.indexOf('=', index);
    if (eqindex < index) {
      break;
    }

    QString name = message.mid(index, eqindex - index).trimmed();
    QString value = QString();
    bool escaped = false;
    bool quoted = false;

    for (index = eqindex + 1; index < length; index++) {
      QChar ch = message.at(index);
      if (escaped) {
        value.append(ch);
        escaped = false;
      } else if (ch == '"') {
        quoted = !quoted;
      } else if (ch == '\\') {
        escaped = true;
      } else if (quoted) {
        value.append(ch);
      } else if (ch == ' ') {
        break;
      } else {
        value.append(ch);
      }
    }
    m_values[name] = value;
  }

#ifdef QT_DEBUG
  logger.log() << "Event:" << type;
  for (auto key : m_values.keys()) {
    logger.log() << key + "=" + m_values[key];
  }
#endif

  /* Report a launch event if we have an APPLICATION_ID and a valid PID */
  if (m_values.contains("APPLICATION_ID") && m_values.contains("PID")) {
    QString pid = m_values["PID"];
    QString path = "/proc/" + pid;
    struct stat st;
    if (stat(path.toLocal8Bit().constData(), &st) == 0) {
      emit appLaunched(m_values["APPLICATION_ID"], st.st_uid, pid.toInt());
    }
    m_corrupt = true;
  }
}

void StartupNotifyWatcher::xeventReady() {
  xcb_generic_event_t* xevent = xcb_poll_for_event(xconn);
  if (xevent == NULL) {
    return;
  }
  if ((xevent->response_type & 0x7f) == XCB_CLIENT_MESSAGE) {
    xcb_client_message_event_t* msg = (xcb_client_message_event_t*)xevent;
    if (msg->type == xatom_begin) {
      m_buffer.clear();
      xeventStartupData(msg);
    } else if (msg->type == xatom_info) {
      xeventStartupData(msg);
    }
  }
  free(xevent);
}
