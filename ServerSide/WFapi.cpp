#include <windows.h>
#include <wtsapi32.h>
#include <iostream>

// Citrix WFAPI definitions start
typedef HANDLE(WINAPI* WFVirtualChannelOpen)(HANDLE, DWORD, LPSTR);

typedef BOOL(WINAPI* WFVirtualChannelQuery)(HANDLE, DWORD, PVOID*, DWORD*);
typedef void(WINAPI* WFFreeMemory)(PVOID);

typedef BOOL(WINAPI* WFVirtualChannelWrite)(
    HANDLE hChannelHandle,
    PCHAR Buffer,
    ULONG Length,
    PULONG pBytesWritten
    );

#pragma pack(push,l)
typedef struct _MODULE_C2H {
    USHORT ByteCount;               /* length of module data in bytes (<2k) */
    BYTE ModuleCount;               /* number of modules headers left to be sent */
    BYTE ModuleClass;               /* module class (MODULECLASS) */
    BYTE VersionL;                  /* lowest supported version */
    BYTE VersionH;                  /* highest supported version */
    BYTE ModuleName[13];            /* client module name (8.3) */
    BYTE HostModuleName[9];         /* optional host module name (9 characters) */
    USHORT ModuleDate;              /* module date in dos format */
    USHORT ModuleTime;              /* module time in dos format */
    ULONG ModuleSize;               /* module file size in bytes */
} MODULE_C2H, * PMODULE_C2H;

/*
 *  Virtual driver flow control - ack
 */
typedef struct _VDFLOWACK {
    USHORT MaxWindowSize;           /* maximum # of bytes we can write */
    USHORT WindowSize;              /* # of bytes we can write before blocking */
} VDFLOWACK, * PVDFLOWACK;

/*
 *  Virtual driver flow control - delay
 */
typedef struct _VDFLOWDELAY {
    ULONG DelayTime;                /* delay time in 1/1000 seconds */
} VDFLOWDELAY, * PVDFLOWDELAY;

/*
 *  Virtual driver flow control - cdm (client drive mapping)
 */
typedef struct _VDFLOWCDM {
    USHORT MaxWindowSize;           /* total # of bytes we can write */
    USHORT MaxByteCount;            /* maximum size of any one write */
} VDFLOWCDM, * PVDFLOWCDM;

/*
 *  Virtual driver flow control structure
 */
typedef struct _VDFLOW {
    BYTE BandwidthQuota;            /* percentage of total bandwidth (0-100) */
    BYTE Flow;                      /* type of flow control */
    BYTE Pad1[2];
    union _VDFLOWU {
        VDFLOWACK Ack;
        VDFLOWDELAY Delay;
        VDFLOWCDM Cdm;
    } VDFLOWU;
} VDFLOW, * PVDFLOW;
/* // end_icasdk */

/*
 *  common virtual driver header
 */
typedef struct _VD_C2H {
    MODULE_C2H Header;

    /* This field is interpreted as follows:
     *
     * If the CAPABILITY_VIRTUAL_MAXIMUM is not supported by both the client and the host then:
     *   -  The ChannelMask field is interpreted as a bit mask of Virtual Channels (VCs)
     *      supported by a client Virtual Driver (VD), where b0=0.
     *   -  Always a single VD_C2H structure is sent per client VD.
     *
     * Otherwise, if the CAPABILITY_VIRTUAL_MAXIMUM is supported by both the client and the host then:
     *   -  The ChannelMask field is interpreted as a single VC number.
     *   -  If a single client VD module handles more than one VC, then for this VD
     *      multiple VD_C2H structures are sent - one for each VC that the VD supports.
     *      These modules are sent in separate PACKET_INIT_RESPONSE packets and are
     *      identical to each other except for the Header.ModuleCount and ChannelMask fields.
     */
    ULONG ChannelMask;

    VDFLOW Flow;
} VD_C2H, * PVD_C2H;
/* // end_icasdk */
#pragma pack(pop)

int trycitrix()
{

    HMODULE hModule = LoadLibrary(TEXT("wfapi.dll"));
    if (hModule == nullptr)
        return 0;
    WFVirtualChannelOpen open =
        (WFVirtualChannelOpen)GetProcAddress(hModule, "WFVirtualChannelOpen");
    if (open == nullptr)
        return 0;

    WFVirtualChannelQuery query =
        (WFVirtualChannelQuery)GetProcAddress(hModule, "WFVirtualChannelQuery");
    if (query == nullptr)
        return 0;

    WFVirtualChannelWrite write =
        (WFVirtualChannelWrite)GetProcAddress(hModule, "WFVirtualChannelWrite");
    if (write == nullptr)
        return 0;

    WFFreeMemory free =
        (WFFreeMemory)GetProcAddress(hModule, "WFFreeMemory");
    if (free == nullptr)
        return 0;

    HANDLE WF_CURRENT_SERVER_HANDLE = nullptr;
    auto WF_CURRENT_SESSION = -1;
    auto m_channelHandle = open(WF_CURRENT_SERVER_HANDLE, WF_CURRENT_SESSION, (char*)"DIKTADV");

    ULONG written = 0;
    char* buf = (char*)calloc(256, 1);
    memcpy_s(buf, 256, "hellofromcitrix", strlen("hellofromcitrix"));

    auto wrote = write(m_channelHandle, buf, 4, &written);

    void* buffer;
    DWORD bufferSize = 0;

    int WFVirtualClientData = 0;
    int bOk = query(m_channelHandle, WFVirtualClientData, &buffer, &bufferSize);
    int errorCode = GetLastError();

    if (bOk != 0 && bufferSize > 0) {
        PVD_C2H p = (PVD_C2H)buffer;
        std::cout << "Loaded " << (p->Header.HostModuleName);
        free(buffer);
    }

    FreeLibrary(hModule);
    return 0;
}
