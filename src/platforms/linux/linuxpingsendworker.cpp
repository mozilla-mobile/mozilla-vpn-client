/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxpingsendworker.h"

#include <QDebug>
#include <QSocketNotifier>

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <unistd.h>

void LinuxPingSendWorker::sendPing(const QString &destination)
{
    qDebug() << "LinuxPingSendWorker - start" << destination;

    struct in_addr dst;
    if (inet_aton(destination.toLocal8Bit().constData(), &dst) == 0) {
        qDebug() << "Lookup error";
        emit pingFailed();
        return;
    }

    Q_ASSERT(m_socket == 0);
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (m_socket < 0) {
        qDebug() << "Socket creation error";
        emit pingFailed();
        releaseObjects();
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr = dst;

    struct icmphdr packet;
    memset(&packet, 0, sizeof packet);
    packet.type = ICMP_ECHO;

    int rc = sendto(m_socket, &packet, sizeof packet, 0, (struct sockaddr *) &addr, sizeof addr);
    if (rc <= 0) {
        qDebug() << "Sending ping failed";
        emit pingFailed();
        releaseObjects();
        return;
    }

    qDebug() << "Ping sent";

    m_socketNotifier = new QSocketNotifier(m_socket, QSocketNotifier::Read, this);
    connect(m_socketNotifier,
            &QSocketNotifier::activated,
            [this](
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
                QSocketDescriptor socket, QSocketNotifier::Type
#else
                int socket
#endif
            ) {
                socklen_t slen = 0;
                unsigned char data[2048];
                int rc = recvfrom(socket, data, sizeof data, 0, NULL, &slen);
                if (rc <= 0) {
                    qDebug() << "Recvfrom failed";
                    emit pingFailed();
                    releaseObjects();
                    return;
                }

                qDebug() << "Ping reply received";
                emit pingSucceeded();
                releaseObjects();
            });
}

void LinuxPingSendWorker::stopPing()
{
    qDebug() << "LinuxPingSendWorker - stopped";
    releaseObjects();
}

void LinuxPingSendWorker::releaseObjects()
{
    if (m_socket > 0) {
        close(m_socket);
        m_socket = 0;
    }
    m_socket = 0;

    if (m_socketNotifier) {
        delete m_socketNotifier;
        m_socketNotifier = nullptr;
    }
}
