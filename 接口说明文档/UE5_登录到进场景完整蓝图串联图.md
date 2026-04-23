# UE5 登录到进场景完整蓝图串联图

本文档把 UE5 客户端从启动、登录、握手、角色选择、进入角色到进入场景的完整流程串成一条可执行的蓝图链路，适合程序员直接照着搭建。

---

# 1. 总体流程图

```text
启动游戏
  -> 初始化 GameInstance
  -> 创建网络子系统
  -> 打开登录 UI
  -> 登录成功
  -> 保存 Token / UserId
  -> 连接游戏服 TCP
  -> 握手
  -> 请求角色列表
  -> 创建 / 选择角色
  -> 进入角色
  -> 进入场景
  -> 创建 HUD
  -> 启动心跳 / 移动同步
```

---

# 2. 蓝图节点串联总览

## 2.1 `BP_GameInstance`

### 负责
- 游戏启动初始化
- 创建网络管理器
- 保存会话信息
- 切换登录/角色选择/游戏主界面

### 节点流程
```text
Event Init
  -> Get or Create GameNetworkSubsystem
  -> Create Widget (WBP_Login)
  -> Add To Viewport
```

---

## 2.2 `WBP_Login`

### 负责
- 输入用户名密码
- 点击登录
- 触发 HTTP 登录

### 节点流程
```text
OnClicked(LoginButton)
  -> Get GameNetworkSubsystem
  -> Call LoginAndConnect(Username, Password)
  -> Show Loading State
```

### 登录成功回调
```text
OnAuthSuccess(Token, UserId)
  -> Save Token / UserId
  -> Remove Login Widget
  -> Create Widget (WBP_CharacterSelect)
  -> Add To Viewport
```

---

## 2.3 `UGameNetworkSubsystem`

### 负责
- HTTP 登录结果处理
- TCP 连接
- 握手
- 游戏消息事件广播

### 关键流程
```text
LoginAndConnect
  -> AuthService.Login
  -> OnLoginSuccess
  -> Set Token / UserId
  -> ConnectGameServer
  -> SendHandshake
```

---

# 3. 角色选择流程

## 3.1 请求角色列表

当进入角色选择界面后，调用：

```text
SendCharacterListRequest
```

如果当前模板中还未封装，可在 `UProtocolManager` 中添加该函数。

### 蓝图流程
```text
WBP_CharacterSelect::Event Construct
  -> Get GameNetworkSubsystem
  -> Call RequestCharacterList
```

### 收到角色列表
```text
OnGameMessage(MsgId=2002)
  -> Parse CharacterListResponse
  -> Clear Old UI
  -> For Each Character
       -> Create CharacterEntry Widget
       -> Fill Name / Level / Scene
       -> Add To ScrollBox
```

---

## 3.2 创建角色

### 蓝图流程
```text
OnClicked(CreateCharacterButton)
  -> Read CharacterName
  -> Call SendCreateCharacter(CharacterName)
  -> Show Loading
```

### 收到创建结果
```text
OnGameMessage(MsgId=2004)
  -> Parse CreateCharacterResponse
  -> If Success
       -> Refresh Character List
  -> Else
       -> Show Error Text
```

---

## 3.3 进入角色

### 蓝图流程
```text
OnClicked(EnterCharacterButton)
  -> Save Selected CharacterId
  -> Call SendEnterCharacter(CharacterId)
```

### 收到进入角色结果
```text
OnGameMessage(MsgId=2004)
  -> Parse EnterCharacterResponse
  -> If Success
       -> Proceed to Enter Scene
```

> 注：当前服务端实现里角色创建/进入响应消息号可能存在复用情况，UE 侧建议在解析时结合返回内容区分。

---

# 4. 进入场景流程

## 4.1 调用进入场景

当角色进入成功后：

```text
Call SendEnterScene(SceneId)
```

### 节点串联
```text
EnterCharacterSuccess
  -> Get Current SceneId
  -> Call SendEnterScene(SceneId)
  -> Wait for MsgId 4002
```

---

## 4.2 进入场景成功后

### 收到 `4002`
```text
OnGameMessage(MsgId=4002)
  -> Parse EnterSceneResponse
  -> Save SceneId
  -> Load Level / Open World Map
  -> Spawn Player Pawn
  -> Spawn Nearby Entities
  -> Create WBP_HUD
  -> Start Heartbeat Timer
  -> Start Movement Sync Timer
```

