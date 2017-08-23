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
// Dlg_Configure.cpp : implementation file
//

#include "stdafx.h"
#include "SSAMAPP.h"
#include "Dlg_Configure.h"
#include "afxdialogex.h"
#include "Dlg_Diagram.h"

// CDlg_Configure dialog

IMPLEMENT_DYNAMIC(CDlg_Configure, CDialog)

CDlg_Configure::CDlg_Configure(CWnd* pParent /*=NULL*/)
	: CDlg_Tab(pParent)
{

}

CDlg_Configure::CDlg_Configure(SP_SSAMDoc pSSAMDoc, CWnd* pParent)
	: CDlg_Tab(pParent)
	, m_pSSAMDoc (pSSAMDoc)
	, m_MaxTTC(pSSAMDoc->GetMaxTTC())
	, m_MaxPET(pSSAMDoc->GetMaxPET())
	, m_MaxRearAngle(pSSAMDoc->GetRearEndAngle())
	, m_MaxCrossingAngle(pSSAMDoc->GetCrossingAngle())
	, m_IsCalcPUEA(0)
	, m_IsWriteDat(0)
{
#ifdef _OPENMP_LOCAL
	m_NumOfThreads = omp_get_num_procs() / 2;
#else
	m_NumOfThreads = 1;
#endif
}

CDlg_Configure::~CDlg_Configure()
{
}

void CDlg_Configure::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TRJFILES, m_ListBox_TrjFiles);
	DDX_Text(pDX, IDC_EDIT_TTC, m_MaxTTC);
	DDX_Text(pDX, IDC_EDIT_PET, m_MaxPET);
	DDX_Text(pDX, IDC_EDIT_REARANGLE, m_MaxRearAngle);
	DDX_Text(pDX, IDC_EDIT_CROSSINGANGLE, m_MaxCrossingAngle);

	DDX_Control(pDX, IDC_SPIN_TTC, m_Spin_MaxTTC);
	DDX_Control(pDX, IDC_SPIN_PET, m_Spin_MaxPET);
	DDX_Control(pDX, IDC_SPIN_REARANGLE, m_Spin_MaxRearAngle);
	DDX_Control(pDX, IDC_SPIN_CROSSINGANGLE, m_Spin_MaxCrossingAngle);

	DDX_Text(pDX, IDC_EDIT_NUMOFTHREADS, m_NumOfThreads);
	DDX_Check(pDX, IDC_CHECK_PUEA, m_IsCalcPUEA);
	DDX_Check(pDX, IDC_CHECK_TRJTOTEXT, m_IsWriteDat);
}

BOOL CDlg_Configure::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_Spin_MaxTTC.SetRange32(0, MAX_TTC);
	m_Spin_MaxTTC.SetPos32(int(m_MaxTTC*10.0));

	m_Spin_MaxPET.SetRange32(0, MAX_PET);
	m_Spin_MaxPET.SetPos32(int(m_MaxPET*10.0));

	m_Spin_MaxRearAngle.SetRange32(0, MAX_ANGLE);
	m_Spin_MaxRearAngle.SetPos32(m_MaxRearAngle);

	m_Spin_MaxCrossingAngle.SetRange32(0, MAX_ANGLE);
	m_Spin_MaxCrossingAngle.SetPos32(m_MaxCrossingAngle);

#ifndef _OPENMP_LOCAL
	GetDlgItem(IDC_STATIC_NUMOFTHREADS)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_NUMOFTHREADS)->EnableWindow(FALSE);
#endif

	for (std::list<std::string>::const_iterator it = m_pSSAMDoc->GetTrjFileNames().begin();
		it != m_pSSAMDoc->GetTrjFileNames().end(); ++it)
	{
		CString fname(it->c_str());
		UpdateListBox(&m_ListBox_TrjFiles, fname+"\n");
	}

	return TRUE;
}

bool CDlg_Configure::SaveData()
{
	return true;
}

