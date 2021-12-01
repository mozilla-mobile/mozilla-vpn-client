#ifndef CRASHCONSTANTS_H
#define CRASHCONSTANTS_H

#include <qsystemdetection.h>

#ifdef Q_OS_WIN
#define IPC_NAME L"\\\\.\\pipe\\MozillaVPN_crash"
#endif

#endif // CRASHCONSTANTS_H
