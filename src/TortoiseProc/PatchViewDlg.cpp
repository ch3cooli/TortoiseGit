// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2008-2014 - TortoiseGit

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
// PatchViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TortoiseProc.h"
#include "PatchViewDlg.h"
#include "registry.h"
#include "UnicodeUtils.h"
// CPatchViewDlg dialog

IMPLEMENT_DYNAMIC(CPatchViewDlg, CDialog)

CPatchViewDlg::CPatchViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPatchViewDlg::IDD, pParent)
	, m_ParentDlg(nullptr)
	, m_pProjectProperties(nullptr)
	, m_bTransparentLoseFocus(false)
{

}

CPatchViewDlg::~CPatchViewDlg()
{
}

void CPatchViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PATCH, m_ctrlPatchView);
	DDX_Check(pDX, IDC_TRANSPARENTLOSTFOCUS, m_bTransparentLoseFocus);
	DDX_Control(pDX, IDC_SLIDER_ALPHA, m_SliderAlpha);
}

BEGIN_MESSAGE_MAP(CPatchViewDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_MOVING()
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_TRANSPARENTLOSTFOCUS, OnBnClickedTransparentLostFocus)
END_MESSAGE_MAP()

// CPatchViewDlg message handlers

BOOL CPatchViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ctrlPatchView.Init(*m_pProjectProperties);
	m_ctrlPatchView.SetFont((CString)CRegString(_T("Software\\TortoiseGit\\LogFontName"), _T("Courier New")), (DWORD)CRegDWORD(_T("Software\\TortoiseGit\\LogFontSize"), 8));

	m_ctrlPatchView.SetUDiffStyle();
	m_SliderAlpha.SetRange(1, 255, TRUE);
	m_SliderAlpha.SetPos(255);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPatchViewDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (void *)&m_SliderAlpha)
	{
		switch (nSBCode)
		{
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			{
				ApplyTransparent();
				break;
			}
		}
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPatchViewDlg::SetText(const CString& text)
{
	m_ctrlPatchView.Call(SCI_SETREADONLY, FALSE);
	m_ctrlPatchView.SetText(text);
	m_ctrlPatchView.Call(SCI_SETREADONLY, TRUE);
	if (!text.IsEmpty())
	{
		m_ctrlPatchView.Call(SCI_GOTOPOS, 0);
		CRect rect;
		m_ctrlPatchView.GetClientRect(rect);
		m_ctrlPatchView.Call(SCI_SETSCROLLWIDTH, rect.Width() - 4);
	}
}

void CPatchViewDlg::ClearView()
{
	SetText(CString());
}

void CPatchViewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (this->IsWindowVisible())
	{
		CRect rect;
		GetClientRect(rect);
		CRect rect2;
		m_ctrlPatchView.GetWindowRect(rect2);
		ScreenToClient(rect2);
		GetDlgItem(IDC_PATCH)->MoveWindow(rect2.left, rect2.top, cx - rect2.left, cy - rect2.top);
		m_ctrlPatchView.GetClientRect(rect);
		m_ctrlPatchView.Call(SCI_SETSCROLLWIDTH, rect.Width() - 4);
	}
}

void CPatchViewDlg::OnMoving(UINT fwSide, LPRECT pRect)
{
#define STICKYSIZE 5
	RECT parentRect;
	m_ParentDlg->GetPatchViewParentWnd()->GetWindowRect(&parentRect);
	if (abs(parentRect.right - pRect->left) < STICKYSIZE)
	{
		int width = pRect->right - pRect->left;
		pRect->left = parentRect.right;
		pRect->right = pRect->left + width;
	}
	CDialog::OnMoving(fwSide, pRect);
}

void CPatchViewDlg::OnClose()
{
	CDialog::OnClose();
	m_ParentDlg->TogglePatchView();
}

void CPatchViewDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	if (nState == WA_INACTIVE)
		ApplyTransparent(false);
	else
		ApplyTransparent();
}

void CPatchViewDlg::OnBnClickedTransparentLostFocus()
{
	UpdateData();
	ApplyTransparent();
}

void CPatchViewDlg::ApplyTransparent(bool active)
{
	BYTE curpos = (BYTE)m_SliderAlpha.GetPos();
	if ((m_bTransparentLoseFocus && !active || !m_bTransparentLoseFocus) && curpos < 255)
	{
		SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, ::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) | 0x00080000);
		SetLayeredWindowAttributes(0, curpos, 0x00000002);
	}
	else
	{
		SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, ::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) & ~0x00080000);
	}
}
