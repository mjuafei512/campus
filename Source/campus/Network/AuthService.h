#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AuthService.generated.h"

DECLARE_DELEGATE_TwoParams(FOnLoginResult, const FString& /*Token*/, int64 /*UserId*/);

UCLASS()
class MYCAMPUSGAME_API UAuthService : public UObject
{
	GENERATED_BODY()

public:
	bool Login(const FString& Username, const FString& Password, FOnLoginResult Callback);
	bool Register(const FString& Username, const FString& Password, FOnLoginResult Callback);

private:
	void SendJsonRequest(const FString& Url, const FString& JsonBody, TFunction<void(bool, const FString&)> OnDone);
};