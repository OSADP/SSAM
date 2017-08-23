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
// Dlg_Map.cpp : implementation file
//

#include "stdafx.h"
#include "SSAMAPP.h"
#include "Dlg_Map.h"
#include "afxdialogex.h"
#include "afxbutton.h"
#include "Dlg_ConflictProperties.h"

// CDlg_Map dialog

IMPLEMENT_DYNAMIC(CDlg_Map, CDialog)

CDlg_Map::CDlg_Map(CWnd* pParent /*=NULL*/)
	: CDlg_Tab(pParent)
	, m_ColorCategory(0)
{

}

CDlg_Map::CDlg_Map(SP_SSAMDoc pSSAMDoc, CWnd* pParent /*=NULL*/)
	: CDlg_Tab(pParent)
	, m_pSSAMDoc (pSSAMDoc)
	, m_ConflictScale (1.0)
	, m_ColorCategory(0)
	, m_PrevFilterStatus(false)
	, m_MapType(SSAMOSG::CONFLICTS)
	, m_GridSize (500)
	, m_NLevels(4)
	, m_IsShownMap(true)
{
	m_OrigGridSize = m_GridSize;
	m_OrigNLevels = m_NLevels;
	for (int i = 0; i < 4; ++i)
	{
		m_MapCoords[i] = 0;
	} 

	for (int i = 0; i < Conflict::NUM_CONFLICT_TYPES - 1; ++i)
	{
		m_ShapeTypes.push_back(i);
	}
	
	m_WhiteColor = RGB(255, 255, 255);

	// initial conflict point color vectors
	std::vector<double> cv(3, 0.0);

	// red
	cv[0] = 1.0;
	m_ConflictColorVecs.push_back(cv);
	m_Color[0] = RGB(255, 0, 0);

	// yellow
	cv[1] = 1.0;
	m_ConflictColorVecs.push_back(cv);
	m_Color[1] = RGB(255, 255, 0);

	// blue
	cv[0] = 0;
	cv[1] = 0;
	cv[2] = 1.0;
	m_ConflictColorVecs.push_back(cv);
	m_Color[2] = RGB(0, 0, 255);

	// white
	cv[2] = 0;
	for (int i = 3; i < SSAMOSG::NUM_TTC_LEVELS; ++i)
	{
		m_ConflictColorVecs.push_back(cv);
		m_Color[i] = m_WhiteColor;
	}
	
	m_MapTypes.push_back("Conflict Points");
	m_MapTypes.push_back("Bar Chart");
	m_MapTypes.push_back("Contour");
	m_MapTypes.push_back("Heat Map");

	m_Units = m_pSSAMDoc->GetUnits();

	m_IntervalColors.push_back(RGB(255, 0, 0)); // red
	m_IntervalColors.push_back(RGB(255, 255, 0)); // yellow
	m_IntervalColors.push_back(RGB(0, 255, 0)); // green
	m_IntervalColors.push_back(RGB(135,206,250)); // light blue
	m_IntervalColors.push_back(RGB(0,0,255)); // blue

	for (int i = 0; i < 4; ++i)
	{
		m_FilteredTileIndexes[i] = -1;
	}
}

CDlg_Map::~CDlg_Map()
{
	for (std::vector<CDialog*>::iterator it = m_ModelessDialogs.begin(); 
		it != m_ModelessDialogs.end(); ++it)
	{
		if (*it != NULL)
			delete *it;
	}
}

void CDlg_Map::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CROSSING, m_Combo_ConflictShapes[Conflict::CROSSING - 1]);
	DDX_Control(pDX, IDC_COMBO_LANECHANGE, m_Combo_ConflictShapes[Conflict::LANE_CHANGE - 1]);
	DDX_Control(pDX, IDC_COMBO_READEND, m_Combo_ConflictShapes[Conflict::REAR_END - 1]);
	DDX_Radio(pDX, IDC_RADIO_TYPE, m_ColorCategory);

	DDX_Text(pDX, IDC_EDIT_ULX, m_MapCoords[0]);
	DDX_Text(pDX, IDC_EDIT_ULY, m_MapCoords[1]);
	DDX_Text(pDX, IDC_EDIT_LRX, m_MapCoords[2]);
	DDX_Text(pDX, IDC_EDIT_LRY, m_MapCoords[3]);

	DDX_Control(pDX, IDC_COMBO_MAPTYPES, m_Combo_MapTypes);

	DDX_Control(pDX, IDC_STATIC_UNIT, m_Static_Unit);
	DDX_Control(pDX, IDC_STATIC_XRANGEVAL, m_Static_XRange);
	DDX_Control(pDX, IDC_STATIC_YRANGEVAL, m_Static_YRange);
	DDX_Text(pDX, IDC_EDIT_GRIDSIZE, m_GridSize);
	DDV_MinMaxDouble(pDX, m_GridSize, 0, INT_MAX);

	DDX_Text(pDX, IDC_EDIT_NLEVELS, m_NLevels);
	DDX_Control(pDX, IDC_COMBO_INTERVALS, m_Combo_Intervals);
}

