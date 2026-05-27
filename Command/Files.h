#pragma once
#include <stdio.h>
#include "Buffer.h"
#include "Strings.h"

unsigned long long GetFileSize(FILE* file);
Buffer* ReadEntireFile(FILE* file);
Buffer* ReadPartialFile(FILE* file, unsigned long long offset, unsigned long long length);
void ReadPartialFileToBuffer(Buffer* emptyBuf, FILE* file, unsigned long long offset, unsigned long long length);
char* GetFileExtension(const char* path);
Buffer* ReadFile(char* filepath, const char* readmode, size_t limit);
Buffer* ReadFile(char* filepath, const char* readmode, unsigned long long offset, unsigned long long length);
size_t WriteFile(char* filepath, Buffer* buffer);
size_t AppendFile(char* filepath, Buffer* buffer);