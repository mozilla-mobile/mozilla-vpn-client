#include "crashreporter.h"

CrashReporter::CrashReporter(QObject *parent) : QObject(parent)
{

}

bool CrashReporter::shouldPromptUser(){
    //eventually this will need to check settings for an opt-in.  For now we always ask
    return true;
}
