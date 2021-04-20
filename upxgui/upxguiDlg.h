#pragma once
// hasher2Dlg.h : header file
//
#include "afxdialogex.h"
#include "afxwin.h"
#include <thread>
#include <fstream>
#include <regex>
using namespace std;
#pragma warning(disable:6262)
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
	void updatevars(bool setmulti, bool multi, bool enfile=false, bool type=false, char* filename=nullptr, size_t* lens=nullptr, int filecount=0);
private:
	CFont Font = {};
	bool trayenable = false;
	bool minimizeen = false;
	CButton* checkbox;
	CButton* trayen;
	CButton* comp;
	CButton* decomp;
	CButton* log;
	CButton* admin;
	CButton* suspicious;
	CButton* backup;
	CStatic* statictxt;
	CComboBox* complvl;
	CStatic* outtext;
	int boxcheck = 0;
	filectrl outbrowse;
	fileinctrl inbrowse;
	char infile[MAX_PATH * 210] = {"\0"};
	char* filesptr[210] = {nullptr};
	size_t lensf[210] = {0};
	int filecounts=0;
	char outfile[500] = { "\0" };
	unsigned long type = 0;
	bool multiplefile = false;
	SECURITY_ATTRIBUTES saAttr;
	HANDLE in_rd = NULL;
	HANDLE in_wr = NULL;
	HANDLE out_rd = NULL;
	HANDLE out_wr = NULL;
	HANDLE outputfile = NULL;
};

