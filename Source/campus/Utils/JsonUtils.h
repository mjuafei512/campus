#pragma once

#include "CoreMinimal.h"
#include "JsonUtils.generated.h"

UCLASS()
class MYCAMPUSGAME_API UJsonUtils : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Utils")
	static bool ParseJsonString(const FString& JsonString, TSharedPtr<FJsonObject>& OutJsonObject);

	UFUNCTION(BlueprintCallable, Category="Utils")
	static FString ObjectToJsonString(const TSharedPtr<FJsonObject>& JsonObject);

	UFUNCTION(BlueprintCallable, Category="Utils")
	static FString GetStringField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName);

	UFUNCTION(BlueprintCallable, Category="Utils")
	static int64 GetInt64Field(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName);

	UFUNCTION(BlueprintCallable, Category="Utils")
	static bool GetBoolField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName);
};