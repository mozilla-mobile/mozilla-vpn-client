#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include <QObject>

class MozillaVPN : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ getState NOTIFY stateChanged)

public:
    explicit MozillaVPN(QObject *parent = nullptr) : QObject(parent) {}

    void initialize();

    QString getState() const { return m_state; }

    Q_INVOKABLE void authenticate();

    Q_INVOKABLE void openLink(const QString &linkName);

signals:
    void stateChanged();

private:
    QString m_state;
};

#endif // MOZILLAVPN_H