BOOL CDlg_Map::PreTranslateMessage(MSG* pMsg)
{
	POINT mouse;
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		if (msg.message != WM_MOUSEWHEEL)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else
		{
			GetCursorPos(&mouse);
			msg.hwnd = WindowFromPoint(mouse)->GetSafeHwnd();
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return TRUE;
}


BOOL CDlg_Map::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetFilterStatus();
	for (int i = 0; i < Conflict::NUM_CONFLICT_TYPES - 1; ++i)
	{
		m_Combo_ConflictShapes[i].AddString("Cone");
		m_Combo_ConflictShapes[i].AddString("Box");
		m_Combo_ConflictShapes[i].AddString("Sphere");
		m_Combo_ConflictShapes[i].SetCurSel(i);
	}

	for (int i = 0; i < SSAMOSG::NUM_MAP_TYPES; ++i)
	{
		m_Combo_MapTypes.AddString(m_MapTypes[i].c_str());
	}
	m_Combo_MapTypes.SetCurSel(0);

	SetDimensions();
	SetIntervalsComboBox();
	
	m_Dlg_OSGViewer = new CDlg_OSGViewer(this);
	m_Dlg_OSGViewer->Create(IDD_DIALOG_OSGVIEWER, this);
	SetRectangle();
	m_Dlg_OSGViewer->InitOSGViewer();

	ShowCtrlsOnMapType();

	return TRUE;
}

void CDlg_Map::SetRectangle()
{
	CRect Rect;
	GetDlgItem(IDC_STATIC_OSGVIEWER)->GetWindowRect(&Rect);
	
	m_Dlg_OSGViewer->SetWindowPos(NULL, Rect.left - 20, Rect.top - 80,
		Rect.Width(), Rect.Height(),
		SWP_SHOWWINDOW);
}

bool CDlg_Map::SaveData()
{
	return true;
}

void CDlg_Map::SetData()
{
	GetFilterStatus();
	if ((m_pSSAMDoc->IsNewCase() 
			|| m_pSSAMDoc->IsFilterApplied() 
			|| (m_pSSAMDoc->IsFilterApplied() != m_PrevFilterStatus)) 
			&& !m_pSSAMDoc->GetConflictList().empty())
	{
		if (m_pSSAMDoc->IsNewCase() )
		{
			m_MapCoords[0] = m_pSSAMDoc->m_Boundary[0];
			m_MapCoords[1] = m_pSSAMDoc->m_Boundary[3];
			m_MapCoords[2] = m_pSSAMDoc->m_Boundary[2];
			m_MapCoords[3] = m_pSSAMDoc->m_Boundary[1];
		}
		UpdateData(FALSE);
		
		SetDimensions();

		if (m_pSSAMDoc->IsNewCase() || !m_pSSAMDoc->IsFilterApplied())
		{
			for (int i = 0; i < 4; ++i)
			{
				m_FilteredTileIndexes[i] = -1;
			}
		}

		CollectConfig();
		RunOSGViewer();

		tmpStr.Format("%d total", m_pConflictList->size());
		GetDlgItem(IDC_STATIC_NUMOFCONFLICTS)->SetWindowText(tmpStr);

		m_pSSAMDoc->SetIsNewCase(false);

		if (m_pSSAMDoc->IsFilterApplied() != m_PrevFilterStatus)
			m_PrevFilterStatus = m_pSSAMDoc->IsFilterApplied();
	}
}

