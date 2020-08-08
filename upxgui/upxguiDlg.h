
// hasher2Dlg.h : header file
//
#include "afxdialogex.h"
#include "afxwin.h"
#include <thread>
#include <fstream>
using namespace std;
#pragma once
#define bufferSize 10
#define WM_SHOWPAGE WM_APP+2
#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 1)
// Chasher2Dlg dialog
class filectrl : public CMFCEditBrowseCtrl
{
public:
	virtual void OnBrowse();
};
class fileinctrl : public CMFCEditBrowseCtrl
{
public:
	virtual void OnBrowse();
};
class UpxguiDlg : public CDialog
{
private:
	BOOL m_bMinimizeToTray;
	BOOL			m_bTrayIconVisible;
	CMenu			m_mnuTrayMenu;
	UINT			m_nDefaultMenuItem;
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
	// Construction
public:
	NOTIFYICONDATA	m_nidIconData;
	UpxguiDlg(CWnd* pParent = nullptr);
	void TraySetMinimizeToTray(BOOL bMinimizeToTray = TRUE);
	BOOL TraySetMenu(UINT nResourceID, UINT nDefaultPos = 0);
	BOOL TraySetMenu(HMENU hMenu, UINT nDefaultPos = 0);
	BOOL TraySetMenu(LPCTSTR lpszMenuName, UINT nDefaultPos = 0);
	BOOL TrayUpdate();
	BOOL TrayShow();
	BOOL TrayHide();
	void TraySetToolTip(LPCTSTR lpszToolTip);
	void TraySetIcon(HICON hIcon);
	void TraySetIcon(UINT nResourceID);
	void TraySetIcon(LPCTSTR lpszResourceName);
	BOOL TrayIsVisible();
	virtual void OnTrayLButtonDown(CPoint pt);
	virtual void OnTrayLButtonDblClk(CPoint pt);
	virtual void OnTrayRButtonDown(CPoint pt);
	virtual void OnTrayRButtonDblClk(CPoint pt);
	virtual void OnTrayMouseMove(CPoint pt);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPXGUI_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:


	afx_msg void OnBnClickedExit();
	afx_msg void OnOpen();
	afx_msg void OnMinimize();
	afx_msg void OnBnClickedMinEn();
	afx_msg void OnBnClickedTrayEn();
	afx_msg void OnBnClickedComp();
	afx_msg void OnBnClickedDecomp();
	afx_msg void OnBnClickedEnter();
	//void executeprogram(char* cmdline, DWORD flags, STARTUPINFOA si, PROCESS_INFORMATION pi,bool needadmin = false);
	//bool ExtractResource(uint16_t ResourceID, char* OutputFileName, char* path, const char* ResType);
	void updatevars(bool type, char *filename);
private:
	static CFont Font;
	static bool trayenable;
	static bool minimizeen;
	static CButton* checkbox;
	static CButton* trayen;
	static CButton* comp;
	static CButton* decomp;
	static CButton* log;
	static CButton* admin;
	static CButton* suspicious;
	static CButton* backup;
	static CStatic* statictxt;
	static CComboBox* complvl;
	static int boxcheck;
	static filectrl outbrowse;
	static fileinctrl inbrowse;
	static char infile[500];
	static char outfile[500];
	static unsigned long type;
	SECURITY_ATTRIBUTES saAttr;
	HANDLE in_rd = NULL;
	HANDLE in_wr = NULL;
	HANDLE out_rd = NULL;
	HANDLE out_wr = NULL;
	HANDLE outputfile = NULL;
};

