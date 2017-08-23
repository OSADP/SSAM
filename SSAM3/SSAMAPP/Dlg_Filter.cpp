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
// Dlg_Filter.cpp : implementation file
//

#include "stdafx.h"
#include "SSAMAPP.h"
#include "Dlg_Filter.h"
#include "afxdialogex.h"
#include <set>


// CDlg_Filter dialog

IMPLEMENT_DYNAMIC(CDlg_Filter, CDialog)

CDlg_Filter::CDlg_Filter(CWnd* pParent /*=NULL*/)
	: CDlg_Tab(pParent)
{

}

CDlg_Filter::CDlg_Filter(SP_SSAMDoc pSSAMDoc, 
						 CWnd* pParent /*=NULL*/)
	: CDlg_Tab(pParent)
	, m_pSSAMDoc (pSSAMDoc)
	, m_pFilterParams(pSSAMDoc->GetFilterParams())
{

}

CDlg_Filter::~CDlg_Filter()
{
}

void CDlg_Filter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_MINTTC, m_pFilterParams->m_MinTTC);
	DDX_Text(pDX, IDC_EDIT_MAXTTC, m_pFilterParams->m_MaxTTC);
	DDX_Text(pDX, IDC_EDIT_MINPET, m_pFilterParams->m_MinPET);
	DDX_Text(pDX, IDC_EDIT_MAXPET, m_pFilterParams->m_MaxPET);
	DDX_Text(pDX, IDC_EDIT_MINMAXS, m_pFilterParams->m_MinMaxS);
	DDX_Text(pDX, IDC_EDIT_MAXMAXS, m_pFilterParams->m_MaxMaxS);
	DDX_Text(pDX, IDC_EDIT_MINDELTAS, m_pFilterParams->m_MinDeltaS);
	DDX_Text(pDX, IDC_EDIT_MAXDELTAS, m_pFilterParams->m_MaxDeltaS);
	DDX_Text(pDX, IDC_EDIT_MINDR, m_pFilterParams->m_MinDR);
	DDX_Text(pDX, IDC_EDIT_MAXDR, m_pFilterParams->m_MaxDR);
	DDX_Text(pDX, IDC_EDIT_MINMAXD, m_pFilterParams->m_MinMaxD);
	DDX_Text(pDX, IDC_EDIT_MAXMAXD, m_pFilterParams->m_MaxMaxD);
	DDX_Text(pDX, IDC_EDIT_MINMAXDELTAV, m_pFilterParams->m_MinMaxDeltaV);
	DDX_Text(pDX, IDC_EDIT_MAXMAXDELTAV, m_pFilterParams->m_MaxMaxDeltaV);
	DDX_Text(pDX, IDC_EDIT_LLX, m_pFilterParams->m_Area[0]);
	DDX_Text(pDX, IDC_EDIT_LLY, m_pFilterParams->m_Area[1]);
	DDX_Text(pDX, IDC_EDIT_URX, m_pFilterParams->m_Area[2]);
	DDX_Text(pDX, IDC_EDIT_URY, m_pFilterParams->m_Area[3]);

	DDX_Control(pDX, IDC_SPIN_MINTTC, m_Spin_MinTTC);
	DDX_Control(pDX, IDC_SPIN_MAXTTC, m_Spin_MaxTTC);
	DDX_Control(pDX, IDC_SPIN_MINPET, m_Spin_MinPET);
	DDX_Control(pDX, IDC_SPIN_MAXPET, m_Spin_MaxPET);
	DDX_Control(pDX, IDC_SPIN_MINMAXS, m_Spin_MinMaxS);
	DDX_Control(pDX, IDC_SPIN_MAXMAXS, m_Spin_MaxMaxS);
	DDX_Control(pDX, IDC_SPIN_MINDELTAS, m_Spin_MinDeltaS);
	DDX_Control(pDX, IDC_SPIN_MAXDELTAS, m_Spin_MaxDeltaS);
	DDX_Control(pDX, IDC_SPIN_MINDR, m_Spin_MinDR);
	DDX_Control(pDX, IDC_SPIN_MAXDR, m_Spin_MaxDR);
	DDX_Control(pDX, IDC_SPIN_MINMAXD, m_Spin_MinMaxD);
	DDX_Control(pDX, IDC_SPIN_MAXMAXD, m_Spin_MaxMaxD);
	DDX_Control(pDX, IDC_SPIN_MINMAXDELTAV, m_Spin_MinMaxDeltaV);
	DDX_Control(pDX, IDC_SPIN_MAXMAXDELTAV, m_Spin_MaxMaxDeltaV);
	DDX_Control(pDX, IDC_SPIN_LLX, m_Spin_Area[0]);
	DDX_Control(pDX, IDC_SPIN_LLY, m_Spin_Area[1]);
	DDX_Control(pDX, IDC_SPIN_URX, m_Spin_Area[2]);
	DDX_Control(pDX, IDC_SPIN_URY, m_Spin_Area[3]);
	
	DDX_Control(pDX, IDC_LIST_CONFLICTTYPES, m_List_ConflictTypes);
	DDX_Control(pDX, IDC_LIST_LINKS, m_List_Links);
	DDX_Control(pDX, IDC_LIST_TRJFILES, m_List_TRJFiles);
}

