# UE5 蓝图节点流程图说明

本文档用于指导 UE5 程序员在蓝图中搭建登录、握手、角色选择、场景进入、状态汇报、消息接收等完整流程。

> 说明：
> - 网络底层建议由 C++ 封装，蓝图只做流程编排和 UI 表现。
> - 本文中的“节点流程图”以文字步骤的方式描述蓝图节点连接逻辑，方便直接实现。

---

# 1. 蓝图工程分层建议

建议把蓝图分成以下几层：

- `UI`：登录、角色选择、HUD、背包、任务、聊天、队伍、副本、宠物、强化
- `Player`：角色移动、状态同步、战斗表现
- `System`：网络管理、协议分发、场景切换
- `World`：怪物、掉落、NPC、场景实体

建议创建这些蓝图：

- `BP_GameInstance`
- `BP_PlayerController`
- `BP_PlayerCharacter`
- `BP_GameState`
- `WBP_Login`
- `WBP_CharacterSelect`
- `WBP_HUD`
- `WBP_Chat`
- `WBP_Inventory`
- `WBP_Task`
- `WBP_Party`
- `WBP_Instance`
- `WBP_Pet`
- `WBP_Upgrade`

---

# 2. 登录页蓝图流程

## 2.1 登录 UI 节点流程

### 蓝图逻辑
1. 打开 `WBP_Login`
2. 输入用户名和密码
3. 点击“登录”按钮
4. 调用 `LoginAndConnect` 或 `Login`
5. 监听登录成功回调
6. 保存 `Token`
7. 保存 `UserId`
8. 跳转到角色选择界面

### 节点示例

```text
OnClicked(LoginButton)
  -> Get GameInstanceSubsystem
  -> Call LoginAndConnect(Username, Password)
  -> Bind OnAuthSuccess
  -> On Success:
       Set Token
       Set UserId
       Remove Login Widget
       Create CharacterSelect Widget
       Add To Viewport
```

## 2.2 登录失败处理

### 建议节点
```text
OnLoginFailed
  -> Show Error Text
  -> Enable Login Button
  -> Clear Loading State
```

---

# 3. 连接游戏服与握手流程

## 3.1 连接流程

登录成功后，执行：

1. `ConnectGameServer`
2. 连接成功后，自动发送 `1001` 握手
3. 等待 `1002` 握手响应
4. 握手成功后，发送角色列表请求 `2001`

### 节点流程
```text
OnAuthSuccess
  -> Call ConnectGameServer
  -> OnTcpConnected(True)
       -> Call SendHandshake
       -> Wait for MsgId 1002
```

## 3.2 握手成功后的流程

```text
OnGameMessage(MsgId=1002)
  -> Parse HandshakeResponse
  -> If Success
       -> Call RequestCharacterList
  -> Else
       -> Show Disconnect/Error
```

---

# 4. 角色选择界面流程

## 4.1 获取角色列表

建议在握手成功后请求：

- `2001`：角色列表请求
- 收到角色列表后显示在 `WBP_CharacterSelect`

### 节点流程
```text
OnGameMessage(MsgId=2002)
  -> Parse CharacterListResponse
  -> Clear Old Character Entries
  -> For Each Character
       -> Create Character Entry Widget
       -> Fill Name / Level / Scene
       -> Add To ScrollBox
```

## 4.2 创建角色

### 节点流程
```text
OnClicked(CreateCharacterButton)
  -> Read Input Name
  -> Call SendCreateCharacter
  -> Wait for MsgId 2004
```

### 创建成功后
```text
OnGameMessage(MsgId=2004)
  -> Parse CreateCharacterResponse
  -> Refresh Character List
```

## 4.3 进入角色

### 节点流程
```text
OnClicked(SelectCharacterButton)
  -> Store Selected CharacterId
  -> Call SendEnterCharacter(CharacterId)
  -> Wait for MsgId 2004 or corresponding enter response
```

---

# 5. 进入场景流程

进入角色后，继续进入场景：

### 节点流程
```text
OnEnterCharacterSuccess
  -> Call SendEnterScene(SceneId)
  -> Wait for MsgId 4002
```

### 场景进入成功后
```text
OnGameMessage(MsgId=4002)
  -> Parse EnterSceneResponse
  -> Load Level / Open World Map
  -> Spawn Player Character
  -> Spawn Nearby Entities
  -> Create HUD
```

---

# 6. 场景内状态同步流程

## 6.1 心跳

建议使用 Timer 每隔 10~30 秒发一次心跳。

### 节点流程
```text
BeginPlay
  -> Set Timer by Event (10s)
  -> Call SendHeartbeat
```

### 心跳消息
- 请求：`3001`
- 响应：`3002` 或当前服务端对应心跳响应

## 6.2 移动同步

### 推荐逻辑
当角色位置变化时发送移动消息：

```text
Tick / Timer
  -> Compare Current Location and Last Location
  -> If Changed
       -> Call SendMove
       -> Update Last Location
```

### 节点建议
```text
OnCharacterMovementUpdated
  -> If Distance > Threshold
       -> SendMove(UserId, X, Y, Z)
```

## 6.3 状态汇报内容
建议客户端同步：
- 位置
- 朝向
- 当前场景
- 战斗状态
- 当前目标
- 当前任务
- 队伍状态
- 宠物状态

---

# 7. 聊天流程

### 节点流程
```text
WBP_Chat.OnClicked(Send)
  -> Read Content
  -> Call SendChat(Content, Channel)
```

### 服务端回调
```text
OnGameMessage(MsgId=7003)
  -> Parse ChatBroadcast
  -> Add Chat Line To ChatBox
```