void CDlg_Configure::SetData()
{
	m_MaxTTC = m_pSSAMDoc->GetMaxTTC();
	m_MaxPET = m_pSSAMDoc->GetMaxPET();
	m_MaxRearAngle = m_pSSAMDoc->GetRearEndAngle();
	m_MaxCrossingAngle = m_pSSAMDoc->GetCrossingAngle();
	m_IsCalcPUEA = m_pSSAMDoc->GetIsCalcPUEA();
	UpdateData(FALSE);

	m_ListBox_TrjFiles.ResetContent();
	for (std::list<std::string>::const_iterator it = m_pSSAMDoc->GetTrjFileNames().begin();
		it != m_pSSAMDoc->GetTrjFileNames().end(); ++it)
	{
		CString fname(it->c_str());
		UpdateListBox(&m_ListBox_TrjFiles, fname+"\n");
	}
}

BEGIN_MESSAGE_MAP(CDlg_Configure, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CDlg_Configure::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CDlg_Configure::OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_ANALYZE, &CDlg_Configure::OnBnClickedButtonAnalyze)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TTC, &CDlg_Configure::OnDeltaposSpinTtc)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PET, &CDlg_Configure::OnDeltaposSpinPet)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlg_Configure::OnBnClickedButton1)
END_MESSAGE_MAP()


// CDlg_Configure message handlers


void CDlg_Configure::OnBnClickedButtonAdd()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_LONGNAMES|OFN_ENABLESIZING|OFN_ALLOWMULTISELECT,
		_T("trj File (*trj)|*.trj|"),NULL,0,true);
	CString fileName;
	
	int nMaxFiles = 256;
	int nBufferSize = nMaxFiles* (MAX_PATH+1) + 1;
	dlg.GetOFN().lpstrFile = fileName.GetBuffer(nBufferSize);
	dlg.GetOFN().nMaxFile = nMaxFiles;

	
	if(dlg.DoModal() == IDOK)
	{
		POSITION pos(dlg.GetStartPosition());
		while (pos)
		{
			CString fname = dlg.GetNextPathName(pos);
			UpdateListBox(&m_ListBox_TrjFiles, fname+"\n");
		}
		UpdateData(FALSE);
	}
	fileName.ReleaseBuffer();
}


void CDlg_Configure::OnBnClickedButtonDelete()
{
	int nCount = m_ListBox_TrjFiles.GetSelCount();
	if (nCount > 0)
	{
		CArray<int, int> arySel;
		arySel.SetSize(nCount);
		m_ListBox_TrjFiles.GetSelItems(nCount, arySel.GetData());

		for (int i = 0; i < nCount; ++i)
		{
			int idx = arySel[i];
			m_ListBox_TrjFiles.DeleteString(idx);
		}
		AFXDUMP(arySel);
		UpdateData(FALSE);
	}
}


