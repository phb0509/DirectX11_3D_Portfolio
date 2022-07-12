#include "Framework.h"

BinaryReader::BinaryReader(wstring filePath): 
    mSize(0)
{
    mFile = CreateFile(
        filePath.c_str(), 
        GENERIC_READ,
        FILE_SHARE_READ,
        0, 
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, 
        nullptr);
}

BinaryReader::BinaryReader(string filePath)
    : mSize(0)
{
    mFile = CreateFileA(
        filePath.c_str(), 
        GENERIC_READ,
        FILE_SHARE_READ, 
        0, 
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
}

BinaryReader::~BinaryReader()
{
}

int BinaryReader::Int()
{
    int temp;
    ReadFile(mFile, &temp, sizeof(int), &mSize, nullptr);
    return temp;
}

UINT BinaryReader::UInt()
{
    UINT temp;
    ReadFile(mFile, &temp, sizeof(UINT), &mSize, nullptr);
    return temp;
}

float BinaryReader::Float()
{
    float temp;
    ReadFile(mFile, &temp, sizeof(float), &mSize, nullptr);
    return temp;
}

string BinaryReader::String()
{
    UINT size = UInt();

    char* temp = new char[size + 1];
    ReadFile(mFile, temp, sizeof(char) * size, &this->mSize, nullptr);
    temp[size] = '\0';

    return temp;
}

XMFLOAT4X4 BinaryReader::Float4x4()
{
    XMFLOAT4X4 temp;
    ReadFile(mFile, &temp, sizeof(XMFLOAT4X4), &mSize, nullptr);    

    return temp;
}

void BinaryReader::Byte(void** data, UINT dataSize)
{
    ReadFile(mFile, *data, dataSize, &mSize, nullptr);
}

void BinaryReader::CloseReader()
{
    CloseHandle(mFile);
}