---

# 5. 主界面与 HUD 初始化

## 5.1 创建 HUD

建议在收到场景进入成功后：

```text
Create Widget (WBP_HUD)
  -> Add To Viewport
```

HUD 中建议包含：
- 角色血条
- 蓝条
- 背包按钮
- 聊天按钮
- 任务按钮
- 队伍按钮
- 副本按钮
- 宠物按钮
- 强化按钮

---

# 6. 心跳与移动同步

## 6.1 心跳

### 节点流程
```text
Set Timer by Event (10s~30s)
  -> Call SendHeartbeat
```

### 建议实现位置
- `BP_GameInstance`
- 或 `UGameNetworkSubsystem`

---

## 6.2 移动同步

### 节点流程
```text
BP_PlayerCharacter::Tick
  -> Compare Current Location and Last Location
  -> If Dist > Threshold
       -> Call SendMove
       -> Update Last Location
```

### 建议
- 不要每帧都发包
- 只在位置变化显著时同步

---

# 7. 蓝图事件总线串联

建议在 `UGameNetworkSubsystem` 中绑定这些事件：

- `OnAuthSuccess`
- `OnTcpConnected`
- `OnGameMessage`
- `OnReconnectStarted`
- `OnReconnectSuccess`
- `OnReconnectFailed`

### 蓝图监听方式
```text
Bind Event to OnAuthSuccess
Bind Event to OnTcpConnected
Bind Event to OnGameMessage
```

---

# 8. 推荐的蓝图图结构

## 8.1 登录蓝图图

```text
WBP_Login
  -> 输入账号密码
  -> 点击登录
  -> 调用 LoginAndConnect
  -> OnAuthSuccess
  -> 打开角色选择界面
```

## 8.2 角色选择蓝图图

```text
WBP_CharacterSelect
  -> 请求角色列表
  -> 显示角色卡片
  -> 创建角色
  -> 进入角色
  -> 进入场景
```

## 8.3 场景主循环蓝图图

```text
WBP_HUD + BP_PlayerCharacter
  -> 心跳
  -> 移动同步
  -> 聊天
  -> 战斗
  -> 背包
  -> 任务
```

---

# 9. 消息接收与界面切换

### 接收到不同消息时的行为

| MsgId | 触发动作 |
| --- | --- |
| `1002` | 握手成功，准备请求角色列表 |
| `2002` | 刷新角色列表 UI |
| `2004` | 创建/进入角色成功 |
| `4002` | 进入场景，创建 HUD |
| `7003` | 更新聊天窗口 |
| `5003` | 更新战斗状态 |
| `8003` | 怪物死亡，处理掉落 |
| `6003` | 拾取掉落，刷新背包 |
| `9004` | 刷新任务进度 |
| `11002~11005` | 刷新队伍 UI |
| `12002~12005` | 刷新副本 UI |
| `13005~13006` | 刷新宠物 UI |
| `14003~14004` | 刷新强化与属性面板 |

---

# 10. 断线重连在流程中的位置

当网络断开时：

```text
OnDisconnected
  -> Show Reconnecting UI
  -> Attempt Reconnect
  -> OnReconnectSuccess
       -> Re-send Handshake
       -> Re-enter Character if needed
       -> Re-enter Scene if needed
       -> Restore HUD and UI
```

---

# 11. 蓝图中的状态保存

建议在 `BP_GameInstance` 中保存：
- `Token`
- `UserId`
- `CharacterId`
- `SceneId`
- `PartyId`
- `InstanceId`
- `PetId`
- `CurrentNetworkState`

这样断线重连后能快速恢复。

---

# 12. 实战开发顺序

建议 UE 团队按这条串联图实现：

1. 登录 UI
2. 连接后发送握手
3. 请求角色列表
4. 创建/选择角色
5. 进入场景
6. HUD 初始化
7. 心跳
8. 移动同步
9. 聊天
10. 战斗
11. 背包
12. 任务
13. 队伍/副本/宠物/强化

---

# 13. 结论

这份串联图是 UE5 客户端接入服务端的“主流程骨架”。按照这条流程实现，能很快跑通一个最小可玩的在线闭环：
- 登录
- 进服
- 选角
- 入场景
- 同步状态
- 开始交互
