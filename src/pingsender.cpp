#include "pingsender.h"
#include "pingsendworker.h"

#ifdef __linux__
#include "platforms/linux/linuxpingsendworker.h"
#else
#include "platforms/dummy/dummypingsendworker.h"
#endif

#include <QDebug>

PingSender::PingSender(QObject *parent) : QObject(parent)
{
    PingSendWorker *worker =
#ifdef __linux__
        new LinuxPingSendWorker();
#else
        new DummyPingSendWorker();
#endif
    worker->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &PingSender::sendPing, worker, &PingSendWorker::sendPing);
    connect(this, &PingSender::stopPing, worker, &PingSendWorker::stopPing);
    connect(worker, &PingSendWorker::pingFailed, this, &PingSender::pingFailed);
    connect(worker, &PingSendWorker::pingSucceeded, this, &PingSender::pingSucceeded);

    m_workerThread.start();
}

PingSender::~PingSender()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

void PingSender::send(const QString &destination)
{
    qDebug() << "PingSender send to" << destination;

    m_active = true;
    emit sendPing(destination);
}

void PingSender::stop()
{
    qDebug() << "PingSender stop";
    m_active = false;
    emit stopPing();
}

void PingSender::pingFailed()
{
    qDebug() << "PingSender - Ping Failed";

    if (m_active) {
        emit completed();
    }
}

void PingSender::pingSucceeded()
{
    qDebug() << "PingSender - Ping Succeded";
    if (m_active) {
        emit completed();
    }
}
