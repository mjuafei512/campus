# UE5 C++ 代码骨架

下面给出一套可以直接复制到 UE5 工程中的网络层代码骨架，用于对接当前游戏服务端。

> 说明：
> - 这里使用 `YOURGAME_API` 作为模块导出宏，实际请替换成你的项目宏。
> - protobuf 相关类型名需要与你在服务端 `shared-proto` 中保持一致。
> - 代码以“能落地实现”为目标，省略了部分错误处理细节，但保留了核心结构。

---

## 1. 目录建议

建议在 UE 工程中创建如下目录：

```text
Source/YourGame/
  Network/
    GameNetworkSubsystem.h
    GameNetworkSubsystem.cpp
    TcpGameClient.h
    TcpGameClient.cpp
    ProtocolManager.h
    ProtocolManager.cpp
    AuthService.h
    AuthService.cpp
  Subsystems/
  Components/
  UI/
```

---

## 2. `UGameNetworkSubsystem`

### `GameNetworkSubsystem.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameNetworkSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthSuccess, const FString&, Token, int64, UserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTcpConnected, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameMessage, int32, MsgId, const TArray<uint8>&, Payload);

class UTcpGameClient;
class UAuthService;
class UProtocolManager;

UCLASS()
class YOURGAME_API UGameNetworkSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable)
    FOnAuthSuccess OnAuthSuccess;

    UPROPERTY(BlueprintAssignable)
    FOnTcpConnected OnTcpConnected;

    UPROPERTY(BlueprintAssignable)
    FOnGameMessage OnGameMessage;

    UFUNCTION(BlueprintCallable, Category="Network")
    void SetSessionToken(const FString& InToken, int64 InUserId);

    UFUNCTION(BlueprintCallable, Category="Network")
    void ClearSession();

    UFUNCTION(BlueprintCallable, Category="Network")
    bool LoginAndConnect(const FString& Username, const FString& Password);

    UFUNCTION(BlueprintCallable, Category="Network")
    bool ConnectGameServer();

    UFUNCTION(BlueprintCallable, Category="Network")
    bool SendHandshake();

    UFUNCTION(BlueprintCallable, Category="Network")
    bool SendHeartbeat();

    UFUNCTION(BlueprintCallable, Category="Network")
    bool SendChat(const FString& Content, int32 Channel = 1);

    UFUNCTION(BlueprintCallable, Category="Network")
    FString GetToken() const { return SessionToken; }

    UFUNCTION(BlueprintCallable, Category="Network")
    int64 GetUserId() const { return SessionUserId; }

private:
    FString SessionToken;
    int64 SessionUserId = 0;

    UPROPERTY()
    TObjectPtr<UTcpGameClient> TcpClient;

    UPROPERTY()
    TObjectPtr<UAuthService> AuthService;

    UPROPERTY()
    TObjectPtr<UProtocolManager> ProtocolManager;

    void HandleTcpPacket(int32 MsgId, const TArray<uint8>& Payload);
};
```

### `GameNetworkSubsystem.cpp`

```cpp
#include "Network/GameNetworkSubsystem.h"
#include "Network/AuthService.h"
#include "Network/ProtocolManager.h"
#include "Network/TcpGameClient.h"
#include "Engine/Engine.h"

void UGameNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    AuthService = NewObject<UAuthService>(this);
    ProtocolManager = NewObject<UProtocolManager>(this);
    TcpClient = NewObject<UTcpGameClient>(this);

    if (TcpClient)
    {
        TcpClient->OnPacket.BindUObject(this, &UGameNetworkSubsystem::HandleTcpPacket);
    }
}

void UGameNetworkSubsystem::Deinitialize()
{
    if (TcpClient)
    {
        TcpClient->Disconnect();
    }

    Super::Deinitialize();
}

void UGameNetworkSubsystem::SetSessionToken(const FString& InToken, int64 InUserId)
{
    SessionToken = InToken;
    SessionUserId = InUserId;
    OnAuthSuccess.Broadcast(SessionToken, SessionUserId);
}

void UGameNetworkSubsystem::ClearSession()
{
    SessionToken.Empty();
    SessionUserId = 0;
}

bool UGameNetworkSubsystem::LoginAndConnect(const FString& Username, const FString& Password)
{
    if (!AuthService)
    {
        return false;
    }

    return AuthService->Login(
        Username,
        Password,
        FOnLoginResult::CreateLambda([this](const FString& Token, int64 UserId)
        {
            SetSessionToken(Token, UserId);
            ConnectGameServer();
        })
    );
}

bool UGameNetworkSubsystem::ConnectGameServer()
{
    if (!TcpClient)
    {
        return false;
    }

    const bool bConnected = TcpClient->Connect(TEXT("127.0.0.1"), 9000);
    OnTcpConnected.Broadcast(bConnected);

    if (bConnected)
    {
        SendHandshake();
    }

    return bConnected;
}

bool UGameNetworkSubsystem::SendHandshake()
{
    if (!TcpClient || !ProtocolManager || SessionToken.IsEmpty())
    {
        return false;
    }

    const TArray<uint8> Payload = ProtocolManager->BuildHandshakePayload(SessionToken);
    return TcpClient->SendPacket(1001, Payload);
}

bool UGameNetworkSubsystem::SendHeartbeat()
{
    if (!TcpClient || SessionUserId <= 0)
    {
        return false;
    }

    const TArray<uint8> Payload = ProtocolManager->BuildHeartbeatPayload(SessionUserId);
    return TcpClient->SendPacket(3001, Payload);
}

bool UGameNetworkSubsystem::SendChat(const FString& Content, int32 Channel)
{
    if (!TcpClient || !ProtocolManager || SessionUserId <= 0)
    {
        return false;
    }

    const TArray<uint8> Payload = ProtocolManager->BuildChatPayload(SessionUserId, Content, Channel);
    return TcpClient->SendPacket(7001, Payload);
}