---

# 8. 战斗流程

## 8.1 发起攻击

### 节点流程
```text
OnClicked(AttackButton)
  -> Get Current TargetId
  -> Call SendAttack(TargetId, SkillId)
  -> Wait for MsgId 5002 / 5003
```

## 8.2 服务端伤害回调

```text
OnGameMessage(MsgId=5003)
  -> Parse DamageEvent
  -> Play Hit Effect
  -> Update Target HP Bar
```

## 8.3 怪物死亡

```text
OnGameMessage(MsgId=8003)
  -> Parse MonsterDefeatEvent
  -> Play Death Animation
  -> Spawn Loot Drop
```

---

# 9. 掉落与拾取流程

## 9.1 掉落出现

```text
OnGameMessage(MsgId=6001)
  -> Parse SpawnDropResponse
  -> Spawn Drop Actor
```

## 9.2 拾取掉落

```text
OnInteractDrop
  -> Call SendPickupDrop(DropId)
  -> Wait for MsgId 6003
```

### 拾取后更新背包
```text
OnGameMessage(MsgId=6003)
  -> Parse PickupDropResponse
  -> Refresh Inventory UI
```

---

# 10. 任务流程

## 10.1 接任务

```text
WBP_Task.OnClicked(Accept)
  -> Call SendAcceptTask(TaskId)
  -> Wait for MsgId 9002
```

## 10.2 更新任务进度

```text
OnMonsterKilled / OnItemCollected / OnObjectiveCompleted
  -> Call SendUpdateTaskProgress
```

## 10.3 提交任务

```text
OnTaskComplete
  -> Call SendSubmitTask
  -> Wait for MsgId 9004 / corresponding submit response
```

---

# 11. 组队流程

## 11.1 创建队伍

```text
WBP_Party.OnClicked(Create)
  -> Call CreateParty(Name)
  -> Wait for response
```

## 11.2 邀请/加入/退出

```text
InviteButton -> SendInviteParty
JoinButton   -> SendJoinParty
LeaveButton  -> SendLeaveParty
```

### 接收队伍变化
```text
OnGameMessage(MsgId=11002~11005)
  -> Refresh Party UI
  -> Update Party Member List
```

---

# 12. 副本流程

## 12.1 创建副本

```text
WBP_Instance.OnClicked(CreateInstance)
  -> Call CreateInstance(SceneId, Name)
```

## 12.2 进入队伍副本

```text
WBP_Instance.OnClicked(EnterPartyInstance)
  -> Call EnterPartyInstance(PartyId, SceneId)
```

## 12.3 波次与完成

```text
OnGameMessage(MsgId=12002)
  -> Update Wave UI

OnGameMessage(MsgId=12004)
  -> Mark Instance Completed
```

---

# 13. 宠物流程

## 13.1 召唤宠物

```text
WBP_Pet.OnClicked(Summon)
  -> Call SummonPet(Name)
```

## 13.2 跟随/召回

```text
FollowButton -> SendPetFollow(True)
RecallButton -> SendRecallPet
```

## 13.3 宠物协战

```text
OnGameMessage(MsgId=13005)
  -> Play Pet Assist Effect
  -> Show Pet Damage Float
```

---

# 14. 强化流程

## 14.1 强化操作

```text
WBP_Upgrade.OnClicked(Enhance)
  -> Call EnhanceItem(ItemId, CurrentLevel)
```

## 14.2 强化结果

```text
OnGameMessage(MsgId=14002 / 14003)
  -> Update Upgrade UI
  -> Play Success / Fail Animation
```

## 14.3 属性更新

```text
OnGameMessage(MsgId=14004)
  -> Refresh Character Attribute Panel
```

---

# 15. 蓝图事件总线建议

建议在 `UGameNetworkSubsystem` 或 `BP_GameInstance` 中建立事件分发：

- `OnAuthSuccess`
- `OnTcpConnected`
- `OnHandshakeSuccess`
- `OnCharacterListReceived`
- `OnEnterSceneSuccess`
- `OnChatReceived`
- `OnDamageReceived`
- `OnMonsterDefeated`
- `OnDropSpawned`
- `OnDropPicked`
- `OnTaskUpdated`
- `OnPartyUpdated`
- `OnInstanceUpdated`
- `OnPetUpdated`
- `OnUpgradeUpdated`

这样 UI 和场景逻辑就能分别订阅。

---

# 16. 蓝图中的状态管理建议

建议在 `BP_GameInstance` 保存以下状态：

- `Token`
- `UserId`
- `CharacterId`
- `SceneId`
- `PartyId`
- `InstanceId`
- `PetId`
- `CurrentTargetId`

这样切换关卡时不会丢失核心信息。

---

# 17. 节点连接的核心原则

1. **网络调用放 C++**
2. **UI 流程放蓝图**
3. **协议解析放 C++**
4. **表现和交互放蓝图**
5. **状态同步用事件驱动**

---

# 18. 推荐开发顺序

UE5 团队建议按这个顺序实现：

1. 登录 UI
2. 角色选择 UI
3. TCP 握手
4. 进场景
5. 心跳 / 移动
6. 聊天
7. 战斗
8. 掉落 / 拾取
9. 背包 / 任务
10. 队伍 / 副本
11. 宠物 / 强化

---

# 19. 最终建议

如果你要让 UE 团队直接开工，建议把这份文档和前一份 C++ 骨架一起发给程序员，并要求先实现：

- HTTP 登录
- TCP 握手
- 角色选择
- 进场景
- 心跳与移动

这是整个客户端接入的最小闭环。
