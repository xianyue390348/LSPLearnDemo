
// LSPLearnDlg.h : 头文件
//

#pragma once

// CLSPLearnDlg 对话框
class CLSPLearnDlg : public CDialogEx
{
// 构造
public:
	CLSPLearnDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LSPLEARN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void OnBnClickedButton1();
	CString m_text;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	void static Proc_ReadPipe(LPVOID pApp);
	bool CheckProcessName(CString szName);
	HANDLE m_hPipe;
};

