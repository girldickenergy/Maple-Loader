#pragma once

struct User
{
	char Username[24];
	char Password[256];
	std::string Session;

	void ResetSensitiveFields()
	{
		memset(Password, 0, sizeof(Password));
	}
};
