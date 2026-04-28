
// hasher2Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "upxgui.h"
#include "upxguiDlg.h"
#include "upx_inproc.h"
#include "afxdialogex.h"
#include "afxwin.h"
#include <fstream>
#include <memory>
#include <array>
#include <string>
#include <vector>
#include <Windows.h>
UpxguiDlg* dialog = nullptr;
CWnd *wnd;
HWND hwnd;
template<class TYPE>
bool RegSetKey(HKEY key, LPSTR keyloc, unsigned long type, REGSAM access, LPSTR name, TYPE indatax);
int RegCrtKey(HKEY key, LPSTR keyloc, REGSAM access);
template<class TYPE>
int RegGetKey(HKEY key, LPSTR keyloc, unsigned long type, REGSAM access, LPSTR name, TYPE outdatax);
#pragma warning(disable:4244)
#pragma warning(disable:6387)
#pragma warning(disable:6011)
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
using namespace std;

namespace {

constexpr size_t MaxSelectedFiles = 210;
constexpr size_t FileDialogBufferSize = MAX_PATH * MaxSelectedFiles;
constexpr UINT UpxThreadStackSize = 8 * 1024 * 1024;

struct UpxOperationJob
{
	HWND hwnd = nullptr;
	vector<string> arguments;
	bool compressMode = true;
};

struct UpxOperationResult
{
	int exitCode = 1;
	bool compressMode = true;
};

bool IsRunningAsAdmin()
{
	BOOL is_admin = FALSE;
	PSID administrators_group = nullptr;
	SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administrators_group))
	{
		CheckTokenMembership(nullptr, administrators_group, &is_admin);
		FreeSid(administrators_group);
	}
	return is_admin == TRUE;
}

void ClearFileSelection()
{
	array<char, FileDialogBufferSize> buffer = {};
	array<size_t, MaxSelectedFiles> lens = {};
	dialog->updatevars(true, false, true, true, buffer.data(), lens.data(), 0);
}

UINT RunUpxOperationThread(LPVOID param)
{
	unique_ptr<UpxOperationJob> job(static_cast<UpxOperationJob*>(param));
	unique_ptr<UpxOperationResult> result(new UpxOperationResult);
	result->compressMode = job->compressMode;
	result->exitCode = upxgui::RunEmbeddedUpx(job->arguments);

	if (!PostMessage(job->hwnd, WM_UPX_OPERATION_FINISHED, 0, reinterpret_cast<LPARAM>(result.get())))
	{
		return 0;
	}
	result.release();
	return 0;
}

}

