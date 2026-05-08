# UE5 C++ 网络封装模板

本文提供一个 UE5 客户端网络封装的参考结构，适用于对接当前游戏服务端的 HTTP 登录、TCP 长连接和 Protobuf 协议。

## 1. 目标

封装目标：
- HTTP 登录 / 注册 / 验证
- TCP 长连接
- 游戏协议帧收发
- Protobuf 序列化/反序列化
- 蓝图可调用接口
- 服务端状态回调广播

## 2. 模块划分

建议创建以下 C++ 类：

- `UGameNetworkSubsystem`
- `UAuthService`
- `UTcpGameClient`
- `UProtocolManager`
- `UPlayerStateSyncComponent`

## 3. `UGameNetworkSubsystem`

### 职责
- 保存 token
- 管理网络对象生命周期
- 暴露给蓝图调用

### 示例

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameNetworkSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthSuccess, const FString&, Token, int64, UserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTcpConnected, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameMessage, int32, MsgId, const TArray<uint8>&, Payload);

UCLASS()
class YOURGAME_API UGameNetworkSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnAuthSuccess OnAuthSuccess;

    UPROPERTY(BlueprintAssignable)
    FOnTcpConnected OnTcpConnected;

    UPROPERTY(BlueprintAssignable)
    FOnGameMessage OnGameMessage;

    UFUNCTION(BlueprintCallable)
    void SetSessionToken(const FString& InToken, int64 InUserId);

    UFUNCTION(BlueprintCallable)
    bool LoginAndConnect(const FString& Username, const FString& Password);

    UFUNCTION(BlueprintCallable)
    bool ConnectGameServer();

    UFUNCTION(BlueprintCallable)
    bool SendHandshake();

    UFUNCTION(BlueprintCallable)
    bool SendHeartbeat();

    UFUNCTION(BlueprintCallable)
    FString GetToken() const { return SessionToken; }

private:
    FString SessionToken;
    int64 SessionUserId = 0;
};
```

## 4. `UAuthService`

### 职责
- 调用 `auth-server`
- 处理登录结果

### 示例

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
    void Login(const FString& Username, const FString& Password, FOnLoginResult Callback);
};
```

> 实际实现中建议用 UE HTTP 模块发送 `POST /api/auth/login`。

## 5. `UTcpGameClient`

### 职责
- 建立 socket
- 发送自定义帧
- 接收服务端消息
- 处理拆包粘包

### 帧格式

```text
[Length: 4 bytes][MsgId: 4 bytes][Payload: N bytes]
```

### 示例结构

```cpp
#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
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
    void TickReceive();

    FOnTcpPacket OnPacket;

private:
    FSocket* Socket = nullptr;
    TArray<uint8> ReceiveBuffer;
};
```

## 6. `UProtocolManager`

### 职责
- 构造 protobuf 请求
- 解析 protobuf 响应
- 消息路由

### 示例

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
    TArray<uint8> BuildMovePayload(int64 UserId, float X, float Y, float Z) const;
    void HandlePacket(int32 MsgId, const TArray<uint8>& Payload);
};
```

## 7. `UPlayerStateSyncComponent`

### 职责
- 同步玩家位置
- 同步战斗状态
- 同步任务/社交状态变化

### 示例

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStateSyncComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YOURGAME_API UPlayerStateSyncComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void SyncPosition(float X, float Y, float Z);

    UFUNCTION(BlueprintCallable)
    void SyncCombatState(bool bInCombat, int64 TargetId);
};
```

## 8. Blueprint 调用示例

### 登录蓝图
1. 在 `WBP_Login` 中输入账号密码
2. 调用 `UGameNetworkSubsystem::LoginAndConnect`
3. 登录成功后切到角色选择界面

### 进入游戏蓝图
1. 角色选择完成
2. 调用 `ConnectGameServer`
3. 成功后自动发 `1001` 握手
4. 收到角色列表 `2002`
5. 调用进入角色 `2003`
6. 调用进入场景 `4001`

### 移动同步蓝图
1. 玩家按键移动
2. `Character` 位置变化
3. 触发 `SyncPosition`
4. 发送 `3002` 或对应移动包

### 聊天蓝图
1. 输入聊天内容
2. 调用 `SendChat`
3. 服务端广播 `7003`
4. UI 订阅并显示消息

## 9. 推荐消息流程

### 登录与进入
- `HTTP /api/auth/login`
- `1001` handshake
- `2001` character list
- `2003` enter character
- `4001` enter scene

### 场景内运行
- `3001` heartbeat
- `3002` move
- `7001` chat
- `5001` attack
- `6002` pickup
- `9001` / `9002` / `9003` task

### 进阶玩法
- `11001` / `11003` / `11004` party
- `12001` / `12002` / `12003` instance
- `13001` / `13004` / `13003` pet
- `14001` / `14002` upgrade

## 10. 蓝图事件建议

建议在 C++ 中暴露以下动态事件：

- `OnAuthSuccess`
- `OnTcpConnected`
- `OnHandshakeSuccess`
- `OnCharacterListReceived`
- `OnEnterSceneSuccess`
- `OnChatReceived`
- `OnMonsterDefeated`
- `OnDropPicked`
- `OnTaskUpdated`
- `OnPartyChanged`
- `OnInstanceChanged`
- `OnPetUpdated`
- `OnUpgradeUpdated`

## 11. 实际落地建议

建议 UE 团队按以下顺序开发：
1. HTTP 登录
2. TCP Socket 连接
3. 握手
4. 角色列表 / 进入角色
5. 进入场景
6. 移动 / 心跳 / 聊天
7. 战斗 / 掉落 / 任务
8. 组队 / 副本 / 宠物 / 强化

## 12. 说明

这个模板的目标是帮助 UE5 客户端快速接上当前服务端。实际工程中，你可以将这些类放入：
- `Source/YourGame/Network/`
- `Source/YourGame/Protocol/`
- `Source/YourGame/Subsystems/`
- `Source/YourGame/Components/`

蓝图侧只负责表现层和流程编排。