void CDlg_Configure::OnBnClickedButtonAnalyze()
{
	try
	{
		UpdateData(TRUE);
		m_pSSAMDoc->ResetDoc();

		// Add trj files
		int nCount = m_ListBox_TrjFiles.GetCount();
		if (nCount == 0)
			return;

		CString str;
		std::string csvFileName;
		for (int i = 0; i < nCount; ++i)
		{
			m_ListBox_TrjFiles.GetText(i, str);
			std::string fname(str);
			m_pSSAMDoc->AddTrjFile(fname);

			if (i==0)
			{
				csvFileName = fname.substr(0, fname.length() - 4) +".csv";
			}
		}
		m_pSSAMDoc->SetCSVFile(csvFileName);

		// validate and set other parameters
		if (m_MaxTTC < 0 || m_MaxTTC > 5)
		{
			CString msg;
			msg.Format("Maximum TTC is not in acceptable range, [0.0, %2.1f]", MAX_TTC/10.0);
			AfxMessageBox(msg);
			return;
		}

		if (m_MaxPET < 0 || m_MaxPET > 10)
		{
			CString msg;
			msg.Format("Maximum TTC is not in acceptable range, [0.0, %2.1f]", MAX_PET/10.0);
			AfxMessageBox(msg);
			return;
		}

		if (m_MaxRearAngle < 0 || m_MaxRearAngle > 180)
		{
			CString msg;
			msg.Format("Rear End Angle is not in acceptable range, [0, %d]", MAX_ANGLE);
			AfxMessageBox(msg);
			return;
		}

		if (m_MaxCrossingAngle < 0 || m_MaxCrossingAngle > 180)
		{
			CString msg;
			msg.Format("Crossing Angle is not in acceptable range, [0, %d]", MAX_ANGLE);
			AfxMessageBox(msg);
			return;
		}
	
		m_pSSAMDoc->SetMaxTTC(m_MaxTTC);
		m_pSSAMDoc->SetMaxPET(m_MaxPET);
		m_pSSAMDoc->SetRearEndAngle(m_MaxRearAngle);
		m_pSSAMDoc->SetCrossingAngle(m_MaxCrossingAngle);
		m_pSSAMDoc->SetIsCalcPUEA(m_IsCalcPUEA == 1);
		m_pSSAMDoc->SetWriteDat(m_IsWriteDat == 1);
#ifdef _OPENMP_LOCAL
		m_pSSAMDoc->SetNThreads(m_NumOfThreads);
#endif

		// Run SSAM
		GetDlgItem(IDC_STATIC_PROGRESS)->SetWindowTextA("Processing trj file...");
		m_pSSAMDoc->Analyze();

		CString msg;
		msg.Format("Analysis is completed. Total analysis time: %d", m_pSSAMDoc->GetAnalysisTime());
		GetDlgItem(IDC_STATIC_PROGRESS)->SetWindowTextA(msg);
		AfxMessageBox("Analysis complete.");

		if (!m_pSSAMDoc->GetConflictList().empty())
		{
			m_pSSAMDoc->InitFilterParams();
		} else
		{
			AfxMessageBox("No conflict was found.");
		}
		
	} catch (std::runtime_error& e)
	{
		std::string errMsg("Analysis error.  Aborting analysis.\nExtra info:\n");
		errMsg += e.what();
		AfxMessageBox(errMsg.c_str());

		CString msg = "Analysis is aborted due to error.";
		GetDlgItem(IDC_STATIC_PROGRESS)->SetWindowTextA(msg);
	}
}

void CDlg_Configure::OnDeltaposSpinTtc(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_MaxTTC = float(pNMUpDown->iPos + pNMUpDown->iDelta)/10.0;
	UpdateData(FALSE);
	*pResult = 0;
}


void CDlg_Configure::OnDeltaposSpinPet(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_MaxPET = float(pNMUpDown->iPos + pNMUpDown->iDelta)/10.0;
	UpdateData(FALSE);
	*pResult = 0;
}

void CDlg_Configure::UpdateListBox(CListBox* pListBox, CString message)
{
	CSize sz = 0;
	int dx = 0;
	CDC* pDC = pListBox->GetDC();
	int idxStart = 0, idxEnd = 0;
	idxEnd = message.Find('\n', idxStart);
	while (idxEnd >= 0)
	{
		CString subStr = message.Mid(idxStart, idxEnd - idxStart);
		sz = pDC->GetTextExtent(subStr);
		if (sz.cx > dx) dx = sz.cx;
		int idx = pListBox->AddString(subStr);
		pListBox->RedrawWindow();
		pListBox->SetCurSel(idx);
		idxStart = idxEnd+1;
		idxEnd = message.Find('\n', idxStart);
	}

	pListBox->ReleaseDC(pDC);
	if (pListBox->GetHorizontalExtent() < dx)
		pListBox->SetHorizontalExtent(dx);
}

void CDlg_Configure::OnBnClickedButton1()
{
	CDlg_Diagram* dlg = new CDlg_Diagram();
	BOOL IsCreated = dlg->Create(IDD_DIALOG_DIAGRAM);
	dlg->ShowWindow(SW_SHOW);
}
