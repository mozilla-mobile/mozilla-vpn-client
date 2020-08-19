#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include "servercountrymodel.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QSettings>

class Task;
class UserData;

class MozillaVPN final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ getState NOTIFY stateChanged)
    Q_PROPERTY(QAbstractListModel *serverCountryModel READ serverCountryModel NOTIFY
                   serverCountryModelChanged)

public:
    explicit MozillaVPN(QObject *parent = nullptr);
    ~MozillaVPN();

    void initialize(int &argc, char *argv[]);

    QString getState() const { return m_state; }

    const QString &getApiUrl() const { return m_apiUrl; }

    Q_INVOKABLE void authenticate();

    Q_INVOKABLE void openLink(const QString &linkName);

    Q_INVOKABLE void activate();

    // Called at the end of the authentication flow. We can continue adding the device
    // if it doesn't exist yet, or we can go to OFF state.
    void authenticationCompleted(UserData *userData, const QString &token);

    // The device has been added.
    void deviceAdded(const QString &deviceName, const QString &publicKey, const QString &privateKey);

    void deviceRemoved(const QString &deviceName);

    void serversFetched(const QByteArray &serverData);

    QString token() const { return m_token; }

    QPointer<UserData> userData() const { return m_userData; }

    QAbstractListModel *serverCountryModel() { return &m_serverCountryModel; }

private:
    void scheduleTask(Task* task);
    void maybeRunTask();

    void scheduleServersFetch();

signals:
    void stateChanged();
    void serverCountryModelChanged();

private:
    QSettings m_settings;

    QString m_token;
    QPointer<UserData> m_userData;

    ServerCountryModel m_serverCountryModel;

    QList<QPointer<Task>> m_tasks;
    bool m_task_running = false;

    QString m_state;
    QString m_apiUrl;
};

#endif // MOZILLAVPN_H
