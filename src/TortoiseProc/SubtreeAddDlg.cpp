// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2014 - TortoiseGit

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
// SubtreeAddDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SubtreeAddDlg.h"

#include "TortoiseProc.h"
#include "AppUtils.h"
#include "BrowseFolder.h"
#include "MessageBox.h"

IMPLEMENT_DYNAMIC(CSubtreeAddDlg, CDialog);

CSubtreeAddDlg::CSubtreeAddDlg(const CString &subFolder, CWnd* pParent /*=NULL*/)
 : CResizableStandAloneDialog(CSubtreeAddDlg::IDD, pParent)
	, m_strPath(subFolder)
{
	m_pDefaultText = MAKEINTRESOURCE(IDS_PROC_AUTOGENERATEDBYGIT);
	m_bAutoloadPuttyKeyFile = CAppUtils::IsSSHPutty();
	m_bSquash = TRUE;
}

CSubtreeAddDlg::~CSubtreeAddDlg()
{
}

void CSubtreeAddDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REMOTE_COMBO, m_Remote);
	DDX_Control(pDX, IDC_REMOTE_BRANCH, m_RemoteBranch);
	DDX_Control(pDX, IDC_LOCAL_PATH, m_PathCtrl);
	DDX_Control(pDX, IDC_LOGMESSAGE, m_cLogMessage);
	DDX_Control(pDX, IDC_PUTTYKEYFILE, m_PuttyKeyCombo);
	DDX_Check(pDX, IDC_PUTTYKEY_AUTOLOAD, m_bAutoloadPuttyKeyFile);
	DDX_Check(pDX, IDC_CHECK_SQUASH, m_bSquash);
}

BEGIN_MESSAGE_MAP(CSubtreeAddDlg, CResizableStandAloneDialog)
	ON_CBN_SELCHANGE(IDC_REMOTE_COMBO, OnCbnSelchangeRemote)
	ON_COMMAND(IDC_LOCAL_PATH_BROWSE, OnPathBrowse)
	ON_BN_CLICKED(IDC_PUTTYKEY_AUTOLOAD, OnBnClickedPuttykeyAutoload)
	ON_BN_CLICKED(IDC_PUTTYKEYFILE_BROWSE, OnBnClickedPuttykeyfileBrowse)
END_MESSAGE_MAP()

