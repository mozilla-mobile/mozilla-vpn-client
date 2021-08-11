#ifndef CONTROLLERCAPABILITIES_H
#  define CONTROLLERCAPABILITIES_H

#  include <QObject>
#  include <QString>

/*
 * This Model defines which capabilitie a controller has.
 */
class ControllerCapabilities : public QObject {
  Q_OBJECT

 public:
  // Serialises the Obj
  QJsonObject serialize();
  // Takes json and populates properties
  void read(const QJsonObject& obj);

  ControllerCapabilities() {}
  ControllerCapabilities(const ControllerCapabilities& other);
  ControllerCapabilities& operator=(const ControllerCapabilities& other);

  // Each capability is defined by it's supportlevel
  enum SupportLevel {
    // Feature is supported and ready to use
    Supported = 1,
    // Not known yet, might happen if the supportlevel is checked
    // before the controller is initialised
    Unknown = 0,
    // Feature is supported but currently unavailable
    // e.g: "failed to communicate with the windows-split tunnel driver"
    Deactivated = -1,
    // Feature is not supported by Controller / OS
    Unsupported = -2,
  };
  Q_ENUM(SupportLevel);

#  define Capability(name, defaultValue) \
    _cap(name, m_##name, is##name##Supported, defaultValue)

#  define _cap(name, m_X, isXsupported, def) \
    Q_PROPERTY(SupportLevel name MEMBER m_X) \
    SupportLevel m_X = Unknown;              \
    bool isXsupported() const { return m_X == SupportLevel::Supported; }

  // Note: When adding a Capability please
  // make sure to update the copy constructor :)

#  if defined(MVPN_ANDROID)
  // Supported by default
  Capability(SplitTunnel, Supported);
#  elif defined(MVPN_IOS) || defined(MVPN_OSX)
  // Definitely not Supported here
  Capability(SplitTunnel, Unsupported);
#  else
  // Unkown, controller will tell.
  Capability(SplitTunnel, Unknown);
#endif

#    undef Capability
#    undef _cap
};

#  endif  // CONTROLLERCAPABILITIES_H
