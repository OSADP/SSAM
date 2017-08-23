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
// Dlg_Conflicts.cpp : implementation file
//

#include "stdafx.h"
#include <fstream>
#include "SSAMAPP.h"
#include "Dlg_Conflicts.h"
#include "afxdialogex.h"
#include "Conflict.h"
#include "Dlg_ConflictProperties.h"

// CDlg_Conflicts dialog

IMPLEMENT_DYNAMIC(CDlg_Conflicts, CDialog)

CDlg_Conflicts::CDlg_Conflicts(CWnd* pParent /*=NULL*/)
	: CDlg_Tab(pParent)
{

}

CDlg_Conflicts::CDlg_Conflicts(SP_SSAMDoc pSSAMDoc, CWnd* pParent /*=NULL*/)
	: CDlg_Tab(pParent)
	, m_pSSAMDoc (pSSAMDoc)
{

}

CDlg_Conflicts::~CDlg_Conflicts()
{
}

void CDlg_Conflicts::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CONFLICTS, m_ListCtrl_Conflicts);
}

BOOL CDlg_Conflicts::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetFilterStatus();
	
	m_ListCtrl_Conflicts.SetExtendedStyle(m_ListCtrl_Conflicts.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	
	// add the headers
	for(int m = 0; m < Conflict::NUM_MEASURES; m++)
	{
		m_ListCtrl_Conflicts.InsertColumn(m, _T(Conflict::MEASURE_LABEL[m].c_str()), LVCFMT_LEFT, 90, -1);
	}
	return TRUE;
}

bool CDlg_Conflicts::SaveData()
{
	return true;
}

void CDlg_Conflicts::SetData()
{
	GetFilterStatus();
	m_ListCtrl_Conflicts.DeleteAllItems();
	
	int iRow = 0;
	for (std::list<SP_Conflict>::iterator it = m_pConflictList->begin();
		it != m_pConflictList->end(); ++it)
	{
		int iCol = 0;
		int nIdx = m_ListCtrl_Conflicts.InsertItem(iRow, _T((*it)->GetValueString(iCol).c_str()));
		for (iCol = 1; iCol < Conflict::NUM_MEASURES; iCol++)
		{
			m_ListCtrl_Conflicts.SetItemText(nIdx, iCol, _T((*it)->GetValueString(iCol).c_str()));
			
		}
		iRow++;
	}

}

BEGIN_MESSAGE_MAP(CDlg_Conflicts, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &CDlg_Conflicts::OnBnClickedButtonExport)
END_MESSAGE_MAP()


// CDlg_Conflicts message handlers


void CDlg_Conflicts::OnBnClickedButtonExport()
{
	CFileDialog fdlg (FALSE, "*.csv", m_pSSAMDoc->GetCsvName().c_str(),OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |OFN_ENABLESIZING | OFN_EXPLORER,
			"CSV (Comma delimited) (*.csv)|*.csv|",NULL,0,true);	

	if(fdlg.DoModal()==IDOK)
	{
		CString path = fdlg.GetPathName  ();
		std::ofstream csvFile(path);

		for(int m = 0; m < Conflict::NUM_MEASURES; m++)
		{
			csvFile << Conflict::MEASURE_LABEL[m] << ",";
		}
		csvFile << std::endl;

		for (std::list<SP_Conflict>::iterator it = m_pConflictList->begin();
		it != m_pConflictList->end(); ++it)
		{
			for (int iCol = 0; iCol < Conflict::NUM_MEASURES; iCol++)
			{
				csvFile << (*it)->GetValueString(iCol) << ",";
			
			}
			csvFile << std::endl;
		}
		csvFile.close();
	}
}

void CDlg_Conflicts::GetFilterStatus()
{
	if (!m_pSSAMDoc->IsFilterApplied())
	{
		GetDlgItem(IDC_STATIC_FILTERMSG)->SetWindowText("NO FILTER APPLIED");
		m_pConflictList = &(m_pSSAMDoc->GetConflictList());
	} else
	{
		GetDlgItem(IDC_STATIC_FILTERMSG)->SetWindowText("FILTER APPLIED");
		m_pConflictList = &(m_pSSAMDoc->GetFilteredConflictList());
	}
}