BOOL CSubtreeAddDlg::OnInitDialog()
{
	__super::OnInitDialog();
	CAppUtils::MarkWindowAsUnpinnable(m_hWnd);

	// source group (top, horizontal stretch)
	AddAnchor(IDC_STATIC_SOURCE, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_REMOTE_COMBO, TOP_LEFT, TOP_RIGHT);		// repo dropdown
	AddAnchor(IDC_REMOTE_BRANCH, TOP_LEFT, TOP_RIGHT);		// branch dropdown
	AddAnchor(IDC_REP_BROWSE, TOP_RIGHT);					// ... browse button (align top right)

	// dest group (top, horizontal stretch)
	AddAnchor(IDC_STATIC_DEST, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_LOCAL_PATH, TOP_LEFT, TOP_RIGHT);			// dropdown
	AddAnchor(IDC_LOCAL_PATH_BROWSE, TOP_RIGHT);			// ... browse button (align top right)

	// message box (middle stretch both, down and right)
	AddAnchor(IDC_LOGMESSAGE, TOP_LEFT, BOTTOM_RIGHT);

	// putty key checkbox, combo, and browse buttons (bottom, stretch horizontal)
	AddAnchor(IDC_PUTTYKEY_AUTOLOAD, BOTTOM_LEFT);			// (bottom left)
	AddAnchor(IDC_PUTTYKEYFILE, BOTTOM_LEFT, BOTTOM_RIGHT);	// (bottom stretch)
	AddAnchor(IDC_PUTTYKEYFILE_BROWSE, BOTTOM_RIGHT);		// (bottom right)

	// ok, cancel, help (bottom right)
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDHELP, BOTTOM_RIGHT);

	// anything leftover will be top left anchored
	AddOthersToAnchor();

	// can we even use the checkbox?
	GetDlgItem(IDC_PUTTYKEY_AUTOLOAD)->EnableWindow(CAppUtils::IsSSHPutty());
	OnBnClickedPuttykeyAutoload();	// initialize with the checkbox state using it's direct callback

	//////////////////////////////////////////////////////////////////////////
	// Set up the remote branch drop down + branch

	// TODO: stash the pull source
	// maybe in the registry or .git/config, or maybe make a new .gitsubtrees
	//		This seems best as a contrib to subtree and not to tortoisegit though

	// For the init, we'll default to, arbitrary url:
	// A) clipboard -- like clone
	// B) history init source -- like clone / submodule
	CheckRadioButton(IDC_REMOTE_RD,IDC_OTHER_RD,IDC_REMOTE_RD);

	m_Remote.SetCaseSensitive(TRUE);
	m_Remote.SetURLHistory(TRUE);
	m_Remote.LoadHistory(_T("Software\\TortoiseGit\\History\\repoURLS"), _T("url"));
	if(m_URL.IsEmpty())
	{
		CString str = CAppUtils::GetClipboardLink(_T("git clone "));
		str.Trim();
		if(str.IsEmpty())
			m_Remote.SetCurSel(0);
		else
			m_Remote.SetWindowText(str);
	}
	else
		m_Remote.SetWindowText(m_URL);

	//Select remote from current branch
	CString currentBranch = g_Git.GetCurrentBranch();
	CString configName;
	configName.Format(L"branch.%s.remote", currentBranch);
	CString remoteStr = g_Git.GetConfigValue(configName);

	STRING_VECTOR list;
	int sel=0;
	if(!g_Git.GetRemoteList(list))
	{
		for (unsigned int i = 0; i < list.size(); ++i)
		{
			m_Remote.AddString(list[i]);
			if (list[i] == remoteStr)
				sel = i;
		}
	}
	m_Remote.SetCurSel(sel);
	OnCbnSelchangeRemote();

	//Select branch from current
	configName.Format(L"branch.%s.merge", currentBranch);
	CString branchStr = CGit::StripRefName(g_Git.GetConfigValue(configName));

	STRING_VECTOR brList;
	int brsel=0;
	if(!g_Git.GetBranchList(brList, NULL, CGit::BRANCH_LOCAL))
	{
		for (unsigned int i = 0; i < brList.size(); ++i)
		{
			m_RemoteBranch.AddString(brList[i]);
			if (brList[i] == branchStr)
				brsel = i;
		}
	}
	m_RemoteBranch.SetCurSel(brsel);

	m_PathCtrl.SetPathHistory(true);
	m_PathCtrl.LoadHistory(_T("Software\\TortoiseGit\\History\\SubtreePath"), _T("url"));
	m_PathCtrl.SetWindowText(m_strPath);

	m_ProjectProperties.ReadProps();
	m_cLogMessage.Init(m_ProjectProperties);
	m_cLogMessage.SetFont((CString)CRegString(_T("Software\\TortoiseGit\\LogFontName"), _T("Courier New")), (DWORD)CRegDWORD(_T("Software\\TortoiseGit\\LogFontSize"), 8));
	m_cLogMessage.RegisterContextMenuHandler(this);
	m_cLogMessage.SetText(m_pDefaultText);

	m_PuttyKeyCombo.SetPathHistory(TRUE);
	m_PuttyKeyCombo.LoadHistory(_T("Software\\TortoiseGit\\History\\puttykey"), _T("key"));
	m_PuttyKeyCombo.SetCurSel(0);

	EnableSaveRestore(_T("SubtreeAddDlg"));
	GetDlgItem(IDOK)->SetFocus();

	return TRUE;
}

void CSubtreeAddDlg::OnBnClickedPuttykeyAutoload()
{
	UpdateData();
	GetDlgItem(IDC_PUTTYKEYFILE)->EnableWindow(m_bAutoloadPuttyKeyFile);
	GetDlgItem(IDC_PUTTYKEYFILE_BROWSE)->EnableWindow(m_bAutoloadPuttyKeyFile);
}

void CSubtreeAddDlg::OnBnClickedPuttykeyfileBrowse()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CString(MAKEINTRESOURCE(IDS_PUTTYKEYFILEFILTER)));
	if (dlg.DoModal()==IDOK)
	{
		m_PuttyKeyCombo.SetWindowText(dlg.GetPathName());
	}
}

void CSubtreeAddDlg::OnPathBrowse()
{
	CBrowseFolder browseFolder;
	browseFolder.m_style = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	browseFolder.SetRootFolder(g_Git.m_CurrentDir);

	CString strDirectory;
	if (browseFolder.Show(GetSafeHwnd(), strDirectory, g_Git.m_CurrentDir + _T("\\") + m_strPath) == CBrowseFolder::OK)
	{
		int subPathLen = g_Git.m_CurrentDir.GetLength();

		if (strDirectory.Left(subPathLen) == g_Git.m_CurrentDir)
		{
			m_strPath = strDirectory.Mid(subPathLen+1);
			m_PathCtrl.SetWindowTextW(m_strPath);
		}
	}
}

void CSubtreeAddDlg::OnCbnSelchangeRemote()
{
}

void CSubtreeAddDlg::OnOK()
{
	m_URL = m_Remote.GetString();
	m_URL.Trim();
	UpdateData(TRUE);
	if(m_URL.IsEmpty() || m_strPath.IsEmpty())
	{
		CMessageBox::Show(NULL, IDS_PROC_CLONE_URLDIREMPTY, IDS_APPNAME, MB_OK);
		return;
	}
}

// CSubtreeAddDlg message handlers
