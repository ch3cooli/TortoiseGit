// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2015 - TortoiseGit

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
#include "BugTraqCom.h"
#include "..\IBugTraqProvider\IBugTraqProvider_h.h"
#include "BstrSafeVector.h"
#include "COMError.h"

struct CBugTraqCom::Internal
{
	CBugTraqAssociation m_BugTraqAssoc;
	CComPtr<IBugTraqProvider> m_BugTraqProvider;
	CComPtr<IBugTraqProvider2> m_BugTraqProvider2;
	IBugTraqProvider* GetBugTraqProvider() { return m_BugTraqProvider2 ? (IBugTraqProvider*)m_BugTraqProvider2 : (IBugTraqProvider*)m_BugTraqProvider; }

	CString m_LastError;
	void ClearError() { m_LastError.Empty(); }
	bool SetError(HRESULT hr) { m_LastError = COMError(hr).GetMessageAndDescription().c_str(); return false; }
	bool NoBugTraqProvider() { m_LastError = "No IBugTraqProvider interface"; return false; }
	bool NoBugTraqProvider2() { m_LastError = "No IBugTraqProvider2 interface"; return false; }
};

CBugTraqCom::CBugTraqCom()
	: m(new Internal())
{
}

CBugTraqCom::~CBugTraqCom()
{
}

bool CBugTraqCom::CreateProvider(const CBugTraqAssociation& assoc)
{
	m->ClearError();

	CComPtr<IBugTraqProvider2> pProvider2;
	HRESULT hr = pProvider2.CoCreateInstance(assoc.GetProviderClass());
	if (SUCCEEDED(hr))
	{
		m->m_BugTraqAssoc = assoc;
		m->m_BugTraqProvider2 = pProvider2;
		return true;
	}

	CComPtr<IBugTraqProvider> pProvider;
	hr = pProvider.CoCreateInstance(assoc.GetProviderClass());
	if (SUCCEEDED(hr))
	{
		m->m_BugTraqAssoc = assoc;
		m->m_BugTraqProvider = pProvider;
		return true;
	}

	return m->SetError(hr);
}

bool CBugTraqCom::HasIBugTraqProvider()
{
	if (HasIBugTraqProvider2())
		return true;
	return m->m_BugTraqProvider != nullptr;
}

bool CBugTraqCom::HasIBugTraqProvider2()
{
	return m->m_BugTraqProvider2 != nullptr;
}

CString CBugTraqCom::GetLastErr()
{
	return m->m_LastError;
}

bool CBugTraqCom::ValidateParameters(HWND hParentWnd, bool& valid)
{
	m->ClearError();
	if (!HasIBugTraqProvider())
		return m->NoBugTraqProvider();

	ATL::CComBSTR parameters(m->m_BugTraqAssoc.GetParameters());
	VARIANT_BOOL svalid = VARIANT_FALSE;

	HRESULT hr = m->GetBugTraqProvider()->ValidateParameters(hParentWnd, parameters, &svalid);
	if (SUCCEEDED(hr))
	{
		valid = !!svalid;
		return true;
	}

	return m->SetError(hr);
}

bool CBugTraqCom::GetLinkText(HWND hParentWnd, CString& linkText)
{
	m->ClearError();
	if (!HasIBugTraqProvider())
		return m->NoBugTraqProvider();

	ATL::CComBSTR temp;
	ATL::CComBSTR parameters(m->m_BugTraqAssoc.GetParameters());

	HRESULT hr = m->GetBugTraqProvider()->GetLinkText(hParentWnd, parameters, &temp);
	if (SUCCEEDED(hr))
	{
		linkText = temp;
		return true;
	}

	return m->SetError(hr);
}

bool CBugTraqCom::GetCommitMessage(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString& newMessage)
{
	m->ClearError();
	if (!HasIBugTraqProvider())
		return m->NoBugTraqProvider();

	ATL::CComBSTR temp;
	ATL::CComBSTR parameters(m->m_BugTraqAssoc.GetParameters());
	ATL::CComBSTR sCommonRoot(commonRoot);
	ATL::CComBSTR sOriginalMessage(originalMessage);
	CBstrSafeVector sPathList((ULONG)pathList.size());
	for (LONG index = 0; index < pathList.size(); ++index)
		sPathList.PutElement(index, pathList[index]);

	HRESULT hr = m->GetBugTraqProvider()->GetCommitMessage(hParentWnd, parameters, sCommonRoot, sPathList, sOriginalMessage, &temp);
	if (SUCCEEDED(hr))
	{
		newMessage = temp;
		return true;
	}

	return m->SetError(hr);
}

