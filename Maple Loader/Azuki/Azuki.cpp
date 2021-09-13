#include "Azuki.h"

#define byteCodeXorKey 0xA2
#define blockSize 15

/**
* Grab the correct function entry/prologue pointer from the specified Module Name and Function Name
*
* @param module The name of the module (with extension) from which the function pointer should be taken from
* @param func The name of the function from which the function pointer will be taken from
* 
* @returns Pointer to the function, or if tampering has been detected, a pointer to the proper function prologue (when re-allocated)
*/
DWORD* Azuki::GetPointer(std::string module, std::string func)
{
	VM_FISH_RED_START
	DWORD* funcAddress = reinterpret_cast<DWORD*>(reinterpret_cast<char*>(GetProcAddress(GetModuleHandleA(module.c_str()), func.c_str())));

	std::tuple<uintptr_t, std::vector<unsigned char>> foundFunc = std::make_tuple(static_cast<uintptr_t>(0), std::vector<unsigned char>());

	for (auto _func : MonitoredSubroutines)
	{
		uintptr_t encryptedAddress = std::get<0>(_func);
		encryptedAddress ^= 0xB1;
		encryptedAddress -= 0x2000;

		if (reinterpret_cast<DWORD*>(encryptedAddress) == funcAddress)
		{
			foundFunc = _func;
			std::get<0>(foundFunc) = encryptedAddress; // <-- Don't forgot to set decrypted value quickly!
		}
	}

	DWORD* functionAddress = reinterpret_cast<DWORD*>(std::get<0>(foundFunc));

	DWORD* prologueAddress = 0;
	VM_FISH_RED_END
	if (functionAddress != NULL)
	{
		for (int i = 0; i < blockSize; i++)
		{
			unsigned char* value = reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(functionAddress) + i);
			unsigned char monitoredValue = std::get<1>(foundFunc)[i];

			// We have found something in the beginning.. let's check it out!
			if (*value != (monitoredValue ^ byteCodeXorKey) || *value == 0xE9)
			{
				if (*value == 0xE9)
				{
					hde32s hde;
					int funcSize = 0;
					unsigned char currInstruction = 0;

					signed int hookAddress = static_cast<signed int>(*reinterpret_cast<DWORD*>((reinterpret_cast<char*>(functionAddress) + i + 0x01)));
					DWORD* absolute = reinterpret_cast<DWORD*>((reinterpret_cast<char*>(functionAddress) + i + hookAddress + 0x05));
					std::vector<uintptr_t> possiblePrologues = std::vector<uintptr_t>();
					while (currInstruction != 0xCC)
					{
						currInstruction = *reinterpret_cast<unsigned char*>((reinterpret_cast<char*>(absolute) + funcSize));

						switch (currInstruction)
						{
						case 0xA0:
						case 0xA1:
						case 0xA2:
						case 0xA3:
						{
							VM_FISH_RED_START
							int prevSize = funcSize;
							funcSize += hde32_disasm(reinterpret_cast<void*>(reinterpret_cast<char*>(absolute) + funcSize), &hde);
							if (funcSize - prevSize == 5)
								possiblePrologues.push_back(*reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>((reinterpret_cast<char*>(absolute) + prevSize + 0x01))));

							VM_FISH_RED_END
							break;
						}
						default:
							VM_FISH_RED_START
							funcSize += hde32_disasm(reinterpret_cast<void*>(reinterpret_cast<char*>(absolute) + funcSize), &hde);
							VM_FISH_RED_END
							break;
						}
					}

					for (auto possiblePrologue : possiblePrologues)
					{
						VM_FISH_RED_START
						unsigned char opcode = 0xFF; // <-- can't use 0x00 here since we scan for that lulw
						int size = 0;
						while (opcode != 0x00) // <-- You always allocate more memory than you use, so the rest is 0x00
						{
							opcode = *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(possiblePrologue) + size);
							if (opcode == 0xE9) // <-- jmp instruction
							{
								signed int hookAddress = static_cast<signed int>(*reinterpret_cast<DWORD*>(reinterpret_cast<char*>(possiblePrologue) + size + 0x01));
								DWORD* absolute = reinterpret_cast<DWORD*>(reinterpret_cast<char*>(possiblePrologue) + size + hookAddress + 0x05);

								if ((reinterpret_cast<uintptr_t>(absolute) - reinterpret_cast<uintptr_t>(functionAddress)) == 0x05) // <-- the jmp in this function heads 5 bytes out from original function -> this is the function prologue
								{
									prologueAddress = reinterpret_cast<DWORD*>(possiblePrologue);
								}
							}
							size += hde32_disasm(reinterpret_cast<void*>(reinterpret_cast<char*>(possiblePrologue) + size), &hde);
						}
						VM_FISH_RED_END
					}
				}
			}
		}
	}

	foundFunc = std::make_tuple(static_cast<uintptr_t>(0), std::vector<unsigned char>()); // <-- Artificial GC!
	if (prologueAddress == 0)
		return functionAddress;
	return prologueAddress;
}

#pragma optimize("", off)

/**
* Adds a subroutine to the Vector by Module Name and Function Name
*
* @param module The name of the module (with extension) from which the function pointer should be taken from
* @param func The name of the function from which the function pointer will be taken from
*/
void Azuki::MonitorSubroutine(std::string module, std::string func)
{
	VM_SHARK_BLACK_START
	MonitorSubroutine(reinterpret_cast<DWORD*>(reinterpret_cast<char*>(GetProcAddress(GetModuleHandleA(module.c_str()), func.c_str()))), func);
	VM_SHARK_BLACK_END
}

/**
* Adds a subroutine to the Vector by Module Base Address and Function Name
*
* @param address The base address from the module from which the function pointer should be taken from
* @param func The name of the function from which the function pointer will be taken from
*/
void Azuki::MonitorSubroutine(DWORD* address, std::string func)
{
	VM_SHARK_BLACK_START
	std::vector<unsigned char> bytes = std::vector<unsigned char>();

	uintptr_t encryptedAddress = reinterpret_cast<uintptr_t>(address);
	encryptedAddress += 0x2000;
	encryptedAddress ^= 0xB1;
	for (int i = 0; i < blockSize; i++)
		bytes.push_back((*reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(address) + i)) ^ byteCodeXorKey);

	MonitoredSubroutines.push_back(std::make_tuple(encryptedAddress, bytes));
	VM_SHARK_BLACK_END
}

/**
* \brief Populates the MonitoredSubroutines Vector with data
*/
void Azuki::Install()
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START
	MonitorSubroutine("ws2_32.dll", "recv");
	MonitorSubroutine("ws2_32.dll", "send");
	STR_ENCRYPT_END
	VM_SHARK_BLACK_END
}
#pragma optimize("", on)

#undef byteCodeXorKey