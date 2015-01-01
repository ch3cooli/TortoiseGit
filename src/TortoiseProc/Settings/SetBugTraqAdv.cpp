// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2014-2015 - TortoiseGit
// Copyright (C) 2008,2010,2014-2015 - TortoiseSVN

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
#include "SetBugTraqAdv.h"
#include "BrowseFolder.h"
#include "BugTraqAssociations.h"
#include "BugTraqWrapper.h"

IMPLEMENT_DYNAMIC(CSetBugTraqAdv, CResizableStandAloneDialog)

CSetBugTraqAdv::CSetBugTraqAdv(CWnd* pParent /*= NULL*/)
	: CResizableStandAloneDialog(CSetBugTraqAdv::IDD, pParent)
	, m_provider_clsid(GUID_NULL)
{
}

CSetBugTraqAdv::CSetBugTraqAdv(const CBugTraqAssociation &assoc, CWnd* pParent /*= NULL*/)
	: CResizableStandAloneDialog(CSetBugTraqAdv::IDD, pParent)
	, m_sPath(assoc.GetPath().GetWinPathString())
	, m_provider_clsid(assoc.GetProviderClass())
	, m_sParameters(assoc.GetParameters())
{
}

CSetBugTraqAdv::~CSetBugTraqAdv()
{
}

void CSetBugTraqAdv::DoDataExchange(CDataExchange* pDX)
{
	CResizableStandAloneDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_BUGTRAQPATH, m_sPath);
	DDX_Control(pDX, IDC_BUGTRAQPROVIDERCOMBO, m_cProviderCombo);
	DDX_Text(pDX, IDC_BUGTRAQPARAMETERS, m_sParameters);
}

BEGIN_MESSAGE_MAP(CSetBugTraqAdv, CResizableStandAloneDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUGTRAQBROWSE, &CSetBugTraqAdv::OnBnClickedBugTraqbrowse)
	ON_CBN_SELCHANGE(IDC_BUGTRAQPROVIDERCOMBO, &CSetBugTraqAdv::OnCbnSelchangeBugtraqprovidercombo)
	ON_BN_CLICKED(IDC_OPTIONS, &CSetBugTraqAdv::OnBnClickedOptions)
END_MESSAGE_MAP()

BOOL CSetBugTraqAdv::OnInitDialog()
{
	CResizableStandAloneDialog::OnInitDialog();

	std::vector<CBugTraqProvider> providers = CBugTraqAssociations::GetAvailableProviders();
	if (providers.empty())
	{
		AfxMessageBox(IDS_ERR_NO_AVAILABLE_BUGTRAQ_PROVIDERS);
		EndDialog(IDCANCEL);
		return TRUE;
	}

	for (std::vector<CBugTraqProvider>::const_iterator it = providers.begin(); it != providers.end(); ++it)
	{
		int index = m_cProviderCombo.AddString(it->name);
		m_cProviderCombo.SetItemDataPtr(index, new CBugTraqProvider(*it));
	}

	// preselect the right provider in the combo; we can't do this above, because the
	// combo will sort the entries.
	if (m_provider_clsid == GUID_NULL)
		m_cProviderCombo.SetCurSel(0);

	for (int i = 0; i < m_cProviderCombo.GetCount(); ++i)
	{
		CBugTraqProvider *p = (CBugTraqProvider *)m_cProviderCombo.GetItemDataPtr(i);
		if (p->clsid == m_provider_clsid)
		{
			m_cProviderCombo.SetCurSel(i);
			break;
		}
	}

	m_tooltips.Create(this);
	UpdateData(FALSE);
	CheckHasOptions();

	AddAnchor(IDC_BUGTRAQWCPATHLABEL, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_BUGTRAQPATH, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_BUGTRAQBROWSE, TOP_RIGHT);
	AddAnchor(IDC_BUGTRAQPROVIDERLABEL, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_BUGTRAQPROVIDERCOMBO, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_BUGTRAQPARAMETERSLABEL, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_BUGTRAQPARAMETERS, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_OPTIONS, TOP_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDHELP, BOTTOM_RIGHT);
	EnableSaveRestore(_T("SetBugTraqAdvDlg"));
	return TRUE;
}

void CSetBugTraqAdv::OnDestroy()
{
	for (int i = 0; i < m_cProviderCombo.GetCount(); ++i)
		delete (CBugTraqProvider *)m_cProviderCombo.GetItemDataPtr(i);

	CResizableStandAloneDialog::OnDestroy();
}

