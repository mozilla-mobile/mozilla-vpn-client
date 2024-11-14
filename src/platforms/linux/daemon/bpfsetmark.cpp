/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bpfsetmark.h"

#include <linux/bpf.h>
#include <linux/filter.h>
#include <bpf/bpf.h>
#include <fcntl.h>
#include <unistd.h>

#include <QScopeGuard>

#include "leakdetector.h"
#include "../linuxdependencies.h"
#include "logger.h"

namespace {
Logger logger("BpfSetMark");
}  // namespace

BpfSetMark::BpfSetMark(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(BpfSetMark);
  logger.debug() << "BpfSetMark created";

  m_cgroupMount = LinuxDependencies::findCgroup2Path();
  if (m_cgroupMount.isEmpty()) {
    logger.debug() << "cgroups2 not found";
    return;
  }

  m_program = loadProgram();
}

BpfSetMark::~BpfSetMark() {
  MZ_COUNT_DTOR(BpfSetMark);
  logger.debug() << "BpfSetMark destroyed";
}

// Load the BPF traffic marking program
int BpfSetMark::loadProgram() {
  // Basic BPF program to set the firewall mark:
  //   r2 = fwmark
  //   sock->mark = r2
  //   r1 = verdict
  //   return
  short int offset = offsetof(struct bpf_sock, mark);
  struct bpf_insn program[] = {
    { BPF_ALU64 | BPF_MOV | BPF_K, BPF_REG_2, 0, 0, 0xca6c},
    { BPF_STX | BPF_W | BPF_MEM, BPF_REG_1, BPF_REG_2, offset, 0},
    { BPF_ALU64 | BPF_MOV | BPF_K, BPF_REG_0, 0, 0, 1},
    { BPF_JMP | BPF_EXIT, 0, 0, 0, 0},
  };
  size_t count = sizeof(program) / sizeof(struct bpf_insn);

  char *bpf_log_buf = (char *)malloc(BPF_LOG_BUF_SIZE);
  auto guard = qScopeGuard([bpf_log_buf]() { free(bpf_log_buf); });
  LIBBPF_OPTS(bpf_prog_load_opts, opts,
    .log_size = BPF_LOG_BUF_SIZE,
    .log_buf = bpf_log_buf,
  );

  // Load the program and return and the program file desciptor.
  int ret = bpf_prog_load(BPF_PROG_TYPE_CGROUP_SOCK, nullptr,
                          "Dual MPL/GPL", program, count, &opts);
  if (ret < 0) {
    logger.debug() << "BPF load failed:" << strerror(errno);
    logger.debug() << "BPF logs:" << bpf_log_buf;
  }
  return ret;
}

// Attach the BPF traffic marking program to the desired control group.
void BpfSetMark::attachCgroup(const QString& cgroup) {
  if (m_cgroupMount.isEmpty() || (m_program < 0)) {
    return;
  }
  QString path = m_cgroupMount + '/' + cgroup;
  logger.debug() << "Attach BPF to:" << path;

  int cgfd = open(path.toLocal8Bit().data(), O_DIRECTORY | O_RDONLY);
  if (cgfd < 0) {
    logger.warning() << "Failed to open cgroup:" << strerror(errno);
    return;
  }
  auto guard = qScopeGuard([cgfd]() { close(cgfd); });

  int ret = bpf_prog_attach(m_program, cgfd, BPF_CGROUP_INET_SOCK_CREATE, 0);
  if (ret < 0) {
    logger.warning() << "Failed to attach eBPF:" << strerror(errno);
  }
}

// Detach the BPF traffic marking program from the desired control group.
void BpfSetMark::detachCgroup(const QString& cgroup) {
  if (m_cgroupMount.isEmpty() || (m_program < 0)) {
    return;
  }
  QString path = m_cgroupMount + '/' + cgroup;
  logger.debug() << "Dropping BPF from:" << path;

  int cgfd = open(path.toLocal8Bit().data(), O_DIRECTORY | O_RDONLY);
  if (cgfd < 0) {
    logger.warning() << "Failed to open cgroup:" << strerror(errno);
  }
  auto guard = qScopeGuard([cgfd]() { close(cgfd); });

  int ret = bpf_prog_detach2(m_program, cgfd, BPF_CGROUP_INET_SOCK_CREATE);
  if (ret < 0) {
    logger.warning() << "Failed to attach eBPF:" << strerror(errno);
  }
}

// Detach the BPF traffic marking program from all control groups.
void BpfSetMark::detachAll() {
  // TODO: Implement Me!
}
