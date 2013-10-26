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
#include "stdafx.h"
#include "ResetCommand.h"

#include "ResetDlg.h"
#include "ProgressDlg.h"
#include "MessageBox.h"
#include "AppUtils.h"

bool ResetCommand::Execute()
{
	CString rev = parser.GetVal(_T("rev"));
	int type = 1;
	if (parser.HasKey(_T("type")))
	{
		CString sType = parser.GetVal(_T("type"));
		if (sType == _T("soft"))
			type = 0;
		else if (sType == _T("mixed"))
			type = 1;
		else if (sType == _T("hard"))
			type = 2;
	}

	bool autoClose = false;
	if (parser.HasVal(_T("closeonend")))
		autoClose = !!parser.GetLongVal(_T("closeonend"));

	return CAppUtils::GitReset(&rev, type, autoClose);
}
