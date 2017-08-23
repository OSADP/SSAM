/*------------------------------------------------------------------------------
   Copyright © 2016-2017
   New Global Systems for Intelligent Transportation Management Corp.
   
   This file is part of SSAM.
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.
  
   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------*/
// SSAMAPPDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SSAMAPP.h"
#include "SSAMAPPDlg.h"
#include "afxdialogex.h"
#include "Dlg_Configure.h"
#include "Dlg_Conflicts.h"
#include "Dlg_Summary.h"
#include "Dlg_Map.h"
#include "Dlg_Filter.h"
#include "Dlg_TTest.h"
#include "Dlg_HTMLTerms.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonView();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_VIEW, &CAboutDlg::OnBnClickedButtonView)
END_MESSAGE_MAP()


// CSSAMAPPDlg dialog



CSSAMAPPDlg::CSSAMAPPDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSSAMAPPDlg::IDD, pParent)
	, m_PrevTab(0)
	, m_pSSAMDoc(std::make_shared<SSAMDoc>())
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSSAMAPPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_TabCtrl_Case);
}

BEGIN_MESSAGE_MAP(CSSAMAPPDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CSSAMAPPDlg::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDOK, &CSSAMAPPDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSSAMAPPDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CSSAMAPPDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CSSAMAPPDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CSSAMAPPDlg::OnBnClickedButtonAbout)
END_MESSAGE_MAP()


// CSSAMAPPDlg message handlers

BOOL CSSAMAPPDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	InitTabCtrl();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSSAMAPPDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSSAMAPPDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSSAMAPPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSSAMAPPDlg::InitTabCtrl()
{
	// Construct tab pages
	TCITEM tcItem;
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Configuration");
	m_TabCtrl_Case.InsertItem(CONFIGURE, &tcItem);

	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Conflicts");
	m_TabCtrl_Case.InsertItem(CONFLICT, &tcItem);
	
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Summary");
	m_TabCtrl_Case.InsertItem(SUMMARY, &tcItem);

	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Filter");
	m_TabCtrl_Case.InsertItem(FILTER, &tcItem);

	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("ttest");
	m_TabCtrl_Case.InsertItem(TTEST, &tcItem);

	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Map");
	m_TabCtrl_Case.InsertItem(MAP, &tcItem);
	
	
	m_pTabs[CONFIGURE] = new CDlg_Configure(m_pSSAMDoc);
	m_pTabs[CONFIGURE]->Create(IDD_DIALOG_CONFIGURE,&m_TabCtrl_Case);

	m_pTabs[CONFLICT] = new CDlg_Conflicts(m_pSSAMDoc);
	m_pTabs[CONFLICT]->Create(IDD_DIALOG_CONFLICTS,&m_TabCtrl_Case);

	m_pTabs[SUMMARY] = new CDlg_Summary(m_pSSAMDoc);
	m_pTabs[SUMMARY]->Create(IDD_DIALOG_SUMMARY,&m_TabCtrl_Case);

	m_pTabs[TTEST] = new CDlg_TTest(m_pSSAMDoc);
	m_pTabs[TTEST]->Create(IDD_DIALOG_TTEST,&m_TabCtrl_Case);

	m_pTabs[FILTER] = new CDlg_Filter(m_pSSAMDoc);
	m_pTabs[FILTER]->Create(IDD_DIALOG_FILTER,&m_TabCtrl_Case);

	m_pTabs[MAP] = new CDlg_Map(m_pSSAMDoc);
	m_pTabs[MAP]->Create(IDD_DIALOG_MAP,&m_TabCtrl_Case);


	for (int iTab = 0; iTab < NUMBER_OF_TABS; iTab++)
	{
		if (iTab == m_PrevTab)
			m_pTabs[iTab]->ShowWindow(SW_SHOW);
		else
			m_pTabs[iTab]->ShowWindow(SW_HIDE);
	}

	SetRectangle();
}

// put child dialogs in proper positions
void CSSAMAPPDlg::SetRectangle()
{
	CRect tabRect, itemRect;
	int nX, nY, nXc, nYc;

	m_TabCtrl_Case.GetClientRect(&tabRect);
	m_TabCtrl_Case.GetItemRect(0, &itemRect);

	nX = itemRect.left;
	nY = itemRect.bottom + 1;
	nXc = tabRect.right - nX - 1;
	nYc = tabRect.bottom - nY - 1;

	for (int iTab = 0; iTab < NUMBER_OF_TABS; ++iTab)
	{
		if (iTab == m_PrevTab)
			m_pTabs[iTab]->SetWindowPos(&m_TabCtrl_Case.wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
		else
			m_pTabs[iTab]->SetWindowPos(&m_TabCtrl_Case.wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);
	}
}

//validate and save data in current tab page
bool CSSAMAPPDlg::ValidateTab()
{
	if (!m_pTabs[m_PrevTab]->SaveData())
	{
		m_TabCtrl_Case.SetCurSel(m_PrevTab);
		return false;
	} 
	return true;
}


void CSSAMAPPDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_TabCtrl_Case.GetCurSel() != m_PrevTab) 
	{
	    if (!ValidateTab()) return;
		m_PrevTab = m_TabCtrl_Case.GetCurSel();
		for (int iTab = 0; iTab < NUMBER_OF_TABS; ++iTab)
	    {
			if (iTab == m_PrevTab) 
			{
				m_pTabs[iTab]->EnableWindow(TRUE);
				m_pTabs[iTab]->ShowWindow(SW_SHOW);
			} else
			{
				m_pTabs[iTab]->EnableWindow(FALSE);
				m_pTabs[iTab]->ShowWindow(SW_HIDE);
			}
		}
		if (m_PrevTab == CONFLICT 
			|| m_PrevTab == SUMMARY 
			|| m_PrevTab == MAP
			|| m_PrevTab == FILTER
			|| m_PrevTab == TTEST)
		{
			m_pTabs[m_PrevTab]->SetData();
		}
	}
			
	*pResult = 0;
	*pResult = 0;
}


void CSSAMAPPDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CSSAMAPPDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CSSAMAPPDlg::OnBnClickedButtonSave()
{
	std::string tmpFileName(m_pSSAMDoc->GetDocName());
	if (tmpFileName.empty())
	{
		tmpFileName = m_pSSAMDoc->GetCSVFile();
		tmpFileName = tmpFileName.substr(0, tmpFileName.length() - 4);
	}
	CFileDialog fdlg (FALSE, NULL, tmpFileName.c_str(),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |OFN_ENABLESIZING | OFN_EXPLORER,
			_T("*.*|*.*|"),NULL,0,true);	

	if(fdlg.DoModal()==IDOK)
	{
		std::string CaseFile = fdlg.GetPathName  ();
		m_pSSAMDoc->Save(CaseFile);
	}
}


void CSSAMAPPDlg::OnBnClickedButtonOpen()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_LONGNAMES|OFN_ENABLESIZING,
		_T("*.*|*.*|"),NULL,0,true);
	if(dlg.DoModal() == IDOK)
	{
		std::string CaseFile = dlg.GetPathName();
		try
		{
			m_pSSAMDoc->Open(CaseFile);
		} catch (std::runtime_error& e)
		{
			std::string errMsg (e.what());
			AfxMessageBox(errMsg.c_str());
			return;
		}
		m_pTabs[CONFIGURE]->SetData();
	}
}


void CSSAMAPPDlg::OnBnClickedButtonAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


void CAboutDlg::OnBnClickedButtonView()
{
	CDlg_HTMLTerms dlg;
	dlg.DoModal();
}
