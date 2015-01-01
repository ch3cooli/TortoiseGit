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
#pragma once

#include "stdafx.h"
#include "BugTraqClr.h"
#include "IBugTraqProviderClr.h"

extern "C" __declspec(dllexport) CBugTraqWrapper* CreateInstance()
{
	return new CBugTraqClr();
}

extern "C" __declspec(dllexport) void DeleteInstance(CBugTraqWrapper* ptr)
{
	delete ptr;
}

CBugTraqClr::CBugTraqClr()
	: ptr(malloc(1))
{
	CBugTraqProviderClr::CreateInstance(ptr);
}

CBugTraqClr::~CBugTraqClr()
{
	CBugTraqProviderClr::DeleteInstance(ptr);
	free(ptr);
}

bool CBugTraqClr::CreateProvider(const CBugTraqAssociation& assoc)
{
	return CBugTraqProviderClr::GetInstance(ptr)->CreateProvider(assoc.GetProviderDll(), assoc.GetParameters());
}

bool CBugTraqClr::HasIBugTraqProvider()
{
	return CBugTraqProviderClr::GetInstance(ptr)->HasIBugTraqProvider();
}

bool CBugTraqClr::HasIBugTraqProvider2()
{
	return CBugTraqProviderClr::GetInstance(ptr)->HasIBugTraqProvider2();
}

CString CBugTraqClr::GetLastErr()
{
	return CBugTraqProviderClr::GetInstance(ptr)->GetLastErr();
}

bool CBugTraqClr::ValidateParameters(HWND hParentWnd, bool& valid)
{
	return CBugTraqProviderClr::GetInstance(ptr)->ValidateParameters(hParentWnd, valid);
}

bool CBugTraqClr::GetLinkText(HWND hParentWnd, CString& linkText)
{
	return CBugTraqProviderClr::GetInstance(ptr)->GetLinkText(hParentWnd, linkText);
}

bool CBugTraqClr::GetCommitMessage(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString& newMessage)
{
	return CBugTraqProviderClr::GetInstance(ptr)->GetCommitMessage(hParentWnd, commonRoot, pathList, originalMessage, newMessage);
}

bool CBugTraqClr::GetCommitMessage2(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString bugID,
    CString& bugIDOut, STRING_VECTOR& revPropNames, STRING_VECTOR& revPropValues, CString& newMessage)
{
	return CBugTraqProviderClr::GetInstance(ptr)->GetCommitMessage2(hParentWnd, commonURL, commonRoot, pathList, originalMessage, bugID,
		bugIDOut, revPropNames, revPropValues, newMessage);
}

bool CBugTraqClr::CheckCommit(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString commitMessage, CString& errorMessage)
{
	return CBugTraqProviderClr::GetInstance(ptr)->CheckCommit(hParentWnd, commonURL, commonRoot, pathList, commitMessage, errorMessage);
}

bool CBugTraqClr::OnCommitFinished(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString logMessage, ULONG revision, CString& error)
{
	return CBugTraqProviderClr::GetInstance(ptr)->OnCommitFinished(hParentWnd, commonRoot, pathList, logMessage, revision, error);
}

bool CBugTraqClr::HasOptions(bool& ret)
{
	return CBugTraqProviderClr::GetInstance(ptr)->HasOptions(ret);
}

bool CBugTraqClr::ShowOptionsDialog(HWND hParentWnd, CString& newparameters)
{
	return CBugTraqProviderClr::GetInstance(ptr)->ShowOptionsDialog(hParentWnd, newparameters);
}
