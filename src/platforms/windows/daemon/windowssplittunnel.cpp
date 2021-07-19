/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowssplittunnel.h"
#include "../windowscommons.h"
#include "../windowsservicemanager.h"
#include "logger.h"

#include <QNetworkInterface>
#define PSAPI_VERSION 2
#include <Windows.h>
#include <psapi.h>
#include <fwpmu.h>
#include <initguid.h>
#include <iphlpapi.h>
#include <QCoreApplication>
#include <QFileInfo>
#include <QScopeGuard>

namespace {
Logger logger(LOG_WINDOWS, "WindowsSplitTunnel");

// ID for the Mullvad Split-Tunnel Firewall Sublayer
DEFINE_GUID(ST_FW_WINFW_BASELINE_SUBLAYER_KEY,
    0xc78056ff, 0x2bc1, 0x4211, 0xaa, 0xdd, 0x7f, 0x35, 0x8d, 0xef, 0x20, 0x2d);
// ID for the Mullvad Split-Tunnel Sublayer Provider
DEFINE_GUID(ST_FW_PROVIDER_KEY,
    0xe2c114ee, 0xf32a, 0x4264, 0xa6, 0xcb, 0x3f, 0xa7, 0x99, 0x63, 0x56, 0xd9);
}

WindowsSplitTunnel::WindowsSplitTunnel(QObject* parent): QObject(parent)
{
    if(!isInstalled()){
         logger.log() << "Driver is not Installed, doing so";
         auto handle = installDriver();
         if(handle == INVALID_HANDLE_VALUE){
            WindowsCommons::windowsLog("Failed to install Driver");
            return;
         }
         logger.log() << "Driver installed";
         CloseServiceHandle(handle);
    }else{
        logger.log() << "Driver is installed";
    }
    initDriver();
}

WindowsSplitTunnel::~WindowsSplitTunnel(){
    CloseHandle(m_driver);
    uninstallDriver();
}

void WindowsSplitTunnel::initDriver()
{
    logger.log() << "Try to open Split Tunnel Driver";
    // Open the Driver Symlink
    m_driver = CreateFileW(DRIVER_SYMLINK, GENERIC_READ | GENERIC_WRITE,
                           0, nullptr, OPEN_EXISTING, 0, nullptr);;

    if(m_driver == INVALID_HANDLE_VALUE){
        WindowsCommons::windowsLog("Failed to open Driver: ");

        // If the handle is not present, try again after the serivce has started;
        auto driver_manager = WindowsServiceManager(DRIVER_SERVICE_NAME);
        QObject::connect(&driver_manager, &WindowsServiceManager::serviceStarted,this,&WindowsSplitTunnel::initDriver);
        driver_manager.startService();
        return;
    }

    logger.log() << "Connected to the Driver";
    // Reset Driver as it has wfp handles probably >:(

    if(!initSublayer()){
        logger.log() << "Init sublayer failed :(";
        return;
    }

    // We need to now check the state and init it, if required

    auto state = getState();
    if(state == STATE_UNKNOWN){
        logger.log() << "Cannot check if driver is initialized";
    }
    if(state >= STATE_INITIALIZED){
        logger.log() << "Driver already initialized: " << state;
        return;
    }
    DWORD bytesReturned;
    auto ok = DeviceIoControl(m_driver, IOCTL_INITIALIZE, nullptr, 0, nullptr, 0, &bytesReturned, nullptr);
    if(!ok){
        auto err = GetLastError();
        logger.log() << "Driver init failed err -" << err;
        logger.log() << "State:" << getState();

        return;
    }
    logger.log() << "Driver initialized" << getState();
}


void WindowsSplitTunnel::setRules(const QStringList& appPaths)
{
    auto state = getState();
    if(state != STATE_READY && state != STATE_RUNNING){
        logger.log() << "Driver is not in the right State to set Rules" << state;
        return;
    }

    logger.log() << "Pushing new Ruleset for Split-Tunnel " << state;
    auto config = generateAppConfiguration(appPaths);

    DWORD bytesReturned;
    auto ok = DeviceIoControl(m_driver,IOCTL_SET_CONFIGURATION,
        &config[0], (DWORD)config.size(), nullptr, 0, &bytesReturned,nullptr);
    if(!ok){
        auto err = GetLastError();
        WindowsCommons::windowsLog("Set Config Failed:");
        logger.log() << "Failed to set Config err code " << err;
        return;
    }
    logger.log() << "New Configuration applied: " << getState();
}

