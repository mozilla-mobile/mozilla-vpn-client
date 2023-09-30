

#include <CoreFoundation/CoreFoundation.h>
#include <xpc/xpc.h>

#include <QObject>
#include <QString>

class XPCBase {
 protected:
  static constexpr auto defaultDictKey = "org.mozilla";
  static constexpr auto vpnDaemonName = "org.mozilla.macos.FirefoxVPN.daemon.test";

  static QString getQStringFromXPCDict(xpc_object_t event,
                                       const QString& key = defaultDictKey) {
    xpc_type_t type = xpc_get_type(event);
    assert(type == XPC_TYPE_DICTIONARY);
    if (type != XPC_TYPE_DICTIONARY) {
      Q_ASSERT(false);
      return QString();
    }
    xpc_object_t xpc_maybe_value =
        xpc_dictionary_get_value(event, key.toLocal8Bit().constData());
    if (xpc_maybe_value == NULL) {
      // Key is not in the dict
      return QString();
    }
    // Get the type of that value
    xpc_type_t value_type = xpc_get_type(xpc_maybe_value);
    assert(type == XPC_TYPE_STRING);
    if (type != XPC_TYPE_STRING) {
      // Value of that dict is not a string
      return QString();
    }
    return copyXPCString(xpc_maybe_value);
  }

  static QString copyXPCString(xpc_object_t event) {
    xpc_type_t type = xpc_get_type(event);
    assert(type == XPC_TYPE_STRING);
    if (type != XPC_TYPE_STRING) {
      return QString();
    }
    size_t len = xpc_string_get_length(event);
    if (len == 0) {
      return QString();
    }
    const char* str_ptr = xpc_string_get_string_ptr(event);
    if (str_ptr == nullptr) {
      return QString();
    }
    // This will copy the string into a new QString.
    QString owned_string = QString::fromLocal8Bit(str_ptr, len);
    return owned_string;
  }

  static QString getXPCError(xpc_object_t event) {
    xpc_type_t type = xpc_get_type(event);
    if (type != XPC_TYPE_ERROR) {
      Q_ASSERT(false);  // "getXPCError should not be called on a non error"
      return QString();
    }
    if (event == XPC_ERROR_CONNECTION_INVALID) {
      // The client process on the other end of the connection has either
      // crashed or cancelled the connection.
      return "xpc_peer_connection invalid";
    }
    if (event == XPC_ERROR_TERMINATION_IMMINENT) {
      return "xpc_peer_connection terminated";
    }
    if (__builtin_available(macOS 12.0, *)) {
      if (event == XPC_ERROR_PEER_CODE_SIGNING_REQUIREMENT) {
        return "XPC_ERROR_PEER_CODE_SIGNING_REQUIREMENT failed";
      }
    }
    return "Unknown Error";
  }
};
