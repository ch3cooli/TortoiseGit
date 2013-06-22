// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2003-2008 - TortoiseSVN
// Copyright (C) 2008-2013 - TortoiseGit

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
#include "ExportDlg.h"

#include "Messagebox.h"
#include "PathUtils.h"
#include "BrowseFolder.h"
#include "AppUtils.h"


IMPLEMENT_DYNAMIC(CExportDlg, CHorizontalResizableStandAloneDialog)
CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CHorizontalResizableStandAloneDialog(CExportDlg::IDD, pParent)
	, CChooseVersion(this)
	, m_Revision(_T("HEAD"))
	, m_strFile(_T(""))
	, m_bUseCommitTime(FALSE)
{
}

CExportDlg::~CExportDlg()
{
}

void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CHorizontalResizableStandAloneDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EXPORTFILE, m_strFile);
	DDX_Check(pDX, IDC_CHECK_USECOMMITTIME, m_bUseCommitTime);
	CHOOSE_VERSION_DDX;
}


BEGIN_MESSAGE_MAP(CExportDlg, CHorizontalResizableStandAloneDialog)
	ON_BN_CLICKED(IDC_EXPORTFILE_BROWSE, OnBnClickedCheckoutdirectoryBrowse)
	ON_EN_CHANGE(IDC_EXPORTFILE, OnEnChangeCheckoutdirectory)
	ON_BN_CLICKED(IDHELP, OnBnClickedHelp)

	CHOOSE_VERSION_EVENT
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CExportDlg::OnInitDialog()
{
	CHorizontalResizableStandAloneDialog::OnInitDialog();
	CAppUtils::MarkWindowAsUnpinnable(m_hWnd);

	AddAnchor(IDC_REPOGROUP, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_EXPORTFILE_LABEL, TOP_LEFT);
	AddAnchor(IDC_EXPORTFILE_BROWSE, TOP_RIGHT);
	AddAnchor(IDC_EXPORTFILE, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_CHECK_USECOMMITTIME, TOP_LEFT);

	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDHELP, BOTTOM_RIGHT);

	AdjustControlSize(IDC_CHECK_USECOMMITTIME);
	AdjustControlSize(IDC_RADIO_HEAD);
	AdjustControlSize(IDC_RADIO_BRANCH);
	AdjustControlSize(IDC_RADIO_TAGS);
	AdjustControlSize(IDC_RADIO_VERSION);

	CString sWindowTitle;
	GetWindowText(sWindowTitle);
	CAppUtils::SetWindowTitle(m_hWnd, g_Git.m_CurrentDir, sWindowTitle);

	CHOOSE_VERSION_ADDANCHOR;
	this->AddOthersToAnchor();
	Init();
	if(this->m_Revision.IsEmpty())
	{
		SetDefaultChoose(IDC_RADIO_HEAD);
	}
	else
	{
		SetDefaultChoose(IDC_RADIO_VERSION);
		this->GetDlgItem(IDC_COMBOBOXEX_VERSION)->SetWindowTextW(m_Revision);
	}

	m_tooltips.Create(this);
	m_tooltips.AddTool(IDC_EXPORTFILE, IDS_EXPORTFILE_TT);

	SHAutoComplete(GetDlgItem(IDC_EXPORTFILE)->m_hWnd, SHACF_FILESYSTEM);

	if ((m_pParentWnd==NULL)&&(hWndExplorer))
		CenterWindow(CWnd::FromHandle(hWndExplorer));
	EnableSaveRestore(_T("ExportDlg"));
	return TRUE;
}

void CExportDlg::OnOK()
{
	if (!UpdateData(TRUE))
		return; // don't dismiss dialog (error message already shown by MFC framework)

	// check it the export path is a valid windows path
	UpdateRevsionName();

	if (m_VersionName.IsEmpty())
	{
		m_tooltips.ShowBalloon(IDC_COMBOBOXEX_VERSION, IDS_ERR_INVALIDREV, IDS_ERR_ERROR, TTI_ERROR);
		return;
	}

	if(::PathFileExists(m_strFile))
	{
		if(::PathIsDirectory(m_strFile))
		{
			CMessageBox::Show(NULL, IDS_PROCEXPORTERRFOLDER, IDS_APPNAME, MB_OK | MB_ICONERROR);
			return;
		}
		CString sMessage;
		sMessage.Format(IDS_PROC_OVERWRITE_CONFIRM, m_strFile);
		if (CMessageBox::Show(NULL, sMessage, _T("TortoiseGit"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES)
		{
			return ;
		}
	}
	else if (m_strFile.IsEmpty())
	{
		CMessageBox::Show(NULL, IDS_PROC_NOZIPFILE, IDS_APPNAME, MB_OK | MB_ICONERROR);
		return;
	}

	UpdateData(FALSE);
	CHorizontalResizableStandAloneDialog::OnOK();
}

void CExportDlg::OnBnClickedCheckoutdirectoryBrowse()
{
	m_tooltips.Pop();	// hide the tooltips
	//
	// Create a folder browser dialog. If the user selects OK, we should update
	// the local data members with values from the controls, copy the checkout
	// directory from the browse folder, then restore the local values into the
	// dialog controls.
	//
	this->UpdateRevsionName();
	CFileDialog dlg(FALSE, _T("zip"), this->m_VersionName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("*.zip"));

	INT_PTR ret = dlg.DoModal();
	SetCurrentDirectory(g_Git.m_CurrentDir);
	if (ret == IDOK)
	{
		UpdateData(TRUE);
		m_strFile = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

BOOL CExportDlg::PreTranslateMessage(MSG* pMsg)
{
	m_tooltips.RelayEvent(pMsg);
	return CHorizontalResizableStandAloneDialog::PreTranslateMessage(pMsg);
}

void CExportDlg::OnEnChangeCheckoutdirectory()
{
	UpdateData(TRUE);
	DialogEnableWindow(IDOK, !m_strFile.IsEmpty());
}

void CExportDlg::OnBnClickedHelp()
{
	OnHelp();
}

void CExportDlg::OnBnClickedShowlog()
{
	m_tooltips.Pop();	// hide the tooltips
}

void CExportDlg::OnDestroy()
{
	WaitForFinishLoading();
	__super::OnDestroy();
}
