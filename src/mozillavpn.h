/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include "captiveportal/captiveportaldetection.h"
#include "connectiondataholder.h"
#include "connectionhealth.h"
#include "controller.h"
#include "errorhandler.h"
#include "localizer.h"
#include "models/devicemodel.h"
#include "models/helpmodel.h"
#include "models/keys.h"
#include "models/servercountrymodel.h"
#include "models/serverdata.h"
#include "models/user.h"
#include "releasemonitor.h"
#include "settingsholder.h"
#include "statusicon.h"

#include <QList>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QStandardPaths>
#include <QTimer>

class QQmlApplicationEngine;
class Task;

#ifdef UNIT_TEST
class TestTasks;
#endif

class MozillaVPN final : public QObject
{
    Q_OBJECT

public:
    enum State {
        StateInitialize,
        StateAuthenticating,
        StatePostAuthentication,
        StateMain,
        StateUpdateRequired,
        StateSubscriptionNeeded,
    };
    Q_ENUM(State);

    enum AlertType {
        NoAlert,
        AuthenticationFailedAlert,
        ConnectionFailedAlert,
        LogoutAlert,
        NoConnectionAlert,
        BackendServiceErrorAlert,
    };
    Q_ENUM(AlertType)

    enum LinkType {
        LinkAccount,
        LinkContact,
        LinkFeedback,
        LinkHelpSupport,
        LinkTermsOfService,
        LinkPrivacyPolicy,
        LinkUpdate,
    };
    Q_ENUM(LinkType)

private:
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(AlertType alert READ alert NOTIFY alertChanged)
    Q_PROPERTY(QString versionString READ versionString CONSTANT)
    Q_PROPERTY(bool updateRecommended READ updateRecommended NOTIFY updateRecommendedChanged)
    Q_PROPERTY(bool userAuthenticated READ userAuthenticated NOTIFY userAuthenticationChanged)
    Q_PROPERTY(bool startMinimized READ startMinimized CONSTANT)

public:
    static void createInstance(QObject *parent, QQmlApplicationEngine *engine, bool startMinimized);
    static MozillaVPN *instance();

    State state() const;
    AlertType alert() const { return m_alert; }

    const QString &getApiUrl() const { return m_apiUrl; }
    const QString &token() const { return m_token; }

    // Exposed QML methods:
    Q_INVOKABLE void authenticate();
    Q_INVOKABLE void cancelAuthentication();
    Q_INVOKABLE void openLink(LinkType linkType);
    Q_INVOKABLE void removeDevice(const QString &deviceName);
    Q_INVOKABLE void hideAlert() { setAlert(NoAlert); }
    Q_INVOKABLE void hideUpdateRecommendedAlert() { setUpdateRecommended(false); }
    Q_INVOKABLE void postAuthenticationCompleted();
    Q_INVOKABLE void subscribe();
    Q_INVOKABLE void viewLogs();
    Q_INVOKABLE QString retrieveLogs();

    // QML object getters:
    QQmlApplicationEngine *engine() { return m_engine; }

    // Internal object getters:
    CaptivePortalDetection *captivePortalDetection()
    {
        return &m_private->m_captivePortalDetection;
    }
    ConnectionDataHolder *connectionDataHolder() { return &m_private->m_connectionDataHolder; }
    ConnectionHealth *connectionHealth() { return &m_private->m_connectionHealth; }
    Controller *controller() { return &m_private->m_controller; }
    ServerData *currentServer() { return &m_private->m_serverData; }
    DeviceModel *deviceModel() { return &m_private->m_deviceModel; }
    const Keys *keys() const { return &m_private->m_keys; }
    HelpModel *helpModel() { return &m_private->m_helpModel; }
    Localizer *localizer() { return &m_private->m_localizer; }
    ServerCountryModel *serverCountryModel() { return &m_private->m_serverCountryModel; }
    SettingsHolder *settingsHolder() { return &m_private->m_settingsHolder; }
    StatusIcon *statusIcon() { return &m_private->m_statusIcon; }
    User *user() { return &m_private->m_user; }

    // Called at the end of the authentication flow. We can continue adding the device
    // if it doesn't exist yet, or we can go to OFF state.
    void authenticationCompleted(const QByteArray &json, const QString &token);

    void deviceAdded(const QString &deviceName, const QString &publicKey, const QString &privateKey);

    void deviceRemoved(const QString &deviceName);

    void serversFetched(const QByteArray &serverData);

    void accountChecked(const QByteArray &json);

    const QList<Server> getServers() const;

    void errorHandle(ErrorHandler::ErrorType error);

    void changeServer(const QString &countryCode, const QString &city);

    const QString versionString() const { return QString(APP_VERSION); }

    void logout();

    bool updateRecommended() const { return m_updateRecommended; }

    void setUpdateRecommended(bool value);

    bool userAuthenticated() const { return m_userAuthenticated; }

    bool startMinimized() const { return m_startMinimized; }

    void setToken(const QString &token);

    [[nodiscard]] bool setServerList(const QByteArray& serverData);

    QNetworkAccessManager *networkAccessManager();

    void showWindow();

private:
    MozillaVPN(QObject *parent, QQmlApplicationEngine *engine, bool startMinimized);
    ~MozillaVPN();

    static void deleteInstance();

    void initialize();

    void setState(State state);

    void scheduleTask(Task *task);
    void maybeRunTask();

    void setUserAuthenticated(bool state);

    void startSchedulingPeriodicOperations();

    void stopSchedulingPeriodicOperations();

    void setAlert(AlertType alert);

    bool writeAndShowLogs(QStandardPaths::StandardLocation location);

    bool writeLogs(QStandardPaths::StandardLocation location,
                   std::function<void(const QString &filename)> &&a_callback);

    bool modelsInitialized() const;

    void maybeActivateController();

public slots:
    void requestSettings();
    void requestAbout();
    void requestViewLogs();

private slots:
    void taskCompleted();

signals:
    void stateChanged();
    void alertChanged();
    void updateRecommendedChanged();
    void userAuthenticationChanged();
    void deviceRemoving(const QString& deviceName);
    void settingsNeeded();
    void aboutNeeded();
    void viewLogsNeeded();

private:
    // QML objects.
    QQmlApplicationEngine *m_engine = nullptr;

    // Internal objects.
    struct Private
    {
        CaptivePortalDetection m_captivePortalDetection;
        ConnectionDataHolder m_connectionDataHolder;
        ConnectionHealth m_connectionHealth;
        Controller m_controller;
        DeviceModel m_deviceModel;
        Keys m_keys;
        HelpModel m_helpModel;
        Localizer m_localizer;
        ReleaseMonitor m_releaseMonitor;
        ServerCountryModel m_serverCountryModel;
        ServerData m_serverData;
        SettingsHolder m_settingsHolder;
        StatusIcon m_statusIcon;
        User m_user;
    };

    Private *m_private = nullptr;

    // Task handling.
    QList<QPointer<Task>> m_tasks;
    bool m_task_running = false;

    QString m_apiUrl;
    QString m_token;

    State m_state = StateInitialize;
    AlertType m_alert = NoAlert;

    QTimer m_alertTimer;
    QTimer m_periodicOperationsTimer;

    bool m_updateRecommended = false;
    bool m_userAuthenticated = false;
    bool m_startMinimized = false;

#ifdef UNIT_TEST
   friend class TestTasks;
#endif
};

#endif // MOZILLAVPN_H
