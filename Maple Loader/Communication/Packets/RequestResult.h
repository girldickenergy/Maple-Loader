#pragma once

enum class RequestResult : int32_t
{
    InvalidRequest = -1,
    Success = 0,
    InvalidCredentials = 1,
    VersionMismatch = 2,
    HWIDMismatch = 3,
    UserBanned = 4,
    InvalidSession = 5,
    NotSubscribed = 6
};