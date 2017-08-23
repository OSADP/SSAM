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
// Dlg_ConflictProperties.cpp : implementation file
//

#include "stdafx.h"
#include "SSAMAPP.h"
#include "Dlg_ConflictProperties.h"
#include "afxdialogex.h"
#include "Dlg_Map.h"


// CDlg_ConflictProperties dialog

IMPLEMENT_DYNAMIC(CDlg_ConflictProperties, CDialog)

CDlg_ConflictProperties::CDlg_ConflictProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_ConflictProperties::IDD, pParent)
	, m_pParent(pParent)
{

}

CDlg_ConflictProperties::CDlg_ConflictProperties(SP_SSAMDoc pSSAMDoc, double* BL, double* TR, int conflictType,
												 int dlgIdx, CWnd* pParent)
	: CDialog(CDlg_ConflictProperties::IDD, pParent)
	, m_pParent(pParent)
	, m_pSSAMDoc (pSSAMDoc)
	, x1(BL[0])
	, y1(BL[1])
	, z1(BL[2])
	, x2(TR[0])
	, y2(TR[1])
	, z2(TR[2])
	, m_ConflictType(conflictType)
	, m_DlgIdx(dlgIdx)
{
	GetFilterStatus();
}

CDlg_ConflictProperties::~CDlg_ConflictProperties()
{
}

void CDlg_ConflictProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CONFLICTS, m_ListCtrl_Conflicts);
}


BOOL CDlg_ConflictProperties::OnInitDialog()
{
	CDialog::OnInitDialog();
		
	m_ListCtrl_Conflicts.SetExtendedStyle(m_ListCtrl_Conflicts.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	
	// add the headers
	for(int m = 0; m < Conflict::NUM_MEASURES; m++)
	{
		if (Conflict::KEY_MEASURES[m])
			m_ListCtrl_Conflicts.InsertColumn(m, _T(Conflict::MEASURE_LABEL[m].c_str()), LVCFMT_LEFT, 90, -1);
	}
	int iRow = 0;
	for (std::list<SP_Conflict>::const_iterator it = m_pConflictList->begin();
		it != m_pConflictList->end(); ++it)
	{
		SP_Conflict pc = *it;
		if (pc->xMinPET >= x1 && pc->xMinPET <= x2
			&& pc->yMinPET >= y1 && pc->yMinPET <= y2
			&& pc->zMinPET >= z1 && pc->zMinPET <= z2
			&& (m_ConflictType == -1 || pc->ConflictType == m_ConflictType))
		{
			int m = 0;
			int nIdx = m_ListCtrl_Conflicts.InsertItem(iRow, _T((*it)->GetValueString(m).c_str()));
			int iCol = 1;
			m++;
			for (; m < Conflict::NUM_MEASURES; m++)
			{
				if (Conflict::KEY_MEASURES[m])
				{
					m_ListCtrl_Conflicts.SetItemText(nIdx, iCol, _T((*it)->GetValueString(m).c_str()));
					iCol++;
				}
			}
			iRow++;
		}
	}
	
	CString total;
	total.Format("Total: %d", iRow);
	GetDlgItem(IDC_STATIC_TOTAL)->SetWindowText(total);
	return TRUE;
}
BEGIN_MESSAGE_MAP(CDlg_ConflictProperties, CDialog)
	ON_WM_NCDESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlg_ConflictProperties message handlers
void CDlg_ConflictProperties::PostNcDestroy()
{
    CDialog::PostNcDestroy();
	((CDlg_Map*)m_pParent)->DeleteModelessDialog(m_DlgIdx);
	delete this; 
}

void CDlg_ConflictProperties::OnOK()
{
    DestroyWindow();
}

void CDlg_ConflictProperties::OnCancel()
{
    DestroyWindow();
}

void CDlg_ConflictProperties::OnNcDestroy()
{
	CDialog::OnNcDestroy();
}


void CDlg_ConflictProperties::OnClose()
{
	DestroyWindow();
}

void CDlg_ConflictProperties::GetFilterStatus()
{
	if (!m_pSSAMDoc->IsFilterApplied())
	{
		m_pConflictList = &(m_pSSAMDoc->GetConflictList());
	} else
	{
		m_pConflictList = &(m_pSSAMDoc->GetFilteredConflictList());
	}
}
