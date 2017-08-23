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
#include <vector>
#include "SSAMDoc.h"
#include "Dlg_Tab.h"

// CDlg_Filter dialog


class CDlg_Filter : public CDlg_Tab
{
	DECLARE_DYNAMIC(CDlg_Filter)

public:
	CDlg_Filter(CWnd* pParent = NULL);   // standard constructor
	CDlg_Filter(SP_SSAMDoc pSSAMDoc, CWnd* pParent = NULL); 
	virtual ~CDlg_Filter();

// Dialog Data
	enum { IDD = IDD_DIALOG_FILTER };

	virtual BOOL OnInitDialog();
	virtual bool SaveData();
	virtual void SetData();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	SP_SSAMDoc m_pSSAMDoc;
	SP_FilterParams m_pFilterParams;

	CSpinButtonCtrl m_Spin_MinTTC, m_Spin_MaxTTC;
	CSpinButtonCtrl m_Spin_MinPET, m_Spin_MaxPET;
	CSpinButtonCtrl m_Spin_MinMaxS, m_Spin_MaxMaxS;
	CSpinButtonCtrl m_Spin_MinDeltaS, m_Spin_MaxDeltaS;
	CSpinButtonCtrl m_Spin_MinDR, m_Spin_MaxDR;
	CSpinButtonCtrl m_Spin_MinMaxD, m_Spin_MaxMaxD;
	CSpinButtonCtrl m_Spin_MinMaxDeltaV, m_Spin_MaxMaxDeltaV;
	CSpinButtonCtrl m_Spin_Area[4];
	CListBox m_List_ConflictTypes;
	CListBox m_List_Links;
	CListBox m_List_TRJFiles;

	char tmp_str[1000];

	void OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult, double& spinValue, double step = 0.05);
	void InitListBoxes();
	void SetListBoxes();
	void UpdateListBox(CListBox* pListBox, CString message);
public:
	afx_msg void OnDeltaposSpinMinttc(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMaxttc(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMinpet(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMaxpet(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMinmaxs(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMaxmaxs(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMindeltas(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMaxdeltas(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMindr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMaxdr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMinmaxd(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMaxmaxd(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMinmaxdeltav(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinMaxmaxdeltav(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinLlx(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinLly(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinUrx(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinUry(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonRectoredefault();
};
