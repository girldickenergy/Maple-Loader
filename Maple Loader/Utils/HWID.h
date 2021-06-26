#pragma once

#include <intrin.h>
#include <memory>
#include <string>
#include <sstream>
#include <windows.h>

#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

class HWID
{
	static std::string getDiskSerial()
	{
		std::string m_sResult;

		HANDLE m_hFile = CreateFileW(L"\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			return { };

		std::unique_ptr< std::remove_pointer <HANDLE >::type, void(*)(HANDLE) > m_hDevice
		{
			m_hFile, [](HANDLE handle)
			{
				CloseHandle(handle);
			}
		};

		STORAGE_PROPERTY_QUERY m_PropertyQuery;
		m_PropertyQuery.PropertyId = StorageDeviceProperty;
		m_PropertyQuery.QueryType = PropertyStandardQuery;

		STORAGE_DESCRIPTOR_HEADER m_DescHeader;
		DWORD m_dwBytesReturned = 0;
		if (!DeviceIoControl(m_hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &m_PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
			&m_DescHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &m_dwBytesReturned, NULL))
			return { };

		const DWORD m_dwOutBufferSize = m_DescHeader.Size;
		std::unique_ptr< BYTE[] > m_pOutBuffer{ new BYTE[m_dwOutBufferSize] { } };

		if (!DeviceIoControl(m_hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &m_PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
			m_pOutBuffer.get(), m_dwOutBufferSize, &m_dwBytesReturned, NULL))
			return { };

		STORAGE_DEVICE_DESCRIPTOR* m_pDeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(m_pOutBuffer.get());
		const DWORD m_dwSerialNumberOffset = m_pDeviceDescriptor->SerialNumberOffset;
		if (m_dwSerialNumberOffset == 0)
			return { };

		m_sResult = reinterpret_cast<const char*>(m_pOutBuffer.get() + m_dwSerialNumberOffset);
		m_sResult.erase(std::remove_if(m_sResult.begin(), m_sResult.end(), std::isspace), m_sResult.end());

		return m_sResult;
	}

	static std::string getCPUVendor()
	{
		int regs[4] = { 0 };
		char vendor[13];
		__cpuid(regs, 0);
		memcpy(vendor, &regs[1], 4);
		memcpy(vendor + 4, &regs[3], 4);
		memcpy(vendor + 8, &regs[2], 4);
		vendor[12] = '\0';

		return std::string(vendor);
	}

	static std::string getMacAddress()
	{
		PIP_ADAPTER_INFO AdapterInfo;
		DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
		char* mac_addr = (char*)malloc(18);

		AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
		if (AdapterInfo == NULL)
		{
			free(mac_addr);
			return { };
		}

		if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW)
		{
			free(AdapterInfo);
			AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
			if (AdapterInfo == NULL)
			{
				free(mac_addr);
				return { };
			}
		}

		if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR)
		{
			PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
			do
			{
				sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
					pAdapterInfo->Address[0], pAdapterInfo->Address[1],
					pAdapterInfo->Address[2], pAdapterInfo->Address[3],
					pAdapterInfo->Address[4], pAdapterInfo->Address[5]);

				pAdapterInfo = pAdapterInfo->Next;
			} while (pAdapterInfo);
		}
		
		free(AdapterInfo);
		std::string addr(mac_addr);
		free(mac_addr);

		return addr;
	}
public:
	static std::string GetHWID()
	{
		std::string diskSerial = getDiskSerial();
		std::string cpuVendor = getCPUVendor();
		std::string macAddress = getMacAddress();
		
		if (diskSerial.empty() || cpuVendor.empty() || macAddress.empty())
			return { };

		std::stringstream hwid;
		hwid << diskSerial << "|" << cpuVendor << "|" << macAddress;

		return hwid.str();
	}
};