BOOL CDlg_Filter::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_List_ConflictTypes.AddString("All");
	for (int i = 0; i < Conflict::NUM_CONFLICT_TYPES; ++i)
	{
		m_List_ConflictTypes.AddString(Conflict::CONFLICT_TYPE_LABEL[i].c_str());
	}
	InitListBoxes();
	SetListBoxes();	
	return TRUE;
}

bool CDlg_Filter::SaveData()
{
	return true;
}

void CDlg_Filter::SetData()
{
	if (!m_pSSAMDoc->GetConflictList().empty())
	{
		m_pFilterParams = m_pSSAMDoc->GetFilterParams();
		UpdateData(FALSE);
		InitListBoxes();
		SetListBoxes();
	}
}

void CDlg_Filter::OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult, double& spinValue, double step )
{
	UpdateData(TRUE);
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	spinValue -= float(pNMUpDown->iDelta) * step;
	UpdateData(FALSE);
	*pResult = 0;
}

void CDlg_Filter::InitListBoxes()
{
	std::set<int> links;

	for (std::list<SP_Conflict>::iterator it = m_pSSAMDoc->GetConflictList().begin();
		it != m_pSSAMDoc->GetConflictList().end(); ++it)
	{
		SP_Conflict pConflict = *it;
		links.insert(pConflict->FirstLink);
		links.insert(pConflict->SecondLink);
	}

	m_List_Links.ResetContent();
	m_List_Links.AddString("All");
	for (std::set<int>::iterator it = links.begin(); it != links.end(); ++it)
	{
		sprintf(tmp_str, "%d", *it);
		m_List_Links.AddString(tmp_str);
	}
	
	m_List_TRJFiles.ResetContent();
	m_List_TRJFiles.AddString("All");

	for (std::list<std::string>::const_iterator it = m_pSSAMDoc->GetTrjFileNames().begin(); 
		it != m_pSSAMDoc->GetTrjFileNames().end(); ++it)
	{
		UpdateListBox(&m_List_TRJFiles, it->c_str());
	}

}

