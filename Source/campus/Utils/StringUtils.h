#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StringUtils.generated.h"

UCLASS()
class MYCAMPUSGAME_API UStringUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Utils")
	static FString IntToString(int32 Value);

	UFUNCTION(BlueprintPure, Category="Utils")
	static int32 StringToInt(const FString& Value);

	UFUNCTION(BlueprintPure, Category="Utils")
	static FString BytesToHex(const TArray<uint8>& Bytes);

	UFUNCTION(BlueprintPure, Category="Utils")
	static TArray<uint8> HexToBytes(const FString& Hex);

	UFUNCTION(BlueprintPure, Category="Utils")
	static bool IsValidUsername(const FString& Username);
};