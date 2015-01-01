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

#using <mscorlib.dll>
#include "stdafx.h"
using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

typedef std::vector<CString> STRING_VECTOR;

ref class CBugTraqProviderClr
{
public:
	static CBugTraqProviderClr^ GetInstance(void* ptr)
	{
		CBugTraqProviderClr^ obj;
		return PtrToObj->TryGetValue((IntPtr)ptr, obj) ? obj : nullptr;
	}

	static CBugTraqProviderClr^ CreateInstance(void* ptr)
	{
		return PtrToObj->AddOrUpdate((IntPtr)ptr, gcnew CBugTraqProviderClr(), gcnew Func<IntPtr,CBugTraqProviderClr^,CBugTraqProviderClr^>(AAA));
	}

	static void DeleteInstance(void* ptr)
	{
		CBugTraqProviderClr^ obj;
		PtrToObj->TryRemove((IntPtr)ptr, obj);
	}

	bool CreateProvider(CString file, CString parameters)
	{
		try
		{
			auto dll = Assembly::LoadFile(gcnew String(file));
			int level;
			auto type = FindPluginType(dll->GetTypes(), level);
			if (type == nullptr)
			{
				lastError = L"Plugin type not found";
				return false;
			}
			auto obj = Activator::CreateInstance(type);
			this->provider = obj;
			this->level = level;
			this->parameters = gcnew String(parameters);
			return true;
		}
		catch (Exception^ ex)
		{
			lastError = ex->Message;
			return false;
		}
	}

	bool HasIBugTraqProvider()
	{
		return provider != nullptr && level >= 1;
	}

	bool HasIBugTraqProvider2()
	{
		return provider != nullptr && level >= 2;
	}

	CString GetLastErr()
	{
		return lastError;
	}

	// IBugTraqProvider methods
	bool ValidateParameters(HWND hParentWnd, bool& valid)
	{
		auto types = gcnew array<Type^> { IntPtr::typeid, String::typeid };
		auto values = gcnew array<Object^> { (IntPtr)hParentWnd, parameters };
		Object^ result;
		if (CallPluginMethod(result, this->provider, L"ValidateParameters", types, values))
		{
			valid = (bool)result;
			return true;
		}
		return false;
	}

	bool GetLinkText(HWND hParentWnd, CString& linkText)
	{
		auto types = gcnew array<Type^> { IntPtr::typeid, String::typeid };
		auto values = gcnew array<Object^> { (IntPtr)hParentWnd, parameters };
		Object^ result;
		if (CallPluginMethod(result, this->provider, L"GetLinkText", types, values))
		{
			linkText = (String^)result;
			return true;
		}
		return false;
	}

	bool GetCommitMessage(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString& newMessage)
	{
		auto types = gcnew array<Type^> { IntPtr::typeid, String::typeid, String::typeid, array<String^>::typeid, String::typeid };
		auto values = gcnew array<Object^> { (IntPtr)hParentWnd, parameters, gcnew String(commonRoot), CStringVectorToClrArray(pathList), gcnew String(originalMessage) };
		Object^ result;
		if (CallPluginMethod(result, this->provider, L"GetCommitMessage", types, values))
		{
			newMessage = (String^)result;
			return true;
		}
		return false;
	}
	
	// IBugTraqProvider2 methods
	bool GetCommitMessage2(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString originalMessage, CString bugID,
		CString& bugIDOut, STRING_VECTOR& revPropNames, STRING_VECTOR& revPropValues, CString& newMessage)
	{
		auto types = gcnew array<Type^> { IntPtr::typeid, String::typeid, String::typeid, String::typeid, array<String^>::typeid, String::typeid, String::typeid, String::typeid->MakeByRefType(), array<String^>::typeid->MakeByRefType(), array<String^>::typeid->MakeByRefType() };
		auto values = gcnew array<Object^> { (IntPtr)hParentWnd, parameters, gcnew String(commonURL), gcnew String(commonRoot), CStringVectorToClrArray(pathList), gcnew String(originalMessage), gcnew String(bugID), nullptr, nullptr, nullptr };
		Object^ result;
		if (CallPluginMethod(result, this->provider, L"GetCommitMessage2", types, values))
		{
			bugIDOut = (String^)values[7];
			ClrArrayToCStringVector(revPropNames, (array<String^>^)values[8]);
			ClrArrayToCStringVector(revPropValues, (array<String^>^)values[9]);
			newMessage = (String^)result;
			return true;
		}
		return false;
	}

	bool CheckCommit(HWND hParentWnd, CString commonURL, CString commonRoot, const STRING_VECTOR& pathList, CString commitMessage, CString& errorMessage)
	{
		auto types = gcnew array<Type^> { IntPtr::typeid, String::typeid, String::typeid, String::typeid, array<String^>::typeid, String::typeid };
		auto values = gcnew array<Object^> { (IntPtr)hParentWnd, parameters, gcnew String(commonURL), gcnew String(commonRoot), CStringVectorToClrArray(pathList), gcnew String(commitMessage) };
		Object^ result;
		if (CallPluginMethod(result, this->provider, L"CheckCommit", types, values))
		{
			errorMessage = (String^)result;
			return true;
		}
		return false;
	}

	bool OnCommitFinished(HWND hParentWnd, CString commonRoot, const STRING_VECTOR& pathList, CString logMessage, ULONG revision, CString& error)
	{
		auto types = gcnew array<Type^> { IntPtr::typeid, String::typeid, array<String^>::typeid, String::typeid, int::typeid };
		auto values = gcnew array<Object^> { (IntPtr)hParentWnd, gcnew String(commonRoot), CStringVectorToClrArray(pathList), gcnew String(logMessage), (int)revision };
		Object^ result;
		if (CallPluginMethod(result, this->provider, L"OnCommitFinished", types, values))
		{
			error = (String^)result;
			return true;
		}
		return false;
	}

	bool HasOptions(bool& ret)
	{
		Object^ result;
		if (CallPluginMethod(result, this->provider, L"HasOptions"))
		{
			ret = (bool)result;
			return true;
		}
		return false;
	}

	bool ShowOptionsDialog(HWND hParentWnd, CString& newparameters)
	{
		auto types = gcnew array<Type^> { IntPtr::typeid, String::typeid };
		auto values = gcnew array<Object^> { (IntPtr)hParentWnd, parameters };
		Object^ result;
		if (CallPluginMethod(result, this->provider, L"ShowOptionsDialog", types, values))
		{
			newparameters = (String^)result;
			return true;
		}
		return false;
	}

private:
	static System::Collections::Concurrent::ConcurrentDictionary<IntPtr, CBugTraqProviderClr^>^ PtrToObj = gcnew System::Collections::Concurrent::ConcurrentDictionary<IntPtr, CBugTraqProviderClr^>();
	Object^ provider;
	int level;
	String^ parameters;
	String^ lastError;

	static Guid guidIBugTraqProvider2 = Guid(L"C5C85E31-2F9B-4916-A7BA-8E27D481EE83");
	static Guid guidIBugTraqProvider = Guid(L"298B927C-7220-423C-B7B4-6E241F00CD93");

	static CBugTraqProviderClr^ AAA(IntPtr a, CBugTraqProviderClr^ b) { return b; }

	bool CallPluginMethod(Object^& result, Object^ obj, String^ name)
	{
		return CallPluginMethod(result, obj, name, gcnew array<Type^>(0), gcnew array<Object^>(0));
	}

	bool CallPluginMethod(Object^& result, Object^ obj, String^ name, array<Type^>^ types, array<Object^>^ values)
	{
		if (obj == nullptr)
			return false;
		try
		{
			auto method = obj->GetType()->GetMethod(name, types);
			result = method->Invoke(obj, values);
			return true;
		}
		catch (Exception^ ex)
		{
			lastError = ex->InnerException != nullptr ? ex->InnerException->Message : ex->Message;
			return false;
		}
	}

	static Type^ FindPluginType(array<Type^>^ types, int& level)
	{
		level = 0;
		for each (auto type in types)
		{
			if (!type->IsClass)
				continue;
			auto itfs = type->GetInterfaces();
			for each (auto itf in itfs)
			{
				auto attrs = itf->GetCustomAttributes(GuidAttribute::typeid, true);
				for each (GuidAttribute^ attr in attrs)
				{
					if (guidIBugTraqProvider2.Equals(Guid::Parse(attr->Value)))
					{
						level = 2;
						return type;
					}
					if (guidIBugTraqProvider.Equals(Guid::Parse(attr->Value)))
					{
						level = 1;
						return type;
					}
				}
			}
		}
		return nullptr;
	}

	static array<String^>^ CStringVectorToClrArray(STRING_VECTOR src)
	{
		auto dst = gcnew array<String^>((int)src.size());
		for (int i = 0; i < src.size(); i++)
			dst[i] = gcnew String(src[i]);
		return dst;
	}

	static void ClrArrayToCStringVector(STRING_VECTOR& dst, array<String^>^ src)
	{
		if (src == nullptr)
			return;
		for (int i = 0; i < src->Length; i++)
			dst.push_back(CString(src[i]));
	}
};