void CDlg_Map::EnableColorCtrls(BOOL IsTTCEnabled)
{
	if (IsTTCEnabled)
	{
		m_ConflictColorVecs.resize(SSAMOSG::NUM_TTC_LEVELS);
	}

	GetDlgItem(IDC_BUTTON_COLOR00)->EnableWindow(IsTTCEnabled);
	GetDlgItem(IDC_BUTTON_COLOR05)->EnableWindow(IsTTCEnabled);
	GetDlgItem(IDC_BUTTON_COLOR10)->EnableWindow(IsTTCEnabled);
	GetDlgItem(IDC_BUTTON_COLOR15)->EnableWindow(IsTTCEnabled);

	GetDlgItem(IDC_BUTTON_COLORCR)->EnableWindow(!IsTTCEnabled);
	GetDlgItem(IDC_BUTTON_COLORLC)->EnableWindow(!IsTTCEnabled);
	GetDlgItem(IDC_BUTTON_COLORRE)->EnableWindow(!IsTTCEnabled);
}

// 0x00bbggrr
void CDlg_Map::ConvertCOLORREFToVec(COLORREF Color, std::vector<double>& colorVec)
{
	for (int i = 0; i < 3; ++i)
	{
		colorVec.push_back(double((Color >> (8*i)) & 0xFF)/255.0);
	}
}

COLORREF CDlg_Map::GetCOLORREF(const std::vector<double>& colorVec)
{
	COLORREF color = 0x00000000;
	for (int i = 0; i < 3; ++i)
	{
		color |= (int(colorVec[i]*255.0) << (8*i));
	}
	return color;
}

void CDlg_Map::CollectConfig()
{
	UpdateData(TRUE);
	switch (m_MapType)
	{
	case SSAMOSG::CONFLICTS:
		// index is conflict type - 1
		m_ShapeTypes.clear();
		for (int i = 0; i < Conflict::NUM_CONFLICT_TYPES-1; ++i)
		{
			m_ShapeTypes.push_back(m_Combo_ConflictShapes[i].GetCurSel());
		}
		// if colorCategory = 0, index is TTC level index
		// if colorCategory = 1, index is conflict type - 1
		m_ConflictColorVecs.clear();
		if (m_ColorCategory == SSAMOSG::TTC)
		{
			m_ConflictColorVecs.resize(SSAMOSG::NUM_TTC_LEVELS);
			for (int i = 0; i < m_ConflictColorVecs.size()-1; ++i)
			{
				ConvertCOLORREFToVec(m_Color[i], m_ConflictColorVecs[i]);
			}
			m_ConflictColorVecs[4] = std::vector<double>( 4, 1.0); // set to white for TTC > 1.5 if exists
		} else if (m_ColorCategory == SSAMOSG::TYPE)
		{
			m_ConflictColorVecs.resize(Conflict::NUM_CONFLICT_TYPES - 1);
			for (int i = 0; i < m_ConflictColorVecs.size(); ++i)
			{
				ConvertCOLORREFToVec(m_Color[i], m_ConflictColorVecs[i]);
			}
		}
		break;
	case SSAMOSG::BARCHART:
		m_ConflictColorVecs.clear();
		m_ColorCategory = SSAMOSG::TYPE;
		m_ConflictColorVecs.resize(Conflict::NUM_CONFLICT_TYPES - 1);
		for (int i = 0; i < m_ConflictColorVecs.size(); ++i)
		{
			ConvertCOLORREFToVec(m_Color[i], m_ConflictColorVecs[i]);
		}
		break;
	case SSAMOSG::CONTOUR:
		break;
	default:
		break;
	}

	m_IntervalColorVecs.clear();
	m_IntervalColorVecs.resize(m_IntervalColors.size());
	for (int i = 0; i < m_IntervalColorVecs.size(); ++i)
	{
		ConvertCOLORREFToVec(m_IntervalColors[i], m_IntervalColorVecs[i]);
	}
}

