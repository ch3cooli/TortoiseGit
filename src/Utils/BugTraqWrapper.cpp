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
#include "BugTraqWrapper.h"
#include "BugTraqCom.h"

typedef CBugTraqWrapper* (*FnCreateInstance)();

static CBugTraqWrapper* CreateInstanceClr()
{
	HMODULE hModule = ::LoadLibrary(_T("IBugTraqProviderClr.dll"));
	if (!hModule)
		return nullptr;
	auto ptr = (FnCreateInstance)::GetProcAddress(hModule, "CreateInstance");
	return ptr();
}

CBugTraqWrapper* CBugTraqWrapper::CreateInstance(const CBugTraqAssociation& assoc)
{
	if (!assoc.GetProviderDll().IsEmpty())
		return CreateInstanceClr();
	return new CBugTraqCom();
}