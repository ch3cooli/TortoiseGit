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

#include "BugTraqAssociations.h"

class CBugTraqWrapper
{
public:
	static CBugTraqWrapper* CreateInstance(const CBugTraqAssociation& assoc);

	virtual bool CreateProvider(const CBugTraqAssociation& assoc) = 0;

	virtual bool HasIBugTraqProvider() = 0;
	virtual bool HasIBugTraqProvider2() = 0;

	virtual CString GetLastErr() = 0;

	// IBugTraqProvider methods
	virtual bool ValidateParameters(HWND hParentWnd, bool& valid) = 0;
	virtual bool GetLinkText(HWND hParentWnd, CString& linkText) = 0;
	virtual bool GetCommitMessage(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString& newMessage) = 0;
	
	// IBugTraqProvider2 methods
	virtual bool GetCommitMessage2(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString bugID,
        CString& bugIDOut, STRING_VECTOR& revPropNames, STRING_VECTOR& revPropValues, CString& newMessage) = 0;
	virtual bool CheckCommit(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString commitMessage, CString& errorMessage) = 0;
	virtual bool OnCommitFinished(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString logMessage, ULONG revision, CString& error) = 0;
	virtual bool HasOptions(bool& ret) = 0;
	virtual bool ShowOptionsDialog(HWND hParentWnd, CString& newparameters) = 0;
};
