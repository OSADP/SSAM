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
#pragma once
#include "Dlg_Tab.h"
#include "SSAMDoc.h"

// CDlg_Summary dialog

class CDlg_Summary : public CDlg_Tab
{
	DECLARE_DYNAMIC(CDlg_Summary)

public:
	CDlg_Summary(CWnd* pParent = NULL);   // standard constructor
	CDlg_Summary(SP_SSAMDoc pSSAMDoc, CWnd* pParent = NULL);
	virtual ~CDlg_Summary();

// Dialog Data
	enum { IDD = IDD_DIALOG_SUMMARY };
	virtual BOOL OnInitDialog();
	virtual bool SaveData();
	virtual void SetData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	SP_SSAMDoc m_pSSAMDoc;
	CListCtrl m_ListCtrl_Measures, m_ListCtrl_Conflicts;
	std::list<SP_Summary>* m_pSummaries;

	void InsertBlankRow(CListCtrl* pListCtrl, int iRow, int nCols);
	void GetFilterStatus();
public:
	afx_msg void OnBnClickedButtonExport();
};
