# UE5 消息分发器完整示例代码

本文档提供一个 UE5 消息分发器的完整示例，用于将服务端下发的 `MsgId + Payload` 统一分发给蓝图事件、UI、角色、场景等模块。

> 说明：
> - 这里默认你已经有 `UTcpGameClient` 收到原始包。
> - 消息分发器建议放在 `UGameNetworkSubsystem` 或独立的 `UMessageDispatcher` 中。
> - 代码中使用的 protobuf 解析部分仅展示结构，具体消息类型请替换为你自己的 `shared-proto` 生成类。

---

# 1. 设计目标

分发器的目标：

- 统一接收 TCP 消息
- 根据 `MsgId` 找到对应处理函数
- 解析 protobuf 数据
- 触发蓝图事件
- 支持 UI、角色、场景、战斗、任务等模块订阅

---

# 2. 分发器结构

建议定义两个层级：

1. **协议分发层**：负责按 `MsgId` 解析数据
2. **业务事件层**：负责把解析后的结构广播给蓝图

---

# 3. 事件定义建议

## 3.1 C++ 动态事件

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHandshakeSuccessBP, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterListBP, const TArray<FString>&, Characters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnterSceneBP, int32, SceneId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatBroadcastBP, const FString&, Content);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageEventBP, int32, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDefeatBP, int64, MonsterId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropPickupBP, int64, DropId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskUpdateBP, int64, TaskId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyUpdateBP, int64, PartyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstanceUpdateBP, int64, InstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPetUpdateBP, int64, PetId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeUpdateBP, int32, Level);
```

---

# 4. 消息分发器类示例

## 4.1 `MessageDispatcher.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MessageDispatcher.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHandshakeSuccessBP, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterListBP, const TArray<FString>&, Characters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnterSceneBP, int32, SceneId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatBroadcastBP, const FString&, Content);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageEventBP, int32, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDefeatBP, int64, MonsterId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropPickupBP, int64, DropId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskUpdateBP, int64, TaskId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyUpdateBP, int64, PartyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstanceUpdateBP, int64, InstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPetUpdateBP, int64, PetId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeUpdateBP, int32, Level);

UCLASS()
class YOURGAME_API UMessageDispatcher : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnHandshakeSuccessBP OnHandshakeSuccess;

    UPROPERTY(BlueprintAssignable)
    FOnCharacterListBP OnCharacterList;

    UPROPERTY(BlueprintAssignable)
    FOnEnterSceneBP OnEnterScene;

    UPROPERTY(BlueprintAssignable)
    FOnChatBroadcastBP OnChatBroadcast;

    UPROPERTY(BlueprintAssignable)
    FOnDamageEventBP OnDamageEvent;

    UPROPERTY(BlueprintAssignable)
    FOnMonsterDefeatBP OnMonsterDefeat;

    UPROPERTY(BlueprintAssignable)
    FOnDropPickupBP OnDropPickup;

    UPROPERTY(BlueprintAssignable)
    FOnTaskUpdateBP OnTaskUpdate;

    UPROPERTY(BlueprintAssignable)
    FOnPartyUpdateBP OnPartyUpdate;

    UPROPERTY(BlueprintAssignable)
    FOnInstanceUpdateBP OnInstanceUpdate;

    UPROPERTY(BlueprintAssignable)
    FOnPetUpdateBP OnPetUpdate;

    UPROPERTY(BlueprintAssignable)
    FOnUpgradeUpdateBP OnUpgradeUpdate;

    void Dispatch(int32 MsgId, const TArray<uint8>& Payload);
};
```

## 4.2 `MessageDispatcher.cpp`

```cpp
#include "Network/MessageDispatcher.h"