bool CBugTraqCom::GetCommitMessage2(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString bugID,
	CString& bugIDOut, STRING_VECTOR& revPropNames, STRING_VECTOR& revPropValues, CString& newMessage)
{
	m->ClearError();
	if (!HasIBugTraqProvider2())
		return m->NoBugTraqProvider2();

	ATL::CComBSTR temp;
	ATL::CComBSTR sRepositoryRoot(commonURL);
	ATL::CComBSTR parameters(m->m_BugTraqAssoc.GetParameters());
	ATL::CComBSTR sCommonRoot(commonRoot);
	ATL::CComBSTR sOriginalMessage(originalMessage);
	ATL::CComBSTR sBugID(bugID);
	CBstrSafeVector sPathList((ULONG)pathList.size());
	ATL::CComBSTR sBugIDOut;
	CBstrSafeVector sRevPropNames;
	CBstrSafeVector sRevPropValues;
	for (LONG index = 0; index < pathList.size(); ++index)
		sPathList.PutElement(index, pathList[index]);

	HRESULT hr = m->m_BugTraqProvider2->GetCommitMessage2(hParentWnd, parameters, sRepositoryRoot, sCommonRoot, sPathList, sOriginalMessage, sBugID,
		&sBugIDOut, &sRevPropNames, &sRevPropValues, &temp);
	if (SUCCEEDED(hr))
	{
		newMessage = temp;
		return true;
	}

	return m->SetError(hr);
}

bool CBugTraqCom::CheckCommit(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString commitMessage, CString& errorMessage)
{
	m->ClearError();
	if (!HasIBugTraqProvider2())
		return m->NoBugTraqProvider2();

	ATL::CComBSTR temp;
	ATL::CComBSTR sRepositoryRoot(commonURL);
	ATL::CComBSTR parameters(m->m_BugTraqAssoc.GetParameters());
	ATL::CComBSTR sCommonRoot(commonRoot);
	ATL::CComBSTR sCommitMessage(commitMessage);
	CBstrSafeVector sPathList((ULONG)pathList.size());
	for (LONG index = 0; index < pathList.size(); ++index)
		sPathList.PutElement(index, pathList[index]);

	HRESULT hr = m->m_BugTraqProvider2->CheckCommit(hParentWnd, parameters, sRepositoryRoot, sCommonRoot, sPathList, sCommitMessage, &temp);
	if (SUCCEEDED(hr))
	{
		errorMessage = temp;
		return true;
	}

	return m->SetError(hr);
}

bool CBugTraqCom::OnCommitFinished(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString logMessage, ULONG revision, CString& error)
{
	m->ClearError();
	if (!HasIBugTraqProvider2())
		return m->NoBugTraqProvider2();

	ATL::CComBSTR temp;
	ATL::CComBSTR sCommonRoot(commonRoot);
	CBstrSafeVector sPathList((ULONG)pathList.size());
	for (LONG index = 0; index < pathList.size(); ++index)
		sPathList.PutElement(index, pathList[index]);
	ATL::CComBSTR sLogMessage(logMessage);

	HRESULT hr = m->m_BugTraqProvider2->OnCommitFinished(hParentWnd, sCommonRoot, sPathList, sLogMessage, revision, &temp);
	if (SUCCEEDED(hr))
	{
		error = temp;
		return true;
	}

	return m->SetError(hr);
}

bool CBugTraqCom::HasOptions(bool& ret)
{
	m->ClearError();
	if (!HasIBugTraqProvider2())
		return m->NoBugTraqProvider2();

	VARIANT_BOOL sRet;

	HRESULT hr = m->m_BugTraqProvider2->HasOptions(&sRet);
	if (SUCCEEDED(hr))
	{
		ret = !!sRet;
		return true;
	}

	return m->SetError(hr);
}

bool CBugTraqCom::ShowOptionsDialog(HWND hParentWnd, CString& newparameters)
{
	m->ClearError();
	if (!HasIBugTraqProvider2())
		return m->NoBugTraqProvider2();

	ATL::CComBSTR sNewparameters;
	ATL::CComBSTR parameters(m->m_BugTraqAssoc.GetParameters());

	HRESULT hr = m->m_BugTraqProvider2->ShowOptionsDialog(hParentWnd, parameters, &sNewparameters);
	if (SUCCEEDED(hr))
	{
		newparameters = sNewparameters;
		m->m_BugTraqAssoc = CBugTraqAssociation(m->m_BugTraqAssoc.GetPath().GetWinPath(), m->m_BugTraqAssoc.GetProviderClass(), m->m_BugTraqAssoc.GetProviderName(), newparameters);
		return true;
	}

	return m->SetError(hr);
}