void WindowsSplitTunnel::start(){
    // To Start we need to send 2 things:
    // Network info (what is vpn what is network)
    logger.log() << "Starting SplitTunnel";
    DWORD bytesReturned;

    if(getState() == STATE_STARTED){
        logger.log() << "Driver needs Init Call";
        DWORD bytesReturned;
        auto ok = DeviceIoControl(m_driver, IOCTL_INITIALIZE, nullptr, 0, nullptr, 0, &bytesReturned, nullptr);
        if(!ok){
            logger.log() << "Driver init failed";
            return;
        }
    }

    // Process Info (what is running already)
    if( getState() == STATE_INITIALIZED){
        logger.log() << "State is Init, requires process config";
        auto config = generateProcessBlob();
        auto ok = DeviceIoControl(m_driver,IOCTL_REGISTER_PROCESSES,
            &config[0], (DWORD)config.size(), nullptr, 0, &bytesReturned,nullptr);
        if(!ok){
            logger.log() << "Failed to set Process Config";
            return;
        }
        logger.log() << "Set Process Config ok || new State:" << getState();
    }

    if( getState() == STATE_INITIALIZED){
         logger.log() << "Driver is still not ready after process list send";
         return;
    }
    logger.log() << "Driver is  ready || new State:" << getState();

    auto config = generateIPConfiguration();
    auto ok = DeviceIoControl(m_driver,IOCTL_REGISTER_IP_ADDRESSES,
        &config[0], (DWORD)config.size(), nullptr, 0, &bytesReturned,nullptr);
    if(!ok){
        logger.log() << "Failed to set Network Config";
        return;
    }
    logger.log() << "New Network Config Applied || new State:" << getState();

}

void WindowsSplitTunnel::stop()
{
    DWORD bytesReturned;
    auto ok = DeviceIoControl(m_driver,IOCTL_CLEAR_CONFIGURATION,
        nullptr, 0, nullptr, 0, &bytesReturned,nullptr);
    if(!ok){
        logger.log() << "Stopping Split tunnel not successfull";
        return;
    }
    logger.log() << "Stopping Split tunnel successfull";
}

void WindowsSplitTunnel::reset()
{
    DWORD bytesReturned;
    auto ok = DeviceIoControl(m_driver,IOCTL_ST_RESET,
        nullptr, 0, nullptr, 0, &bytesReturned,nullptr);
    if(!ok){
        logger.log() << "Reset Split tunnel not successfull";
        return;
    }
    logger.log() << "Reset Split tunnel successfull";
}

DRIVER_STATE WindowsSplitTunnel::getState(){
    if(m_driver == INVALID_HANDLE_VALUE){
         logger.log() << "Can't query State from non Opened Driver";
        return STATE_UNKNOWN;
    }
    DWORD bytesReturned;
    SIZE_T outBuffer;
    bool ok = DeviceIoControl(m_driver, IOCTL_GET_STATE,
        nullptr, 0, &outBuffer, sizeof(outBuffer), &bytesReturned,nullptr);
    if(!ok){
        WindowsCommons::windowsLog("getState response failure");
        return STATE_UNKNOWN;
    }
    if(bytesReturned == 0){
        WindowsCommons::windowsLog("getState response is empty");
        return STATE_UNKNOWN;
    }
    return static_cast<DRIVER_STATE>(outBuffer);
}


std::vector<uint8_t> WindowsSplitTunnel::generateAppConfiguration(const QStringList& appPaths){
    // Step 1: Calculate how much size the buffer will need
    size_t cummulated_string_size =0;
    QStringList dosPaths;
    for(auto const& path : appPaths){
       auto dosPath = convertPath(path);
       dosPaths.append(dosPath);
       cummulated_string_size += dosPath.toStdWString().size()* sizeof (wchar_t);
       logger.log() << dosPath;
    }
    size_t bufferSize = sizeof(CONFIGURATION_HEADER)
            + (sizeof(CONFIGURATION_ENTRY) * appPaths.size())
            + cummulated_string_size;
    std::vector<uint8_t> outBuffer(bufferSize);

    auto header = (CONFIGURATION_HEADER*)&outBuffer[0];
    auto entry = (CONFIGURATION_ENTRY*)(header + 1);

    auto stringDest = &outBuffer[0] + sizeof(CONFIGURATION_HEADER)
            + (sizeof(CONFIGURATION_ENTRY) * appPaths.size());

    SIZE_T stringOffset = 0;

    for (const QString &path : dosPaths)
    {
        auto wstr= path.toStdWString();
        auto cstr = wstr.c_str();
        auto stringLength = wstr.size() * sizeof(wchar_t);

        entry->ImageNameLength = (USHORT)stringLength;
        entry->ImageNameOffset = stringOffset;

        memcpy(stringDest, cstr ,stringLength);

        ++entry;
        stringDest += stringLength;
        stringOffset += stringLength;
    }

    header->NumEntries = appPaths.length();
    header->TotalLength = bufferSize;

    return outBuffer;
}