void CDlg_Filter::SetListBoxes()
{
	if (m_pFilterParams->m_ConflictTypes.empty())
	{
		m_List_ConflictTypes.SetSel(0);
		for (int i = 1; i < m_List_ConflictTypes.GetCount(); ++i)
		{
			m_List_ConflictTypes.SetSel(i, FALSE);
		}
	} else
	{
		m_List_ConflictTypes.SetSel(0, FALSE);
		for (std::set<int>::iterator it = m_pFilterParams->m_ConflictTypes.begin();
			it != m_pFilterParams->m_ConflictTypes.end(); ++it)
		{
			int idx = m_List_ConflictTypes.FindString(-1, Conflict::CONFLICT_TYPE_LABEL[*it].c_str());
			if (idx != LB_ERR)
			{
				m_List_ConflictTypes.SetSel(idx);
			}
		}
	}

	if (m_pFilterParams->m_Links.empty())
	{
		m_List_Links.SetSel(0);
		for (int i = 1; i < m_List_Links.GetCount(); ++i)
		{
			m_List_Links.SetSel(i, FALSE);
		}
	} else
	{
		m_List_Links.SetSel(0, FALSE);
		for (std::set<int>::iterator it = m_pFilterParams->m_Links.begin();
			it != m_pFilterParams->m_Links.end(); ++it)
		{
			sprintf(tmp_str, "%d", *it);
			int idx = m_List_Links.FindString(-1, tmp_str);
			if (idx != LB_ERR)
			{
				m_List_Links.SetSel(idx);
			}
		}
	}

	if (m_pFilterParams->m_TRJFiles.empty())
	{
		m_List_TRJFiles.SetSel(0);
		for (int i = 1; i < m_List_TRJFiles.GetCount(); ++i)
		{
			m_List_TRJFiles.SetSel(i, FALSE);
		}
	} else
	{
		m_List_TRJFiles.SetSel(0, FALSE);
		for (std::vector<std::string>::iterator it = m_pFilterParams->m_TRJFiles.begin();
			it != m_pFilterParams->m_TRJFiles.end(); ++it)
		{
			int idx = m_List_TRJFiles.FindString(-1, it->c_str());
			if (idx != LB_ERR)
			{
				m_List_TRJFiles.SetSel(idx);
			}
		}
	}
}

BEGIN_MESSAGE_MAP(CDlg_Filter, CDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MINTTC, &CDlg_Filter::OnDeltaposSpinMinttc)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAXTTC, &CDlg_Filter::OnDeltaposSpinMaxttc)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MINPET, &CDlg_Filter::OnDeltaposSpinMinpet)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAXPET, &CDlg_Filter::OnDeltaposSpinMaxpet)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MINMAXS, &CDlg_Filter::OnDeltaposSpinMinmaxs)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAXMAXS, &CDlg_Filter::OnDeltaposSpinMaxmaxs)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MINDELTAS, &CDlg_Filter::OnDeltaposSpinMindeltas)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAXDELTAS, &CDlg_Filter::OnDeltaposSpinMaxdeltas)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MINDR, &CDlg_Filter::OnDeltaposSpinMindr)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAXDR, &CDlg_Filter::OnDeltaposSpinMaxdr)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MINMAXD, &CDlg_Filter::OnDeltaposSpinMinmaxd)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAXMAXD, &CDlg_Filter::OnDeltaposSpinMaxmaxd)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MINMAXDELTAV, &CDlg_Filter::OnDeltaposSpinMinmaxdeltav)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAXMAXDELTAV, &CDlg_Filter::OnDeltaposSpinMaxmaxdeltav)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LLX, &CDlg_Filter::OnDeltaposSpinLlx)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LLY, &CDlg_Filter::OnDeltaposSpinLly)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_URX, &CDlg_Filter::OnDeltaposSpinUrx)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_URY, &CDlg_Filter::OnDeltaposSpinUry)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDlg_Filter::OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_RECTOREDEFAULT, &CDlg_Filter::OnBnClickedButtonRectoredefault)
END_MESSAGE_MAP()


// CDlg_Filter message handlers


void CDlg_Filter::OnDeltaposSpinMinttc(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MinTTC);
}


void CDlg_Filter::OnDeltaposSpinMaxttc(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MaxTTC);
}


void CDlg_Filter::OnDeltaposSpinMinpet(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MinPET);
}


void CDlg_Filter::OnDeltaposSpinMaxpet(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MaxPET);
}


void CDlg_Filter::OnDeltaposSpinMinmaxs(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MinMaxS);
}


void CDlg_Filter::OnDeltaposSpinMaxmaxs(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MaxMaxS);
}


void CDlg_Filter::OnDeltaposSpinMindeltas(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MinDeltaS);
}


void CDlg_Filter::OnDeltaposSpinMaxdeltas(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MaxDeltaS);
}


void CDlg_Filter::OnDeltaposSpinMindr(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MinDR);
}


void CDlg_Filter::OnDeltaposSpinMaxdr(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MaxDR);
}


