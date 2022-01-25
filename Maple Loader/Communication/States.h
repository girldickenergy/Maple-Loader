#pragma once

enum class States : int
{
	Idle = 0,
	LoggingIn = 1,
	LoggedIn = 2,
	LoadingDLL = 4
};