void CDlg_Map::RunOSGViewer()
{
	m_Dlg_OSGViewer->SetMapType(m_MapType);
	m_Dlg_OSGViewer->SetMapFile(m_MapFile);
	m_Dlg_OSGViewer->SetMapCoords(m_MapCoords);
	m_Dlg_OSGViewer->SetConflictScale(m_ConflictScale);
	m_Dlg_OSGViewer->SetShapeTypes(m_ShapeTypes);
	m_Dlg_OSGViewer->SetColorCategory(m_ColorCategory);
	m_Dlg_OSGViewer->SetColorVecs(m_ConflictColorVecs);
	m_Dlg_OSGViewer->SetGridSize(m_GridSize);
	m_Dlg_OSGViewer->SetNLevels(m_NLevels);
	m_Dlg_OSGViewer->SetIntervalColorVecs(m_IntervalColorVecs);
	m_Dlg_OSGViewer->SetMapBoundaries(m_pSSAMDoc->m_Boundary);
	m_Dlg_OSGViewer->SetFilteredTileIndexes(m_FilteredTileIndexes);

	GetDlgItem(IDC_STATIC_LOADSTATUS)->ShowWindow(TRUE);
	m_Dlg_OSGViewer->RedrawOSGViewer(m_pSSAMDoc, *m_pConflictList, m_pSSAMDoc->IsNewCase());
	GetDlgItem(IDC_STATIC_LOADSTATUS)->ShowWindow(FALSE);
}

BEGIN_MESSAGE_MAP(CDlg_Map, CDialog)
	ON_MESSAGE(WM_OSGDATA, &CDlg_Map::OnOSGDataMsg)
	ON_BN_CLICKED(IDC_BUTTON_SAVEIMAGE, &CDlg_Map::OnBnClickedButtonSaveimage)
	ON_BN_CLICKED(IDC_BUTTON_COLOR00, &CDlg_Map::OnBnClickedButtonColor00)
	ON_BN_CLICKED(IDC_BUTTON_SHOWCONFLICTS, &CDlg_Map::OnBnClickedButtonShowconflicts)
	ON_BN_CLICKED(IDC_BUTTON_COLOR05, &CDlg_Map::OnBnClickedButtonColor05)
	ON_BN_CLICKED(IDC_BUTTON_COLOR10, &CDlg_Map::OnBnClickedButtonColor10)
	ON_BN_CLICKED(IDC_BUTTON_COLOR15, &CDlg_Map::OnBnClickedButtonColor15)
	ON_BN_CLICKED(IDC_BUTTON_COLORCR, &CDlg_Map::OnBnClickedButtonColorcr)
	ON_BN_CLICKED(IDC_BUTTON_COLORLC, &CDlg_Map::OnBnClickedButtonColorlc)
	ON_BN_CLICKED(IDC_BUTTON_COLORRE, &CDlg_Map::OnBnClickedButtonColorre)
	ON_BN_CLICKED(IDC_BUTTON_SCALEUPICON, &CDlg_Map::OnBnClickedButtonScaleupicon)
	ON_BN_CLICKED(IDC_BUTTON_SCALEDOWNICON, &CDlg_Map::OnBnClickedButtonScaledownicon)
	ON_BN_CLICKED(IDC_RADIO_SEVERITY, &CDlg_Map::OnBnClickedRadioSeverity)
	ON_BN_CLICKED(IDC_RADIO_TYPE, &CDlg_Map::OnBnClickedRadioType)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, &CDlg_Map::OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_EDITMAP, &CDlg_Map::OnBnClickedButtonEditmap)
	ON_CBN_SELCHANGE(IDC_COMBO_MAPTYPES, &CDlg_Map::OnCbnSelchangeComboMaptypes)
	ON_EN_KILLFOCUS(IDC_EDIT_NLEVELS, &CDlg_Map::OnEnKillfocusEditNlevels)
	ON_BN_CLICKED(IDC_BUTTON_FIT, &CDlg_Map::OnBnClickedButtonFit)
	ON_BN_CLICKED(IDC_BUTTON_INTERVALCOLOR, &CDlg_Map::OnBnClickedButtonIntervalcolor)
	ON_BN_CLICKED(IDC_BUTTON_MAPSWITCH, &CDlg_Map::OnBnClickedButtonMapswitch)
	ON_BN_CLICKED(IDC_BUTTON_DELETEIMG, &CDlg_Map::OnBnClickedButtonDeleteimg)
END_MESSAGE_MAP()


// CDlg_Map message handlers