void CSetBugTraqAdv::OnOK()
{
	UpdateData();

	m_provider_clsid = GUID_NULL;

	int index = m_cProviderCombo.GetCurSel();
	if (index != CB_ERR)
	{
		CBugTraqProvider *provider = (CBugTraqProvider *)m_cProviderCombo.GetItemDataPtr(index);
		m_provider_clsid = provider->clsid;
	}

	CBugTraqAssociation assoc(_T(""), m_provider_clsid, _T(""), m_sParameters);
	auto pBugTraqWrapper = CBugTraqWrapper::CreateInstance(assoc);
	if (!pBugTraqWrapper->CreateProvider(assoc))
	{
		m_tooltips.ShowBalloon(IDC_BUGTRAQPROVIDERCOMBO, IDS_ERR_MISSING_PROVIDER, IDS_ERR_ERROR, TTI_ERROR);
		return;
	}

	bool valid = false;
	if (!pBugTraqWrapper->ValidateParameters(GetSafeHwnd(), valid))
	{
		ShowEditBalloon(IDC_BUGTRAQPARAMETERS, IDS_ERR_PROVIDER_VALIDATE_FAILED, IDS_ERR_ERROR, TTI_ERROR);
		return;
	}

	if (!valid)
		return;	// It's assumed that the provider will have done this.

	CResizableStandAloneDialog::OnOK();
}

void CSetBugTraqAdv::OnBnClickedBugTraqbrowse()
{
	CBrowseFolder browser;
	CString sPath;
	GetDlgItemText(IDC_BUGTRAQPATH, sPath);
	browser.SetInfo(CString(MAKEINTRESOURCE(IDS_SETTINGS_BUGTRAQ_SELECTFOLDERPATH)));
	browser.m_style = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	if (browser.Show(m_hWnd, sPath) == CBrowseFolder::OK)
		SetDlgItemText(IDC_BUGTRAQPATH, sPath);
}

CBugTraqAssociation CSetBugTraqAdv::GetAssociation() const
{
	return CBugTraqAssociation(m_sPath, m_provider_clsid, CBugTraqAssociations::LookupProviderName(m_provider_clsid), m_sParameters);
}

void CSetBugTraqAdv::CheckHasOptions()
{
	m_provider_clsid = GUID_NULL;

	int index = m_cProviderCombo.GetCurSel();
	if (index != CB_ERR)
	{
		CBugTraqProvider *provider = (CBugTraqProvider *)m_cProviderCombo.GetItemDataPtr(index);
		m_provider_clsid = provider->clsid;
	}

	CBugTraqAssociation assoc(_T(""), m_provider_clsid, _T(""), m_sParameters);
	auto pBugTraqWrapper = CBugTraqWrapper::CreateInstance(assoc);
	if (pBugTraqWrapper->CreateProvider(assoc) && pBugTraqWrapper->HasIBugTraqProvider2())
	{
		bool hasOptions = false;
		if (pBugTraqWrapper->HasOptions(hasOptions))
		{
			if (hasOptions)
			{
				GetDlgItem(IDC_OPTIONS)->EnableWindow(TRUE);
				return;
			}
		}
	}

	GetDlgItem(IDC_OPTIONS)->EnableWindow(FALSE);
}

void CSetBugTraqAdv::OnCbnSelchangeBugtraqprovidercombo()
{
	CheckHasOptions();
}

void CSetBugTraqAdv::OnBnClickedOptions()
{
	m_provider_clsid = GUID_NULL;

	int index = m_cProviderCombo.GetCurSel();
	if (index != CB_ERR)
	{
		CBugTraqProvider *provider = (CBugTraqProvider *)m_cProviderCombo.GetItemDataPtr(index);
		m_provider_clsid = provider->clsid;
	}

	CString p;
	GetDlgItemText(IDC_BUGTRAQPARAMETERS, p);
	CBugTraqAssociation assoc(_T(""), m_provider_clsid, _T(""), p);
	auto pBugTraqWrapper = CBugTraqWrapper::CreateInstance(assoc);
	if (pBugTraqWrapper->CreateProvider(assoc) && pBugTraqWrapper->HasIBugTraqProvider2())
	{
		CString temp;
		if (pBugTraqWrapper->ShowOptionsDialog(GetSafeHwnd(), temp))
		{
			SetDlgItemText(IDC_BUGTRAQPARAMETERS, temp);
		}
	}
}

BOOL CSetBugTraqAdv::PreTranslateMessage(MSG* pMsg)
{
	m_tooltips.RelayEvent(pMsg);
	return CResizableStandAloneDialog::PreTranslateMessage(pMsg);
}
