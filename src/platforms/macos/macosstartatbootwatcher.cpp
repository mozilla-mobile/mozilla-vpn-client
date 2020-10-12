#include "macosstartatbootwatcher.h"
#include "macosutils.h"

#include <QDebug>

MacOSStartAtBootWatcher::MacOSStartAtBootWatcher(bool startAtBoot)
{
    qDebug() << "StartAtBoot watcher";
    MacOSUtils::enableLoginItem(startAtBoot);
}

void MacOSStartAtBootWatcher::startAtBootChanged(bool startAtBoot)
{
    qDebug() << "StartAtBoot changed:" << startAtBoot;
    MacOSUtils::enableLoginItem(startAtBoot);
}
