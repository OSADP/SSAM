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
#include "SSAMDoc.h"

// CDlg_ConflictProperties dialog

class CDlg_ConflictProperties : public CDialog
{
	DECLARE_DYNAMIC(CDlg_ConflictProperties)

public:
	CDlg_ConflictProperties(CWnd* pParent = NULL);   // standard constructor
	CDlg_ConflictProperties(SP_SSAMDoc pSSAMDoc, double* BL, double* TR, int conflictType, int dlgIdx, 
		CWnd* pParent = NULL);
	virtual ~CDlg_ConflictProperties();

// Dialog Data
	enum { IDD = IDD_DIALOG_CONFLICTPROP };
	BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual void PostNcDestroy();
	virtual void OnOK();
    virtual void OnCancel();
private:
	CWnd* m_pParent;
	SP_SSAMDoc m_pSSAMDoc;
	double x1, y1, z1, x2, y2, z2;
	int m_ConflictType;
	int m_DlgIdx;
	const std::list<SP_Conflict>* m_pConflictList;

	CListCtrl m_ListCtrl_Conflicts;
	
	void GetFilterStatus();
public:
	afx_msg void OnNcDestroy();
	afx_msg void OnClose();
};
