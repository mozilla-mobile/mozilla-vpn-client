#include "linuxpingsender.h"
#include "linuxpingsendworker.h"

#include <QDebug>

LinuxPingSender::LinuxPingSender(QObject *parent) : PingSender(parent)
{
    LinuxPingSendWorker *worker = new LinuxPingSendWorker();
    worker->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &LinuxPingSender::sendPing, worker, &LinuxPingSendWorker::sendPing);
    connect(this, &LinuxPingSender::stopPing, worker, &LinuxPingSendWorker::stopPing);
    connect(worker, &LinuxPingSendWorker::pingFailed, this, &LinuxPingSender::pingFailed);
    connect(worker, &LinuxPingSendWorker::pingSucceeded, this, &LinuxPingSender::pingSucceeded);

    m_workerThread.start();
}

LinuxPingSender::~LinuxPingSender()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

void LinuxPingSender::send(const QString &destination)
{
    qDebug() << "LinuxPingSender send to" << destination;

    m_active = true;
    emit sendPing(destination);
}

void LinuxPingSender::stop()
{
    qDebug() << "LinuxPingSender stop";
    m_active = false;
    emit stopPing();
}

void LinuxPingSender::pingFailed()
{
    qDebug() << "LinuxPingSender - Ping Failed";

    if (m_active) {
        emit completed();
    }
}

void LinuxPingSender::pingSucceeded()
{
    qDebug() << "LinuxPingSender - Ping Succeded";
    if (m_active) {
        emit completed();
    }
}
