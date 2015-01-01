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

#include "BugTraqWrapper.h"

class CBugTraqCom : public CBugTraqWrapper
{
public:
	CBugTraqCom();
	virtual ~CBugTraqCom();

	bool CreateProvider(const CBugTraqAssociation& assoc);

	bool HasIBugTraqProvider();
	bool HasIBugTraqProvider2();

	CString GetLastErr();

	// IBugTraqProvider methods
	bool ValidateParameters(HWND hParentWnd, bool& valid);
	bool GetLinkText(HWND hParentWnd, CString& linkText);
	bool GetCommitMessage(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString& newMessage);

	// IBugTraqProvider2 methods
	bool GetCommitMessage2(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString bugID,
		CString& bugIDOut, STRING_VECTOR& revPropNames, STRING_VECTOR& revPropValues, CString& newMessage);
	bool CheckCommit(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString commitMessage, CString& errorMessage);
	bool OnCommitFinished(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString logMessage, ULONG revision, CString& error);
	bool HasOptions(bool& ret);
	bool ShowOptionsDialog(HWND hParentWnd, CString& newparameters);

private:
	struct Internal;
	std::shared_ptr<Internal> m;
};