void UpxguiDlg::updatevars(bool setmulti, bool multi,bool enfile,bool type, char *filename,size_t *lens,int filecount)
{
	if (setmulti)
	{
		multiplefile = multi;
		if (multi)
		{
			outbrowse.ShowWindow(SW_HIDE);
			outtext->ShowWindow(SW_HIDE);
		}
		else
		{
			outbrowse.ShowWindow(SW_SHOW);
			outtext->ShowWindow(SW_SHOW);
		}
		if (enout != nullptr)
		{
			enout->EnableWindow(!upxrunning && !multiplefile);
			RefreshOutputFileControls(!upxrunning);
		}
	}
	if (enfile)
	{
		if (type == 1)
		{
			char *ptr = infile;
			//ZeroMemory(infile, sizeof(infile));
			//memcpy_s(infile, sizeof(infile), "\0", 2);
			memcpy(infile, filename,MAX_PATH*210);
			filecounts = filecount;
			if (lens != nullptr)
			{
				memcpy_s(lensf, sizeof(size_t*) * 210, lens, sizeof(size_t*) * 210);
				for (int d = 0; d < filecounts; d++)
				{
					filesptr[d] = ptr;
					ptr = ptr + lensf[d]+1;
				}
			}
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
}
void filectrl::OnBrowse()
{
	CFileDialog filedialog(FALSE, ".exe", NULL, OFN_NOTESTFILECREATE | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, CString("Application (*.exe)|*.exe|"), wnd, NULL, true);
	INT_PTR res = filedialog.DoModal();
	if (res == IDOK)
	{
		array<char, MAX_PATH * 2> buffer = {};
		CT2A file(filedialog.GetFileName());
		CT2A path(filedialog.GetFolderPath());
		sprintf_s(buffer.data(), buffer.size(), "%s\\%s", path.m_psz, file.m_psz);
		SetWindowText(buffer.data());
		dialog->updatevars(false, false, true, false, buffer.data(), nullptr, 0);
	}
	else
	{
		char buffer[MAX_PATH] = { '\0' };
		dialog->updatevars(true, false, true, false, buffer, nullptr, 0);
	}
}
void fileinctrl::OnBrowse()
{
	array<size_t, MaxSelectedFiles> lens = {};
	vector<char> selectedFiles(FileDialogBufferSize, '\0');
	static const char filter[] = "Application (*.exe)\0*.exe\0All Files (*.*)\0*.*\0\0";
	CFileDialog filedialog(TRUE);
	OPENFILENAME& ofn=filedialog.GetOFN();
	ofn.Flags |= OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = ".exe";
	ofn.lpstrFilter = filter;
	ofn.hwndOwner = hwnd;
	ofn.nMaxFile = static_cast<DWORD>(selectedFiles.size());
	ofn.lpstrFile = selectedFiles.data();
	INT_PTR res = filedialog.DoModal();
	if (res == IDOK)
	{
		UINT i = 0;
		size_t leng = 0;
		array<char, MAX_PATH> smalbuf = {};
		array<char, FileDialogBufferSize> buffer = {};
		int filecount=0;
		array<char*, MaxSelectedFiles> filenamesptr = {};
		size_t len = { 0 };
		char* folderpath = selectedFiles.data();
		char* bufstart = selectedFiles.data();
		len = strlen(bufstart);
		char* filestart = bufstart + len+1;
		cmatch match;
		//(^[a-zA-Z0-9:\\]+$)+(?!^[a-zA-Z0-9:\\]+$^\\.exe$)+
		regex reg("(\\.exe$)+");
		if (regex_search(bufstart, match, reg))
		{
			filecount = 1;
			sprintf_s(buffer.data(), buffer.size(), "%s", bufstart);
			lens[0] = strlen(buffer.data());
			goto skip;
		}
		
		while(i < MaxSelectedFiles)
		{
			len = strlen(filestart);
			if(len != 0)
			{ 
			filenamesptr[i] = filestart;
			filecount++;
			filestart = filestart + len+1;
			i++;
			}
			else
			{
				break;
			}
		}
		//size_t lengs = 0;
		for (int o = 0; o < filecount; o++)
		{
			
			sprintf_s(smalbuf.data(), smalbuf.size(), "%s\\%s", folderpath, filenamesptr[o]);
			lens[o] = strlen(smalbuf.data());
			if (leng + lens[o] + 1 >= buffer.size())
			{
				break;
			}
			memcpy(buffer.data() + leng, smalbuf.data(), lens[o] + 1);
			leng = leng+lens[o]+1;
			//lengs = lengs + strlen(buffer+lengs);
			buffer[leng] = *"\0";
		}
		skip:
		//CT2A file(filedialog.GetFileName());
		//CT2A path(filedialog.GetFolderPath());
		//strcpy(buffer, path.m_psz);
		//strcat(buffer, "\\");
		//strcat(buffer, file.m_psz);
		SetWindowText(buffer.data());
		if (filecount == 1)
		{
			dialog->updatevars(true, false);
		}
		else if (filecount > 1)
		{
			dialog->updatevars(true, true);
		}
		dialog->updatevars(false,false,true,true,buffer.data(),lens.data(),filecount);
	}
	else
	{
		ClearFileSelection();
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
	dialog = this;
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
	ON_BN_CLICKED(IDC_ADMIN, &UpxguiDlg::OnBnClickedAdmin)
	ON_BN_CLICKED(IDC_ENTER, &UpxguiDlg::OnBnClickedEnter)
	ON_BN_CLICKED(IDC_ENOUT, &UpxguiDlg::OnBnClickedEnout)
	ON_MESSAGE(WM_UPX_OPERATION_FINISHED, &UpxguiDlg::OnUpxOperationFinished)
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
		hwnd = this->m_hWnd;
		comp = (CButton*)GetDlgItem(IDC_COMP);
		admin = (CButton*)GetDlgItem(IDC_ADMIN);
		decomp = (CButton*)GetDlgItem(IDC_DECOMP);
		log = (CButton*)GetDlgItem(IDC_LOG);
		suspicious = (CButton*)GetDlgItem(IDC_SUSPICIOUS);
		backup = (CButton*)GetDlgItem(IDC_BACKUP);
		enout = (CButton*)GetDlgItem(IDC_ENOUT);
		statictxt = (CStatic*)GetDlgItem(IDC_STATICTXT);
		checkbox = (CButton*)GetDlgItem(IDC_MINTRAY);
		trayen = (CButton*)GetDlgItem(IDC_TRAYEN);
		complvl = (CComboBox*)GetDlgItem(IDC_COMPLVL);
		outtext = (CStatic*)GetDlgItem(IDC_OUTEXT);
		comp->SetCheck(1);
		decomp->SetCheck(0);
		enout->SetCheck(BST_UNCHECKED);
		complvl->SetCurSel(8);
		RefreshOutputFileControls(TRUE);
		if (IsRunningAsAdmin())
		{
			admin->SetWindowTextA("Running as admin");
			admin->EnableWindow(FALSE);
		}
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
	if (upxrunning)
	{
		MessageBox("UPX operation is still running.", "UPX GUI", MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
		return;
	}
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
	if (upxrunning && (nID & 0xFFF0) == SC_CLOSE)
	{
		MessageBox("UPX operation is still running.", "UPX GUI", MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
		return;
	}
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

vector<string> UpxguiDlg::BuildUpxArguments()
{
	vector<string> arguments;
	arguments.emplace_back("upx");
	if (suspicious->GetCheck() == BST_CHECKED && decomp->GetCheck() == BST_UNCHECKED)
	{
		arguments.emplace_back("-f");
	}
	if (backup->GetCheck() == BST_CHECKED)
	{
		arguments.emplace_back("-k");
	}
	CString outputPath;
	GetDlgItemText(IDC_FILEOUT, outputPath);
	outputPath.Trim();
	if (enout != nullptr && enout->GetCheck() == BST_CHECKED && !outputPath.IsEmpty() && multiplefile == false)
	{
		CT2A outputPathA(outputPath);
		arguments.emplace_back("-o");
		arguments.emplace_back(outputPathA.m_psz);
	}
	if (decomp->GetCheck() == BST_UNCHECKED)
	{
		char level[5] = { '\0' };
		GetDlgItemTextA(IDC_COMPLVL, level, sizeof(level));
		arguments.emplace_back(string("-") + level);
	}
	else
	{
		arguments.emplace_back("-d");
	}
	for (int i = 0; i < filecounts; ++i)
	{
		arguments.emplace_back(filesptr[i]);
	}
	return arguments;
}

int UpxguiDlg::RunUpxOperation(const vector<string>& arguments)
{
	return upxgui::RunEmbeddedUpx(arguments);
}

void UpxguiDlg::SetUpxControlsEnabled(BOOL enabled)
{
	GetDlgItem(IDC_ENTER)->EnableWindow(enabled);
	GetDlgItem(IDC_ENTER)->SetWindowTextA(enabled ? "(De)Compress" : "Working...");
	GetDlgItem(ID_EXIT)->EnableWindow(enabled);
	inbrowse.EnableWindow(enabled);
	comp->EnableWindow(enabled);
	decomp->EnableWindow(enabled);
	if (enout != nullptr)
	{
		enout->EnableWindow(enabled && !multiplefile);
	}
	RefreshOutputFileControls(enabled);
	backup->EnableWindow(enabled);
	log->EnableWindow(enabled);
	trayen->EnableWindow(enabled);
	checkbox->EnableWindow(enabled && trayenable);
	admin->EnableWindow(enabled && !IsRunningAsAdmin());

	const BOOL compressionEnabled = enabled && comp->GetCheck() == BST_CHECKED;
	statictxt->EnableWindow(compressionEnabled);
	suspicious->EnableWindow(compressionEnabled);
	complvl->EnableWindow(compressionEnabled);
}

void UpxguiDlg::RefreshOutputFileControls(BOOL enabled)
{
	const BOOL outputEnabled = enabled && !multiplefile && enout != nullptr && enout->GetCheck() == BST_CHECKED;
	outbrowse.EnableWindow(outputEnabled);
	outtext->EnableWindow(outputEnabled);
}

void UpxguiDlg::OnBnClickedEnout()
{
	RefreshOutputFileControls(!upxrunning);
}

void UpxguiDlg::ShowUpxResult(int exitCode, bool compressMode)
{
	UINT MASK;
	bool mask;
	char msg[250] = { '\0' };
	if (compressMode)
	{
		switch (exitCode)
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
		switch (exitCode)
		{
		case 0:
		{
			mask = true;
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
}

LRESULT UpxguiDlg::OnUpxOperationFinished(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	unique_ptr<UpxOperationResult> result(reinterpret_cast<UpxOperationResult*>(lParam));
	upxrunning = false;
	SetUpxControlsEnabled(TRUE);
	if (result)
	{
		ShowUpxResult(result->exitCode, result->compressMode);
	}
	return 0;
}

void UpxguiDlg::OnBnClickedAdmin()
{
	if (IsRunningAsAdmin())
	{
		return;
	}

	char module_path[MAX_PATH] = { '\0' };
	GetModuleFileNameA(NULL, module_path, sizeof(module_path));

	SHELLEXECUTEINFOA execute_info = { 0 };
	execute_info.cbSize = sizeof(SHELLEXECUTEINFOA);
	execute_info.fMask = SEE_MASK_NOCLOSEPROCESS;
	execute_info.hwnd = this->m_hWnd;
	execute_info.lpVerb = "runas";
	execute_info.lpFile = module_path;
	execute_info.nShow = SW_SHOWNORMAL;

	if (!ShellExecuteExA(&execute_info))
	{
		MessageBox("Unable to restart application as administrator.", "Restart as admin", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		return;
	}

	if (execute_info.hProcess != nullptr)
	{
		CloseHandle(execute_info.hProcess);
	}
	EndDialog(IDCANCEL);
}


void UpxguiDlg::OnBnClickedEnter()
{
	if (upxrunning)
	{
		return;
	}

	UpxOperationJob* job = new UpxOperationJob;
	job->hwnd = m_hWnd;
	job->arguments = BuildUpxArguments();
	job->compressMode = comp->GetCheck() == BST_CHECKED;

	upxrunning = true;
	SetUpxControlsEnabled(FALSE);

	CWinThread* thread = AfxBeginThread(RunUpxOperationThread, job, THREAD_PRIORITY_NORMAL, UpxThreadStackSize);
	if (thread == nullptr)
	{
		delete job;
		upxrunning = false;
		SetUpxControlsEnabled(TRUE);
		MessageBox("Unable to start UPX operation thread.", "UPX GUI", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}
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
