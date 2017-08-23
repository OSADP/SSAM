/*------------------------------------------------------------------------------
   Copyright � 2016-2017
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
#include "afxwin.h"

// CDlg_Tab
class CDlg_Tab : public CDialog
{
	DECLARE_DYNAMIC(CDlg_Tab)

public:
	CDlg_Tab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_Tab();

// Dialog Data
	
	virtual BOOL OnInitDialog() = 0;
	virtual bool SaveData() = 0;
	virtual void SetData() = 0;
	
	void PostNcDestroy()
	{
	    CDialog::PostNcDestroy();
		delete this; 
	}
}; // end CDlg_Tab
