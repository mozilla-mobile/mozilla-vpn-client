#include "windowssplittunnel.h"
#include "../windowscommons.h"
#include "../windowsservicemanager.h"
#include "logger.h"
#include <QNetworkInterface>
#define PSAPI_VERSION 2
#include <psapi.h>
#include <QCoreApplication>
#include <QFileInfo>
namespace {
Logger logger(LOG_WINDOWS, "WindowsSplitTunnel");

FILETIME GetProcessCreationTime(HANDLE processHandle)
{
    FILETIME creationTime, dummy;

    const auto status = GetProcessTimes(processHandle, &creationTime, &dummy, &dummy, &dummy);

    if (FALSE == status)
    {
        //THROW_WINDOWS_ERROR(GetLastError(), "GetProcessTimes");
    }

    return creationTime;
}

std::wstring GetProcessDevicePath(HANDLE processHandle)
{
    size_t bufferSize = 512;
    std::vector<wchar_t> buffer;

    for (;;)
    {
        buffer.resize(bufferSize);

        const auto charsWritten = K32GetProcessImageFileNameW(processHandle,
            &buffer[0], static_cast<DWORD>(buffer.size()));

        if (0 == charsWritten)
        {
            if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
                bufferSize *= 2;
                continue;
            }

            //THROW_WINDOWS_ERROR(GetLastError(), "K32GetProcessImageFileNameW");
        }

        //
        // K32GetProcessImageFileNameW writes a null terminator
        // but doesn't account for it in the return value.
        //

        return std::wstring(&buffer[0], &buffer[0] + charsWritten);
    }
}


std::vector<ProcessInfo> CompileProcessInfo()
{
    auto snapshot = new HANDLE(
        (HANDLE)CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));

    if (INVALID_HANDLE_VALUE == *snapshot)
    {
        //THROW_WINDOWS_ERROR(GetLastError(), "Snapshot processes");
    }

    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    if (FALSE == Process32First(*snapshot, &processEntry))
    {
        //THROW_WINDOWS_ERROR(GetLastError(), "Initiate process enumeration");
    }

    std::map<DWORD, ProcessInfo> processes;

    //
    // Discover all processes.
    //

    do
    {
        auto handle = new HANDLE(OpenProcess(
            PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processEntry.th32ProcessID));

        if (NULL == *handle)
        {
            //THROW_WINDOWS_ERROR(GetLastError(), "Open process");
            continue;
        }

        ProcessInfo pi;

        pi.ProcessId = processEntry.th32ProcessID;
        pi.ParentProcessId = processEntry.th32ParentProcessID;

        try
        {
            pi.CreationTime = GetProcessCreationTime(*handle);
        }
        catch (...)
        {
            pi.CreationTime = { 0,0 };
        }

        try
        {
            pi.DevicePath = GetProcessDevicePath(*handle);
        }
        catch (...)
        {
            //
            // Including a process without a path might seem useless.
            // But it enables ancestor discovery.
            //

            pi.DevicePath = L"";
        }

        processes.insert(std::make_pair(pi.ProcessId, pi));
    }
    while (FALSE != Process32NextW(*snapshot, &processEntry));

    //
    // Find instances of PID recycling.
    //
    // This can be done by checking the creation time of the parent
    // process and discovering that the age of the claimed parent process
    // is lower than that of the child process.
    //

    for (auto& [pid, process] : processes)
    {
        auto parentIter = processes.find(process.ParentProcessId);

        if (parentIter != processes.end())
        {
            ULARGE_INTEGER parentTime;
            parentTime.LowPart = parentIter->second.CreationTime.dwLowDateTime;
            parentTime.HighPart = parentIter->second.CreationTime.dwHighDateTime;

            ULARGE_INTEGER processTime;
            processTime.LowPart = process.CreationTime.dwLowDateTime;
            processTime.HighPart = process.CreationTime.dwHighDateTime ;

            if (0 != parentTime.QuadPart
                && parentTime.QuadPart < processTime.QuadPart)
            {
                continue;
            }
        }

        process.ParentProcessId = 0;
    }

    //
    // Store process records into vector.
    //

    std::vector<ProcessInfo> output;

    output.reserve(processes.size());

    std::transform(processes.begin(), processes.end(), std::back_inserter(output),
        [](const std::map<DWORD, ProcessInfo>::value_type &entry)
        {
            return entry.second;
        });

    return output;
}


std::vector<uint8_t> MakeConfiguration(const std::vector<std::wstring> &imageNames)
{
    size_t totalStringLength = 0;

    for (const auto &imageName : imageNames)
    {
        totalStringLength += imageName.size() * sizeof(wchar_t);
    }

    size_t totalBufferSize = sizeof(CONFIGURATION_HEADER)
        + (sizeof(CONFIGURATION_ENTRY) * imageNames.size())
        + totalStringLength;

    std::vector<uint8_t> buffer(totalBufferSize);

    auto header = (CONFIGURATION_HEADER*)&buffer[0];
    auto entry = (CONFIGURATION_ENTRY*)(header + 1);

    auto stringDest = &buffer[0] + sizeof(CONFIGURATION_HEADER)
        + (sizeof(CONFIGURATION_ENTRY) * imageNames.size());

    SIZE_T stringOffset = 0;

    for (const auto &imageName : imageNames)
    {
        auto stringLength = imageName.size() * sizeof(wchar_t);

        entry->ImageNameLength = (USHORT)stringLength;
        entry->ImageNameOffset = stringOffset;

        memcpy(stringDest, imageName.c_str(), stringLength);

        ++entry;
        stringDest += stringLength;
        stringOffset += stringLength;
    }

    header->NumEntries = imageNames.size();
    header->TotalLength = totalBufferSize;

    return buffer;
}












}



