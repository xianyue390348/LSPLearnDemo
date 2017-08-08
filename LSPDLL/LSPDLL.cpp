// LSPDLL.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "LSPDLL.h"

#include <Winsock2.h>
#include <Ws2spi.h>
#include <MyDebugTools.h>
#pragma comment(lib, "Ws2_32.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifndef __DEBUG_H__
#define __DEBUG_H__
#ifdef _DEBUG
#define ODS(szOut) \
{ \
	OutputDebugString(szOut); \
}
#define ODS1(szOut, var) \
{ \
	TCHAR sz[1024]; \
	_stprintf(sz, szOut, var); \
	OutputDebugString(sz); \
}
#else
#endif // _DEBUG
#endif // __DEBUG_H__

//
//TODO:  如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。  这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CLSPDLLApp

BEGIN_MESSAGE_MAP(CLSPDLLApp, CWinApp)
END_MESSAGE_MAP()


// CLSPDLLApp 构造

CLSPDLLApp::CLSPDLLApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CLSPDLLApp 对象

CLSPDLLApp theApp;

#define MSGTYPE_PROCESS_NAME 0x1
#define MSGTYPE_PROXY_INFO 0x2
struct PipeMessage
{
	DWORD Type;
	CHAR Text[MAX_PATH + 1];
};

// CLSPDLLApp 初始化
WSPUPCALLTABLE g_pUpCallTable; // 上层函数列表。如果LSP 创建了自己的伪句柄，才使用这个函数列表
WSPPROC_TABLE g_NextProcTable; // 下层函数列表
TCHAR g_szCurrentApp[MAX_PATH]; // 当前调用本DLL 的程序的名称
HANDLE m_hPipe;

CString SendPipeMessage(PipeMessage sPipe)
{
	WaitNamedPipe(L"\\\\.\\pipe\\XueLspPipe", //如果是跨网络通信，则在圆点处指定服务器端程序所在的主机名  
		NMPWAIT_WAIT_FOREVER);
	//打开可用的命名管道，并与服务器端进程进行通信  
	HANDLE m_hPipe = CreateFile(L"\\\\.\\pipe\\XueLspPipe",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == m_hPipe)
	{
		OutputDebugString(L"SendFail!");
		return NULL;
	}
	DWORD dwRead = 0;
	CStringA m_send(sPipe.Text);
	WriteFile(m_hPipe, &sPipe, sizeof(sPipe), &dwRead, NULL);
	WCHAR buffer[1024] = { 0 };
	OutputDebugString(L"wating return!");
	dwRead = 0;
	ReadFile(m_hPipe, buffer, 1024, &dwRead, NULL);
	OutputDebugString(L"get return!");
	DisconnectNamedPipe(m_hPipe);
	CloseHandle(m_hPipe);
	return CString((CHAR*)buffer);
}

BOOL CLSPDLLApp::InitInstance()
{
	CWinApp::InitInstance();
	::GetModuleFileName(NULL, g_szCurrentApp, MAX_PATH);
	//auto pid = _getpid();
	//CString str;
	//str.Format(L"进程路径:%ws 进程PID：%d", g_szCurrentApp, pid);
	//CString t_str = SendPipeMessage(str);
	//OutputDebugString(t_str);
	//MessageBox(0,t_str,0,0);
	return TRUE;
}

LPWSAPROTOCOL_INFOW GetProvider(LPINT lpnTotalProtocols)
{
	int nError;
	DWORD dwSize = 0;
	LPWSAPROTOCOL_INFOW pProtoInfo = NULL;
	// 取得需要的缓冲区长度
	if (::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &nError) == SOCKET_ERROR)
	{
		if (nError != WSAENOBUFS)
			return NULL;
	}
	// 申请缓冲区，再次调用WSCEnumProtocols 函数
	pProtoInfo = (LPWSAPROTOCOL_INFOW)::GlobalAlloc(GPTR, dwSize);
	*lpnTotalProtocols = ::WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &nError);
	return pProtoInfo;
}
void FreeProvider(LPWSAPROTOCOL_INFOW pProtoInfo)
{
	::GlobalFree(pProtoInfo);
}



