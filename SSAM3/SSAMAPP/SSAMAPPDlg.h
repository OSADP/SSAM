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
// SSAMAPPDlg.h : header file
//

#pragma once
#include "Dlg_Tab.h"
#include "SSAMDoc.h"


// CSSAMAPPDlg dialog
class CSSAMAPPDlg : public CDialogEx
{
// Construction
public:
	CSSAMAPPDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SSAMAPP_DIALOG };
	const static int NUMBER_OF_TABS = 6;
	void SetRectangle();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
private:
	enum {CONFIGURE, CONFLICT, SUMMARY, FILTER, TTEST, MAP};
	
	CTabCtrl m_TabCtrl_Case;
	CDlg_Tab* m_pTabs[NUMBER_OF_TABS];
	int m_PrevTab;
	SP_SSAMDoc m_pSSAMDoc;
	
	void InitTabCtrl();
	bool ValidateTab();
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonAbout();
};

