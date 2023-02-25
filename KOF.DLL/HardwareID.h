#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <ntddscsi.h>

#pragma comment(lib, "wbemuuid.lib")

class HardwareID
{
public:
	static std::wstring SafeString(const wchar_t* pString)
	{
		return std::wstring((pString == nullptr ? L"(null)" : pString));
	}

	static void RemoveWhitespaces(std::wstring& String)
	{
		String.erase(std::remove(String.begin(), String.end(), L' '), String.end());
	}

private:
	std::wstring GetHKLM(std::wstring SubKey, std::wstring Value)
	{
		DWORD Size{};
		std::wstring Ret{};

		RegGetValueW(HKEY_LOCAL_MACHINE,
			SubKey.c_str(),
			Value.c_str(),
			RRF_RT_REG_SZ,
			nullptr,
			nullptr,
			&Size);

		Ret.resize(Size);

		RegGetValueW(HKEY_LOCAL_MACHINE,
			SubKey.c_str(),
			Value.c_str(),
			RRF_RT_REG_SZ,
			nullptr,
			&Ret[0],
			&Size);

		return Ret.c_str();
	}

	template <typename T = const wchar_t*>
	void QueryWMI(std::wstring WMIClass, std::wstring Field, std::vector <T>& Value, const wchar_t* ServerName = L"ROOT\\CIMV2")
	{
		std::wstring Query(L"SELECT ");
		Query.append(Field.c_str()).append(L" FROM ").append(WMIClass.c_str());

		IWbemLocator* Locator{};
		IWbemServices* Services{};
		IEnumWbemClassObject* Enumerator{};
		IWbemClassObject* ClassObject{};
		VARIANT Variant{};
		DWORD Returned{};

		HRESULT hResult{ CoInitializeEx(nullptr, COINIT_MULTITHREADED) };

		if (FAILED(hResult)) {
			return;
		}

		hResult = CoInitializeSecurity(nullptr,
			-1,
			nullptr,
			nullptr,
			RPC_C_AUTHN_LEVEL_DEFAULT,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			nullptr,
			EOAC_NONE,
			nullptr);

		if (FAILED(hResult))
		{
			CoUninitialize();
			return;
		}

		hResult = CoCreateInstance(CLSID_WbemLocator,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWbemLocator,
			reinterpret_cast<PVOID*>(&Locator));

		if (FAILED(hResult))
		{
			CoUninitialize();
			return;
		}

		hResult = Locator->ConnectServer(_bstr_t(ServerName),
			nullptr,
			nullptr,
			nullptr,
			NULL,
			nullptr,
			nullptr,
			&Services);

		if (FAILED(hResult))
		{
			Locator->Release();
			CoUninitialize();
			return;
		}

		hResult = CoSetProxyBlanket(Services,
			RPC_C_AUTHN_WINNT,
			RPC_C_AUTHZ_NONE,
			nullptr,
			RPC_C_AUTHN_LEVEL_CALL,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			nullptr,
			EOAC_NONE);

		if (FAILED(hResult))
		{
			Services->Release();
			Locator->Release();
			CoUninitialize();
			return;
		}

		hResult = Services->ExecQuery(bstr_t(L"WQL"),
			bstr_t(Query.c_str()),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			nullptr,
			&Enumerator);

		if (FAILED(hResult))
		{
			Services->Release();
			Locator->Release();
			CoUninitialize();
			return;
		}

		while (Enumerator)
		{

			HRESULT Res = Enumerator->Next(WBEM_INFINITE,
				1,
				&ClassObject,
				&Returned);

			if (!Returned)
			{
				break;
			}

			Res = ClassObject->Get(Field.c_str(),
				0,
				&Variant,
				nullptr,
				nullptr);

			if (typeid(T) == typeid(long) || typeid(T) == typeid(int))
			{
				Value.push_back((T)Variant.intVal);
			}
			else if (typeid(T) == typeid(bool))
			{
				Value.push_back((T)Variant.boolVal);
			}
			else if (typeid(T) == typeid(unsigned int))
			{
				Value.push_back((T)Variant.uintVal);
			}
			else if (typeid(T) == typeid(unsigned short))
			{
				Value.push_back((T)Variant.uiVal);
			}
			else if (typeid(T) == typeid(long long))
			{
				Value.push_back((T)Variant.llVal);
			}
			else if (typeid(T) == typeid(unsigned long long))
			{
				Value.push_back((T)Variant.ullVal);
			}
			else
			{
				Value.push_back((T)((bstr_t)Variant.bstrVal).copy());
			}

			VariantClear(&Variant);
			ClassObject->Release();
		}

		if (!Value.size())
		{
			Value.resize(1);
		}

		Services->Release();
		Locator->Release();
		Enumerator->Release();
		CoUninitialize();
	}

	void QuerySMBIOS()
	{
		std::vector <const wchar_t*> SerialNumber{};
		QueryWMI(L"Win32_BaseBoard", L"SerialNumber", SerialNumber);
		this->SMBIOS.SerialNumber = SafeString(SerialNumber.at(0));
	}


	void QueryProcessor()
	{
		std::vector <const wchar_t*> ProcessorId{};

		QueryWMI(L"Win32_Processor", L"ProcessorId", ProcessorId);
		this->CPU.ProcessorID = SafeString(ProcessorId.at(0));
	}

	void QuerySystem()
	{
		std::vector <const wchar_t*> SystemName{};
		std::vector <const wchar_t*> OSSerialNumber{};

		QueryWMI(L"Win32_ComputerSystem", L"Name", SystemName);
		QueryWMI(L"Win32_OperatingSystem", L"SerialNumber", OSSerialNumber);

		this->System.Name = SafeString(SystemName.at(0));
		this->System.OSSerialNumber = SafeString(OSSerialNumber.at(0));
	}

	void QueryRegistry()
	{
		this->Registry.ComputerHardwareID = SafeString(GetHKLM(L"SYSTEM\\CurrentControlSet\\Control\\SystemInformation", L"ComputerHardwareId").c_str());
	}

public:
	struct
	{
		std::wstring SerialNumber{};
	} SMBIOS;

	struct
	{
		std::wstring ProcessorID{};

	} CPU;

	struct
	{
		std::wstring Name{};
		std::wstring OSSerialNumber{};

	} System;

	struct
	{
		std::wstring ComputerHardwareID{};
	} Registry;


	std::unique_ptr<HardwareID> Pointer()
	{
		return std::make_unique<HardwareID>(*this);
	}

	void GetHardwareID()
	{
		QuerySMBIOS();
		QueryProcessor();
		QuerySystem();
		QueryRegistry();
	}

	HardwareID()
	{
		//GetHardwareID();
	}
};