#include "Network.h"


bool IsIp(const char* string)
{
    size_t sep = CharCount(string, '.');

    if (sep == 3)
    {
        StringArray* myArr = Splitter(string, ".");

        for (size_t i = 0; i < myArr->count; i++)
        {
            if (StringToUnsignedWord(string) > 256)
            {
                myArr->FreeEverything();
                delete myArr;
                return false;
            }
        }

        myArr->FreeEverything();
        delete myArr;
    }
    else
        return false;
    return true;
}

ULONG IPToDwordNetwork(const char* ipAddress)
{
    ULONG result;
    unsigned char parts[4];
    StringArray* ipItems = Splitter(ipAddress, ".");

    if (ipItems->count == 4)
    {
        for (size_t i = 0; i < ipItems->count; i++)
        {
            parts[i] = StringToUnsignedChar(ipItems->items[i]);
        }
        ipItems->FreeEverything();
        delete ipItems;
    }
    else
    {
        ipItems->FreeEverything();
        delete ipItems;
        return -1;
    }

    ((char*)(&result))[0] = parts[0];
    ((char*)(&result))[1] = parts[1];
    ((char*)(&result))[2] = parts[2];
    ((char*)(&result))[3] = parts[3];

    return result;
}