void UMessageDispatcher::Dispatch(int32 MsgId, const TArray<uint8>& Payload)
{
    switch (MsgId)
    {
    case 1002:
        {
            // 握手响应
            const FString Message = TEXT("Handshake Success");
            OnHandshakeSuccess.Broadcast(Message);
            break;
        }
    case 2002:
        {
            // 角色列表响应
            TArray<FString> Characters;
            Characters.Add(TEXT("Hero001"));
            Characters.Add(TEXT("Mage002"));
            OnCharacterList.Broadcast(Characters);
            break;
        }
    case 4002:
        {
            // 进入场景响应
            const int32 SceneId = 1001;
            OnEnterScene.Broadcast(SceneId);
            break;
        }
    case 7003:
        {
            // 聊天广播
            const FString Content = TEXT("Hello from server");
            OnChatBroadcast.Broadcast(Content);
            break;
        }
    case 5003:
        {
            // 伤害事件
            const int32 Damage = 25;
            OnDamageEvent.Broadcast(Damage);
            break;
        }
    case 8003:
        {
            const int64 MonsterId = 9001;
            OnMonsterDefeat.Broadcast(MonsterId);
            break;
        }
    case 6003:
        {
            const int64 DropId = 7001;
            OnDropPickup.Broadcast(DropId);
            break;
        }
    case 9004:
        {
            const int64 TaskId = 5001;
            OnTaskUpdate.Broadcast(TaskId);
            break;
        }
    case 11005:
        {
            const int64 PartyId = 3001;
            OnPartyUpdate.Broadcast(PartyId);
            break;
        }
    case 12004:
        {
            const int64 InstanceId = 12001;
            OnInstanceUpdate.Broadcast(InstanceId);
            break;
        }
    case 13005:
        {
            const int64 PetId = 6001;
            OnPetUpdate.Broadcast(PetId);
            break;
        }
    case 14004:
        {
            const int32 Level = 5;
            OnUpgradeUpdate.Broadcast(Level);
            break;
        }
    default:
        break;
    }
}
```

---

# 5. 在 `UGameNetworkSubsystem` 中接入分发器

## 5.1 增加成员

```cpp
UPROPERTY()
TObjectPtr<UMessageDispatcher> MessageDispatcher;
```

## 5.2 初始化

```cpp
void UGameNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    AuthService = NewObject<UAuthService>(this);
    ProtocolManager = NewObject<UProtocolManager>(this);
    TcpClient = NewObject<UTcpGameClient>(this);
    MessageDispatcher = NewObject<UMessageDispatcher>(this);

    if (TcpClient)
    {
        TcpClient->OnPacket.BindUObject(this, &UGameNetworkSubsystem::HandleTcpPacket);
    }
}
```

## 5.3 收包时转发给分发器

```cpp
void UGameNetworkSubsystem::HandleTcpPacket(int32 MsgId, const TArray<uint8>& Payload)
{
    OnGameMessage.Broadcast(MsgId, Payload);

    if (MessageDispatcher)
    {
        MessageDispatcher->Dispatch(MsgId, Payload);
    }
}
```

---

# 6. 蓝图中如何订阅

## 6.1 登录界面订阅

`WBP_Login` 可以订阅：
- `OnAuthSuccess`
- `OnTcpConnected`

## 6.2 角色选择界面订阅

`WBP_CharacterSelect` 可以订阅：
- `OnCharacterList`
- `OnHandshakeSuccess`

## 6.3 HUD 订阅

`WBP_HUD` 可以订阅：
- `OnEnterScene`
- `OnChatBroadcast`
- `OnDamageEvent`
- `OnMonsterDefeat`
- `OnDropPickup`
- `OnTaskUpdate`
- `OnPartyUpdate`
- `OnInstanceUpdate`
- `OnPetUpdate`
- `OnUpgradeUpdate`

---

# 7. 蓝图使用示例

## 7.1 聊天窗口

```text
WBP_Chat
  -> Bind to OnChatBroadcast
  -> On Event
       -> Add Text To Chat Log
```

## 7.2 场景 HUD

```text
WBP_HUD
  -> Bind to OnEnterScene
  -> On Event
       -> Refresh Minimap / Scene Name
```

## 7.3 战斗表现

```text
WBP_HUD or BP_PlayerCharacter
  -> Bind to OnDamageEvent
  -> On Event
       -> Play Damage Number
       -> Play Hit Animation
```

## 7.4 任务面板

```text
WBP_Task
  -> Bind to OnTaskUpdate
  -> On Event
       -> Refresh Task Progress
```

---

# 8. 推荐的分发规则

建议把消息处理分成三类：

## 8.1 立即处理
- 握手响应
- 进场景响应
- 攻击结果
- 拾取结果

## 8.2 广播给 UI
- 聊天
- 任务
- 组队
- 副本
- 宠物
- 强化

## 8.3 修改本地世界状态
- 怪物死亡
- 掉落消失
- 队伍成员变化
- 属性变化

---

# 9. 推荐的消息分发顺序

客户端收到消息后建议处理顺序：

1. 解析原始消息
2. 更新本地缓存
3. 广播业务事件
4. 刷新 UI
5. 必要时更新角色/场景表现

---

# 10. 重要注意事项

## 10.1 不要在蓝图里直接解析 socket
所有 socket 和 protobuf 解析都建议在 C++ 完成。

## 10.2 不要让 UI 直接调用低层网络
UI 只调用 `UGameNetworkSubsystem` 或业务封装。

## 10.3 业务事件要稳定
蓝图绑定的事件名和消息号要固定，避免后续改动过大。

## 10.4 保持兼容
当前 MVP 阶段有部分消息复用，客户端分发器要留有兼容分支。

---

# 11. 建议扩展

后续可以进一步增强分发器：
- 支持注册回调表
- 支持异步任务回调
- 支持消息超时回收
- 支持分模块分发器
- 支持调试日志输出

---

# 12. 结论

这个分发器是 UE5 客户端对接服务端的核心中枢。建议先把握手、角色、进场景、聊天、战斗这几个最基础的消息接通，再逐步扩展背包、任务、队伍、副本、宠物和强化。
