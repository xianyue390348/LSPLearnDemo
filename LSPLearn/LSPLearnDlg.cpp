
// LSPLearnDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LSPLearn.h"
#include "LSPLearnDlg.h"
#include "afxdialogex.h"
#include <MyDebugTools.h>
#include <WS2spi.h>
#include <SpOrder.h>
#include "LspHelper.h"
#include <thread>
#include <MySocks5ClientHelper.h>

#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
int InstallProvider(WCHAR *wszDllPath);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CLSPLearnDlg 对话框


CLSPLearnDlg::CLSPLearnDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LSPLEARN_DIALOG, pParent)
	, m_text(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLSPLearnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_text);
}

BEGIN_MESSAGE_MAP(CLSPLearnDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CLSPLearnDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CLSPLearnDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CLSPLearnDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CLSPLearnDlg 消息处理程序

BOOL CLSPLearnDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	// TODO: 在此添加额外的初始化代码
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Proc_ReadPipe, this, 0, 0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CLSPLearnDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLSPLearnDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLSPLearnDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLSPLearnDlg::OnBnClickedButton1()
{
	DWORD dwSize = 0;
	WSAPROTOCOL_INFO ProtoInfo = { 0 };
	WSAEnumProtocols(NULL, &ProtoInfo, &dwSize);
	if (dwSize <= 0) {
		return;
	}
	LPWSAPROTOCOL_INFO pProtoInfos = new WSAPROTOCOL_INFO[dwSize / sizeof(WSAPROTOCOL_INFO)];
	DWORD iCount = WSAEnumProtocols(NULL, pProtoInfos, &dwSize);
	if (pProtoInfos != NULL) {
		for (int i = 0; i < iCount; i++) {
			tssc(pProtoInfos->szProtocol);
			m_text += CString(pProtoInfos->szProtocol) + L"\r\n";
			pProtoInfos++;
		}
	}
	pProtoInfos -= iCount;
	delete pProtoInfos;
	UpdateData(FALSE);
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

void CLSPLearnDlg::OnBnClickedButton2()
{
	WCHAR buffer[MAX_PATH + 1] = {0};
	GetCurrentDirectory(MAX_PATH, buffer);
	CString path(buffer);
	path += L"\\LSPDLL.dll";
	InstallProvider(path.GetBuffer());
}

GUID ProviderGuid = { 0xd3c21122, 0x85e1, 0x48f3,{ 0x9a,0xb6,0x23,0xd9,0x0c,0x73,0x07,0xef } };

int InstallProvider(WCHAR *wszDllPath)
{
	int i = 0;
	WCHAR wszLSPName[] = L"MyLsp"; // 我们的LSP 的名称
	int nError = NO_ERROR;
	LPWSAPROTOCOL_INFOW pProtoInfo;
	int nProtocols;
	WSAPROTOCOL_INFOW UDPLayeredInfo, UDPChainInfo; // 我们要安装的UDP 分层协议和协议链
	DWORD dwUdpOrigCatalogId, dwLayeredCatalogId;
	// 在Winsock 目录中找到原来的UDP 服务提供者，我们的LSP 要安装在它之上
	// 枚举所有服务程序提供者
	pProtoInfo = GetProvider(&nProtocols); // 此函数的实现请参考本章前面的内容
	for (int i = 0; i < nProtocols; i++)
	{
		if (pProtoInfo[i].iAddressFamily == AF_INET && pProtoInfo[i].iProtocol == IPPROTO_TCP)
		{
			memcpy(&UDPChainInfo, &pProtoInfo[i], sizeof(UDPLayeredInfo));
			// 去掉XP1_IFS_HANDLES 标志
			UDPChainInfo.dwServiceFlags1 = UDPChainInfo.dwServiceFlags1 & ~XP1_IFS_HANDLES;
			// 保存原来的入口ID
			dwUdpOrigCatalogId = pProtoInfo[i].dwCatalogEntryId;
			break;
		}
	}
	// 首先安装分层协议，获取一个Winsock 库安排的目录ID 号，即dwLayeredCatalogId
	// 直接使用下层协议的WSAPROTOCOL_INFOW结构即可
	memcpy(&UDPLayeredInfo, &UDPChainInfo, sizeof(UDPLayeredInfo));
	// 修改协议名称，类型，设置PFL_HIDDEN 标志
	wcscpy(UDPLayeredInfo.szProtocol, wszLSPName);
	UDPLayeredInfo.ProtocolChain.ChainLen = LAYERED_PROTOCOL; // LAYERED_PROTOCOL即0
	UDPLayeredInfo.dwProviderFlags |= PFL_HIDDEN;
	// 安装
	if (::WSCInstallProvider(&ProviderGuid,
		wszDllPath, &UDPLayeredInfo, 1, &nError) == SOCKET_ERROR)
		return nError;
	// 重新枚举协议，获取分层协议的目录ID 号
	FreeProvider(pProtoInfo);
	pProtoInfo = GetProvider(&nProtocols);
	for (i = 0; i < nProtocols; i++)
	{
		if (memcmp(&pProtoInfo[i].ProviderId, &ProviderGuid, sizeof(ProviderGuid)) == 0)
		{
			dwLayeredCatalogId = pProtoInfo[i].dwCatalogEntryId;
			break;
		}
	}
	// 安装协议链
	// 修改协议名称，类型
	WCHAR wszChainName[WSAPROTOCOL_LEN + 1];
	swprintf(wszChainName, L"%ws over %ws", wszLSPName, UDPChainInfo.szProtocol);
	wcscpy(UDPChainInfo.szProtocol, wszChainName);
	if (UDPChainInfo.ProtocolChain.ChainLen == 1)
	{
		UDPChainInfo.ProtocolChain.ChainEntries[1] = dwUdpOrigCatalogId;
	}
	else
	{
		for (i = UDPChainInfo.ProtocolChain.ChainLen; i > 0; i--)
		{
			UDPChainInfo.ProtocolChain.ChainEntries[i] = UDPChainInfo.ProtocolChain.ChainEntries[i - 1];
		}
	}
	UDPChainInfo.ProtocolChain.ChainLen++;
	// 将我们的分层协议置于此协议链的顶层
		UDPChainInfo.ProtocolChain.ChainEntries[0] = dwLayeredCatalogId;
	// 获取一个Guid，安装之
	GUID ProviderChainGuid;
	if (::UuidCreate(&ProviderChainGuid) == RPC_S_OK)
	{
		if (::WSCInstallProvider(&ProviderChainGuid,
			wszDllPath, &UDPChainInfo, 1, &nError) == SOCKET_ERROR)
			return nError;
	}
	else
		return GetLastError();
	// 重新排序Winsock 目录，将我们的协议链提前
	// 重新枚举安装的协议
	FreeProvider(pProtoInfo);
	pProtoInfo = GetProvider(&nProtocols);
	DWORD dwIds[20];
	int nIndex = 0;
	// 添加我们的协议链
	for (i = 0; i < nProtocols; i++)
	{
		if ((pProtoInfo[i].ProtocolChain.ChainLen > 1) &&
			(pProtoInfo[i].ProtocolChain.ChainEntries[0] == dwLayeredCatalogId))
			dwIds[nIndex++] = pProtoInfo[i].dwCatalogEntryId;
	}
	// 添加其他协议
	for (i = 0; i < nProtocols; i++)
	{
		if ((pProtoInfo[i].ProtocolChain.ChainLen <= 1) ||
			(pProtoInfo[i].ProtocolChain.ChainEntries[0] != dwLayeredCatalogId))
			dwIds[nIndex++] = pProtoInfo[i].dwCatalogEntryId;
	}
	// 重新排序Winsock 目录
	nError = ::WSCWriteProviderOrder(dwIds, nIndex);
	FreeProvider(pProtoInfo);
	return nError;
}

void RemoveProvider()
{
	int i = 0;
	LPWSAPROTOCOL_INFOW pProtoInfo;
	int nProtocols;
	DWORD dwLayeredCatalogId;
	// 根据Guid 取得分层协议的目录ID 号
	pProtoInfo = GetProvider(&nProtocols);
	int nError;
	for (int i = 0; i < nProtocols; i++)
	{
		if (memcmp(&ProviderGuid, &pProtoInfo[i].ProviderId, sizeof(ProviderGuid)) == 0)
		{
			dwLayeredCatalogId = pProtoInfo[i].dwCatalogEntryId;
			break;
		}
	}
	if (i < nProtocols)
	{ // 移除协议链
		for (i = 0; i < nProtocols; i++)
		{
			if ((pProtoInfo[i].ProtocolChain.ChainLen > 1) &&
				(pProtoInfo[i].ProtocolChain.ChainEntries[0] == dwLayeredCatalogId))
			{
				::WSCDeinstallProvider(&pProtoInfo[i].ProviderId, &nError);
			}
		}
		// 移除分层协议
		::WSCDeinstallProvider(&ProviderGuid, &nError);
	}
}

void CLSPLearnDlg::OnBnClickedButton3()
{
	RemoveProvider();
}

#define MSGTYPE_PROCESS_NAME 0x1
#define MSGTYPE_PROXY_INFO 0x2
struct PipeMessage
{
	DWORD Type;
	CHAR Text[MAX_PATH + 1];
};

void CLSPLearnDlg::Proc_ReadPipe(LPVOID pApp)
{
	CLSPLearnDlg* App = (CLSPLearnDlg*)pApp;
	HANDLE m_hPipe = CreateNamedPipe(L"\\\\.\\pipe\\XueLspPipe", PIPE_ACCESS_DUPLEX, 0, 1, 1024, 1024, 0, NULL);
	ConnectNamedPipe(m_hPipe, NULL);
	WCHAR BUFFER[1024];
	DWORD dwRead = 0;
	while (TRUE)
	{
		ZeroMemory(BUFFER, 1024 * 2);
		if (!ReadFile(m_hPipe, BUFFER, 1024, &dwRead, NULL))
		{
			DisconnectNamedPipe(m_hPipe);
			CloseHandle(m_hPipe);
			HANDLE m_hPipe = CreateNamedPipe(L"\\\\.\\pipe\\XueLspPipe",
				PIPE_ACCESS_DUPLEX, 0, 255, 1024, 1024, 0, NULL);
			ConnectNamedPipe(m_hPipe, NULL);
			App->m_text += CString(L"[+]重建管道") + L"\r\n";
		}
		if (dwRead > 0) {
			PipeMessage* pPmsg = (PipeMessage*)BUFFER;
			if (pPmsg->Type == MSGTYPE_PROCESS_NAME) {
				//比对进程名称是否需要进行代理
				CStringA ret;
				if (App->CheckProcessName(CString(pPmsg->Text))) {
					ret = "1";
				} else {
					ret = "-1";
				}
				dwRead = 0;
				WriteFile(m_hPipe, ret.GetBuffer(), ret.GetLength(), &dwRead, NULL);
			}
			else if (pPmsg->Type == MSGTYPE_PROXY_INFO) {
				SOCKADDR_IN *psi = (SOCKADDR_IN*)pPmsg->Text;
				CHAR* s5t = inet_ntoa((*psi).sin_addr);
				OutputDebugStringA(s5t);
				//套接字取随机端口
				SOCKET temp_socket = socket(AF_INET, SOCK_STREAM, 0);
				SOCKADDR_IN si;
				si.sin_family = AF_INET;
				si.sin_port = 0;
				si.sin_addr.S_un.S_addr = INADDR_ANY;
				if (bind(temp_socket, (SOCKADDR*)&si, sizeof(si)) == SOCKET_ERROR)
				{
					OutputDebugStringW(L"bind socket error!");
					return;
				}
				listen(temp_socket, 5);
				int isize = sizeof(si);
				getsockname(temp_socket, (SOCKADDR*)&si, &isize);
				//取出端口
				int iPort = ntohs(si.sin_port);
				CStringA ret;
				ret.Format("%d", iPort);
				WriteFile(m_hPipe, ret.GetBuffer(), ret.GetLength(), &dwRead, NULL);
				//返回完成，进行线程处理
				std::thread t1([](SOCKET m_socket, SOCKADDR_IN si)
				{
					SOCKET m_accpet = accept(m_socket, NULL,NULL);
					if (m_accpet == INVALID_SOCKET) {
						OutputDebugStringW(L"Accept Socket Error");
						return;
					}
					OutputDebugStringW(L"Get a client connect！");
					//成功接受客户端套接字
					Socks5Client sc;
					sc.Init(L"192.168.199.242",1080,L"admin", L"admin");
					CHAR* s5t = inet_ntoa(si.sin_addr);
					OutputDebugStringA(s5t);
					CString StrDbg;
					StrDbg.Format(L"Socks5 代理尝试连接%s:%d", s5t, ntohs(si.sin_port));
					OutputDebugStringW(StrDbg);
					if (!sc.Connect()) {
						OutputDebugStringW(L"Sockets connect fail!");
					}
					SOCKET s5_socket = sc.Socket_Connect(si);
					if (s5_socket == INVALID_SOCKET) {
						OutputDebugStringW(L"Socket connect target fail!");
					}
					auto proc_swap = [](SOCKET s1, SOCKET s2) {
						CHAR buffer[1024];
						while (true)
						{
							OutputDebugStringW(L"begin swap data");
							ZeroMemory(buffer, 1024);
							if (recv(s1, buffer, 1024, 0) <= 0) { OutputDebugStringW(L"Socket Swap Error Exit!"); break; }
							OutputDebugStringA(buffer);
							if (send(s2, buffer, 1024, 0) <= 0) { OutputDebugStringW(L"Socket Swap Error Exit!"); break; }
						}
					};
					std::thread swap_0(proc_swap, m_accpet, s5_socket);
					std::thread swap_1(proc_swap, s5_socket,m_accpet);
					swap_0.detach();
					swap_1.detach();
				}, temp_socket, *psi);
				t1.detach();
			}
		}
	}
}


bool CLSPLearnDlg::CheckProcessName(CString szName)
{
	if (szName.Mid(szName.ReverseFind('\\')+1) == L"1.exe") {
		return true;
	}
	if (szName.Mid(szName.ReverseFind('\\') + 1) == L"2.exe") {
		return true;
	}
	return false;
}