std::vector<uint8_t> WindowsSplitTunnel::generateIPConfiguration(){
    std::vector<uint8_t> out(sizeof(IP_ADDRESSES_CONFIG));

    auto config = reinterpret_cast<IP_ADDRESSES_CONFIG*>(&out[0]);
    /* How to Choose the Right Adapter:
     * Windows will has their own thing Called metric to rate each connection
     * and use the lowest-cost network adapter possible.
     *
     * So the VPN-Adapter starts with a metric of 0, making it the most optimal route
     * making it always the default.
     * So as the "outside" Connection we want the 2nd best metric.
     *
     * We can't get the exact number without using powershell but lucky us:
     * From msdn (GetAdaptersAddresses):
     * >Starting with Windows 10, the order in which adapters appear in
     * >the list is determined by the IPv4 or IPv6 route metric.
     *
     * So  QNetworkInterface::allInterfaces() is perfectly sorted for us.
     */

    auto ifaces = QNetworkInterface::allInterfaces();
    // Always the VPN
    getAddress(ifaces.at(0).index(), &config->TunnelIpv4, &config->TunnelIpv6);
    // 2nd best route
    getAddress(ifaces.at(1).index(), &config->InternetIpv4, &config->InternetIpv6);
    return out;
}
void WindowsSplitTunnel::getAddress(int adapterIndex, IN_ADDR* out_ipv4, IN6_ADDR* out_ipv6){
    QNetworkInterface target = QNetworkInterface::interfaceFromIndex(adapterIndex);
    logger.log() <<"Getting adapter info for:" << target.humanReadableName();


    // take the first v4/v6 Adress and convert to in_addr
    for(auto address: target.addressEntries()){
        if(address.ip().protocol() == QAbstractSocket::IPv4Protocol){
           auto adrr = address.ip().toString();
           std::wstring wstr = adrr.toStdWString();
           logger.log() <<"IpV4" << adrr;
           PCWSTR w_str_ip = wstr.c_str();
           auto ok = InetPtonW(AF_INET, w_str_ip, out_ipv4);
           if( ok != 1){
            logger.log() << "Ipv4 Conversation error" << WSAGetLastError();
           }
            break;
        }
    }
    for(auto address: target.addressEntries()){
        if(address.ip().protocol() == QAbstractSocket::IPv6Protocol){
            auto adrr = address.ip().toString();
            std::wstring wstr = adrr.toStdWString();
            logger.log() <<"IpV6" << adrr;
            PCWSTR w_str_ip = wstr.c_str();
            auto ok = InetPtonW(AF_INET6, w_str_ip, out_ipv6);
            if( ok != 1){
             logger.log() << "Ipv6 Conversation error" << WSAGetLastError();
            }
            break;
        }
    }


}


