#include "HardwareUtilities.h"

#include <intrin.h>
#include <sstream>

#include <glfw3.h>

#include "smbios.h"
#include "../Crypto/CryptoUtilities.h"
#include "../Security/xorstr.hpp"

std::string HardwareUtilities::getGPUModel()
{
	const GLubyte* renderer = glGetString(GL_RENDERER);

	return std::string((char*)renderer);
}

std::string HardwareUtilities::getCPUVendor()
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

std::string HardwareUtilities::getMotherboardInfo()
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

		if (header->type == smbios::types::baseboard_info)
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

std::string HardwareUtilities::GetPCName()
{
	char buffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD lpnSize = MAX_COMPUTERNAME_LENGTH + 1;
	if (GetComputerNameA(buffer, &lpnSize) == FALSE)
		return xorstr_("mplaudioservice");

	return std::string(buffer);
}

std::string HardwareUtilities::GetHWID()
{
	const std::string gpuModel = getGPUModel();
	const std::string cpuVendor = getCPUVendor();
	const std::string motherboardInfo = getMotherboardInfo();

	if (gpuModel.empty() && cpuVendor.empty() && motherboardInfo.empty())
		return { };

	std::stringstream hwid;
	hwid << gpuModel << xorstr_("|") << cpuVendor << xorstr_("|") << motherboardInfo;

	return CryptoUtilities::GetMD5Hash(hwid.str());
}
