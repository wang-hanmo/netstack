
// browserDlg.h: 头文件
//
#pragma once
#include "CEXPLORER1.h"
#include <direct.h>

class CbrowserDlgAutoProxy;


// CbrowserDlg 对话框
class CbrowserDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CbrowserDlg);
	friend class CbrowserDlgAutoProxy;



// 构造
public:
	CbrowserDlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~CbrowserDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BROWSER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	CbrowserDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit1();
	CEXPLORER1 m_web;
};
