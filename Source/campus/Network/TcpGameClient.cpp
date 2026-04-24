#include "Network/TcpGameClient.h"
#include "SocketSubsystem.h"
#include "Common/TcpListener.h"

bool UTcpGameClient::Connect(const FString& Host, int32 Port)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		return false;
	}

	FIPv4Address IpAddress;
	FIPv4Address::Parse(Host, IpAddress);
	TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
	Addr->SetIp(IpAddress.Value);
	Addr->SetPort(Port);

	Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("GameTcpClient"), false);
	if (!Socket)
	{
		return false;
	}

	return Socket->Connect(*Addr);
}

void UTcpGameClient::Disconnect()
{
	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}

bool UTcpGameClient::SendPacket(int32 MsgId, const TArray<uint8>& Payload)
{
	if (!Socket)
	{
		return false;
	}

	const int32 PayloadSize = Payload.Num();
	const int32 Length = 4 + PayloadSize;
	TArray<uint8> Packet;
	Packet.SetNumUninitialized(8 + PayloadSize);

	int32 Offset = 0;
	auto WriteInt32BE = [&Packet, &Offset](int32 Value)
	{
		Packet[Offset++] = (Value >> 24) & 0xFF;
		Packet[Offset++] = (Value >> 16) & 0xFF;
		Packet[Offset++] = (Value >> 8) & 0xFF;
		Packet[Offset++] = Value & 0xFF;
	};

	WriteInt32BE(Length);
	WriteInt32BE(MsgId);
	FMemory::Memcpy(Packet.GetData() + Offset, Payload.GetData(), PayloadSize);

	int32 BytesSent = 0;
	return Socket->Send(Packet.GetData(), Packet.Num(), BytesSent);
}

void UTcpGameClient::PumpReceive()
{
	if (!Socket)
	{
		return;
	}

	uint32 PendingSize = 0;
	while (Socket->HasPendingData(PendingSize))
	{
		int32 ReadSize = FMath::Min<int32>(PendingSize, 65507);
		int32 Offset = ReceiveBuffer.AddUninitialized(ReadSize);
		int32 BytesRead = 0;
		Socket->Recv(ReceiveBuffer.GetData() + Offset, ReadSize, BytesRead);
	}
	ParseBuffer();
}

bool UTcpGameClient::ReadExact(uint8* Dest, int32 NumBytes)
{
	if (!Socket)
	{
		return false;
	}

	int32 BytesRead = 0;
	return Socket->Recv(Dest, NumBytes, BytesRead);
}

void UTcpGameClient::ParseBuffer()
{
	while (ReceiveBuffer.Num() >= 8)
	{
		auto ReadInt32BE = [this](int32 Index) -> int32
		{
			return (ReceiveBuffer[Index] << 24) |
				(ReceiveBuffer[Index + 1] << 16) |
				(ReceiveBuffer[Index + 2] << 8) |
				(ReceiveBuffer[Index + 3]);
		};

		const int32 Length = ReadInt32BE(0);
		if (ReceiveBuffer.Num() < 4 + Length)
		{
			return;
		}

		const int32 MsgId = ReadInt32BE(4);
		const int32 PayloadLen = Length - 4;

		TArray<uint8> Payload;
		Payload.Append(ReceiveBuffer.GetData() + 8, PayloadLen);

		OnPacket.ExecuteIfBound(MsgId, Payload);

		ReceiveBuffer.RemoveAt(0, 8 + PayloadLen, false);
	}
}