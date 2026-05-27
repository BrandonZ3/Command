#pragma once
#include <Windows.h>
#include <stdint.h>

class IntegerList
{
public:
    uint64_t* buffer = NULL;
    int count = 0;
    int size = 0;

    IntegerList(const IntegerList&) = delete;
    IntegerList(unsigned int initialcount);
    int Add(uint64_t character);
    void Clear();
    ~IntegerList();
};