std::vector<uint8_t> WindowsSplitTunnel::generateProcessBlob(){

    // Get a Snapshot of all processes that are running:
    HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot_handle == INVALID_HANDLE_VALUE){
        WindowsCommons::windowsLog("Creating Process snapshot failed");
        return std::vector<uint8_t>(0);
    }
    // Load the First Entry, later iterate over all
    PROCESSENTRY32W currentProcess;
    currentProcess.dwSize = sizeof(PROCESSENTRY32W);

    if (FALSE ==(Process32First(snapshot_handle, &currentProcess)))
    {
        WindowsCommons::windowsLog("Cant read first entry");
    }

    QMap<DWORD, ProcessInfo> processes;

    do{
        auto process_handle = OpenProcess(
                    PROCESS_QUERY_LIMITED_INFORMATION, FALSE, currentProcess.th32ProcessID);

        if(process_handle == INVALID_HANDLE_VALUE){
            continue;
        }
        ProcessInfo info = getProcessInfo(process_handle,currentProcess);
        processes.insert(info.ProcessId,info);

    }while (FALSE != (Process32NextW(snapshot_handle, &currentProcess)));


    auto process_list = processes.values();
    if( process_list.isEmpty()){
        logger.log() << "Process Snapshot list was empty";
        return std::vector<uint8_t>(0);
    }

    logger.log() << "Reading Processes NUM: " << process_list.size();
    //Determine the Size of the outBuffer:
    size_t totalStringSize = 0;

    for (const auto &process : process_list)
    {
        totalStringSize += (process.DevicePath.size() * sizeof(wchar_t));
    }
    auto bufferSize = sizeof(PROCESS_DISCOVERY_HEADER)
        + (sizeof(PROCESS_DISCOVERY_ENTRY) * processes.size())
        + totalStringSize;

    std::vector<uint8_t> out(bufferSize);

    auto header = reinterpret_cast<PROCESS_DISCOVERY_HEADER*>(&out[0]);
    auto entry = reinterpret_cast<PROCESS_DISCOVERY_ENTRY*>(header + 1);
    auto stringBuffer = reinterpret_cast<uint8_t *>(entry + processes.size());

    SIZE_T currentStringOffset = 0;

    for (const auto &process : process_list)
    {
        // Wierd DWORD -> Handle Pointer magic.
        entry->ProcessId = (HANDLE)((size_t) process.ProcessId);
        entry->ParentProcessId =(HANDLE)((size_t) process.ParentProcessId);

        if (process.DevicePath.empty())
        {
            entry->ImageNameOffset = 0;
            entry->ImageNameLength = 0;
        }else{
            const auto imageNameLength = process.DevicePath.size() * sizeof(wchar_t);

            entry->ImageNameOffset = currentStringOffset;
            entry->ImageNameLength = static_cast<USHORT>(imageNameLength);

            RtlCopyMemory(stringBuffer + currentStringOffset, &process.DevicePath[0], imageNameLength);

            currentStringOffset += imageNameLength;
        }
        ++entry;
    }

    header->NumEntries = processes.size();
    header->TotalLength = bufferSize;

    return out;
}

void WindowsSplitTunnel::close(){
    CloseHandle(m_driver);
    m_driver = INVALID_HANDLE_VALUE;
}


ProcessInfo WindowsSplitTunnel::getProcessInfo(HANDLE process, const PROCESSENTRY32W& processMeta){

    ProcessInfo pi;
    pi.ParentProcessId = processMeta.th32ParentProcessID;
    pi.ProcessId = processMeta.th32ProcessID;
    pi.CreationTime = {0,0};
    pi.DevicePath = L"";

    FILETIME creationTime, null_time;
    auto ok = GetProcessTimes(process, &creationTime, &null_time, &null_time, &null_time);
    if(ok){
        pi.CreationTime = creationTime;
    }
    wchar_t imagepath[MAX_PATH + 1];
    if(K32GetProcessImageFileNameW(process, imagepath, sizeof(imagepath) / sizeof(*imagepath)) != 0){
        pi.DevicePath = imagepath;
    }
    return pi;
}

// static
SC_HANDLE WindowsSplitTunnel::installDriver(){
    LPCWSTR displayName = L"Mozilla Split Tunnel Service";
    QFileInfo driver(qApp->applicationDirPath()+"/"+ DRIVER_FILENAME);
    if(!driver.exists()){
        logger.log() << "Split Tunnel Driver File not found " << driver.absoluteFilePath();
        return (SC_HANDLE) INVALID_HANDLE_VALUE;
    }
    auto path = driver.absolutePath() + "/"+ DRIVER_FILENAME;
    LPCWSTR binPath = (const wchar_t*) path.utf16();
    auto scm_rights = SC_MANAGER_ALL_ACCESS;
    auto serviceManager = OpenSCManager(NULL,  // local computer
                                     NULL,  // servicesActive database
                                     scm_rights);
    auto service = CreateService(serviceManager, DRIVER_SERVICE_NAME, displayName,
                            SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                            SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                            binPath, nullptr, 0,nullptr, nullptr, nullptr);
    CloseServiceHandle(serviceManager);
    return service;
}
//static
bool WindowsSplitTunnel::uninstallDriver(){
    auto scm_rights = SC_MANAGER_ALL_ACCESS;
    auto serviceManager = OpenSCManager(NULL,  // local computer
                                     NULL,  // servicesActive database
                                     scm_rights);

    auto servicehandle = OpenService(serviceManager,DRIVER_SERVICE_NAME,GENERIC_READ);
    auto result = DeleteService(servicehandle);
    if(result){
        logger.log() << "Split Tunnel Driver Removed";
    }
    return result;
}
// static
bool WindowsSplitTunnel::isInstalled(){
    // Check if the Drivers I/O File is present
    auto symlink = QFileInfo(QString::fromWCharArray(DRIVER_SYMLINK));
    if(symlink.exists()){
        return true;
    }
    // If not check with SCM, if the kernel service exists
    auto scm_rights = SC_MANAGER_ALL_ACCESS;
    auto serviceManager = OpenSCManager(NULL,  // local computer
                                        NULL,  // servicesActive database
                                        scm_rights);
    auto servicehandle = OpenService(serviceManager,DRIVER_SERVICE_NAME,GENERIC_READ);
    auto err = GetLastError();
    CloseServiceHandle(serviceManager);
    CloseServiceHandle(servicehandle);
    return err != ERROR_SERVICE_DOES_NOT_EXIST;
}

