
// DlgProxy.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "browser.h"
#include "DlgProxy.h"
#include "browserDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CbrowserDlgAutoProxy

IMPLEMENT_DYNCREATE(CbrowserDlgAutoProxy, CCmdTarget)

CbrowserDlgAutoProxy::CbrowserDlgAutoProxy()
{
	EnableAutomation();

	// 为使应用程序在自动化对象处于活动状态时一直保持
	//	运行，构造函数调用 AfxOleLockApp。
	AfxOleLockApp();

	// 通过应用程序的主窗口指针
	//  来访问对话框。  设置代理的内部指针
	//  指向对话框，并设置对话框的后向指针指向
	//  该代理。
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(CbrowserDlg, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CbrowserDlg)))
		{
			m_pDialog = reinterpret_cast<CbrowserDlg*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

CbrowserDlgAutoProxy::~CbrowserDlgAutoProxy()
{
	// 为了在用 OLE 自动化创建所有对象后终止应用程序，
	//	析构函数调用 AfxOleUnlockApp。
	//  除了做其他事情外，这还将销毁主对话框
	if (m_pDialog != nullptr)
		m_pDialog->m_pAutoProxy = nullptr;
	AfxOleUnlockApp();
}

void CbrowserDlgAutoProxy::OnFinalRelease()
{
	// 释放了对自动化对象的最后一个引用后，将调用
	// OnFinalRelease。  基类将自动
	// 删除该对象。  在调用该基类之前，请添加您的
	// 对象所需的附加清理代码。

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CbrowserDlgAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CbrowserDlgAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// 注意: 我们添加了对 IID_Ibrowser 的支持来支持类型安全绑定
//  以支持来自 VBA 的类型安全绑定。  此 IID 必须同附加到 .IDL 文件中的
//  调度接口的 GUID 匹配。

// {ebb0bf12-bade-4363-a9fc-a5f810576c9c}
static const IID IID_Ibrowser =
{0xebb0bf12,0xbade,0x4363,{0xa9,0xfc,0xa5,0xf8,0x10,0x57,0x6c,0x9c}};

BEGIN_INTERFACE_MAP(CbrowserDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CbrowserDlgAutoProxy, IID_Ibrowser, Dispatch)
END_INTERFACE_MAP()

// IMPLEMENT_OLECREATE2 宏是在此项目的 pch.h 中定义的
// {73d80c10-388a-46c1-81b5-c2930900b617}
IMPLEMENT_OLECREATE2(CbrowserDlgAutoProxy, "browser.Application", 0x73d80c10,0x388a,0x46c1,0x81,0xb5,0xc2,0x93,0x09,0x00,0xb6,0x17)


// CbrowserDlgAutoProxy 消息处理程序
