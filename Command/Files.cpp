#include "Files.h"

unsigned long long GetFileSize(FILE* file)
{
    char buffer[1024];

    int len = 1;

    unsigned long long filesize = 0;

    fpos_t currentPosition;

    fgetpos(file, &currentPosition);

    fseek(file, 0, SEEK_SET);

    while (len > 0)
    {
        len = fread_s(&buffer, 1024, 1, 1024, file);
        if (len > 0)
            filesize += len;
    }

    fseek(file, currentPosition, SEEK_SET);

    return filesize;
}

Buffer* ReadEntireFile(FILE* file)
{
    fpos_t lastPosition = 0;
    fgetpos(file, &lastPosition);

    fseek(file, 0, SEEK_SET);

    Buffer* fileData = new Buffer(4096);

    int len = 1;
    char tempbuf[1024];

    while (len > 0)
    {
        len = fread_s(&tempbuf, 1024, 1, 1024, file);

        if (len > 0)
            fileData->Add(tempbuf, len);
    }

    fseek(file, lastPosition, SEEK_SET);

    return fileData;
}

Buffer* ReadPartialFile(FILE* file, unsigned long long offset, unsigned long long length)
{
    fpos_t lastPosition = 0;
    fgetpos(file, &lastPosition);

    fseek(file, offset, SEEK_SET);

    Buffer* fileData = new Buffer(4096);

    int len = 1;
    char tempbuf[1024];
    int max = 1024;

    while (len > 0)
    {
        if (length - fileData->count < max)
            max = length - fileData->count;

        len = fread_s(&tempbuf, 1024, 1, max, file);

        if (len > 0)
            fileData->Add(tempbuf, len);
    }

    fseek(file, lastPosition, SEEK_SET);

    return fileData;
}

void ReadPartialFileToBuffer(Buffer* emptyBuf, FILE* file, unsigned long long offset, unsigned long long length)
{
    fpos_t lastPosition = 0;
    fgetpos(file, &lastPosition);

    fseek(file, offset, SEEK_SET);


    int len = 1;
    char tempbuf[8192];
    int max = 8192;

    while (len > 0)
    {
        if (length - emptyBuf->count < max)
            max = length - emptyBuf->count;

        len = fread_s(&tempbuf, 8192, 1, max, file);

        if (len > 0)
            emptyBuf->Add(tempbuf, len);
    }

    fseek(file, lastPosition, SEEK_SET);
}

char* GetFileExtension(const char* path)
{

    int length = strlen(path);
    int queryLoc = FirstIndexOf(path, "?");
    int dotLoc = -1;

    char* output = NULL;

    if (queryLoc != -1)
    {
        // Not Tested
        for (int i = queryLoc; i > 0; i--)
        {
            if (path[i] == '.')
            {
                dotLoc = i;
                break;
            }
        }

        output = (char*)malloc(queryLoc - dotLoc);
        output[queryLoc - dotLoc] = 0;
        int pos = 0;

        for (int i = dotLoc + 1; i < queryLoc; i++)
        {
            output[pos] = path[i];
            pos++;
        }
    }
    else
    {
        for (int i = length - 1; i > 0; i--)
        {
            if (path[i] == '.')
            {
                dotLoc = i;
                break;
            }
        }

        int nsiz = length - dotLoc;

        output = (char*)malloc(nsiz);
        output[nsiz - 1] = 0;
        int pos = 0;

        for (int i = dotLoc + 1; i < length; i++)
        {
            output[pos] = path[i];
            pos++;
        }
    }

    return output;
}


Buffer* ReadFile(char* filepath, const char* readmode, size_t limit)
{
    Buffer* output = NULL;

    FILE* file;

    fopen_s(&file, filepath, readmode);

    if (file)
    {
        size_t fsize = GetFileSize(file);

        if (limit > fsize)
            output = ReadEntireFile(file);
    }

    fclose(file);

    return output;
}


Buffer* ReadFile(char* filepath, const char* readmode, unsigned long long offset, unsigned long long length)
{
    Buffer* output = NULL;

    FILE* file;

    fopen_s(&file, filepath, readmode);

    if (file)
    {
        output = ReadPartialFile(file, offset, length);
    }

    fclose(file);

    return output;
}

size_t WriteFile(char* filepath, Buffer* buffer)
{
    size_t total = 0;
    FILE* file;

    fopen_s(&file, filepath, "w+");

    if (file)
    {
        total = fwrite(buffer->buffer, 1, buffer->count, file);
    }

    fclose(file);

    return total;
}

size_t AppendFile(char* filepath, Buffer* buffer)
{
    size_t total = 0;
    FILE* file;

    fopen_s(&file, filepath, "a+");

    if (file)
    {
        total = fwrite(buffer->buffer, 1, buffer->count, file);
    }

    fclose(file);

    return total;
}
