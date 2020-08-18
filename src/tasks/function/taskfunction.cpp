#include "taskfunction.h"

void TaskFunction::run(MozillaVPN *vpn)
{
    m_callback(vpn);
}
