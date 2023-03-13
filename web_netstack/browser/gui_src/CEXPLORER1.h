#pragma once

// 计算机生成了通过 Microsoft Visual C++ 创建的 IDispatch 包装类

// 注意: 请勿修改此文件的内容。如果此类
// 通过 Microsoft Visual C++ 重新生成，则你的修改将被覆盖。

/////////////////////////////////////////////////////////////////////////////

#include "afxwin.h"
#include "afxdisp.h"

class CEXPLORER1 : public CWnd
{
protected:
	DECLARE_DYNCREATE(CEXPLORER1)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= {0x8856f961,0x340a,0x11d0,{0xa9,0x6b,0x00,0xc0,0x4f,0xd7,0x05,0xa2}};
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID, 
						CCreateContext* pContext = nullptr)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID);
	}

	BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
				UINT nID, CFile* pPersist = nullptr, BOOL bStorage = FALSE,
				BSTR bstrLicKey = nullptr)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); 
	}

// 特性
public:
enum
{
	CSC_UPDATECOMMANDS = -1,
	CSC_NAVIGATEFORWARD = 1,
	CSC_NAVIGATEBACK = 2
}CommandStateChangeConstants;

enum
{
	OLECMDID_OPEN = 1,
	OLECMDID_NEW = 2,
	OLECMDID_SAVE = 3,
	OLECMDID_SAVEAS = 4,
	OLECMDID_SAVECOPYAS = 5,
	OLECMDID_PRINT = 6,
	OLECMDID_PRINTPREVIEW = 7,
	OLECMDID_PAGESETUP = 8,
	OLECMDID_SPELL = 9,
	OLECMDID_PROPERTIES = 10,
	OLECMDID_CUT = 11,
	OLECMDID_COPY = 12,
	OLECMDID_PASTE = 13,
	OLECMDID_PASTESPECIAL = 14,
	OLECMDID_UNDO = 15,
	OLECMDID_REDO = 16,
	OLECMDID_SELECTALL = 17,
	OLECMDID_CLEARSELECTION = 18,
	OLECMDID_ZOOM = 19,
	OLECMDID_GETZOOMRANGE = 20,
	OLECMDID_UPDATECOMMANDS = 21,
	OLECMDID_REFRESH = 22,
	OLECMDID_STOP = 23,
	OLECMDID_HIDETOOLBARS = 24,
	OLECMDID_SETPROGRESSMAX = 25,
	OLECMDID_SETPROGRESSPOS = 26,
	OLECMDID_SETPROGRESSTEXT = 27,
	OLECMDID_SETTITLE = 28,
	OLECMDID_SETDOWNLOADSTATE = 29,
	OLECMDID_STOPDOWNLOAD = 30,
	OLECMDID_ONTOOLBARACTIVATED = 31,
	OLECMDID_FIND = 32,
	OLECMDID_DELETE = 33,
	OLECMDID_HTTPEQUIV = 34,
	OLECMDID_HTTPEQUIV_DONE = 35,
	OLECMDID_ENABLE_INTERACTION = 36,
	OLECMDID_ONUNLOAD = 37,
	OLECMDID_PROPERTYBAG2 = 38,
	OLECMDID_PREREFRESH = 39,
	OLECMDID_SHOWSCRIPTERROR = 40,
	OLECMDID_SHOWMESSAGE = 41,
	OLECMDID_SHOWFIND = 42,
	OLECMDID_SHOWPAGESETUP = 43,
	OLECMDID_SHOWPRINT = 44,
	OLECMDID_CLOSE = 45,
	OLECMDID_ALLOWUILESSSAVEAS = 46,
	OLECMDID_DONTDOWNLOADCSS = 47,
	OLECMDID_UPDATEPAGESTATUS = 48,
	OLECMDID_PRINT2 = 49,
	OLECMDID_PRINTPREVIEW2 = 50,
	OLECMDID_SETPRINTTEMPLATE = 51,
	OLECMDID_GETPRINTTEMPLATE = 52,
	OLECMDID_PAGEACTIONBLOCKED = 55,
	OLECMDID_PAGEACTIONUIQUERY = 56,
	OLECMDID_FOCUSVIEWCONTROLS = 57,
	OLECMDID_FOCUSVIEWCONTROLSQUERY = 58,
	OLECMDID_SHOWPAGEACTIONMENU = 59,
	OLECMDID_ADDTRAVELENTRY = 60,
	OLECMDID_UPDATETRAVELENTRY = 61,
	OLECMDID_UPDATEBACKFORWARDSTATE = 62,
	OLECMDID_OPTICAL_ZOOM = 63,
	OLECMDID_OPTICAL_GETZOOMRANGE = 64,
	OLECMDID_WINDOWSTATECHANGED = 65,
	OLECMDID_ACTIVEXINSTALLSCOPE = 66,
	OLECMDID_UPDATETRAVELENTRY_DATARECOVERY = 67,
	OLECMDID_SHOWTASKDLG = 68,
	OLECMDID_POPSTATEEVENT = 69,
	OLECMDID_VIEWPORT_MODE = 70,
	OLECMDID_LAYOUT_VIEWPORT_WIDTH = 71,
	OLECMDID_VISUAL_VIEWPORT_EXCLUDE_BOTTOM = 72,
	OLECMDID_USER_OPTICAL_ZOOM = 73,
	OLECMDID_PAGEAVAILABLE = 74,
	OLECMDID_GETUSERSCALABLE = 75,
	OLECMDID_UPDATE_CARET = 76,
	OLECMDID_ENABLE_VISIBILITY = 77,
	OLECMDID_MEDIA_PLAYBACK = 78,
	OLECMDID_SETFAVICON = 79,
	OLECMDID_SET_HOST_FULLSCREENMODE = 80,
	OLECMDID_EXITFULLSCREEN = 81,
	OLECMDID_SCROLLCOMPLETE = 82,
	OLECMDID_ONBEFOREUNLOAD = 83,
	OLECMDID_SHOWMESSAGE_BLOCKABLE = 84,
	OLECMDID_SHOWTASKDLG_BLOCKABLE = 85
}OLECMDID;

