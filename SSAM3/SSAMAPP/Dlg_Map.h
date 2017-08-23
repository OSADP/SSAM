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
#include "Dlg_Tab.h"
#include "Dlg_OSGViewer.h"

// CDlg_Map dialog

class CDlg_Map : public CDlg_Tab
{
	DECLARE_DYNAMIC(CDlg_Map)

public:
	CDlg_Map(CWnd* pParent = NULL);   // standard constructor
	CDlg_Map(SP_SSAMDoc pSSAMDoc, CWnd* pParent = NULL);
	virtual ~CDlg_Map();

// Dialog Data
	enum { IDD = IDD_DIALOG_MAP };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual bool SaveData();
	virtual void SetData();

	void DeleteModelessDialog(int i) 
	{
		if (i < m_ModelessDialogs.size()) 
			m_ModelessDialogs[i] = NULL;
	}
private:
	SP_SSAMDoc m_pSSAMDoc;
	CDlg_OSGViewer* m_Dlg_OSGViewer; 
	std::string m_ImageFile;
	std::string m_MapFile;
	double m_MapCoords[4];
	double m_ConflictScale;
	// index is conflict type - 1
	std::vector<int> m_ShapeTypes;
	int m_ColorCategory;
	// if colorCategory = 0, index is TTC category index
	// if colorCategory = 1, index is conflict type - 1
	std::vector<std::vector<double> > m_ConflictColorVecs;
	std::list<SP_Conflict>* m_pConflictList;
	bool m_PrevFilterStatus;
	std::vector<std::string> m_MapTypes;
	int m_MapType;
	int m_Units;
	double m_GridSize, m_OrigGridSize;
	int m_NLevels, m_OrigNLevels;
	std::vector<COLORREF> m_IntervalColors;
	std::vector<std::vector<double> > m_IntervalColorVecs;
	std::vector<CDialog*> m_ModelessDialogs; 
	int m_FilteredTileIndexes[4]; //mini, maxi, minj, maxj
	bool m_IsShownMap;

	CComboBox m_Combo_ConflictShapes[Conflict::NUM_CONFLICT_TYPES - 1];
	COLORREF m_Color[SSAMOSG::NUM_TTC_LEVELS];
	COLORREF m_WhiteColor;
	CComboBox m_Combo_MapTypes;
	CStatic m_Static_Unit;
	CStatic m_Static_XRange;
	CStatic m_Static_YRange;
	CComboBox m_Combo_Intervals;
	
	CString tmpStr;

	
	// 0x00bbggrr
	void ConvertCOLORREFToVec(COLORREF Color, std::vector<double>& colorVec);
	COLORREF GetCOLORREF(const std::vector<double>& colorVec);
	void SetRectangle();
	void EnableColorCtrls(BOOL IsTTCEnabled);
	void OnBnClickedColorButton(int idx, int nIDC);
	void OnBnClickedScaleButton(double scaleParam);
	void CollectConfig();
	void RunOSGViewer();
	void GetFilterStatus();
	void SetDimensions();
	void SetIntervalsComboBox();
	void ShowCtrlsOnMapType();
	void ShowConflictCtrls(int nCmdShow);
	void ShowBarChartCtrls(int nCmdShow);
	void ShowContourCtrls(int nCmdShow);
	void ShowGridCtrls(int nCmdShow);
	void ShowTypeColorCtrls(int nCmdShow);
	void DisplayConflictProp(double* BL, double* TR, int conflictType);

	std::string& GetFileName(const std::string& fullName)
	{
		std::string fileName(fullName);
		std::size_t pos = fileName.find_last_of('\\');
		if (pos != std::string::npos)
		{
			fileName = fileName.substr(pos+1);
		}
		return fileName;
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnOSGDataMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonSaveimage();
	afx_msg void OnBnClickedButtonColor00();
	afx_msg void OnBnClickedButtonShowconflicts();
	afx_msg void OnBnClickedButtonColor05();
	afx_msg void OnBnClickedButtonColor10();
	afx_msg void OnBnClickedButtonColor15();
	afx_msg void OnBnClickedButtonColorcr();
	afx_msg void OnBnClickedButtonColorlc();
	afx_msg void OnBnClickedButtonColorre();
	afx_msg void OnBnClickedButtonScaleupicon();
	afx_msg void OnBnClickedButtonScaledownicon();
	afx_msg void OnBnClickedRadioSeverity();
	afx_msg void OnBnClickedRadioType();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonEditmap();
	afx_msg void OnCbnSelchangeComboMaptypes();
	afx_msg void OnEnKillfocusEditNlevels();
	afx_msg void OnBnClickedButtonFit();
	afx_msg void OnBnClickedButtonIntervalcolor();
	afx_msg void OnBnClickedButtonMapswitch();
	afx_msg void OnBnClickedButtonDeleteimg();
};
