/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxnetworkwatcher.h"
#include "linuxnetworkwatcherworker.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_LINUX, "LinuxNetworkWatcher");
}

LinuxNetworkWatcher::LinuxNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(LinuxNetworkWatcher);

  m_thread.start();
}

LinuxNetworkWatcher::~LinuxNetworkWatcher() {
  MVPN_COUNT_DTOR(LinuxNetworkWatcher);

  delete m_worker;

  m_thread.quit();
  m_thread.wait();
}

void LinuxNetworkWatcher::initialize() {
  logger.log() << "initialize";

  m_worker = new LinuxNetworkWatcherWorker(&m_thread);

  connect(this, &LinuxNetworkWatcher::initializeInThread, m_worker,
          &LinuxNetworkWatcherWorker::initialize);

  connect(this, &LinuxNetworkWatcher::checkDevicesInThread, m_worker,
          &LinuxNetworkWatcherWorker::checkDevices);

  connect(m_worker, &LinuxNetworkWatcherWorker::unsecuredNetwork, this,
          &LinuxNetworkWatcher::unsecuredNetwork);

  emit initializeInThread();
}

void LinuxNetworkWatcher::start() {
  logger.log() << "actived";
  NetworkWatcherImpl::start();
  emit checkDevicesInThread();
}
