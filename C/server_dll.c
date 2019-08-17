#include<windows.h>
#include <stdlib.h>

DWORD g_dwServiceType;
SERVICE_STATUS_HANDLE g_hServiceStatus;
DWORD g_dwCurrState;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

int TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress)
{
    SERVICE_STATUS srvStatus;
    srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    srvStatus.dwCurrentState = g_dwCurrState = dwState;
    srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    srvStatus.dwWin32ExitCode = dwExitCode;
    srvStatus.dwServiceSpecificExitCode = 0;
    srvStatus.dwCheckPoint = dwProgress;
    srvStatus.dwWaitHint = 1000;
    return SetServiceStatus(g_hServiceStatus, &srvStatus);
}

void __stdcall ServiceHandler(DWORD dwControl)
{
    switch (dwControl)
    {
    case SERVICE_CONTROL_STOP:
        TellSCM(SERVICE_STOP_PENDING, 0, 1);
        Sleep(10);
        TellSCM(SERVICE_STOPPED, 0, 0);
        break;
    case SERVICE_CONTROL_PAUSE:
        TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
        TellSCM(SERVICE_PAUSED, 0, 0);
        break;
    case SERVICE_CONTROL_CONTINUE:
        TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
        TellSCM(SERVICE_RUNNING, 0, 0);
        break;
    case SERVICE_CONTROL_INTERROGATE:
        TellSCM(g_dwCurrState, 0, 0);
        break;
    }
}

__declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[])
{
    char szSvcName[MAX_PATH];
    wcstombs(szSvcName, argv[0], sizeof szSvcName);

    // 注册一个服务控制程序
    g_hServiceStatus = RegisterServiceCtrlHandler(szSvcName, (LPHANDLER_FUNCTION)ServiceHandler);
    if (g_hServiceStatus == NULL)
    {
        return;
    } else FreeConsole();

    TellSCM(SERVICE_START_PENDING, 0, 1); // 通知服务控制管理器该服务开始
    TellSCM(SERVICE_RUNNING, 0, 0);  // 通知服务控制管理器该服务运行

    do {
        Sleep(100);
    } while (g_dwCurrState != SERVICE_STOP_PENDING && g_dwCurrState != SERVICE_STOPPED);
        return;
}