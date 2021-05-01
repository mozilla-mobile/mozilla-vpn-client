/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pidtracker.h"
#include "leakdetector.h"
#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>

constexpr size_t CN_MCAST_MSG_SIZE =
    sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op);

namespace {
Logger logger(LOG_LINUX, "PidTracker");
}

PidTracker::PidTracker(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(PidTracker);
  logger.log() << "PidTracker created.";
}

PidTracker::~PidTracker() {
  MVPN_COUNT_DTOR(PidTracker);
  logger.log() << "PidTracker destroyed.";

  if (nlsock > 0) {
    close(nlsock);
  }
}

void PidTracker::initialize() {
  logger.log() << "initialize";

  nlsock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
  if (nlsock < 0) {
    logger.log() << "Failed to create netlink socket:" << strerror(errno);
    return;
  }

  struct sockaddr_nl nladdr;
  nladdr.nl_family = AF_NETLINK;
  nladdr.nl_groups = CN_IDX_PROC;
  nladdr.nl_pid = getpid();
  nladdr.nl_pad = 0;
  if (bind(nlsock, (struct sockaddr*)&nladdr, sizeof(nladdr)) < 0) {
    logger.log() << "Failed to bind netlink socket:" << strerror(errno);
    close(nlsock);
    nlsock = -1;
    return;
  }

  char buf[NLMSG_SPACE(CN_MCAST_MSG_SIZE)];
  struct nlmsghdr* nlmsg = (struct nlmsghdr*)buf;
  struct cn_msg* cnmsg = (struct cn_msg*)NLMSG_DATA(nlmsg);
  enum proc_cn_mcast_op mcast_op = PROC_CN_MCAST_LISTEN;

  memset(buf, 0, sizeof(buf));
  nlmsg->nlmsg_len = NLMSG_LENGTH(CN_MCAST_MSG_SIZE);
  nlmsg->nlmsg_type = NLMSG_DONE;
  nlmsg->nlmsg_flags = 0;
  nlmsg->nlmsg_seq = 0;
  nlmsg->nlmsg_pid = getpid();

  cnmsg->id.idx = CN_IDX_PROC;
  cnmsg->id.val = CN_VAL_PROC;
  cnmsg->seq = 0;
  cnmsg->ack = 0;
  cnmsg->len = sizeof(mcast_op);
  memcpy(cnmsg->data, &mcast_op, sizeof(mcast_op));

  if (send(nlsock, nlmsg, sizeof(buf), 0) != sizeof(buf)) {
    logger.log() << "Failed to send netlink message:" << strerror(errno);
    close(nlsock);
    nlsock = -1;
    return;
  }

  m_socket = new QSocketNotifier(nlsock, QSocketNotifier::Read, this);
  connect(m_socket, SIGNAL(activated(QSocketDescriptor, QSocketNotifier::Type)),
          SLOT(readData()));
}

void PidTracker::track(const QString& name, int rootpid) {
  if (m_processTree.contains(rootpid)) {
    logger.log() << "Ignoring attempt to track duplicate PID";
    return;
  }
  ProcessGroup* group = new ProcessGroup(name, rootpid);
  group->refcount = 1;

  m_processGroups.append(group);
  m_processTree[rootpid] = group;
}

void PidTracker::handleProcEvent(struct cn_msg* cnmsg) {
  struct proc_event* ev = (struct proc_event*)cnmsg->data;

  if (ev->what == proc_event::PROC_EVENT_FORK) {
    ProcessGroup* group = m_processTree[ev->event_data.fork.parent_pid];
    if (!group) {
      return;
    }
    m_processTree[ev->event_data.fork.child_pid] = group;
    group->refcount++;
    emit pidForked(group->name, ev->event_data.fork.parent_pid,
                   ev->event_data.fork.child_pid);
  }

  if (ev->what == proc_event::PROC_EVENT_EXIT) {
    ProcessGroup* group = m_processTree[ev->event_data.exit.process_pid];
    if (!group) {
      return;
    }
    emit pidExited(group->name, ev->event_data.exit.process_pid);
    m_processTree.remove(ev->event_data.exit.process_pid);

    Q_ASSERT(group->refcount > 0);
    group->refcount--;
    if (group->refcount == 0) {
      emit terminated(group->name, group->rootpid);
      m_processGroups.removeAll(group);
      delete group;
    }
  }
}

void PidTracker::readData() {
  struct sockaddr_nl src;
  socklen_t srclen = sizeof(src);
  size_t recvlen;

  recvlen = recvfrom(nlsock, readBuf, sizeof(readBuf), MSG_DONTWAIT,
                     (struct sockaddr*)&src, &srclen);
  if (recvlen == ENOBUFS) {
    logger.log()
        << "Failed to read netlink socket: buffer full, message dropped";
    return;
  }
  if (recvlen < 0) {
    logger.log() << "Failed to read netlink socket:" << strerror(errno);
    return;
  }
  if (srclen != sizeof(src)) {
    logger.log() << "Failed to read netlink socket: invalid address length";
    return;
  }

  /* We are only interested in process-control messages from the kernel */
  if ((src.nl_groups != CN_IDX_PROC) || (src.nl_pid != 0)) {
    return;
  }

  /* Handle the process-control messages. */
  struct nlmsghdr* msg;
  for (msg = (struct nlmsghdr*)readBuf; NLMSG_OK(msg, recvlen);
       msg = NLMSG_NEXT(msg, recvlen)) {
    struct cn_msg* cnmsg = (struct cn_msg*)NLMSG_DATA(msg);
    if (msg->nlmsg_type == NLMSG_NOOP) {
      continue;
    }
    if ((msg->nlmsg_type == NLMSG_ERROR) ||
        (msg->nlmsg_type == NLMSG_OVERRUN)) {
      break;
    }
    handleProcEvent(cnmsg);
    if (msg->nlmsg_type == NLMSG_DONE) {
      break;
    }
  }
}