WindowsSplitTunnel::WindowsSplitTunnel(QObject* parent): QObject(parent)
{
    if(!isInstalled()){
         logger.log() << "Driver is not Installed, doing so";
         auto handle = installDriver();
         if(handle == INVALID_HANDLE_VALUE){
            WindowsCommons::windowsLog("Failed to install Driver");
         }
         logger.log() << "Driver installed";
         CloseServiceHandle(handle);
    }else{
        logger.log() << "Driver is installed";
    }
    auto driver_manager = WindowsServiceManager(DRIVER_SERVICE_NAME);
    QObject::connect(&driver_manager, &WindowsServiceManager::serviceStarted,this,&WindowsSplitTunnel::initDriver);
    driver_manager.startService();
    return;
}

WindowsSplitTunnel::~WindowsSplitTunnel(){
    uninstallDriver();
}

void WindowsSplitTunnel::initDriver()
{
logger.log() << "Try to open Split Tunnel Driver";
    // Open the Driver Symlink
    m_driver = CreateFileW(DRIVER_SYMLINK, GENERIC_READ | GENERIC_WRITE,
                           0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);;

    if(m_driver == INVALID_HANDLE_VALUE){
        WindowsCommons::windowsLog("Failed to open Driver: ");
        return;
    }

    logger.log() << "Connected to the Driver";

    // We need to now check the state and init it, if required

    auto state = getState();
    if(state == STATE_UNKNOWN){
        logger.log() << "Cannot check if driver is initialised";
    }
    if(state >= STATE_INITIALIZED){
        logger.log() << "Driver already initialised";
        return;
    }
    DWORD bytesReturned;
    auto ok = DeviceIoControl(m_driver, IOCTL_INITIALIZE, nullptr, 0, nullptr, 0, &bytesReturned, nullptr);
    if(!ok){
        logger.log() << "Driver init failed";
        return;
    }
    logger.log() << "Driver initialised";
}


void WindowsSplitTunnel::setRules(const QStringList& appPaths)
{
    auto state = getState();
    if(state != STATE_READY && state != STATE_RUNNING){
        logger.log() << "Driver is not in the right State to set Rules" << state;
    }
    logger.log() << "Pushing new Ruleset for Split-Tunnel " << state;
    auto config = generateAppConfiguration(appPaths);

    logger.log() << "Basti config size:" << config.size();
    std::vector<std::wstring> names;
    //for(const auto& p: appPaths){
       // names.push_back(p.toStdWString());
    //}
    const std::wstring path = L"\\Device\\HarddiskVolume2\\Program Files (x86)\\Mozilla Firefox\\firefox.exe";
    names.push_back(path);
    logger.log() <<"Applist sitze" <<names.size();


    config = MakeConfiguration(names);
    logger.log() << "Refrence impl config size:" << config.size();




    DWORD bytesReturned;
    auto ok = DeviceIoControl(m_driver,IOCTL_SET_CONFIGURATION,
        &config[0], (DWORD)config.size(), nullptr, 0, &bytesReturned,nullptr);
    if(!ok){
        logger.log() << "Failed to set Config";
        return;
    }
    logger.log() << "New Configuration applied";
}

void WindowsSplitTunnel::start(){
    // To Start we need to send 2 things:
    // Network info (what is vpn what is network)
    logger.log() << "Starting SplitTunnel";
    DWORD bytesReturned;

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
        logger.log() << "Set Process Config || State" << getState();
    }

    if( getState() == STATE_INITIALIZED){
         logger.log() << "Driver is still not ready after process list send";
         return;
    }



    auto config = generateIPConfiguration();
    auto ok = DeviceIoControl(m_driver,IOCTL_REGISTER_IP_ADDRESSES,
        &config[0], (DWORD)config.size(), nullptr, 0, &bytesReturned,nullptr);
    if(!ok){
        logger.log() << "Failed to set Network Config";
        return;
    }

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
    for(auto const& path : appPaths){
       cummulated_string_size += path.toStdWString().size()* sizeof (wchar_t);
       logger.log() << path;
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

    for (const auto &path : appPaths)
    {
        auto wstr= path.toStdWString();
        auto cstr = wstr.c_str();
        auto stringLength = wstr.size() * sizeof(wchar_t);

        entry->ImageNameLength = (USHORT)stringLength;
        entry->ImageNameOffset = stringOffset;

        memcpy(stringDest, cstr , wstr.size());

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
    // TODO: Write cool stuff that termines the Adapter the Splitted App should use:
    getAddress(17, &config->TunnelIpv4, &config->TunnelIpv6);
    getAddress(5, &config->InternetIpv4, &config->InternetIpv6);
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

    if (!(Process32First(snapshot_handle, &currentProcess)))
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

    }while (!(Process32NextW(snapshot_handle, &currentProcess)));


    /*auto process_list = processes.values();
    if( process_list.isEmpty()){
        logger.log() << "Process Snapshot list was empty";
        return std::vector<uint8_t>(0);
    }*/

    auto process_list = CompileProcessInfo();
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

