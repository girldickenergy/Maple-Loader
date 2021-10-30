#pragma once

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "../Crypto/md5.h"

#include <string>
#include <sstream>

#include "smbios.h"
#include "../UI/UI.h"

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
		m_sResult.erase(std::remove_if(m_sResult.begin(), m_sResult.end(), [](unsigned char x) { return std::isspace(x); }), m_sResult.end());

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

	static std::string getMotherboardInfo()
	{
		std::string manufacturer;
		std::string product;
		
		const DWORD smbios_data_size = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);

		auto* const heap_handle = GetProcessHeap();
		auto* const smbios_data = static_cast<smbios::raw_smbios_data*>(HeapAlloc(heap_handle, 0, static_cast<size_t>(smbios_data_size)));
		if (!smbios_data)
		{
			return { };
		}

		const DWORD bytes_written = GetSystemFirmwareTable('RSMB', 0, smbios_data, smbios_data_size);
		if (bytes_written != smbios_data_size)
		{
			return { };
		}

		smbios::parser meta;
		const BYTE* buff = smbios_data->smbios_table_data;
		const auto buff_size = static_cast<size_t>(smbios_data_size);

		meta.feed(buff, buff_size);
		for (auto& header : meta.headers)
		{
			smbios::string_array_t strings;
			smbios::parser::extract_strings(header, strings);

			if (header->type ==  smbios::types::baseboard_info)
			{
				auto* const x = reinterpret_cast<smbios::baseboard_info*>(header);

				if (x->length == 0)
					break;

				manufacturer = strings[x->manufacturer_name];
				product = strings[x->product_name];
			}
		}

		HeapFree(heap_handle, 0, smbios_data);

		if (manufacturer.empty() || product.empty())
			return { };

		return manufacturer + '-' + product;
	}
public:
	static std::string GetHWID()
	{
		std::string diskSerial = getDiskSerial();
		std::string cpuVendor = getCPUVendor();
		std::string motherboardInfo = getMotherboardInfo();
		
		if (diskSerial.empty() || cpuVendor.empty() || motherboardInfo.empty())
			return { };

		std::stringstream hwid;
		hwid << diskSerial << "|" << cpuVendor << "|" << motherboardInfo;

		CryptoPP::Weak1::MD5 hash;
		byte digest[CryptoPP::Weak1::MD5::DIGESTSIZE];

		hash.CalculateDigest(digest, (byte*)hwid.str().c_str(), hwid.str().length());

		CryptoPP::HexEncoder encoder;
		std::string hwidHashed;
		encoder.Attach(new CryptoPP::StringSink(hwidHashed));
		encoder.Put(digest, sizeof(digest));
		encoder.MessageEnd();
		
		return hwidHashed;
	}
};
