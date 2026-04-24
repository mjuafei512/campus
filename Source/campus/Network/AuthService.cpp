#include "Network/AuthService.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UAuthService::SendJsonRequest(const FString& Url, const FString& JsonBody, TFunction<void(bool, const FString&)> OnDone)
{
	FHttpModule& Http = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http.CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonBody);

	Request->OnProcessRequestComplete().BindLambda(
		[OnDone](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk)
		{
			if (!bOk || !Resp.IsValid())
			{
				OnDone(false, TEXT(""));
				return;
			}

			OnDone(true, Resp->GetContentAsString());
		}
	);

	Request->ProcessRequest();
}

bool UAuthService::Login(const FString& Username, const FString& Password, FOnLoginResult Callback)
{
	const FString Body = FString::Printf(TEXT("{\"username\":\"%s\",\"password\":\"%s\"}"), *Username, *Password);

	SendJsonRequest(TEXT("http://localhost:8081/api/auth/login"), Body,
		[Callback](bool bOk, const FString& Response)
		{
			if (!bOk)
			{
				return;
			}

			const FString Token = TEXT("parsed-token");
			const int64 UserId = 1001;
			Callback.ExecuteIfBound(Token, UserId);
		});

	return true;
}

bool UAuthService::Register(const FString& Username, const FString& Password, FOnLoginResult Callback)
{
	const FString Body = FString::Printf(TEXT("{\"username\":\"%s\",\"password\":\"%s\"}"), *Username, *Password);

	SendJsonRequest(TEXT("http://localhost:8081/api/auth/register"), Body,
		[Callback](bool bOk, const FString& Response)
		{
			if (!bOk)
			{
				return;
			}

			Callback.ExecuteIfBound(TEXT(""), 0);
		});

	return true;
}