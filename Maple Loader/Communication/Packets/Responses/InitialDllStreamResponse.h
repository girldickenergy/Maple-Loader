#pragma once

#include "Response.h"

enum class InitialDllStreamResult : unsigned char
{
	Success = 0x0,
	NotSubscribed = 0x1,
	InvalidSession = 0x2,
	InternalError = 0x3
};

struct Import
{
public:
	std::string ModuleName;
	std::string FunctionName;
	bool ImportByOrdinal;

	Import(std::string moduleName, std::string functionName, bool importByOrdinal)
	{
		ModuleName = moduleName;
		FunctionName = functionName;
		ImportByOrdinal = importByOrdinal;
	}
};

class InitialDllStreamResponse : public Response
{
public:
	InitialDllStreamResult Result;
	unsigned int AllocationSize;
	std::vector<Import> Imports;
	
	InitialDllStreamResponse(const char* msg, size_t size, MatchedClient* matchedClient);
};
