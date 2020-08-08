
// hasher2Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "upxgui.h"
#include "upxguiDlg.h"
#include "afxdialogex.h"
#include "afxwin.h"
#include <fstream>
#include <Windows.h>
CFont UpxguiDlg::Font = {};
bool UpxguiDlg::trayenable = {};
bool UpxguiDlg::minimizeen = {};
CButton* UpxguiDlg::checkbox = {};
CButton* UpxguiDlg::trayen = {};
CButton* UpxguiDlg::comp = {};
CButton* UpxguiDlg::log = {};
CButton* UpxguiDlg::decomp = {};
CButton* UpxguiDlg::admin = {};
CButton* UpxguiDlg::suspicious = {};
CButton* UpxguiDlg::backup = {};
CStatic* UpxguiDlg::statictxt = {};
CComboBox* UpxguiDlg::complvl = {};
int UpxguiDlg::boxcheck = {};
filectrl UpxguiDlg::outbrowse = {};
fileinctrl UpxguiDlg::inbrowse = {};
char UpxguiDlg::infile[500] = {'\0'};
char UpxguiDlg::outfile[500] = {'\0'};
unsigned long UpxguiDlg::type = {};
CWnd *wnd;
template<class TYPE>
bool RegSetKey(HKEY key, LPSTR keyloc, unsigned long type, REGSAM access, LPSTR name, TYPE indatax);
int RegCrtKey(HKEY key, LPSTR keyloc, REGSAM access);
template<class TYPE>
int RegGetKey(HKEY key, LPSTR keyloc, unsigned long type, REGSAM access, LPSTR name, TYPE outdatax);
#pragma warning( disable:4244)
#pragma warning(disable:6387)
#pragma warning(disable:6011)
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
using namespace std;
void UpxguiDlg::updatevars(bool type, char *filename)
{
	if (type == 1)
	{
		//ZeroMemory(infile, sizeof(infile));
		//memcpy_s(infile, sizeof(infile), "\0", 2);
		strcpy(infile, filename);
		//strcpy(indfile, filename);
	}
	else
	{
		//ZeroMemory(outfile, sizeof(outfile));
		//memcpy_s(outfile, sizeof(outfile), "\0", 2);
		strcpy(outfile, filename);
		//strcpy(outdfile, filename);
	}
}
void filectrl::OnBrowse()
{
	UpxguiDlg dlg;
	CFileDialog filedialog(FALSE, ".exe", NULL, OFN_NOTESTFILECREATE | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, CString("Application (*.exe)|*.exe|"), wnd, NULL, true);
	INT_PTR res = filedialog.DoModal();
	if (res == IDOK)
	{
		char buffer[700] = {};
		CT2A file(filedialog.GetFileName());
		CT2A path(filedialog.GetFolderPath());
		strcpy(buffer, path.m_psz);
		strcat(buffer, "\\");
		strcat(buffer, file.m_psz);
		SetWindowText(buffer);
		dlg.updatevars(0, buffer);
	}
	else if (res == IDCANCEL || res == IDABORT)
	{
		dlg.updatevars(0, "\0");
		SetWindowText("");
	}
}
void fileinctrl::OnBrowse()
{
	UpxguiDlg dlg;
	CFileDialog filedialog(TRUE, ".exe", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, CString("Application (*.exe)|*.exe|"), wnd, NULL, true);
	INT_PTR res = filedialog.DoModal();
	if (res == IDOK)
	{
		char buffer[700] = {};
		CT2A file(filedialog.GetFileName());
		CT2A path(filedialog.GetFolderPath());
		strcpy(buffer, path.m_psz);
		strcat(buffer, "\\");
		strcat(buffer, file.m_psz);
		SetWindowText(buffer);
		dlg.updatevars(1, buffer);
	}
	else if (res == IDCANCEL || res == IDABORT)
	{
		dlg.updatevars(1, "\0");
		SetWindowText("");
	}
}
UpxguiDlg::UpxguiDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_UPXGUI_DIALOG, pParent)
{
	m_nidIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_nidIconData.hWnd = 0;
	m_nidIconData.uID = 1;
	m_nidIconData.uCallbackMessage = WM_TRAY_ICON_NOTIFY_MESSAGE;
	m_nidIconData.hIcon = 0;
	m_nidIconData.szTip[0] = 0;
	m_nidIconData.uFlags = NIF_MESSAGE;
	m_bTrayIconVisible = FALSE;
	m_nDefaultMenuItem = 0;
	m_bMinimizeToTray = TRUE;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_ICON);
}

void UpxguiDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_FILEOUT, outbrowse);
	DDX_Control(pDX, IDC_FILEIN, inbrowse);
	CDialog::DoDataExchange(pDX);
}
template<class TYPE>
bool RegSetKey(HKEY key,LPSTR keyloc,unsigned long type, REGSAM access,LPSTR name,TYPE indatax)
{
	unsigned long size = sizeof(type);
	char errorbuf[200];
	HKEY keyval;
	bool onerr = 1;
	int err;
	err = RegOpenKeyExA(key, keyloc, NULL, access, &keyval);
	if (err != ERROR_SUCCESS)
	{
		sprintf_s(errorbuf, "%i\n", err);
		onerr = 0;
		ASSERT(errorbuf);
	} else if(err == ERROR_SUCCESS)
	{ 
		err = RegSetValueExA(keyval, name, NULL, type, (BYTE*)indatax, size);
		if (err != ERROR_SUCCESS)
		{
			sprintf_s(errorbuf, "%i\n", err);
			onerr = 0;
			ASSERT(errorbuf);
		}
	}
	
		CloseHandle(keyval);
	return onerr;
}
int RegCrtKey(HKEY key, LPSTR keyloc, REGSAM access)
{
	HKEY keyval;
	int err;
	char errorbuf[200];
	DWORD dispvalue;
	err = RegCreateKeyExA(key, keyloc, NULL, NULL, REG_OPTION_NON_VOLATILE, access,NULL, &keyval, &dispvalue);
	CloseHandle(keyval);
	if (err == ERROR_SUCCESS)
	{
		if (dispvalue == REG_CREATED_NEW_KEY)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
	else
	{
		sprintf_s(errorbuf, "%i\n", err);
		ASSERT(errorbuf);
		return 0;
	}
	//return onerr;
}
template<class TYPE>
int RegGetKey(HKEY key, LPSTR keyloc, unsigned long type, REGSAM access, LPSTR name, TYPE outdatax)
{
	unsigned long size = sizeof(type);
	char errorbuf[200];
	HKEY keyval;
	int onerr = 0;
	int err;
	err = RegOpenKeyExA(key, keyloc, NULL, access, &keyval);
	if (err != ERROR_SUCCESS)
	{
		onerr = false;
	}
	err = RegQueryValueExA(keyval, name, NULL, &type, (BYTE*)outdatax, &size);
	switch (err)
	{
	case ERROR_FILE_NOT_FOUND:
	{
		onerr = 2;
		break;
	} 
	case ERROR_MORE_DATA:
	{
		onerr = 3;
		break;
	}
	case ERROR_SUCCESS:
	{
		onerr = 1;
		break;
	}
	default:
	{
		sprintf_s(errorbuf, "%i\n", err);
		ASSERT(errorbuf);
		onerr = 0;
		break;
	}
	}
		CloseHandle(keyval);
	return onerr;
}
BEGIN_MESSAGE_MAP(UpxguiDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_EXIT, &UpxguiDlg::OnBnClickedExit)
	ON_MESSAGE(WM_TRAY_ICON_NOTIFY_MESSAGE, OnTrayNotify)
	ON_BN_CLICKED(IDC_TRAYEN, &UpxguiDlg::OnBnClickedTrayEn)
	ON_BN_CLICKED(IDC_MINTRAY, &UpxguiDlg::OnBnClickedMinEn)
	ON_COMMAND(ID_MENU_CLOSE, &UpxguiDlg::OnBnClickedExit)
	ON_COMMAND(ID_MENU_OPEN, &UpxguiDlg::OnOpen)
	ON_COMMAND(ID_MENU_MINIMIZE, &UpxguiDlg::OnMinimize)
	ON_BN_CLICKED(IDC_COMP, &UpxguiDlg::OnBnClickedComp)
	ON_BN_CLICKED(IDC_DECOMP, &UpxguiDlg::OnBnClickedDecomp)
	ON_BN_CLICKED(IDC_ENTER, &UpxguiDlg::OnBnClickedEnter)
END_MESSAGE_MAP()

BOOL UpxguiDlg::OnInitDialog()
{
	ZeroMemory(infile, 500);
	ZeroMemory(outfile, 500);
	int out;
	DWORD indata =  1;
	DWORD outdata = 0;
	BYTE cmp = 1;
	type = REG_DWORD;
	Font.CreateFont(12,                            // Height
		0,                             // Width	
		0,                             // Escapement
		0,                             // Orientation
		FW_NORMAL,                       // Weight
		FALSE,                         // Italic
		FALSE,                          // Underline
		0,                             // StrikeOut
		ANSI_CHARSET,                  // CharSet
		OUT_DEFAULT_PRECIS,            // OutPrecision
		CLIP_DEFAULT_PRECIS,           // ClipPrecision
		DEFAULT_QUALITY,               // Quality
		DEFAULT_PITCH | FF_SWISS,      // PitchAndFamily
		"Arial");
	//AfxInitRichEdit2();
	UpxguiDlg::ShowWindow(SW_SHOW);
	UpxguiDlg::RedrawWindow();
	UpxguiDlg::CenterWindow();
	CDialog::OnInitDialog();
	out = RegCrtKey(HKEY_CURRENT_USER, "Software\\UpxGui", KEY_ALL_ACCESS | KEY_WOW64_64KEY);
	if (out == 1)
	{
		RegSetKey(HKEY_CURRENT_USER, "Software\\UpxGui", REG_DWORD, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayEnable", &indata);
		RegSetKey(HKEY_CURRENT_USER, "Software\\UpxGui", REG_DWORD, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayMinimize", &indata);
	}
	else if(out == 0)
	{
		trayenable = 1;
		minimizeen = 1;
		m_bMinimizeToTray = TRUE;
	}
	else if (out == 2)
	{
		out = RegGetKey(HKEY_CURRENT_USER, "Software\\UpxGui", NULL, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayMinimize", &outdata);
		if (out == 2)
		{
			
			RegSetKey(HKEY_CURRENT_USER, "Software\\UpxGui", REG_DWORD, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayMinimize", &indata);
			
			minimizeen = 1;
			m_bMinimizeToTray = TRUE;
		} else if (out == 1)
		{ 
			
			if (outdata == 1)
			{
				minimizeen = 1;
				m_bMinimizeToTray = TRUE;
			}
			else 
			{
				minimizeen = 0;
				m_bMinimizeToTray = FALSE;
			}
		}
		out = RegGetKey(HKEY_CURRENT_USER, "Software\\UpxGui", REG_DWORD, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayEnable", &outdata);
		if (out == 2)
		{

			RegSetKey(HKEY_CURRENT_USER, "Software\\UpxGui", REG_DWORD, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayMinimize", &indata);
			trayenable = 1;
		}
		else if (out == 1)
		{ 
			if (outdata == 1)
			{
				trayenable = 1;
			}
			else
			{
				trayenable = 0;
			}
		}
	}
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	TraySetIcon(m_hIcon);
	TraySetToolTip("HashGen Tray icon");
	TraySetMenu(ID_MENU_MINIMIZE);
	
	if (UpxguiDlg::IsWindowVisible() != 0)
	{
		wnd = FromHandle(this->m_hWnd);
		comp = (CButton*)GetDlgItem(IDC_COMP);
		admin = (CButton*)GetDlgItem(IDC_ADMIN);
		decomp = (CButton*)GetDlgItem(IDC_DECOMP);
		log = (CButton*)GetDlgItem(IDC_LOG);
		suspicious = (CButton*)GetDlgItem(IDC_SUSPICIOUS);
		backup = (CButton*)GetDlgItem(IDC_BACKUP);
		statictxt = (CStatic*)GetDlgItem(IDC_STATICTXT);
		checkbox = (CButton*)GetDlgItem(IDC_MINTRAY);
		trayen = (CButton*)GetDlgItem(IDC_TRAYEN);
		complvl = (CComboBox*)GetDlgItem(IDC_COMPLVL);
		comp->SetCheck(1);
		decomp->SetCheck(0);
		complvl->SetCurSel(0);
		if (minimizeen)
		{
			CheckDlgButton(IDC_MINTRAY, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(IDC_MINTRAY, BST_UNCHECKED);
		}
		if (trayenable)
		{
			CheckDlgButton(IDC_TRAYEN, BST_CHECKED);
			TrayShow();
		}
		else
		{
			CheckDlgButton(IDC_TRAYEN, BST_UNCHECKED);
			TrayHide();
		}
		
	}
	
	return TRUE;
}
void UpxguiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	UpxguiDlg::ShowWindow(SW_SHOW);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR UpxguiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void UpxguiDlg::OnBnClickedExit()
{
	//UpxguiDlg::OnDestroy();
	//exit(2);
	PostQuitMessage(2);
}

void UpxguiDlg::OnBnClickedMinEn()
{
	DWORD indata = 0;
	boxcheck = checkbox->GetCheck();
	if (boxcheck == BST_CHECKED)
	{
		indata = 1;
		RegSetKey(HKEY_CURRENT_USER, "Software\\UpxGui", REG_DWORD, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayMinimize", &indata);
		m_bMinimizeToTray = TRUE;
		minimizeen = true;
	}
	else
	{
		indata = 0;
		RegSetKey(HKEY_CURRENT_USER, "Software\\UpxGui", REG_DWORD, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayMinimize", &indata);
		m_bMinimizeToTray = FALSE;
		minimizeen = false;
	}
}
void UpxguiDlg::OnBnClickedTrayEn()
{
	int tren;
	DWORD indata = 0;
	tren = trayen->GetCheck();
	if (tren == BST_CHECKED)
	{
		indata = 1;
		RegSetKey(HKEY_CURRENT_USER, "Software\\UpxGui", REG_DWORD, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayEnable", &indata);
		trayenable = true;
		checkbox->EnableWindow();
		TrayShow();
	}
	else
	{
		indata = 0;
		RegSetKey(HKEY_CURRENT_USER, "Software\\UpxGui", REG_DWORD, KEY_ALL_ACCESS | KEY_WOW64_64KEY, "TrayEnable", &indata);
		trayenable = false;
		m_bMinimizeToTray = FALSE;
		checkbox->EnableWindow(0);
		TrayHide();
	}
}
void UpxguiDlg::OnOpen()
{
	if (this->IsWindowVisible())
	{ 
		this->ShowWindow(SW_NORMAL);
		this->SetFocus();
	}
	else
	{	
		this->ShowWindow(SW_SHOW);
		this->SetFocus();
	}
	
	
}

int UpxguiDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_nidIconData.hWnd = this->m_hWnd;
	m_nidIconData.uID = 1;

	return 0;
}

void UpxguiDlg::OnDestroy()
{
	CDialog::OnDestroy();
	if (m_nidIconData.hWnd && m_nidIconData.uID > 0 && TrayIsVisible())
	{
		Shell_NotifyIcon(NIM_DELETE, &m_nidIconData);
	}
}

BOOL UpxguiDlg::TrayIsVisible()
{
	return m_bTrayIconVisible;
}

void UpxguiDlg::TraySetIcon(HICON hIcon)
{
	ASSERT(hIcon);

	m_nidIconData.hIcon = hIcon;
	m_nidIconData.uFlags |= NIF_ICON;
}

void UpxguiDlg::TraySetIcon(UINT nResourceID)
{
	ASSERT(nResourceID > 0);
	HICON hIcon = 0;
	hIcon = AfxGetApp()->LoadIcon(nResourceID);
	if (hIcon)
	{
		m_nidIconData.hIcon = hIcon;
		m_nidIconData.uFlags |= NIF_ICON;
	}
	else
	{
		TRACE0("FAILED TO LOAD ICON\n");
	}
}

void UpxguiDlg::TraySetIcon(LPCTSTR lpszResourceName)
{
	HICON hIcon = 0;
	hIcon = AfxGetApp()->LoadIcon(lpszResourceName);
	if (hIcon)
	{
		m_nidIconData.hIcon = hIcon;
		m_nidIconData.uFlags |= NIF_ICON;
	}
	else
	{
		TRACE0("FAILED TO LOAD ICON\n");
	}
}

void UpxguiDlg::TraySetToolTip(LPCTSTR lpszToolTip)
{
	ASSERT(strlen((char*)lpszToolTip) > 0 && strlen((char*)lpszToolTip) < 64);
#if defined(UNICODE) || defined(_UNICODE)
	wcscpy(m_nidIconData.szTip,lpszToolTip);
#else
	strcpy(m_nidIconData.szTip, lpszToolTip);
#endif


	m_nidIconData.uFlags |= NIF_TIP;
}

BOOL UpxguiDlg::TrayShow()
{
	BOOL bSuccess = FALSE;
	if (!m_bTrayIconVisible && trayenable)
	{
		bSuccess = Shell_NotifyIcon(NIM_ADD, &m_nidIconData);
		if (bSuccess)
			m_bTrayIconVisible = TRUE;
	}
	else
	{
		TRACE0("ICON ALREADY VISIBLE");
	}
	return bSuccess;
}

BOOL UpxguiDlg::TrayHide()
{
	BOOL bSuccess = FALSE;
	if (m_bTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_DELETE, &m_nidIconData);
		if (bSuccess)
			m_bTrayIconVisible = FALSE;
	}
	else
	{
		TRACE0("ICON ALREADY HIDDEN");
	}
	bSuccess = TRUE;
	return bSuccess;
}

BOOL UpxguiDlg::TrayUpdate()
{
	BOOL bSuccess = FALSE;
	if (m_bTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_MODIFY, &m_nidIconData);
	}
	else
	{
		TRACE0("ICON NOT VISIBLE");
	}
	return bSuccess;
}


BOOL UpxguiDlg::TraySetMenu(UINT nResourceID, UINT nDefaultPos)
{
	BOOL bSuccess;
	bSuccess = m_mnuTrayMenu.LoadMenu(nResourceID);
	return bSuccess;
}


BOOL UpxguiDlg::TraySetMenu(LPCTSTR lpszMenuName, UINT nDefaultPos)
{
	BOOL bSuccess;
	bSuccess = m_mnuTrayMenu.LoadMenu(lpszMenuName);
	return bSuccess;
}

BOOL UpxguiDlg::TraySetMenu(HMENU hMenu, UINT nDefaultPos)
{
	m_mnuTrayMenu.Attach(hMenu);
	return TRUE;
}

LRESULT UpxguiDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
	UINT uID;
	UINT uMsg;

	uID = (UINT)wParam;
	uMsg = (UINT)lParam;

	if (uID != 1)
		return false;

	CPoint pt;

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
		GetCursorPos(&pt);
		ClientToScreen(&pt);
		OnTrayMouseMove(pt);
		break;
	case WM_LBUTTONDOWN:
		GetCursorPos(&pt);
		ClientToScreen(&pt);
		OnTrayLButtonDown(pt);
		break;
	case WM_LBUTTONDBLCLK:
		GetCursorPos(&pt);
		ClientToScreen(&pt);
		OnTrayLButtonDblClk(pt);
		break;

	case WM_RBUTTONDOWN:
	case WM_CONTEXTMENU:
		GetCursorPos(&pt);
		//ClientToScreen(&pt);
		OnTrayRButtonDown(pt);
		break;
	case WM_RBUTTONDBLCLK:
		GetCursorPos(&pt);
		ClientToScreen(&pt);
		OnTrayRButtonDblClk(pt);
		break;
	}
	return true;
}
void UpxguiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (m_bMinimizeToTray)
	{
		if ((nID & 0xFFF0) == SC_MINIMIZE)
		{
		
			if (TrayShow() || m_bTrayIconVisible)
			{
				this->ShowWindow(SW_HIDE);
			}
		}
		else
		{
			CDialog::OnSysCommand(nID, lParam);
		}
	}
	else
	{ 
		CDialog::OnSysCommand(nID, lParam);
	}
}
void UpxguiDlg::TraySetMinimizeToTray(BOOL bMinimizeToTray)
{
	m_bMinimizeToTray = bMinimizeToTray;
}


void UpxguiDlg::OnTrayRButtonDown(CPoint pt)
{
	m_mnuTrayMenu.GetSubMenu(0)->TrackPopupMenu(TPM_BOTTOMALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	m_mnuTrayMenu.GetSubMenu(0)->SetDefaultItem(m_nDefaultMenuItem, TRUE);
}

void UpxguiDlg::OnTrayLButtonDown(CPoint pt)
{

}

void UpxguiDlg::OnTrayLButtonDblClk(CPoint pt)
{
	if (m_bMinimizeToTray)
		if (TrayHide())
		{ 
			this->ShowWindow(SW_SHOW);
			this->SetFocus();
		}
}

void UpxguiDlg::OnTrayRButtonDblClk(CPoint pt)
{
}

void UpxguiDlg::OnTrayMouseMove(CPoint pt)
{
}
void UpxguiDlg::OnMinimize()
{
	if (this->IsWindowVisible())
	{
		if (trayenable && !minimizeen)
		{
			ShowWindow(SW_MINIMIZE);
		}
		else if (trayenable && minimizeen)
		{
			ShowWindow(SW_HIDE);
		}
	}
	else
	{
		ShowWindow(SW_SHOW);
		SetFocus();
	}
}

void UpxguiDlg::OnBnClickedComp()
{
	comp->SetCheck(1);
	decomp->SetCheck(0);
	statictxt->EnableWindow(1);
	suspicious->EnableWindow(1);
	complvl->EnableWindow(1);
}


void UpxguiDlg::OnBnClickedDecomp()
{
	comp->SetCheck(0);
	decomp->SetCheck(1);
	statictxt->EnableWindow(0);
	suspicious->EnableWindow(0);
	complvl->EnableWindow(0);
}


void UpxguiDlg::OnBnClickedEnter()
{
	UINT MASK;
	bool mask;
	char path[MAX_PATH] = { '\0' }, drive[3] = { '\0' }, dir[MAX_PATH] = { '\0' }, msg[250] = { '\0' }, buff[MAX_PATH * 2] = { '\0' }, cmdline[300] = { '\0' }, tmp[MAX_PATH] = { '\0' };
	SHELLEXECUTEINFOA ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = this->m_hWnd;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = path;
	ShExecInfo.lpParameters = cmdline;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	GetModuleFileName(NULL, path, sizeof(path));
	_splitpath(path, drive, dir, new char[0], new char[0]);
	ZeroMemory(path, sizeof(path));
	strcpy(path, drive);
	strcat(path, dir);
	strcat(path, "\\upx.exe");
	
	if (!PathFileExistsA(path))
	{
		MessageBox("ERROR: UPX.exe is not installed in a same location as this program. please install it before use.","ERROR upx.exe",MB_OK | MB_ICONERROR | MB_TASKMODAL);
		goto end;
	}
	if (admin->GetCheck() == BST_CHECKED)
	{
		ShExecInfo.lpVerb = "runas";
	}
	if (suspicious->GetCheck() == BST_CHECKED && decomp->GetCheck() == BST_UNCHECKED)
	{
		strcat_s(cmdline, sizeof(cmdline), "-f ");
	}
	if (backup->GetCheck() == BST_CHECKED)
	{
		strcat_s(cmdline, sizeof(cmdline), "-k ");
	}
	if (strcmp(outfile, "\0"))
	{
		char buf[MAX_PATH + 6] = { '\0' };
		sprintf_s(buf, "-o \"%s\"", outfile);
		strcat_s(cmdline, sizeof(cmdline), buf);
		strcat_s(cmdline, sizeof(cmdline), " ");
	}
	if (decomp->GetCheck() == BST_UNCHECKED)
	{
		char str[5];
		GetDlgItemTextA(IDC_COMPLVL, str, sizeof(str));
		sprintf_s(buff, "-%s", str);
		strcat_s(cmdline, sizeof(cmdline), buff);
		strcat_s(cmdline, sizeof(cmdline), " ");
	}
	else if (decomp->GetCheck() == BST_CHECKED)
	{
		strcat_s(cmdline, sizeof(cmdline), "-d ");
	}
	ZeroMemory(buff, sizeof(buff));
	sprintf_s(buff, "\"%s\"", infile);
	strcat_s(cmdline, sizeof(cmdline), buff);
	GetEnvironmentVariable("TEMP", tmp, sizeof(tmp));
	//ZeroMemory(buff, sizeof(buff));
	//sprintf_s(buff, " >>%s\\upxgui.log", tmp);
	//strcat_s(cmdline, sizeof(cmdline), buff);
	sprintf_s(msg, "command line is:\n%s", cmdline);
	MessageBox(msg, "Debug info", MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
	ShellExecuteExA(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	DWORD x = -1;
	GetExitCodeProcess(ShExecInfo.hProcess, &x);
	ZeroMemory(msg, sizeof(msg));
	if (comp->GetCheck() == BST_CHECKED)
	{
		switch (x)
		{
		case 0:
		{
			mask = true;
			sprintf_s(msg, "File has been succesfully compressed.");
			break;
		}
		case 2:
		{
			mask = false;
			sprintf_s(msg, "ERROR:File has been already packed with UPX.");
			break;
		}
		case 1:
		{
			mask = false;
			sprintf_s(msg, "ERROR:.NET applications are not supported or specified file is not valid executeable.");
			break;
		}
		default:
		{
			mask = false;
			sprintf_s(msg, "ERROR:Unspecified error");
			break;
		}
		}
	}
	else
	{
		switch (x)
		{
		case 0:
		{
			mask = false;
			sprintf_s(msg, "File has been succesfully decompressed.");
			break;
		}
		case 2:
		{
			mask = false;
			sprintf_s(msg, "ERROR:File has not been packed with UPX.");
			break;
		}
		case 1:
		{
			mask = false;
			sprintf_s(msg, "ERROR:.NET applications are not supported or specified file is not valid executeable.");
			break;
		}
		default:
		{
			mask = false;
			sprintf_s(msg, "ERROR:Unspecified error");
			break;
		}
		}
	}
	if (mask == true)
	{
		MASK = MB_ICONINFORMATION;
	}
	else
	{
		MASK = MB_ICONERROR;
	}

		MessageBox(msg, "File compression result", MB_OK | MASK | MB_TASKMODAL);
	end:
		{}
		/*if (log->GetCheck() == BST_CHECKED)
		{
			char buffer[MAX_PATH];
			sprintf_s(buffer, "%s\\upxgui.log", tmp);
			SHELLEXECUTEINFOA ShExecInfoA = { 0 };
			ShExecInfoA.cbSize = sizeof(SHELLEXECUTEINFOA);
			ShExecInfoA.fMask = SEE_MASK_DEFAULT;
			ShExecInfoA.hwnd = NULL;
			ShExecInfoA.lpVerb = "edit";
			ShExecInfoA.lpFile = buffer;
			ShExecInfoA.lpParameters = "";
			ShExecInfoA.lpDirectory = NULL;
			ShExecInfoA.nShow = SW_SHOW;
			ShExecInfoA.hInstApp = NULL;
			ShellExecuteExA(&ShExecInfoA);
			WaitForSingleObject(ShExecInfoA.hProcess, INFINITE);
		}
		DeleteFile(buff);
		char fullcmdline[500];
		sprintf_s(fullcmdline, "%s%s\0",path,cmdline);
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		si.cb = sizeof(STARTUPINFO);
		si.hStdError = out_wr;
		si.hStdOutput = out_wr;
		si.hStdInput = in_rd;
		si.dwFlags |= STARTF_USESTDHANDLES;
		executeprogram(fullcmdline,NULL,si,pi);*/
		
}
/* //maybe future use
bool UpxguiDlg::ExtractResource(uint16_t ResourceID, char* OutputFileName, char* path, const char* ResType)
{
	try
	{
		HRSRC hResource = FindResourceA(nullptr, MAKEINTRESOURCEA(ResourceID), ResType);
		if (hResource == nullptr)
		{
			return false;
		}

		HGLOBAL hFileResource = LoadResource(nullptr, hResource);
		if (hFileResource == nullptr)
		{
			return false;
		}

		void* lpFile = LockResource(hFileResource);
		if (lpFile == nullptr)
		{
			return false;
		}

		std::uint32_t dwSize = SizeofResource(nullptr, hResource);
		if (dwSize == 0)
		{
			return false;
		}
		char paths[300];
		sprintf_s(paths, "%s\\%s", path, OutputFileName);
		HANDLE hFile = CreateFileA(paths, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		HANDLE hFilemap = CreateFileMappingA(hFile, nullptr, PAGE_READWRITE, 0, dwSize, nullptr);
		if (hFilemap == nullptr)
		{
			return false;
		}

		void* lpBaseAddress = MapViewOfFile(hFilemap, FILE_MAP_WRITE, 0, 0, 0);
		CopyMemory(lpBaseAddress, lpFile, dwSize);
		UnmapViewOfFile(lpBaseAddress);
		CloseHandle(hFilemap);
		CloseHandle(hFile);

		return true;
	}
	catch (...) {}
	return false;
}
*/