LRESULT CDlg_Map::OnOSGDataMsg(WPARAM wParam, LPARAM lParam)
{
	OSGMsgParam* param = (OSGMsgParam*)lParam;
	if (param->m_FilterFlag == 0)
	{
		DisplayConflictProp(param->m_BL,param->m_TR, param->m_ConflictType);
	} else
	{
		m_pSSAMDoc->GetFilterParams()->m_Area[0] = param->m_BL[0];
		m_pSSAMDoc->GetFilterParams()->m_Area[1] = param->m_BL[1];
		m_pSSAMDoc->GetFilterParams()->m_Area[2] = param->m_TR[0];
		m_pSSAMDoc->GetFilterParams()->m_Area[3] = param->m_TR[1];
		if (m_pSSAMDoc->ApplyFilter(true))
		{
			AfxMessageBox("Filter Applied. Please go to the Conflict panel and Summary panel to view results.");
			
			for (int i = 0; i < 4; ++i)
			{
				m_FilteredTileIndexes[i] = param->m_FilteredIdx[i];
			}
			SetData();
		} else
		{
			AfxMessageBox("The Filter did not generate any values. Thus filter not applied.");
		}
	}
	delete param;
	return 0;
}

void CDlg_Map::OnBnClickedButtonSaveimage()
{
	std::string tmpFileName(m_pSSAMDoc->GetCsvName());
	CFileDialog fdlg (FALSE, "*.bmp", tmpFileName.substr(0, tmpFileName.length() - 4).c_str(),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |OFN_ENABLESIZING | OFN_EXPLORER,
			"BMP (Bitmap Image) (*.bmp)|*.bmp|",NULL,0,true);	

	if(fdlg.DoModal()==IDOK)
	{
		m_ImageFile = fdlg.GetPathName  ();
		m_Dlg_OSGViewer->SaveImageFile(m_ImageFile);
	}
}

void CDlg_Map::OnBnClickedScaleButton(double scaleParam)
{
	m_ConflictScale *= scaleParam;
	if (!m_pSSAMDoc->GetConflictList().empty())
	{
		m_Dlg_OSGViewer->ResetConflictScale(m_ConflictScale);
	}
}


void CDlg_Map::OnBnClickedColorButton(int idx, int nIDC)
{
	CColorDialog dlg(m_Color[idx]); 
	if (dlg.DoModal() == IDOK) 
	{ 
		m_Color[idx] = dlg.GetColor(); 
		if (m_Color[idx] != m_WhiteColor)
			((CMFCButton*)GetDlgItem(nIDC))->SetTextColor(m_Color[idx]);
	}
}

void CDlg_Map::OnBnClickedButtonShowconflicts()
{
	if (!m_pSSAMDoc->GetConflictList().empty())
	{
		CollectConfig();
		if (m_MapType == SSAMOSG::CONFLICTS)
		{
			// set reset flag to 0x06, meaning reset shape types and conflict colors
			m_Dlg_OSGViewer->ResetConflictProp(m_ConflictScale, m_ShapeTypes, m_ColorCategory, m_ConflictColorVecs,
				0x06);
		} else
		{
			if (m_MapType == SSAMOSG::BARCHART)
				m_Dlg_OSGViewer->ResetColors(m_ColorCategory, m_ConflictColorVecs);

			if (m_OrigGridSize != m_GridSize)
			{
				m_Dlg_OSGViewer->ResetGridSize(m_GridSize);
				m_OrigGridSize = m_GridSize;
			}


			if (m_OrigNLevels != m_NLevels)
			{
				m_Dlg_OSGViewer->ResetNLevels(m_NLevels);
				m_OrigNLevels = m_NLevels;
			} 
			m_Dlg_OSGViewer->ResetIntervalColorVecs(m_IntervalColorVecs);
			
		}
	}
}



void CDlg_Map::OnBnClickedButtonColor00()
{
	OnBnClickedColorButton(0, IDC_BUTTON_COLOR00);
}


void CDlg_Map::OnBnClickedButtonColor05()
{
	OnBnClickedColorButton(1, IDC_BUTTON_COLOR05);
}

void CDlg_Map::OnBnClickedButtonColor10()
{
	OnBnClickedColorButton(2, IDC_BUTTON_COLOR10);
}


void CDlg_Map::OnBnClickedButtonColor15()
{
	OnBnClickedColorButton(3, IDC_BUTTON_COLOR15);
}


void CDlg_Map::OnBnClickedButtonColorcr()
{
	OnBnClickedColorButton(Conflict::CROSSING - 1, IDC_BUTTON_COLORCR);
}


