#include "linuxpingsendworker.h"

#include <QDebug>
#include <QSocketNotifier>

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

int identifier()
{
    return (getpid() & 0xFFFF);
}

} // namespace

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
    packet.un.echo.id = identifier();

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
            [this](QSocketDescriptor socket, QSocketNotifier::Type type) {
                Q_UNUSED(type);
                struct msghdr msg;
                bzero(&msg, sizeof(msg));

                struct sockaddr_in addr;
                msg.msg_name = (caddr_t) &addr;
                msg.msg_namelen = sizeof(addr);

                struct iovec iov;
                msg.msg_iov = &iov;
                msg.msg_iovlen = 1;

                u_char packet[IP_MAXPACKET];
                iov.iov_base = packet;
                iov.iov_len = IP_MAXPACKET;

                int rc = recvmsg(socket, &msg, 0);
                if (rc <= 0) {
                    qDebug() << "Recvmsg failed";
                    emit pingFailed();
                    releaseObjects();
                    return;
                }

                struct ip *ip = (struct ip *) packet;
                int hlen = ip->ip_hl << 2;
                struct icmp *icmp = (struct icmp *) (((char *) packet) + hlen);

                if (icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_id == identifier()) {
                    qDebug() << "Ping reply received";
                    emit pingSucceeded();
                    releaseObjects();
                }
                /*
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
*/
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
