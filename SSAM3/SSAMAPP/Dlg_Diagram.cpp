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
// Dlg_Diagram.cpp : implementation file
//

#include "stdafx.h"
#include "SSAMAPP.h"
#include "Dlg_Diagram.h"
#include "afxdialogex.h"


// CDlg_Diagram dialog

IMPLEMENT_DYNAMIC(CDlg_Diagram, CDialog)

CDlg_Diagram::CDlg_Diagram(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_Diagram::IDD, pParent)
{

}

CDlg_Diagram::~CDlg_Diagram()
{
}

void CDlg_Diagram::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CDlg_Diagram::PostNcDestroy()
{
    CDialog::PostNcDestroy();
	delete this; 
}

BEGIN_MESSAGE_MAP(CDlg_Diagram, CDialog)
END_MESSAGE_MAP()


// CDlg_Diagram message handlers
