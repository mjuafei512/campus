#include "Network/ProtocolManager.h"

static void WriteVarInt(TArray<uint8>& Out, uint32 Value)
{
	while (Value >= 0x80)
	{
		Out.Add(static_cast<uint8>((Value & 0x7F) | 0x80));
		Value >>= 7;
	}
	Out.Add(static_cast<uint8>(Value));
}

static void WriteStringField(TArray<uint8>& Out, uint32 FieldNumber, const FString& Value)
{
	const uint32 Tag = (FieldNumber << 3) | 2;
	WriteVarInt(Out, Tag);

	FTCHARToUTF8 Utf8(*Value);
	WriteVarInt(Out, static_cast<uint32>(Utf8.Length()));
	Out.Append(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
}

static void WriteInt64Field(TArray<uint8>& Out, uint32 FieldNumber, int64 Value)
{
	const uint32 Tag = (FieldNumber << 3) | 0;
	WriteVarInt(Out, Tag);
	WriteVarInt(Out, static_cast<uint32>(Value));
}

TArray<uint8> UProtocolManager::BuildHandshakePayload(const FString& Token) const
{
	TArray<uint8> Out;
	WriteStringField(Out, 2, Token);
	return Out;
}

TArray<uint8> UProtocolManager::BuildHeartbeatPayload(int64 UserId) const
{
	TArray<uint8> Out;
	WriteInt64Field(Out, 1, UserId);
	return Out;
}

TArray<uint8> UProtocolManager::BuildChatPayload(int64 UserId, const FString& Content, int32 Channel) const
{
	TArray<uint8> Out;
	WriteInt64Field(Out, 1, UserId);
	WriteInt64Field(Out, 2, 1001);
	WriteInt64Field(Out, 3, Channel);
	WriteStringField(Out, 4, Content);
	return Out;
}

TArray<uint8> UProtocolManager::BuildMovePayload(int64 UserId, float X, float Y, float Z) const
{
	TArray<uint8> Out;
	WriteInt64Field(Out, 1, UserId);
	return Out;
}