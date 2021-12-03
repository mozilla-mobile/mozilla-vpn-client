#include "crashclient.h"


#include <QCoreApplication>
#include <QString>
#include <vector>

using namespace crashpad;
using namespace std;

CrashClient::CrashClient()
{
    m_client = make_unique<CrashpadClient>();
}

bool CrashClient::start(){
    QString handlerPath = QCoreApplication::applicationFilePath();
    vector<string> args;
    args.push_back("--crashreporter");
    base::FilePath db;
    base::FilePath handler(handlerPath.toStdWString());
    if(!m_client->StartHandler(handler, db, db, "", {}, args, true, true)){
        return false;
    }
    return m_client->WaitForHandlerStart(INFINITE);
}