void CDlg_Map::OnBnClickedButtonColorlc()
{
	OnBnClickedColorButton(Conflict::LANE_CHANGE - 1, IDC_BUTTON_COLORLC);
}


void CDlg_Map::OnBnClickedButtonColorre()
{
	OnBnClickedColorButton(Conflict::REAR_END - 1, IDC_BUTTON_COLORRE);
}


void CDlg_Map::OnBnClickedButtonScaleupicon()
{
	OnBnClickedScaleButton(1.25);
}


void CDlg_Map::OnBnClickedButtonScaledownicon()
{
	OnBnClickedScaleButton(0.75);
}

void CDlg_Map::OnBnClickedRadioSeverity()
{
	EnableColorCtrls(TRUE);
}


void CDlg_Map::OnBnClickedRadioType()
{
	EnableColorCtrls(FALSE);
}


void CDlg_Map::OnBnClickedButtonEdit()
{
	UpdateData(TRUE);
	if (!m_pSSAMDoc->GetConflictList().empty()
		&& !m_MapFile.empty())
	{
		CollectConfig();
		m_Dlg_OSGViewer->ResetMapCoords(m_MapCoords);
	}
}


void CDlg_Map::OnBnClickedButtonEditmap()
{
	if (m_pSSAMDoc->GetConflictList().empty())
	{
		AfxMessageBox("No SSAM result available");
		return;
	}
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_LONGNAMES|OFN_ENABLESIZING,
		_T("JPEG (*.jpg)|*.jpg|"),NULL,0,true);

	if(dlg.DoModal() == IDOK)
	{
		m_MapFile = dlg.GetPathName();
		GetDlgItem(IDC_STATIC_MAP)->SetWindowText(GetFileName(m_MapFile).c_str());
		m_Dlg_OSGViewer->EditMap(m_MapFile);
	}
}

void CDlg_Map::GetFilterStatus()
{
	if (!m_pSSAMDoc->IsFilterApplied())
	{
		m_pConflictList = &(m_pSSAMDoc->GetConflictList());
	} else
	{
		m_pConflictList = &(m_pSSAMDoc->GetFilteredConflictList());
	}
}

void CDlg_Map::SetDimensions()
{
	m_Units = m_pSSAMDoc->GetUnits();
	tmpStr.Format("(unit: %s)", (m_Units==Dimensions::ENGLISH_UNITS) ? "feet" : "meters");
	m_Static_Unit.SetWindowText(tmpStr);

	tmpStr.Format("[%d, %d]", m_pSSAMDoc->m_Boundary[0], m_pSSAMDoc->m_Boundary[2]);
	m_Static_XRange.SetWindowText(tmpStr);

	tmpStr.Format("[%d, %d]", m_pSSAMDoc->m_Boundary[1], m_pSSAMDoc->m_Boundary[3]);
	m_Static_YRange.SetWindowText(tmpStr);

}

void CDlg_Map::ShowCtrlsOnMapType()
{
	switch (m_MapType)
	{
	case SSAMOSG::CONFLICTS:
		ShowBarChartCtrls(SW_HIDE);
		ShowContourCtrls(SW_HIDE);
		ShowConflictCtrls(SW_SHOW);
		break;
	case SSAMOSG::BARCHART:
		m_ColorCategory = 0;
		EnableColorCtrls(FALSE);
		ShowConflictCtrls(SW_HIDE);
		ShowContourCtrls(SW_HIDE);
		ShowBarChartCtrls(SW_SHOW);
		break;
	case SSAMOSG::CONTOUR:
	case SSAMOSG::HEAT:
		ShowConflictCtrls(SW_HIDE);
		ShowBarChartCtrls(SW_HIDE);
		ShowContourCtrls(SW_SHOW);
		break;
	default:
		break;
	}
}

