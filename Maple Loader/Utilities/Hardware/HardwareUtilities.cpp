#include "HardwareUtilities.h"

#include <dxgi.h>
#include <intrin.h>
#include <set>
#include <sstream>

#include <glfw3.h>

#include "smbios.h"
#include "../Crypto/CryptoUtilities.h"
#include "../Security/xorstr.hpp"

std::string HardwareUtilities::getGPUModel()
{
	std::string gpuModel;

	IDXGIFactory* pFactory = nullptr;

	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory))))
		return xorstr_("Generic GPU");

	IDXGIAdapter* pAdapter = nullptr;
	std::set<std::wstring> uniqueGPUs;
	for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);

		// skip Microsoft software adapters
		if (desc.VendorId == 0x1414 && desc.DeviceId == 0x8C)
		{
			pAdapter->Release();

			continue;
		}

		// skip duplicates
		if (uniqueGPUs.contains(desc.Description))
		{
			pAdapter->Release();

			continue;
		}

		uniqueGPUs.insert(desc.Description);

		std::wstring modelWide = std::wstring(desc.Description) + L"_" + std::to_wstring(desc.DedicatedVideoMemory / (1024 * 1024));

		gpuModel += (gpuModel.empty() ? "" : ":") + std::string(modelWide.begin(), modelWide.end());

		pAdapter->Release();
	}

	pFactory->Release();

	return gpuModel.empty() ? xorstr_("Generic GPU") : gpuModel;
}

std::string HardwareUtilities::getCPUVendor()
{
	int cpuInfo[4] = { 0 };
	char cpuModel[0x40] = { 0 };

	__cpuid(cpuInfo, 0x80000002);
	memcpy(cpuModel, cpuInfo, sizeof(cpuInfo));
	__cpuid(cpuInfo, 0x80000003);
	memcpy(cpuModel + 16, cpuInfo, sizeof(cpuInfo));
	__cpuid(cpuInfo, 0x80000004);
	memcpy(cpuModel + 32, cpuInfo, sizeof(cpuInfo));

	return { cpuModel };
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

std::string HardwareUtilities::getUserName()
{
    char buffer[128];
    DWORD bufferSize = 128;
    if (GetUserNameA(buffer, &bufferSize) == FALSE)
        return xorstr_("GenericUser");

    return { buffer };
}

std::string HardwareUtilities::getWindowsInstallDate()
{
	HKEY hKey;
    if (RegOpenKeyA(HKEY_LOCAL_MACHINE, xorstr_("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return xorstr_("-1");
	}

	DWORD installDateType = REG_DWORD;
	DWORD installDate;
    DWORD installDateSize = sizeof(DWORD);
	if (RegQueryValueExA(hKey, xorstr_("InstallDate"), 0, &installDateType, (LPBYTE)&installDate, &installDateSize)  != ERROR_SUCCESS)
	{
        RegCloseKey(hKey);

		return xorstr_("-1");
	}

	RegCloseKey(hKey);

	return std::to_string(installDate);
}

std::string HardwareUtilities::getWindowsProductName()
{
	HKEY hKey;
    if (RegOpenKeyA(HKEY_LOCAL_MACHINE, xorstr_("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return xorstr_("Generic product name");
	}

	DWORD productNameType = REG_SZ;
    char productName[256]{};
    DWORD productNameSize = 256;

	if (RegGetValueA(hKey, NULL, xorstr_("ProductName"), RRF_RT_REG_SZ, &productNameType, &productName, &productNameSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return xorstr_("Generic product name");
	}

	RegCloseKey(hKey);

	return { productName };
}

std::string HardwareUtilities::GetPCName()
{
	char buffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD lpnSize = MAX_COMPUTERNAME_LENGTH + 1;
	if (GetComputerNameA(buffer, &lpnSize) == FALSE)
		return xorstr_("mplaudioservice");

	return { buffer };
}

std::string HardwareUtilities::GetHWID()
{
	const std::string gpuModel = getGPUModel();
	const std::string cpuVendor = getCPUVendor();
	const std::string motherboardInfo = getMotherboardInfo();
    const std::string computerName = GetPCName();
    const std::string userName = getUserName();
    const std::string windowsInstallDate = getWindowsInstallDate();
    const std::string windowsProductName = getWindowsProductName();

	std::stringstream hwid;
	std::stringstream sfid;

	if (gpuModel.empty() && cpuVendor.empty() && motherboardInfo.empty())
        hwid << xorstr_("Generic HWID");
    else
		hwid << gpuModel << xorstr_("|") << cpuVendor << xorstr_("|") << motherboardInfo;

	if (computerName.empty() && userName.empty() && windowsInstallDate.empty() && windowsProductName.empty())
        sfid << xorstr_("Generic SFID");
    else
		sfid << computerName << xorstr_("|") << userName << xorstr_("|") << windowsInstallDate << xorstr_("|") << windowsProductName;

	printf("%s|%s\n", hwid.str().c_str(), sfid.str().c_str());

	return CryptoUtilities::GetMD5Hash(hwid.str()) + xorstr_("|") + CryptoUtilities::GetMD5Hash(sfid.str());
}
