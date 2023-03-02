#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "SkCrypter.h"

class HardwareID
{
public:
	HardwareID()
	{
		m_szProcessorId.clear();
		m_szSystemName.clear();
		m_szBaseBoardSerial.clear();
		m_szHddSerial.clear();
		m_szuid.clear();
		m_szSystemSerialNumber.clear();
	};

	~HardwareID()
	{
		m_szProcessorId.clear();
		m_szSystemName.clear();
		m_szBaseBoardSerial.clear();
		m_szHddSerial.clear();
		m_szuid.clear();
		m_szSystemSerialNumber.clear();
	};

public:
	std::string GetProcesorId() { return m_szProcessorId; };
	std::string GetSystemName() { return m_szSystemName; };
	std::string GetBaseBoardSerial() { return m_szBaseBoardSerial; };
	std::string GetHddSerial() { return m_szHddSerial; };
	std::string GetUUID() { return m_szuid; };
	std::string GetSystemSerialNumber() { return m_szSystemSerialNumber; };

	bool Query()
	{
		auto strip_keyword = [](std::string& buffer, const bool filter_digits = false)
		{
			std::string current, stripped;
			std::istringstream iss(buffer);

			buffer.clear();
			auto first_tick = false;
			while (std::getline(iss, current))
			{
				if (!first_tick)
				{
					first_tick = true;
					continue;
				}

				if (filter_digits && std::isdigit(current.at(0)))
				{
					continue;
				}

				buffer.append(current).append("\n");
			}
			if (buffer.back() == '\n')
			{
				buffer.pop_back();
			}
		};

		if (!WMIC(skCryptDec("wmic cpu get processorId"), m_szProcessorId) ||
			!WMIC(skCryptDec("wmic os get csname"), m_szSystemName) ||
			!WMIC(skCryptDec("wmic baseboard get serialnumber"), m_szBaseBoardSerial) ||
			!WMIC(skCryptDec("wmic diskdrive get SerialNumber"), m_szHddSerial) ||
			!WMIC(skCryptDec("wmic csproduct get uuid"), m_szuid) ||
			!WMIC(skCryptDec("wmic os get serialnumber"), m_szSystemSerialNumber))


		{
			return false;
		}

		strip_keyword(m_szProcessorId);
		strip_keyword(m_szSystemName);
		strip_keyword(m_szBaseBoardSerial);
		strip_keyword(m_szHddSerial, true);
		strip_keyword(m_szuid);
		strip_keyword(m_szSystemSerialNumber);

		return true;
	}

private:

	bool Cmd(const std::string& input)
	{
		auto* shell_cmd = _popen(input.c_str(), "r");

		if (!shell_cmd)
		{
			_pclose(shell_cmd);
			return false;
		}
		else
		{
			_pclose(shell_cmd);
			return true;
		}
	}

	bool WMIC(const std::string& input, std::string& out)
	{
		auto* shell_cmd = _popen(input.c_str(), "r");
		if (!shell_cmd)
		{
			return false;
		}

		static char buffer[1024] = {};

		while (fgets(buffer, 1024, shell_cmd))
		{
			out.append(buffer);
		}

		_pclose(shell_cmd);

		while (out.back() == '\n' ||
			out.back() == '\0' ||
			out.back() == ' ' ||
			out.back() == '\r' ||
			out.back() == '\t') {
			out.pop_back();
		}

		return !out.empty();
	}

private:
	std::string m_szProcessorId;
	std::string m_szSystemName;
	std::string m_szBaseBoardSerial;
	std::string m_szHddSerial;
	std::string m_szuid;
	std::string m_szSystemSerialNumber;
};

