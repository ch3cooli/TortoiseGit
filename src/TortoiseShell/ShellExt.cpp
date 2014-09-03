// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2011-2014 - TortoiseGit
// Copyright (C) 2003-2013 - TortoiseSVN

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

// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
#include <initguid.h>
#include "Guids.h"

#include "ShellExt.h"
#include "ShellObjects.h"
#include "LangDll.h"
#include "..\version.h"
#undef swprintf

extern ShellObjects g_shellObjects;

// *********************** CShellExt *************************
CShellExt::CShellExt(FileState state)
	: m_State(state)
	, itemStates(0)
	, itemStatesFolder(0)
	, space(0)
#if ENABLE_CRASHHANLDER
	, m_crasher(L"TortoiseGit", TGIT_VERMAJOR, TGIT_VERMINOR, TGIT_VERMICRO, TGIT_VERBUILD, TGIT_VERDATE, false)
#endif
	,regDiffLater(L"Software\\TortoiseGit\\DiffLater", L"")
{
	m_cRef = 0L;
	InterlockedIncrement(&g_cRefThisDll);

	{
		AutoLocker lock(g_csGlobalCOMGuard);
		g_shellObjects.Insert(this);
	}

	INITCOMMONCONTROLSEX used = {
		sizeof(INITCOMMONCONTROLSEX),
			ICC_LISTVIEW_CLASSES | ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES
	};
	InitCommonControlsEx(&used);
	LoadLangDll();
}

CShellExt::~CShellExt()
{
	AutoLocker lock(g_csGlobalCOMGuard);
	InterlockedDecrement(&g_cRefThisDll);
	g_shellObjects.Erase(this);
}

void LoadLangDll()
{
	if ((g_langid != g_ShellCache.GetLangID())&&((g_langTimeout == 0)||(g_langTimeout < GetTickCount())))
	{
		g_langid = g_ShellCache.GetLangID();
		DWORD langId = g_langid;
		CLangDll langDLL;
		HINSTANCE hInst = langDLL.Init(_T("TortoiseProc"), langId, g_hmodThisDll);
		if (hInst != NULL)
		{
			g_hResInst = hInst;
			g_langTimeout = 0;
		}
		else
		{
			if (g_hResInst != g_hmodThisDll)
				FreeLibrary(g_hResInst);
			g_hResInst = g_hmodThisDll;
			g_langid = 1033;
			// set a timeout of 10 seconds
			if (g_ShellCache.GetLangID() != 1033)
				g_langTimeout = GetTickCount() + 10000;
		}
	} // if (g_langid != g_ShellCache.GetLangID())
}

STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	if(ppv == 0)
		return E_POINTER;

	*ppv = NULL;

	if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
	{
		*ppv = static_cast<LPSHELLEXTINIT>(this);
	}
	else if (IsEqualIID(riid, IID_IContextMenu))
	{
		*ppv = static_cast<LPCONTEXTMENU>(this);
	}
	else if (IsEqualIID(riid, IID_IContextMenu2))
	{
		*ppv = static_cast<LPCONTEXTMENU2>(this);
	}
	else if (IsEqualIID(riid, IID_IContextMenu3))
	{
		*ppv = static_cast<LPCONTEXTMENU3>(this);
	}
	else if (IsEqualIID(riid, IID_IShellIconOverlayIdentifier))
	{
		*ppv = static_cast<IShellIconOverlayIdentifier*>(this);
	}
	else if (IsEqualIID(riid, IID_IShellPropSheetExt))
	{
		*ppv = static_cast<LPSHELLPROPSHEETEXT>(this);
	}
	else if (IsEqualIID(riid, IID_IShellCopyHook))
	{
		*ppv = static_cast<ICopyHook*>(this);
	}
	else
	{
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef()
{
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release()
{
	if (--m_cRef)
		return m_cRef;

	delete this;

	return 0L;
}

// IPersistFile members
STDMETHODIMP CShellExt::GetClassID(CLSID *pclsid)
{
	if(pclsid == 0)
		return E_POINTER;
	*pclsid = CLSID_Tortoisegit_UNCONTROLLED;
	return S_OK;
}

STDMETHODIMP CShellExt::Load(LPCOLESTR /*pszFileName*/, DWORD /*dwMode*/)
{
	return S_OK;
}

// ICopyHook member
UINT __stdcall CShellExt::CopyCallback(HWND hWnd, UINT wFunc, UINT wFlags, LPCTSTR pszSrcFile, DWORD dwSrcAttribs, LPCTSTR pszDestFile, DWORD dwDestAttribs)
{
	__try
	{
		return CopyCallback_Wrap(hWnd, wFunc, wFlags, pszSrcFile, dwSrcAttribs, pszDestFile, dwDestAttribs);
	}
	__except(CCrashReport::Instance().SendReport(GetExceptionInformation()))
	{
	}
	return IDYES;
}

UINT __stdcall CShellExt::CopyCallback_Wrap(HWND /*hWnd*/, UINT wFunc, UINT /*wFlags*/, LPCTSTR pszSrcFile, DWORD /*dwSrcAttribs*/, LPCTSTR /*pszDestFile*/, DWORD /*dwDestAttribs*/)
{
	switch (wFunc)
	{
	case FO_MOVE:
	case FO_DELETE:
	case FO_RENAME:
		if (pszSrcFile && pszSrcFile[0])
		{
			CString topDir;
			if (g_GitAdminDir.HasAdminDir(pszSrcFile, &topDir))
				m_CachedStatus.m_GitStatus.ReleasePath(topDir);
			m_remoteCacheLink.ReleaseLockForPath(CTGitPath(pszSrcFile));
		}
		break;
	default:
		break;
	}

	// we could now wait a little bit to give the cache time to release the handles.
	// but the explorer/shell already retries any action for about two seconds
	// if it first fails. So if the cache hasn't released the handle yet, the explorer
	// will retry anyway, so we just leave here immediately.
	return IDYES;
}
