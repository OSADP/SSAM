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
#include "SSAMOSG.h"

// CDlg_OSGViewer dialog
class CDlg_Map;
class CDlg_OSGViewer : public CDialog
{
	DECLARE_DYNAMIC(CDlg_OSGViewer)

public:
	CDlg_OSGViewer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_OSGViewer();

// Dialog Data
	enum { IDD = IDD_DIALOG_OSGVIEWER };

	virtual void PostNcDestroy();
	BOOL OnInitDialog();
	
	void InitOSGViewer();
	void RedrawOSGViewer(SP_SSAMDoc m_pSSAMDoc, const std::list<SP_Conflict>& conflictList, bool IsNewCase);
	bool SaveImageFile(const std::string& f);
	
	void SetMapType(int mt) { m_MapType = mt;}
	void SetMapFile(const std::string& mf ) {m_MapFile = mf;}
	void SetMapCoords(double c[]) 
	{
		for (int i = 0; i < 4; ++i)
		{
			m_MapCoords[i] = c[i];
		}
	}
	void SetConflictScale(double cs) {m_ConflictScale = cs;}
	void SetShapeTypes(const std::vector<int>& STs){m_ShapeTypes = STs;}
	void SetColorCategory(int cc) {m_ColorCategory = cc;}
	void SetColorVecs(const std::vector<std::vector<double> >& CCVs) 
	{
		m_ConflictColorVecs = CCVs;	
	}
	void SetGridSize(double gs) { m_GridSize = gs;}
	void SetNLevels(int nl) { m_NLevels = nl;}
	void SetIntervalColorVecs(const std::vector<std::vector<double> >& ICVs)
	{
		m_IntervalColorVecs = ICVs;
	}
	void SetMapBoundaries(int c[]) 
	{
		for (int i = 0; i < 4; ++i)
		{
			m_Boundaries[i] = c[i];
		}

	}
	void SetFilteredTileIndexes(int idx[]) 
	{
		for (int i = 0; i < 4; ++i)
		{
			m_FilteredTileIndexes[i] = idx[i];
		}
	}

	void ResetMapType(int mt);
	void ResetConflictScale(double cs);
	void ResetConflictProp(double cs, 
		const std::vector<int>& STs,
		int cc,
		const std::vector<std::vector<double> >& CCVs,
		char ResetFlag);
	void ResetMapCoords(double c[]) ;
	void ResetColors(int cc, const std::vector<std::vector<double> >& CCVs);
	void ResetGridSize(double gs) ;
	void ResetNLevels(int n);
	void ResetIntervalColorVecs(const std::vector<std::vector<double> >& ICVs) ;

	void ResetViewer();
	void EditMap(const std::string& f);
	void SwitchMap(bool IsShown);
	
private:
	CDlg_Map* m_pDlg_Map;
	SSAMOSG* m_OSG;
    CRenderingThread* m_ThreadHandle;
	std::string m_ImgFileName;
	std::string m_MapFile;
	double m_MapCoords[4];
	int m_Boundaries[4];
	double m_ConflictScale;
	// index is conflict type - 1
	std::vector<int> m_ShapeTypes;
	int m_ColorCategory;
	// if m_ColorCategory = 0, index is category index
	// if m_ColorCategory = 1, index is conflict type - 1
	std::vector<std::vector<double> > m_ConflictColorVecs;
	int m_MapType;
	double m_GridSize;
	int m_NLevels;
	std::vector<std::vector<double> > m_IntervalColorVecs;
	int m_FilteredTileIndexes[4]; //mini, maxi, minj, maxj
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
