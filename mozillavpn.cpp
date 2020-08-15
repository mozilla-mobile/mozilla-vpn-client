#include "mozillavpn.h"
#include <QDebug>

constexpr const char *STATE_INITIALIZE = "INITIALIZE";
constexpr const char *STATE_CONNECTING = "CONNECTING";

void MozillaVPN::initialize()
{
    qDebug() << "MozillaVPN Initialization";

    // TODO: read the config file
    m_state = STATE_INITIALIZE;
}

void MozillaVPN::authenticate()
{
    qDebug() << "Authenticate";

    m_state = STATE_CONNECTING;
    emit stateChanged();
}

void MozillaVPN::openLink(const QString &linkName)
{
    qDebug() << "Opening link: " << linkName;
    // TODO
}
