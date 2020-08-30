#ifndef LINUXPINGSENDER_H
#define LINUXPINGSENDER_H

#include "pingsender.h"

class LinuxPingSender : public PingSender
{
public:
    LinuxPingSender(QObject *parent) : PingSender(parent) {}

    void send(const QString &gateway) override;

    void stop() override;
};

#endif // LINUXPINGSENDER_H
