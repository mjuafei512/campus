# UE5 启动时序图说明

本文档用时序图的方式描述 UE5 客户端从启动到进入游戏场景的完整流程，便于程序员和策划快速理解系统链路。

---

# 1. 总体启动时序

```text
UE5 客户端启动
  -> BP_GameInstance 初始化
  -> 创建网络子系统
  -> 打开登录界面
  -> 用户输入账号密码
  -> 调用 auth-server 登录
  -> 获取 Token / UserId
  -> 连接 game-server TCP
  -> 发送握手 1001
  -> 收到握手响应 1002
  -> 请求角色列表 2001
  -> 展示角色选择 UI
  -> 创建角色 / 选择角色
  -> 发送进入角色请求 2003
  -> 收到进入角色响应 2004
  -> 发送进入场景请求 4001
  -> 收到进入场景响应 4002
  -> 创建 HUD
  -> 启动心跳
  -> 启动移动同步
  -> 进入可玩状态
```

---

# 2. 分阶段时序说明

## 2.1 启动阶段

```text
Player -> UE5 Client
UE5 Client -> BP_GameInstance: Init
BP_GameInstance -> UGameNetworkSubsystem: Create / Initialize
BP_GameInstance -> UI Manager: Open Login Widget
```

### 说明
- 游戏开始时只需要启动基础 UI 和网络对象
- 不要在启动时直接加载游戏场景
- 会话信息此时尚未存在

---

## 2.2 登录阶段

```text
Player -> WBP_Login: Input Username/Password
WBP_Login -> UGameNetworkSubsystem: LoginAndConnect
UGameNetworkSubsystem -> UAuthService: HTTP Login Request
UAuthService -> auth-server: POST /api/auth/login
auth-server -> UAuthService: JWT Response
UAuthService -> UGameNetworkSubsystem: Login Success Callback
UGameNetworkSubsystem -> BP_GameInstance: Save Token/UserId
```

### 说明
- 登录只走 HTTP，不走 TCP
- 登录成功后才能进入 TCP 游戏连接阶段

---

## 2.3 连接与握手阶段

```text
UGameNetworkSubsystem -> UTcpGameClient: Connect(game-server)
UTcpGameClient -> game-server: TCP Connect
UTcpGameClient -> UGameNetworkSubsystem: OnTcpConnected
UGameNetworkSubsystem -> UProtocolManager: BuildHandshakePayload(Token)
UGameNetworkSubsystem -> UTcpGameClient: SendPacket(1001)
game-server -> UTcpGameClient: 1002 Handshake Response
UTcpGameClient -> UGameNetworkSubsystem: OnGameMessage(1002)
```

### 说明
- TCP 连接后必须先握手
- 握手成功后再请求角色列表
- 这一步是游戏协议正式开始的分界线

---

## 2.4 角色选择阶段

```text
UGameNetworkSubsystem -> UTcpGameClient: SendPacket(2001)
game-server -> UTcpGameClient: Character List Response
UTcpGameClient -> UGameNetworkSubsystem: OnGameMessage(2002)
UGameNetworkSubsystem -> WBP_CharacterSelect: Update List

Player -> WBP_CharacterSelect: Click Create/Enter
WBP_CharacterSelect -> UGameNetworkSubsystem: Send Create/Enter Character
```

### 说明
- 角色选择页只负责展示和发出选择命令
- 角色列表数据应由网络层统一分发

---

## 2.5 进入场景阶段

```text
UGameNetworkSubsystem -> UTcpGameClient: SendPacket(2003)
game-server -> UTcpGameClient: Enter Character Response
UTcpGameClient -> UGameNetworkSubsystem: OnGameMessage(2004)
UGameNetworkSubsystem -> UTcpGameClient: SendPacket(4001)
game-server -> UTcpGameClient: Enter Scene Response
UTcpGameClient -> UGameNetworkSubsystem: OnGameMessage(4002)
UGameNetworkSubsystem -> SceneManager: Load Scene
SceneManager -> BP_PlayerCharacter: Spawn
SceneManager -> WBP_HUD: Create / Show
```

### 说明
- 进入角色成功后再进入场景
- 场景进入成功后才开始真正的游戏循环

---

# 3. 正常游戏循环时序

## 3.1 心跳

```text
Timer Tick -> UGameNetworkSubsystem -> SendHeartbeat (3001)
game-server -> UGameNetworkSubsystem: Heartbeat Response
```

## 3.2 移动

```text
BP_PlayerCharacter Move
  -> UPlayerStateSyncComponent
  -> UGameNetworkSubsystem: SendMove
  -> game-server: Position Update
  -> OnGameMessage: Move Response
```

## 3.3 聊天

```text
Player -> WBP_Chat
WBP_Chat -> UGameNetworkSubsystem: SendChat (7001)
game-server -> Broadcast Chat (7003)
UGameNetworkSubsystem -> WBP_Chat: Display Message
```

---

# 4. 断线重连时序

```text
Socket Disconnect Detected
  -> UGameNetworkSubsystem: Enter Reconnecting State
  -> Show Reconnect UI
  -> Close Old Socket
  -> Wait Backoff Time
  -> Reconnect TCP
  -> Send Handshake (1001)
  -> Re-enter Character / Scene if needed
  -> Restore HUD and Sync Timers
  -> Back to InGame
```

### 说明
- 断线后不要直接继续发送玩法包
- 必须先恢复握手和会话
- 恢复后再重新开始移动/心跳同步

---

# 5. 战斗与实体同步时序

```text
Player Attack
  -> SendAttack (5001)
  -> game-server validates
  -> Damage Event (5003)
  -> Update HP bar
  -> If Monster Dead -> 8003
  -> Spawn Drop -> 6001
  -> Pickup -> 6002 / 6003
```

### 说明
- 战斗是高频链路，要确保事件分发及时
- 受击、死亡、掉落要分别处理

---

# 6. 角色状态保存时序

建议在以下时点保存状态：

- 登录成功后：保存 Token / UserId
- 进入角色后：保存 CharacterId
- 进入场景后：保存 SceneId
- 创建/进入队伍后：保存 PartyId
- 进入副本后：保存 InstanceId
- 召唤宠物后：保存 PetId

---

# 7. 蓝图调用链推荐

```text
WBP_Login
  -> LoginAndConnect
  -> OnAuthSuccess
  -> Open WBP_CharacterSelect

WBP_CharacterSelect
  -> Request Character List
  -> Create / Enter Character
  -> OnEnterCharacterSuccess
  -> Send Enter Scene

WBP_HUD
  -> Heartbeat Timer
  -> Movement Sync
  -> Chat / Inventory / Task / Party / Pet / Upgrade Buttons
```

---

# 8. 实际联调建议

建议把调试顺序固定为：

1. 登录成功
2. TCP 连接成功
3. 握手成功
4. 角色列表显示正确
5. 创建/进入角色成功
6. 进入场景成功
7. 心跳正常
8. 移动正常
9. 聊天正常
10. 战斗正常
11. 掉落正常
12. 断线重连正常

---

# 9. 结论

这份时序图说明可以作为 UE5 客户端开发初期的全局参考。建议团队在实现时始终沿着这条时序推进，避免把登录、网络和场景逻辑分散到不可维护的状态。