bool WindowsSplitTunnel::initSublayer(){
    DWORD result = ERROR_SUCCESS;
    HANDLE m_wfp = INVALID_HANDLE_VALUE;
    FWPM_SESSION0 session;
    memset(&session, 0, sizeof(session));

    logger.log() << "Opening the filter engine";
    result = FwpmEngineOpen0(
        NULL,
        RPC_C_AUTHN_WINNT,
        NULL,
        &session,
        &m_wfp );
    if (result != ERROR_SUCCESS){
        logger.log() << "FwpmEngineOpen0 failed. Return value:.\n" << result;
        return false;
    }
    auto cleanup = qScopeGuard([&] { 
         FwpmEngineClose0(m_wfp);
     });
    // Check if the Layer Already Exists
    FWPM_SUBLAYER0* maybeLayer;
    result = FwpmSubLayerGetByKey0(m_wfp,&ST_FW_WINFW_BASELINE_SUBLAYER_KEY,&maybeLayer);
    if(result == ERROR_SUCCESS){
        logger.log() << "The Sublayer Already Exists!";
        FwpmFreeMemory0((void**) &maybeLayer); 
        return true;
    }

    //Step 1: Start Transaction
    result = FwpmTransactionBegin(m_wfp,NULL);
    if(result != ERROR_SUCCESS){
        logger.log() << "FwpmTransactionBegin0 failed. Return value:.\n" << result;
        return false;
    }

    // Step 3: Add Sublayer
    FWPM_SUBLAYER0 subLayer;
    memset(&subLayer, 0, sizeof(subLayer));
    subLayer.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;
    subLayer.displayData.name = (PWSTR)L"MozillaVPN-SplitTunnel-Sublayer";
    subLayer.displayData.description = (PWSTR)L"Filters that enforce a good baseline";
    //subLayer.providerKey= const_cast<GUID*>(&ST_FW_PROVIDER_KEY);
    subLayer.weight = 0xFFFF;

    result = FwpmSubLayerAdd0(m_wfp, &subLayer, NULL);
    if (result != ERROR_SUCCESS){
           logger.log() << "FwpmSubLayerAdd0 failed. Return value:.\n" << result;
           return false;
    }
    // Step 4: Commit!
    result = FwpmTransactionCommit0(m_wfp);
    if(result != ERROR_SUCCESS){
        logger.log() << "FwpmTransactionCommit0 failed. Return value:.\n" << result;
        return false;
    }
    logger.log() << "Initialised Sublayer";
    return true;
}

QString WindowsSplitTunnel::convertPath(const QString& path){
    auto parts =  path.split("/");
    QString driveLetter = parts.takeFirst();
    if(!driveLetter.contains(":") || parts.size() == 0){
      // device should contain : for e.g C:
      return "";
    }
    QByteArray buffer(2048,0xFF);
    auto ok =QueryDosDeviceW(qUtf16Printable(driveLetter),(wchar_t*) buffer.data(), buffer.size()/2);

    if( ok == ERROR_INSUFFICIENT_BUFFER){
        buffer.resize(buffer.size() *2);
        ok = QueryDosDeviceW(qUtf16Printable(driveLetter),(wchar_t*)buffer.data(), buffer.size()/2);
    }
    if( ok == 0){
        WindowsCommons::windowsLog("Err fetching dos path");
        return "";
    }
    QString deviceName;
    deviceName = QString::fromWCharArray((wchar_t*) buffer.data());
    parts.prepend(deviceName);

    return parts.join("\\");
}
