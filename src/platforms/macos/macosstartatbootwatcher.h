#ifndef MACOSSTARTATBOOTWATCHER_H
#define MACOSSTARTATBOOTWATCHER_H

#include <QObject>

class MacOSStartAtBootWatcher : public QObject
{
    Q_OBJECT

public:
    explicit MacOSStartAtBootWatcher(bool startAtBoot);

public Q_SLOTS:
    void startAtBootChanged(bool value);
};

#endif // MACOSSTARTATBOOTWATCHER_H
