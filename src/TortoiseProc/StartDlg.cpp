// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2003-2008 - TortoiseSVN
// Copyright (C) 2009-2013 - TortoiseGit

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "stdafx.h"
#include "TortoiseProc.h"
#include "StartDlg.h"
#include "AppUtils.h"
#include "Git.h"

CStartDlg::CStartDlg(CWnd* pParent /*=NULL*/)
	: CStandAloneDialog(CStartDlg::IDD, pParent)
{
}

CStartDlg::~CStartDlg()
{
}

void CStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CStandAloneDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CStartDlg, CStandAloneDialog)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

#define BANNER_WIDTH	468
#define BANNER_HEIGHT	64
#define BANNER_LEFT		15
#define BANNER_TOP		5

BOOL CStartDlg::OnInitDialog()
{
	CStandAloneDialog::OnInitDialog();
	CAppUtils::MarkWindowAsUnpinnable(m_hWnd);

	CPictureHolder tmpPic;
	tmpPic.CreateFromBitmap(IDB_LOGOFLIPPED);
	m_renderSrc.Create32BitFromPicture(&tmpPic, BANNER_WIDTH, BANNER_HEIGHT);
	m_renderDest.Create32BitFromPicture(&tmpPic, BANNER_WIDTH, BANNER_HEIGHT);

	m_waterEffect.Create(BANNER_WIDTH, BANNER_HEIGHT);
	SetTimer(ID_EFFECTTIMER, 40, NULL);
	SetTimer(ID_DROPTIMER, 1500, NULL);

	RECT rectTab;
	GetDlgItem(IDC_TABSTART)->GetWindowRect(&rectTab);
	m_wndTab.Create(CMFCTabCtrl::STYLE_3D_ROUNDED, rectTab, this, 1, CMFCTabCtrl::LOCATION_TOP);
	m_wndButton1.Create(_T("Hi 1"), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), &m_wndTab, 1);
	m_wndTab.AddTab(&m_wndButton1, _T("One"), 0, FALSE);
	m_wndButton2.Create(_T("Hi 2"), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), &m_wndTab, 2);
	m_wndTab.AddTab(&m_wndButton2, _T("Two"), 1, FALSE);

	CenterWindow(CWnd::FromHandle(hWndExplorer));
	GetDlgItem(IDOK)->SetFocus();
	return FALSE;
}

void CStartDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == ID_EFFECTTIMER)
	{
		m_waterEffect.Render((DWORD *)m_renderSrc.GetDIBits(), (DWORD *)m_renderDest.GetDIBits());
		CClientDC dc(this);
		CPoint ptOrigin(BANNER_LEFT, BANNER_TOP);
		m_renderDest.Draw(&dc, ptOrigin);
	}
	if (nIDEvent == ID_DROPTIMER)
	{
		CRect r;
		r.left = BANNER_LEFT;
		r.top = BANNER_TOP;
		r.right = r.left + m_renderSrc.GetWidth();
		r.bottom = r.top + m_renderSrc.GetHeight();
		m_waterEffect.Blob(random(r.left, r.right), random(r.top, r.bottom), 5, 800, m_waterEffect.m_iHpage);
	}

	CStandAloneDialog::OnTimer(nIDEvent);
}

void CStartDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect r;
	r.left = BANNER_LEFT;
	r.top = BANNER_TOP;
	r.right = r.left + m_renderSrc.GetWidth();
	r.bottom = r.top + m_renderSrc.GetHeight();

	if (r.PtInRect(point))
	{
		// dibs are drawn upside down...
		point.y -= BANNER_TOP;
		point.y = BANNER_HEIGHT - point.y;

		if (nFlags & MK_LBUTTON)
			m_waterEffect.Blob(point.x - BANNER_LEFT, point.y, 10, 1600, m_waterEffect.m_iHpage);
		else
			m_waterEffect.Blob(point.x - BANNER_LEFT, point.y, 5, 50, m_waterEffect.m_iHpage);
	}

	CStandAloneDialog::OnMouseMove(nFlags, point);
}
