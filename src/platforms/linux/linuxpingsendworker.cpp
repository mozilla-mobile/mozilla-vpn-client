#include "linuxpingsendworker.h"

#include <QDebug>
#include <QSocketNotifier>

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/select.h>
#include <sys/socket.h>

void LinuxPingSendWorker::sendPing(const QString &destination)
{
    qDebug() << "LinuxPingSendWorker - start" << destination;

    struct in_addr dst;
    if (inet_aton(destination.toLocal8Bit().constData(), &dst) == 0) {
        qDebug() << "Lookup error";
        emit pingFailed();
        return;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (sock < 0) {
        qDebug() << "Socket creation error";
        emit pingFailed();
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr = dst;

    struct icmphdr icmp_hdr;
    memset(&icmp_hdr, 0, sizeof icmp_hdr);
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.un.echo.id = 0; // TODO

    icmp_hdr.un.echo.sequence = 0;

    unsigned char data[2048];
    memcpy(data, &icmp_hdr, sizeof icmp_hdr);
    memcpy(data + sizeof icmp_hdr, "mozillaVPN", 11); //icmp payload
    int rc = sendto(sock, data, sizeof icmp_hdr + 11, 0, (struct sockaddr *) &addr, sizeof addr);
    if (rc <= 0) {
        qDebug() << "Sending ping failed";
        emit pingFailed();
        return;
    }

    qDebug() << "Ping sent";

    fd_set read_set;
    memset(&read_set, 0, sizeof read_set);
    FD_SET(sock, &read_set);

    m_socketNotifier = new QSocketNotifier(sock, QSocketNotifier::Read, this);
    connect(m_socketNotifier,
            &QSocketNotifier::activated,
            [this](QSocketDescriptor socket, QSocketNotifier::Type type) {
                Q_UNUSED(type);

                socklen_t slen = 0;
                unsigned char data[2048];
                int rc = recvfrom(socket, data, sizeof data, 0, NULL, &slen);
                if (rc <= 0) {
                    qDebug() << "Recvfrom failed";
                    emit pingFailed();
                    return;
                }

                qDebug() << "Ping reply received";
                emit pingSucceeded();
            });
}

void LinuxPingSendWorker::stopPing()
{
    qDebug() << "LinuxPingSendWorker - stopped";

    if (m_socketNotifier) {
        delete m_socketNotifier;
        m_socketNotifier = nullptr;
    }
}
