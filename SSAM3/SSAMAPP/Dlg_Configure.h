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
#include <string>
#include <list>
#include <vector>
#include "Dlg_Tab.h"
#include "SSAMDoc.h"

// CDlg_Configure dialog

class CDlg_Configure : public CDlg_Tab
{
	DECLARE_DYNAMIC(CDlg_Configure)

public:
	CDlg_Configure(CWnd* pParent = NULL);   // standard constructor
	CDlg_Configure(SP_SSAMDoc pSSAMDoc, CWnd* pParent = NULL);
	virtual ~CDlg_Configure();

// Dialog Data
	enum { IDD = IDD_DIALOG_CONFIGURE };

	static const int MAX_TTC = 50; // need to divide by 10
	static const int MAX_PET = 100; // need to divide by 10
	static const int MAX_ANGLE = 180;
	
	virtual BOOL OnInitDialog();
	virtual bool SaveData();
	virtual void SetData();
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	float m_MaxTTC, m_MaxPET;
	int m_MaxRearAngle, m_MaxCrossingAngle;
	int m_NumOfThreads;
	int m_IsCalcPUEA, m_IsWriteDat;
	SP_SSAMDoc m_pSSAMDoc;
	
	CListBox m_ListBox_TrjFiles;
	CSpinButtonCtrl m_Spin_MaxTTC, m_Spin_MaxPET;
	CSpinButtonCtrl m_Spin_MaxRearAngle, m_Spin_MaxCrossingAngle;

	void UpdateListBox(CListBox* pListBox, CString message);
public:
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonAnalyze();
	afx_msg void OnDeltaposSpinTtc(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinPet(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton1();
};