void CDlg_Map::ShowConflictCtrls(int nCmdShow)
{
	GetDlgItem(IDC_STATIC_SCALE)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_BUTTON_SCALEUPICON)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_BUTTON_SCALEDOWNICON)->ShowWindow(nCmdShow);

	GetDlgItem(IDC_STATIC_SHAPE)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_CROSSING)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_COMBO_CROSSING)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_REAREND)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_COMBO_READEND)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_LANECHANGE)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_COMBO_LANECHANGE)->ShowWindow(nCmdShow);

	GetDlgItem(IDC_STATIC_COLOR)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_RADIO_SEVERITY)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_BUTTON_COLOR00)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_BUTTON_COLOR05)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_BUTTON_COLOR10)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_BUTTON_COLOR15)->ShowWindow(nCmdShow);

	GetDlgItem(IDC_RADIO_TYPE)->ShowWindow(nCmdShow);
	ShowTypeColorCtrls(nCmdShow);
}

void CDlg_Map::ShowBarChartCtrls(int nCmdShow)
{
	ShowGridCtrls(nCmdShow);
	GetDlgItem(IDC_STATIC_BARCOLOR)->ShowWindow(nCmdShow);	
	ShowTypeColorCtrls(nCmdShow);
}

void CDlg_Map::ShowContourCtrls(int nCmdShow)
{
	ShowGridCtrls(nCmdShow);
	GetDlgItem(IDC_STATIC_NLEVELS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDIT_NLEVELS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_INTERVAL)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_COMBO_INTERVALS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_BUTTON_INTERVALCOLOR)->ShowWindow(nCmdShow);
}

void CDlg_Map::ShowGridCtrls(int nCmdShow)
{
	GetDlgItem(IDC_STATIC_DIMENSIONS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_UNIT)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_XRANGE)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_XRANGEVAL)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_YRANGE)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_YRANGEVAL)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC_GRID)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDIT_GRIDSIZE)->ShowWindow(nCmdShow);
}

void CDlg_Map::ShowTypeColorCtrls(int nCmdShow)
{
	GetDlgItem(IDC_BUTTON_COLORCR)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_BUTTON_COLORRE)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_BUTTON_COLORLC)->ShowWindow(nCmdShow);
}

void CDlg_Map::OnCbnSelchangeComboMaptypes()
{
	int prevMapType = m_MapType;
	m_MapType = m_Combo_MapTypes.GetCurSel();
	if (m_MapType != prevMapType)
	{
		ShowCtrlsOnMapType();
		m_Dlg_OSGViewer->ResetMapType(m_MapType);
	}
}


void CDlg_Map::OnEnKillfocusEditNlevels()
{
	int prevNLevels = m_NLevels;
	UpdateData(TRUE);
	if (m_NLevels+1 > m_IntervalColors.size())
	{
		COLORREF lowColor = m_IntervalColors.back();
		m_IntervalColors.resize(m_NLevels+1, lowColor);
	}

	if (prevNLevels != m_NLevels)
		SetIntervalsComboBox();
}

void CDlg_Map::SetIntervalsComboBox()
{
	m_Combo_Intervals.ResetContent();
	for (int i = 0; i <= m_NLevels; ++i)
	{
		m_Combo_Intervals.AddString(std::to_string(i).c_str());
	}
	m_Combo_Intervals.SetCurSel(0);

}

void CDlg_Map::DisplayConflictProp(double* BL, double* TR, int conflictType)
{
	CDlg_ConflictProperties* dlg = new CDlg_ConflictProperties(m_pSSAMDoc, BL,TR, conflictType, 
		m_ModelessDialogs.size(), this);
	BOOL IsCreated = dlg->Create(IDD_DIALOG_CONFLICTPROP);
	dlg->ShowWindow(SW_SHOW);

	m_ModelessDialogs.push_back(dlg);
}

void CDlg_Map::OnBnClickedButtonFit()
{
	m_Dlg_OSGViewer->ResetViewer();
}

void CDlg_Map::OnBnClickedButtonIntervalcolor()
{
	int idx = m_Combo_Intervals.GetCurSel();
	CColorDialog dlg(m_IntervalColors[idx]); 
	if (dlg.DoModal() == IDOK) 
	{ 
		m_IntervalColors[idx] = dlg.GetColor(); 
	}
}

void CDlg_Map::OnBnClickedButtonMapswitch()
{
	m_IsShownMap = !m_IsShownMap;
	m_Dlg_OSGViewer->SwitchMap(m_IsShownMap);
}


void CDlg_Map::OnBnClickedButtonDeleteimg()
{
	m_MapFile = "";
	GetDlgItem(IDC_STATIC_MAP)->SetWindowText("none");
	m_Dlg_OSGViewer->EditMap("");
}