enum
{
	OLECMDF_SUPPORTED = 1,
	OLECMDF_ENABLED = 2,
	OLECMDF_LATCHED = 4,
	OLECMDF_NINCHED = 8,
	OLECMDF_INVISIBLE = 16,
	OLECMDF_DEFHIDEONCTXTMENU = 32
}OLECMDF;

enum
{
	OLECMDEXECOPT_DODEFAULT = 0,
	OLECMDEXECOPT_PROMPTUSER = 1,
	OLECMDEXECOPT_DONTPROMPTUSER = 2,
	OLECMDEXECOPT_SHOWHELP = 3
}OLECMDEXECOPT;

enum
{
	READYSTATE_UNINITIALIZED = 0,
	READYSTATE_LOADING = 1,
	READYSTATE_LOADED = 2,
	READYSTATE_INTERACTIVE = 3,
	READYSTATE_COMPLETE = 4
}tagREADYSTATE;

enum
{
	secureLockIconUnsecure = 0,
	secureLockIconMixed = 1,
	secureLockIconSecureUnknownBits = 2,
	secureLockIconSecure40Bit = 3,
	secureLockIconSecure56Bit = 4,
	secureLockIconSecureFortezza = 5,
	secureLockIconSecure128Bit = 6
}SecureLockIconConstants;

enum
{
	ProtectedModeRedirect = 1
}NewProcessCauseConstants;

enum
{
	SWC_EXPLORER = 0,
	SWC_BROWSER = 1,
	SWC_3RDPARTY = 2,
	SWC_CALLBACK = 4,
	SWC_DESKTOP = 8
}ShellWindowTypeConstants;

enum
{
	SWFO_NEEDDISPATCH = 1,
	SWFO_INCLUDEPENDING = 2,
	SWFO_COOKIEPASSED = 4
}ShellWindowFindWindowOptions;



// 操作
public:
// IWebBrowser

