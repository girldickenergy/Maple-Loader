#pragma once

#include <vector>

enum class LoaderStreamResult
{
	Success = 0,
	InvalidSession = 1,
	NotSubscribed = 2,
	UnknownError = 3
};

class LoaderStreamResponse
{
	LoaderStreamResult result;
	std::vector<unsigned char> loaderData;

	LoaderStreamResponse(LoaderStreamResult result, const std::vector<unsigned char>& loaderData);
public:
	LoaderStreamResult GetResult();
	const std::vector<unsigned char>& GetLoaderData();

	static LoaderStreamResponse Deserialize(const std::vector<unsigned char>& payload);
};