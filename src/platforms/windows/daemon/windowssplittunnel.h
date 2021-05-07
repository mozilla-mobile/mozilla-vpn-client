#ifndef WINDOWSSPLITTUNNEL_H
#define WINDOWSSPLITTUNNEL_H

#include <QString>
#include <QStringList>

// Note: include order matters here.
#include <ws2tcpip.h>
#include <windows.h>
#include <Ws2ipdef.h>
#include <tlhelp32.h>
// States for GetState
enum DRIVER_STATE
{
    STATE_UNKNOWN = -1,
    STATE_NONE = 0,
    STATE_STARTED = 1,
    STATE_INITIALIZED = 2,
    STATE_READY = 3,
    STATE_RUNNING = 4,
    STATE_ZOMBIE = 5,
};

// Known ControlCodes
#define IOCTL_INITIALIZE \
    CTL_CODE(0x8000, 1, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_DEQUEUE_EVENT \
    CTL_CODE(0x8000, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_REGISTER_PROCESSES \
    CTL_CODE(0x8000, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_REGISTER_IP_ADDRESSES \
    CTL_CODE(0x8000, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_IP_ADDRESSES \
    CTL_CODE(0x8000, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_CONFIGURATION \
    CTL_CODE(0x8000, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_CONFIGURATION \
    CTL_CODE(0x8000, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CLEAR_CONFIGURATION \
    CTL_CODE(0x8000, 8, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_GET_STATE \
    CTL_CODE(0x8000, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_QUERY_PROCESS \
    CTL_CODE(0x8000, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ST_RESET \
    CTL_CODE(0x8000, 11, METHOD_NEITHER, FILE_ANY_ACCESS)


// Driver Configuration structures

typedef struct
{
    // Offset into buffer region that follows all entries.
    // The image name uses the device path.
    SIZE_T ImageNameOffset;
    // Length of the String
    USHORT ImageNameLength;
}
CONFIGURATION_ENTRY;

typedef struct
{
    // Number of entries immediately following the header.
    SIZE_T NumEntries;

    // Total byte length: header + entries + string buffer.
    SIZE_T TotalLength;
}
CONFIGURATION_HEADER;

// Used to Configure Which IP is network/vpn
typedef struct
{
    IN_ADDR TunnelIpv4;
    IN_ADDR InternetIpv4;

    IN6_ADDR TunnelIpv6;
    IN6_ADDR InternetIpv6;
}
IP_ADDRESSES_CONFIG;

// Used to Define Which Processes are alive on activation
typedef struct
{
    SIZE_T NumEntries;
    SIZE_T TotalLength;
}
PROCESS_DISCOVERY_HEADER;

typedef struct
{
    HANDLE ProcessId;
    HANDLE ParentProcessId;

    SIZE_T ImageNameOffset;
    USHORT ImageNameLength;
}
PROCESS_DISCOVERY_ENTRY;


typedef struct
{
    DWORD ProcessId;
    DWORD ParentProcessId;
    FILETIME CreationTime;
    std::wstring DevicePath;
}ProcessInfo;


class WindowsSplitTunnel
{
public:
    WindowsSplitTunnel();


    //void excludeApps(const QStringList& paths);
    //Excludes an Application from the VPN
    void setRules(const QStringList& appPaths);

    // Fetches and Pushed needed info to move to engaged mode
    void start();
    // Deletes Rules and puts it into passive mode
    void stop();

private:
    HANDLE m_driver = INVALID_HANDLE_VALUE;
    constexpr static const auto DRIVER_SYMLINK = L"\\\\.\\MULLVADSPLITTUNNEL";

    DRIVER_STATE getState();


    // Generates a Configuration for Each APP
    std::vector<uint8_t> generateAppConfiguration(const QStringList& appPaths);
    // Generates a Configuration which IP's are VPN and which network
    std::vector<uint8_t> generateIPConfiguration();
    std::vector<uint8_t> generateProcessBlob();

    void getAddress(int adapterIndex, IN_ADDR* out_ipv4, IN6_ADDR* out_ipv6);
    // Collects info about an Opened Process
    ProcessInfo getProcessInfo(HANDLE process, const PROCESSENTRY32W& processMeta);
};

#endif // WINDOWSSPLITTUNNEL_H