// 函数
//

	void GoBack()
	{
		InvokeHelper(0x64, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void GoForward()
	{
		InvokeHelper(0x65, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void GoHome()
	{
		InvokeHelper(0x66, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void GoSearch()
	{
		InvokeHelper(0x67, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void Navigate(LPCTSTR URL, VARIANT * Flags, VARIANT * TargetFrameName, VARIANT * PostData, VARIANT * Headers)
	{
		static BYTE parms[] = VTS_BSTR VTS_UNKNOWN VTS_UNKNOWN VTS_UNKNOWN VTS_UNKNOWN;
		InvokeHelper(0x68, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, URL, Flags, TargetFrameName, PostData, Headers);
	}
	void Refresh()
	{
		InvokeHelper(0xFFFFFDDA, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void Refresh2(VARIANT * Level)
	{
		static BYTE parms[] = VTS_UNKNOWN;
		InvokeHelper(0x69, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Level);
	}
	void Stop()
	{
		InvokeHelper(0x6A, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	LPDISPATCH get_Application()
	{
		LPDISPATCH result;
		InvokeHelper(0xC8, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
		return result;
	}
	LPDISPATCH get_Parent()
	{
		LPDISPATCH result;
		InvokeHelper(0xC9, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
		return result;
	}
	LPDISPATCH get_Container()
	{
		LPDISPATCH result;
		InvokeHelper(0xCA, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
		return result;
	}
	LPDISPATCH get_Document()
	{
		LPDISPATCH result;
		InvokeHelper(0xCB, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
		return result;
	}
	BOOL get_TopLevelContainer()
	{
		BOOL result;
		InvokeHelper(0xCC, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
		return result;
	}
	CString get_Type()
	{
		CString result;
		InvokeHelper(0xCD, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
		return result;
	}
	long get_Left()
	{
		long result;
		InvokeHelper(0xCE, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
		return result;
	}
	void put_Left(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0xCE, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}
	long get_Top()
	{
		long result;
		InvokeHelper(0xCF, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
		return result;
	}
	void put_Top(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0xCF, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}
	long get_Width()
	{
		long result;
		InvokeHelper(0xD0, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
		return result;
	}
	void put_Width(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0xD0, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}
	long get_Height()
	{
		long result;
		InvokeHelper(0xD1, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
		return result;
	}
	void put_Height(long newValue)
	{
		static BYTE parms[] = VTS_I4;
		InvokeHelper(0xD1, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms, newValue);
	}
	CString get_LocationName()
	{
		CString result;
		InvokeHelper(0xD2, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
		return result;
	}
	CString get_LocationURL()
	{
		CString result;
		InvokeHelper(0xD3, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
		return result;
	}
	BOOL get_Busy()
	{
		BOOL result;
		InvokeHelper(0xD4, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
		return result;
	}

// 属性
//

// DWebBrowserEvents

// 函数
//

	void BeforeNavigate(LPCTSTR URL, long Flags, LPCTSTR TargetFrameName, VARIANT * PostData, LPCTSTR Headers, BOOL * Cancel)
	{
		static BYTE parms[] = VTS_BSTR VTS_I4 VTS_BSTR VTS_UNKNOWN VTS_BSTR VTS_UNKNOWN;
		InvokeHelper(0x64, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, URL, Flags, TargetFrameName, PostData, Headers, Cancel);
	}
	void NavigateComplete(LPCTSTR URL)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x65, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, URL);
	}
	void StatusTextChange(LPCTSTR Text)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x66, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Text);
	}
	void ProgressChange(long Progress, long ProgressMax)
	{
		static BYTE parms[] = VTS_I4 VTS_I4;
		InvokeHelper(0x6C, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Progress, ProgressMax);
	}
	void DownloadComplete()
	{
		InvokeHelper(0x68, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void CommandStateChange(long Command, BOOL Enable)
	{
		static BYTE parms[] = VTS_I4 VTS_BOOL;
		InvokeHelper(0x69, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Command, Enable);
	}
	void DownloadBegin()
	{
		InvokeHelper(0x6A, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void NewWindow(LPCTSTR URL, long Flags, LPCTSTR TargetFrameName, VARIANT * PostData, LPCTSTR Headers, BOOL * Processed)
	{
		static BYTE parms[] = VTS_BSTR VTS_I4 VTS_BSTR VTS_UNKNOWN VTS_BSTR VTS_UNKNOWN;
		InvokeHelper(0x6B, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, URL, Flags, TargetFrameName, PostData, Headers, Processed);
	}
	void TitleChange(LPCTSTR Text)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x71, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Text);
	}
	void FrameBeforeNavigate(LPCTSTR URL, long Flags, LPCTSTR TargetFrameName, VARIANT * PostData, LPCTSTR Headers, BOOL * Cancel)
	{
		static BYTE parms[] = VTS_BSTR VTS_I4 VTS_BSTR VTS_UNKNOWN VTS_BSTR VTS_UNKNOWN;
		InvokeHelper(0xC8, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, URL, Flags, TargetFrameName, PostData, Headers, Cancel);
	}
	void FrameNavigateComplete(LPCTSTR URL)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0xC9, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, URL);
	}
	void FrameNewWindow(LPCTSTR URL, long Flags, LPCTSTR TargetFrameName, VARIANT * PostData, LPCTSTR Headers, BOOL * Processed)
	{
		static BYTE parms[] = VTS_BSTR VTS_I4 VTS_BSTR VTS_UNKNOWN VTS_BSTR VTS_UNKNOWN;
		InvokeHelper(0xCC, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, URL, Flags, TargetFrameName, PostData, Headers, Processed);
	}
	void Quit(BOOL * Cancel)
	{
		static BYTE parms[] = VTS_UNKNOWN;
		InvokeHelper(0x67, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Cancel);
	}
	void WindowMove()
	{
		InvokeHelper(0x6D, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void WindowResize()
	{
		InvokeHelper(0x6E, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void WindowActivate()
	{
		InvokeHelper(0x6F, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
	}
	void PropertyChange(LPCTSTR Property)
	{
		static BYTE parms[] = VTS_BSTR;
		InvokeHelper(0x70, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Property);
	}




};
