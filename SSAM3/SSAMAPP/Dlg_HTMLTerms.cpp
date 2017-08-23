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
// Dlg_HTMLTerms.cpp : implementation file
//

#include "stdafx.h"
#include "SSAMAPP.h"
#include "Dlg_HTMLTerms.h"
#include <string>

// CDlg_HTMLTerms dialog

IMPLEMENT_DYNCREATE(CDlg_HTMLTerms, CDHtmlDialog)

CDlg_HTMLTerms::CDlg_HTMLTerms(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CDlg_HTMLTerms::IDD, CDlg_HTMLTerms::IDH, pParent)
{

}

CDlg_HTMLTerms::~CDlg_HTMLTerms()
{
}

void CDlg_HTMLTerms::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CDlg_HTMLTerms::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	ShowScrollBar(SB_VERT, TRUE);
	GetWindowRect(m_rect);
	m_nScrollPos = 0;

	char buffer[MAX_PATH];
	GetModuleFileName( NULL, buffer, MAX_PATH );
	std::string tmpFolder(buffer);
	std::string::size_type pos = tmpFolder.find_last_of( "\\" );
	std::string aboutFile(tmpFolder);
	if (pos != std::string::npos)
		aboutFile = tmpFolder.substr(0, pos);
	aboutFile += "\\about.htm";
	Navigate(aboutFile.c_str());

	return TRUE;  
}

BEGIN_MESSAGE_MAP(CDlg_HTMLTerms, CDHtmlDialog)
	ON_WM_SIZE()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlg_HTMLTerms)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CDlg_HTMLTerms message handlers

HRESULT CDlg_HTMLTerms::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CDlg_HTMLTerms::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}


void CDlg_HTMLTerms::OnSize(UINT nType, int cx, int cy)
{
	CDHtmlDialog::OnSize(nType, cx, cy);

	m_nCurHeight = cy;
	int nScrollMax;
	if (cy < m_rect.Height())
	{
		nScrollMax = m_rect.Height() - cy;
	}
	else
	{
		nScrollMax = 0;
	}

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nMin = 0;
	si.nMax = nScrollMax;
	si.nPage = si.nMax / 10;
	si.nPos = 0;
	SetScrollInfo(SB_VERT, &si, TRUE);
}


void CDlg_HTMLTerms::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int nDelta = 0;
	int nMaxPos = m_rect.Height() - m_nCurHeight;

	switch (nSBCode)
	{
	case SB_LINEDOWN:
		if (m_nScrollPos >= nMaxPos)
			return;
		nDelta = min(nMaxPos/100, nMaxPos - m_nScrollPos);
		break;
	case SB_LINEUP:
		if (m_nScrollPos <= 0)
			return;
		nDelta = -min(nMaxPos/100, m_nScrollPos);
		break;
	case SB_PAGEDOWN:
		if (m_nScrollPos >= nMaxPos)
			return;
		nDelta = min(nMaxPos/10, nMaxPos - m_nScrollPos);
		break;
	case SB_THUMBPOSITION:
		nDelta = (int)nPos - m_nScrollPos;
		break;
	case SB_PAGEUP:
		if (m_nScrollPos <= 0)
			return;
		nDelta = -min(nMaxPos/10, m_nScrollPos);
		break;
	default:
		break;
	}
	m_nScrollPos += nDelta;
	SetScrollPos(SB_VERT, m_nScrollPos, TRUE);
	ScrollWindow(0, -nDelta);

	CDHtmlDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}