int WSPAPI WSPConnect(
	__in   SOCKET s,
	__in   const struct sockaddr *name,
	__in   int namelen,
	__in   LPWSABUF lpCallerData,
	__out  LPWSABUF lpCalleeData,
	__in   LPQOS lpSQOS,
	__in   LPQOS lpGQOS,
	__out  LPINT lpErrno
)
{
	SOCKADDR_IN *psi;
	psi = (SOCKADDR_IN*)name;
	ODS1(L"query send to... %s", g_szCurrentApp);
	CString target_address(inet_ntoa(psi->sin_addr));
	CString local_address(L"127.0.0.1");
	tssc(target_address);
	tssc(local_address);
	if (target_address == local_address) {
		return g_NextProcTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
	}

	USES_CONVERSION;
	PipeMessage pmsg;
	pmsg.Type = MSGTYPE_PROCESS_NAME;
	strcpy(pmsg.Text, W2A(g_szCurrentApp));
	CString ret_str = SendPipeMessage(pmsg);
	OutputDebugStringW(ret_str);
	if (ret_str == L"1")
	{
		OutputDebugStringW(L"change ip");
		pmsg.Type = MSGTYPE_PROXY_INFO;
		memcpy(pmsg.Text, name, sizeof(SOCKADDR_IN));
		ret_str = SendPipeMessage(pmsg);
		SOCKADDR_IN si = { 0 };
		si.sin_family = AF_INET;
		si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		int Port = _wtoi(ret_str);
		si.sin_port = htons(Port);
		return g_NextProcTable.lpWSPConnect(s, (SOCKADDR*)&si, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
	}
	
	return g_NextProcTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
	 
}

extern "C" int WSPAPI WSPStartup(
	WORD wVersionRequested,
	LPWSPDATA lpWSPData,
	LPWSAPROTOCOL_INFO lpProtocolInfo,
	WSPUPCALLTABLE UpcallTable,
	LPWSPPROC_TABLE lpProcTable
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// 此处为普通函数体
	int i = 0;
	ODS1(L" WSPStartup... %s \n", g_szCurrentApp);
	if (lpProtocolInfo->ProtocolChain.ChainLen <= 1)
	{
		return WSAEPROVIDERFAILEDINIT;
	}
	// 保存向上调用的函数表指针（这里我们不使用它）
	g_pUpCallTable = UpcallTable;
	// 枚举协议，找到下层协议的WSAPROTOCOL_INFOW结构
	WSAPROTOCOL_INFOW NextProtocolInfo;
	int nTotalProtos;
	LPWSAPROTOCOL_INFOW pProtoInfo = GetProvider(&nTotalProtos);
	// 下层入口ID
	DWORD dwBaseEntryId = lpProtocolInfo->ProtocolChain.ChainEntries[1];
	for (int i = 0; i < nTotalProtos; i++)
	{
		if (pProtoInfo[i].dwCatalogEntryId == dwBaseEntryId)
		{
			memcpy(&NextProtocolInfo, &pProtoInfo[i], sizeof(NextProtocolInfo));
			break;
		}
	}
	if (i >= nTotalProtos)
	{
		ODS(L" WSPStartup: Can not find underlying protocol \n");
		return WSAEPROVIDERFAILEDINIT;
	}
	// 加载下层协议的DLL
	int nError;
	TCHAR szBaseProviderDll[MAX_PATH];
	int nLen = MAX_PATH;
	// 取得下层提供者DLL 路径
	if (::WSCGetProviderPath(&NextProtocolInfo.ProviderId,
		szBaseProviderDll, &nLen, &nError) == SOCKET_ERROR)
	{
		ODS1(L" WSPStartup: WSCGetProviderPath() failed %d \n", nError);
		return WSAEPROVIDERFAILEDINIT;
	}
	if (!::ExpandEnvironmentStrings(szBaseProviderDll, szBaseProviderDll, MAX_PATH))
	{
		ODS1(L" WSPStartup: ExpandEnvironmentStrings() failed %d \n", ::GetLastError());
		return WSAEPROVIDERFAILEDINIT;
	}
	// 加载下层提供者
	HMODULE hModule = ::LoadLibrary(szBaseProviderDll);
	if (hModule == NULL)
	{
		ODS1(L" WSPStartup: LoadLibrary() failed %d \n", ::GetLastError());
		return WSAEPROVIDERFAILEDINIT;
	}
	// 导入下层提供者的WSPStartup 函数
	LPWSPSTARTUP pfnWSPStartup = NULL;
	pfnWSPStartup = (LPWSPSTARTUP)::GetProcAddress(hModule, "WSPStartup");
	if (pfnWSPStartup == NULL)
	{
		ODS1(L" WSPStartup: GetProcAddress() failed %d \n", ::GetLastError());
		return WSAEPROVIDERFAILEDINIT;
	}
	// 调用下层提供者的WSPStartup 函数
	LPWSAPROTOCOL_INFOW pInfo = lpProtocolInfo;
	if (NextProtocolInfo.ProtocolChain.ChainLen == BASE_PROTOCOL)
		pInfo = &NextProtocolInfo;
	int nRet = pfnWSPStartup(wVersionRequested, lpWSPData, pInfo, UpcallTable, lpProcTable);
	if (nRet != ERROR_SUCCESS)
	{
		ODS1(L" WSPStartup: underlying provider's WSPStartup() failed %d \n", nRet);
		return nRet;
	}
	// 保存下层提供者的函数表
	g_NextProcTable = *lpProcTable;
	// 修改传递给上层的函数表，Hook 感兴趣的函数，这里做为示例，仅Hook 了WSPSendTo 函数
	// 您还可以Hook 其他函数，如WSPSocket、WSPCloseSocket、WSPConnect 等
	lpProcTable->lpWSPConnect = WSPConnect;
	FreeProvider(pProtoInfo);
	return nRet;

}

