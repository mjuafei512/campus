#include "Utils/JsonUtils.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool UJsonUtils::ParseJsonString(const FString& JsonString, TSharedPtr<FJsonObject>& OutJsonObject)
{
	TSharedRef<TJsonReader<>> Reader = TJsonReader<>::Create(JsonString);
	return FJsonSerializer::Deserialize(Reader, OutJsonObject);
}

FString UJsonUtils::ObjectToJsonString(const TSharedPtr<FJsonObject>& JsonObject)
{
	return FJsonSerializer::Serialize(JsonObject.ToSharedRef(), EJsonScheme::Json);
}

FString UJsonUtils::GetStringField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
	if (!JsonObject.IsValid())
	{
		return TEXT("");
	}

	const TSharedPtr<FJsonValue>* Value = JsonObject->Values.Find(FieldName);
	if (Value && (*Value)->Type == EJson::String)
	{
		return (*Value)->AsString();
	}
	return TEXT("");
}

int64 UJsonUtils::GetInt64Field(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
	if (!JsonObject.IsValid())
	{
		return 0;
	}

	const TSharedPtr<FJsonValue>* Value = JsonObject->Values.Find(FieldName);
	if (Value && (*Value)->Type == EJson::Number)
	{
		return static_cast<int64>((*Value)->AsNumber());
	}
	return 0;
}

bool UJsonUtils::GetBoolField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonValue>* Value = JsonObject->Values.Find(FieldName);
	if (Value && (*Value)->Type == EJson::Boolean)
	{
		return (*Value)->AsBool();
	}
	return false;
}