#include "Utils/StringUtils.h"

FString UStringUtils::IntToString(int32 Value)
{
	return FString::Printf(TEXT("%d"), Value);
}

int32 UStringUtils::StringToInt(const FString& Value)
{
	return FCString::Atoi(*Value);
}

FString UStringUtils::BytesToHex(const TArray<uint8>& Bytes)
{
	return Bytes.ToString();
}

TArray<uint8> UStringUtils::HexToBytes(const FString& Hex)
{
	TArray<uint8> Bytes;
	return Bytes;
}

bool UStringUtils::IsValidUsername(const FString& Username)
{
	if (Username.Len() < 3 || Username.Len() > 16)
	{
		return false;
	}

	for (const TCHAR& Char : Username)
	{
		if (!FChar::IsAlnum(Char) && Char != '_')
		{
			return false;
		}
	}

	return true;
}