void UGameNetworkSubsystem::HandleTcpPacket(int32 MsgId, const TArray<uint8>& Payload)
{
    OnGameMessage.Broadcast(MsgId, Payload);
}
```

---

## 3. `UAuthService`

### `AuthService.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AuthService.generated.h"

DECLARE_DELEGATE_TwoParams(FOnLoginResult, const FString& /*Token*/, int64 /*UserId*/);

UCLASS()
class YOURGAME_API UAuthService : public UObject
{
    GENERATED_BODY()

public:
    bool Login(const FString& Username, const FString& Password, FOnLoginResult Callback);
    bool Register(const FString& Username, const FString& Password, FOnLoginResult Callback);

private:
    void SendJsonRequest(const FString& Url, const FString& JsonBody, TFunction<void(bool, const FString&)> OnDone);
};
```

### `AuthService.cpp`

```cpp
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

            // 实际项目里请解析 JSON 响应中的 token / userId
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
```

---

## 4. `UTcpGameClient`

### `TcpGameClient.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Sockets.h"
#include "TcpGameClient.generated.h"

DECLARE_DELEGATE_TwoParams(FOnTcpPacket, int32 /*MsgId*/, const TArray<uint8>& /*Payload*/);

UCLASS()
class YOURGAME_API UTcpGameClient : public UObject
{
    GENERATED_BODY()

public:
    bool Connect(const FString& Host, int32 Port);
    void Disconnect();
    bool SendPacket(int32 MsgId, const TArray<uint8>& Payload);
    void PumpReceive();

    FOnTcpPacket OnPacket;

private:
    FSocket* Socket = nullptr;
    TArray<uint8> ReceiveBuffer;

    bool ReadExact(uint8* Dest, int32 NumBytes);
    void ParseBuffer();
};
```

### `TcpGameClient.cpp`

```cpp
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
```

---

## 5. `UProtocolManager`

### `ProtocolManager.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ProtocolManager.generated.h"

UCLASS()
class YOURGAME_API UProtocolManager : public UObject
{
    GENERATED_BODY()

public:
    TArray<uint8> BuildHandshakePayload(const FString& Token) const;
    TArray<uint8> BuildHeartbeatPayload(int64 UserId) const;
    TArray<uint8> BuildChatPayload(int64 UserId, const FString& Content, int32 Channel) const;
    TArray<uint8> BuildMovePayload(int64 UserId, float X, float Y, float Z) const;
};
```

### `ProtocolManager.cpp`

```cpp
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
    // HandshakeRequest.token = 2
    WriteStringField(Out, 2, Token);
    return Out;
}

TArray<uint8> UProtocolManager::BuildHeartbeatPayload(int64 UserId) const
{
    TArray<uint8> Out;
    // HeartbeatRequest.userId = 1
    WriteInt64Field(Out, 1, UserId);
    return Out;
}

TArray<uint8> UProtocolManager::BuildChatPayload(int64 UserId, const FString& Content, int32 Channel) const
{
    TArray<uint8> Out;
    WriteInt64Field(Out, 1, UserId);
    WriteInt64Field(Out, 2, 1001); // characterId 示例，实际应使用当前角色 ID
    WriteInt64Field(Out, 3, Channel);
    WriteStringField(Out, 4, Content);
    return Out;
}

TArray<uint8> UProtocolManager::BuildMovePayload(int64 UserId, float X, float Y, float Z) const
{
    TArray<uint8> Out;
    WriteInt64Field(Out, 1, UserId);
    // 这里为了演示，移动字段建议使用真正的 protobuf float 编码
    // 实际项目请用 protobuf 生成代码直接序列化
    return Out;
}
```

---

## 6. 蓝图调用示例

### 登录按钮蓝图
```text
WBP_Login -> OnClicked(Login)
  -> Get GameInstanceSubsystem
  -> Call LoginAndConnect(Username, Password)
  -> OnAuthSuccess / OnTcpConnected
  -> Open CharacterSelect Widget
```

### 场景同步蓝图
```text
BP_PlayerCharacter -> Tick / Timer
  -> Compare Current Position with Last Position
  -> If Changed, call SendMove
  -> Every N seconds call SendHeartbeat
```

### 聊天蓝图
```text
WBP_Chat -> OnClicked(Send)
  -> Get GameNetworkSubsystem
  -> Call SendChat(Content, Channel)
```

### 监听服务端消息
```text
GameNetworkSubsystem -> OnGameMessage
  -> Switch MsgId
  -> 1002: Handshake Response
  -> 2004: Enter Character Response
  -> 4002: Enter Scene Response
  -> 7003: Chat Broadcast
  -> 5003: Damage Event
```

---

## 7. 服务端协议对接建议

建议 UE5 客户端至少先接通以下消息：

- `1001` / `1002`：握手
- `2001` / `2002` / `2004`：角色列表与进入角色
- `4001` / `4002`：进入场景
- `3001` / `3002` / `3003`：心跳与移动
- `7001` / `7002` / `7003`：聊天
- `5001` / `5002` / `5003`：战斗
- `6001` / `6002` / `6003`：掉落
- `9001`~`9004`：任务
- `11001`~`11005`：组队
- `12001`~`12005`：副本
- `13001`~`13006`：宠物
- `14001`~`14004`：强化

---

## 8. 下一步建议

如果你要真正开工，我建议下一步把这份骨架继续细化成：
- UE5 登录界面的蓝图流程图
- UE5 TCP 收发包的完整代码
- Protobuf 生成与 UE 集成说明
- 角色选择/场景加载/聊天/战斗的蓝图节点图

这会让 UE 程序员可以直接照着实现。
