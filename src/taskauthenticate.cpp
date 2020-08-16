#include "taskauthenticate.h"
#include "mozillavpn.h"

#include <QDebug>
#include <QNetworkRequest>

void TaskAuthenticate::Run(MozillaVPN* aVPN)
{
    Q_ASSERT(aVPN);

    qDebug() << "TaskAuthenticate::Run";

    QNetworkRequest request;
    request.setUrl(QUrl(aVPN->getApiUrl()));
    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

    // TODO
    emit completed();
}