void CDlg_Filter::OnDeltaposSpinMinmaxd(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MinMaxD);
}


void CDlg_Filter::OnDeltaposSpinMaxmaxd(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MaxMaxD);
}


void CDlg_Filter::OnDeltaposSpinMinmaxdeltav(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MinMaxDeltaV);
}


void CDlg_Filter::OnDeltaposSpinMaxmaxdeltav(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_MaxMaxDeltaV);
}



void CDlg_Filter::OnDeltaposSpinLlx(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_Area[0], 0.5);
}


void CDlg_Filter::OnDeltaposSpinLly(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_Area[1], 0.5);
}


void CDlg_Filter::OnDeltaposSpinUrx(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_Area[2], 0.5);
}


void CDlg_Filter::OnDeltaposSpinUry(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposSpin(pNMHDR, pResult, m_pFilterParams->m_Area[3], 0.5);
}


void CDlg_Filter::OnBnClickedButtonApply()
{
	if (!UpdateData(TRUE))
		return;

	if (m_pFilterParams->m_MinTTC > m_pFilterParams->m_MaxTTC
		|| m_pFilterParams->m_MinPET > m_pFilterParams->m_MaxPET
		|| m_pFilterParams->m_MinMaxS > m_pFilterParams->m_MaxMaxS
		|| m_pFilterParams->m_MinDeltaS > m_pFilterParams->m_MaxDeltaS
		|| m_pFilterParams->m_MinDR > m_pFilterParams->m_MaxDR
		|| m_pFilterParams->m_MinMaxD > m_pFilterParams->m_MaxMaxD
		|| m_pFilterParams->m_MinMaxDeltaV > m_pFilterParams->m_MaxMaxDeltaV)
	{
		AfxMessageBox("Minimum value cannot greater than maximum value.");
		return;
	}

	m_pFilterParams->m_ConflictTypes.clear();
	for (int i = 1; i < m_List_ConflictTypes.GetCount(); ++i)
	{
		if (m_List_ConflictTypes.GetSel(i) > 0)
			m_pFilterParams->m_ConflictTypes.insert(i-1);
	}

	m_pFilterParams->m_Links.clear();
	for (int i = 1; i < m_List_Links.GetCount(); ++i)
	{
		if (m_List_Links.GetSel(i) > 0)
		{
			CString str;
			m_List_Links.GetText(i, str);
			m_pFilterParams->m_Links.insert(stoi(std::string(str)));
		}
	}

	m_pFilterParams->m_TRJFiles.clear();
	for (int i = 1; i < m_List_TRJFiles.GetCount(); ++i)
	{
		if (m_List_TRJFiles.GetSel(i) > 0)
		{
			CString str;
			m_List_TRJFiles.GetText(i, str);
			m_pFilterParams->m_TRJFiles.push_back(std::string(str));
		}
	}

	if (m_pSSAMDoc->ApplyFilter())
	{
		AfxMessageBox("Filter Applied. Please go to the Conflict panel and Summary panel to view results.");
	} else
	{
		AfxMessageBox("The Filter did not generate any values. Thus filter not applied.");
	}
}

void CDlg_Filter::OnBnClickedButtonRectoredefault()
{
	m_pSSAMDoc->InitFilterParams();
	UpdateData(FALSE);
	SetListBoxes();
	AfxMessageBox("Defaults Restored. The conflict and summary panels have also been restored to default values.");
}

void CDlg_Filter::UpdateListBox(CListBox* pListBox, CString message)
{
	CSize sz = 0;
	int dx = 0;
	CDC* pDC = pListBox->GetDC();
	int idxStart = 0, idxEnd = message.GetLength() - 1;
	if (idxEnd >= 0)
	{
		CString subStr = message;
		sz = pDC->GetTextExtent(subStr);
		if (sz.cx > dx) dx = sz.cx;
		int idx = pListBox->AddString(subStr);
		pListBox->RedrawWindow();
		pListBox->SetCurSel(idx);
	}

	pListBox->ReleaseDC(pDC);
	if (pListBox->GetHorizontalExtent() < dx)
		pListBox->SetHorizontalExtent(dx);
}
