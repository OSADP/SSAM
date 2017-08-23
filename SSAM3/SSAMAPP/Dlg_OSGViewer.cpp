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
// Dlg_OSGViewer.cpp : implementation file
//

#include "stdafx.h"
#include "SSAMAPP.h"
#include "Dlg_Map.h"
#include "Dlg_OSGViewer.h"
#include "afxdialogex.h"


// CDlg_OSGViewer dialog

IMPLEMENT_DYNAMIC(CDlg_OSGViewer, CDialog)

CDlg_OSGViewer::CDlg_OSGViewer(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_OSGViewer::IDD, pParent)
	, m_pDlg_Map((CDlg_Map*)pParent)
	, m_OSG(NULL)
	, m_ThreadHandle (NULL)
{

}

CDlg_OSGViewer::~CDlg_OSGViewer()
{
	if (m_OSG != NULL)
		delete m_OSG;

	if (m_ThreadHandle != NULL)
		delete m_ThreadHandle;
}

void CDlg_OSGViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CDlg_OSGViewer::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;
}

void CDlg_OSGViewer::PostNcDestroy()
{
    CDialog::PostNcDestroy();
	delete this; 
}


BEGIN_MESSAGE_MAP(CDlg_OSGViewer, CDialog)
END_MESSAGE_MAP()

void CDlg_OSGViewer::InitOSGViewer()
{
	m_OSG = new SSAMOSG(m_hWnd, m_pDlg_Map->GetSafeHwnd());
	m_OSG->InitOSG();
	m_ThreadHandle = new CRenderingThread(m_OSG);
	m_ThreadHandle->start();
}

void CDlg_OSGViewer::RedrawOSGViewer(SP_SSAMDoc pSSAMDoc, const std::list<SP_Conflict>& conflictList, bool IsNewCase)
{
	if (m_ThreadHandle != NULL)
	{
		m_ThreadHandle->SetMapType(m_MapType);
		m_ThreadHandle->SetMapFile(m_MapFile);
		m_ThreadHandle->SetMapCoords(m_MapCoords);
		m_ThreadHandle->SetConflictScale(m_ConflictScale);
		m_ThreadHandle->SetShapeTypes(m_ShapeTypes);
		m_ThreadHandle->SetColorCategory(m_ColorCategory);
		m_ThreadHandle->SetColorVecs(m_ConflictColorVecs);
		m_ThreadHandle->SetGridSize(m_GridSize);
		m_ThreadHandle->SetNLevels(m_NLevels);
		m_ThreadHandle->SetIntervalColorVecs(m_IntervalColorVecs);
		m_ThreadHandle->SetMapBoundaries(m_Boundaries);
		m_ThreadHandle->SetFilteredTileIndexes(m_FilteredTileIndexes);

		m_ThreadHandle->SetSceneGraph(pSSAMDoc->GetCSVFile(), conflictList, IsNewCase);
	}
	
}

bool CDlg_OSGViewer::SaveImageFile(const std::string& f)
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->SetImageFileName(f);

	return true;
}

void CDlg_OSGViewer::ResetMapType(int mt)
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->ResetMapType(mt);
}

void CDlg_OSGViewer::ResetConflictScale(double cs)
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->ResetConflictScale(cs);

	
}

void CDlg_OSGViewer::ResetConflictProp(double cs, 
		const std::vector<int>& STs,
		int cc,
		const std::vector<std::vector<double> >& CCVs,
		char ResetFlag)
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->ResetConflictProp(cs, STs, cc, CCVs, ResetFlag);
}

void CDlg_OSGViewer::ResetMapCoords(double c[])
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->ResetMapCoords(c);
}

void CDlg_OSGViewer::ResetViewer()
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->ResetViewer();
}

void CDlg_OSGViewer::EditMap(const std::string& f)
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->EditMap(f);
}

void CDlg_OSGViewer::SwitchMap(bool IsShown)
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->SwitchMap(IsShown);
}

void CDlg_OSGViewer::ResetColors(int cc, const std::vector<std::vector<double> >& CCVs)
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->ResetColors(cc, CCVs);
}

void CDlg_OSGViewer::ResetGridSize(double gs)
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->ResetGridSize(gs);
}

void CDlg_OSGViewer::ResetNLevels(int n)
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->ResetNLevels(n);
}

void CDlg_OSGViewer::ResetIntervalColorVecs(const std::vector<std::vector<double> >& ICVs)  
{
	if (m_ThreadHandle != NULL)
		m_ThreadHandle->ResetIntervalColorVecs(ICVs);
}
