#pragma once
#include <Windows.h>
#include "StringArray.h"

bool IsIp(const char* string);
ULONG IPToDwordNetwork(const char* ipAddress);