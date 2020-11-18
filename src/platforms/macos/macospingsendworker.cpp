/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macospingsendworker.h"
#include "logger.h"

#include <QSocketNotifier>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>

namespace {

Logger logger({LOG_MACOS, LOG_NETWORKING}, "MacOSPingSendWorker");

int identifier()
{
    return (getpid() & 0xFFFF);
}

// From ping.c implementation:
u_short in_cksum(u_short *addr, int len)
{
    int nleft, sum;
    u_short *w;
    union {
        u_short us;
        u_char uc[2];
    } last;
    u_short answer;

    nleft = len;
    sum = 0;
    w = addr;

    /*
         * Our algorithm is simple, using a 32 bit accumulator (sum), we add
         * sequential 16 bit words to it, and at the end, fold back all the
         * carry bits from the top 16 bits into the lower 16 bits.
         */
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1) {
        last.uc[0] = *(u_char *) w;
        last.uc[1] = 0;
        sum += last.us;
    }

    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);                 /* add carry */
    answer = ~sum;                      /* truncate to 16 bits */
    return (answer);
}

}; // namespace

void MacOSPingSendWorker::sendPing(const QString &destination)
{
    logger.log() << "MacOSPingSendWorker - sending ping to:" << destination;

    Q_ASSERT(m_socket == 0);

    if (getuid()) {
        m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    } else {
        m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    }

    if (m_socket < 0) {
        logger.log() << "Socket creation failed";
        emit pingFailed();
        releaseObjects();
        return;
    }

    struct sockaddr_in dst;
    bzero(&dst, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_len = sizeof(dst);

    if (inet_aton(destination.toLocal8Bit().constData(), &dst.sin_addr) == 0) {
        logger.log() << "DNS lookup failed";
        emit pingFailed();
        releaseObjects();
        return;
    }

    struct icmp packet;
    bzero(&packet, sizeof packet);
    packet.icmp_type = ICMP_ECHO;
    packet.icmp_id = identifier();
    packet.icmp_cksum = in_cksum((u_short *) &packet, sizeof(packet));

    if (sendto(m_socket, (char *) &packet, sizeof(packet), 0, (struct sockaddr *) &dst, sizeof(dst))
        != sizeof(packet)) {
        logger.log() << "Package sending failed";
        emit pingFailed();
        releaseObjects();
        return;
    }

    logger.log() << "Ping sent";

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
                    logger.log() << "Recvmsg failed";
                    emit pingFailed();
                    releaseObjects();
                    return;
                }

                struct ip *ip = (struct ip *) packet;
                int hlen = ip->ip_hl << 2;
                struct icmp *icmp = (struct icmp *) (((char *) packet) + hlen);

                if (icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_id == identifier()) {
                    logger.log() << "Ping reply received";
                    emit pingSucceeded();
                    releaseObjects();
                }
            });
}

void MacOSPingSendWorker::releaseObjects()
{
    if (m_socket > 0) {
        close(m_socket);
    }
    m_socket = 0;

    if (m_socketNotifier) {
        delete m_socketNotifier;
        m_socketNotifier = nullptr;
    }
}
