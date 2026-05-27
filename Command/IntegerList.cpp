#include "IntegerList.h"


IntegerList::IntegerList(unsigned int initialcount)
{
    this->size = initialcount;
    this->buffer = (uint64_t*)malloc(this->size * sizeof(uint64_t));
}

int IntegerList::Add(uint64_t character)
{
    if (this->count + 1 == this->size)
    {
        this->size *= 2;

        uint64_t* test = (uint64_t*)realloc(this->buffer, this->size * sizeof(uint64_t));

        if (test)
        {
            this->buffer = test;
            this->buffer[this->count] = character;
            this->count++;
            return this->count - 2;
        }
        else
            return -1;
    }
    else
    {
        this->buffer[this->count] = character;
        this->count++;
    }
}

void IntegerList::Clear()
{
    this->count = 0;
}

IntegerList::~IntegerList()
{
    free(this->buffer);
}