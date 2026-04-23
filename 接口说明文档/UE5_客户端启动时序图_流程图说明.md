# UE5 客户端启动时序图 / 流程图说明

本文档把 UE5 客户端从启动到进入场景、同步运行、断线重连的完整时序串联起来，方便开发、联调与验收。

---

# 1. 启动总览

```text
[启动游戏]
    -> 初始化 GameInstance
    -> 创建 NetworkSubsystem
    -> 打开登录界面
    -> 用户登录
    -> HTTP 登录成功
    -> 保存 Token / UserId
    -> TCP 连接 game-server
    -> 发送握手 1001
    -> 收到握手响应 1002
    -> 请求角色列表 2001
    -> 创建 / 选择角色
    -> 进入角色 2003
    -> 进入场景 4001
    -> 收到场景响应 4002
    -> 创建 HUD
    -> 启动心跳
    -> 启动移动同步
    -> 正常游戏运行
```

---

# 2. 启动阶段时序

## 2.1 引擎启动

```text
UE5 启动
  -> Load Config
  -> Init GameInstance
  -> Init GameNetworkSubsystem
  -> Init UI Manager
  -> Open WBP_Login
```

### 关键点
- `GameInstance` 必须先准备好全局状态
- 网络子系统在这里创建，以便整个生命周期可复用

---

## 2.2 登录阶段

```text
WBP_Login
  -> 输入账号密码
  -> 点击登录按钮
  -> 调用 LoginAndConnect
  -> auth-server 返回 token / userId
  -> 保存会话
  -> 进入 TCP 连接阶段
```

### 时序说明
```text
Player -> Login UI -> AuthService -> auth-server
AuthService -> Login Result -> GameNetworkSubsystem
GameNetworkSubsystem -> 保存 Token / UserId -> 连接 game-server
```

---

# 3. TCP 连接与握手时序

## 3.1 TCP 连接建立

```text
GameNetworkSubsystem
  -> ConnectGameServer
  -> TcpGameClient.Connect(host, port)
  -> Socket Connected
  -> OnTcpConnected(True)
```

## 3.2 握手消息

```text
GameNetworkSubsystem
  -> SendHandshake(1001)
  -> TcpGameClient.SendPacket
  -> game-server 处理握手
  -> 返回 1002
  -> C++ 消息分发器收到 1002
  -> 蓝图收到握手成功事件
```

### 握手作用
- 确认 token 有效
- 确认 TCP 通道有效
- 确认游戏服可用

---

# 4. 角色选择阶段时序

## 4.1 请求角色列表

```text
Handshake Success
  -> RequestCharacterList(2001)
  -> game-server 返回角色列表(2002)
  -> CharacterSelect UI 刷新列表
```

## 4.2 创建角色

```text
WBP_CharacterSelect
  -> 输入角色名
  -> 点击创建
  -> SendCreateCharacter(2003)
  -> 返回创建结果(2004)
  -> 刷新角色列表
```

## 4.3 进入角色

```text
WBP_CharacterSelect
  -> 选中某角色
  -> SendEnterCharacter(2003)
  -> 返回进入成功(2004)
  -> 准备进入场景
```

> 注：当前 MVP 协议中部分消息号在实现里存在复用，客户端需要按返回内容进一步判断。

---

# 5. 进入场景时序

## 5.1 请求进入场景

```text
EnterCharacter Success
  -> SendEnterScene(4001)
  -> game-server 返回场景信息(4002)
```

## 5.2 场景加载

```text
OnGameMessage(4002)
  -> Parse EnterSceneResponse
  -> Set Current SceneId
  -> Clear Old Scene Entities
  -> Load Map
  -> Spawn Player Pawn
  -> Spawn Nearby NPC / Monsters / Drops / Pets
  -> Create HUD
```

### 进入场景后必须做的事情
- 显示 UI
- 开启心跳
- 开启移动同步
- 开启实体同步
- 允许玩家输入

---

# 6. 场景运行时序

## 6.1 心跳时序

```text
Timer Tick
  -> SendHeartbeat(3001)
  -> Wait for Server Response
  -> If Timeout -> Network Error Handling
```

## 6.2 移动同步时序

```text
Player Move
  -> Detect Position Change
  -> SendMove(3002)
  -> game-server 更新状态
  -> 必要时修正客户端位置
```

## 6.3 聊天时序

```text
WBP_Chat -> SendChat(7001)
  -> game-server 广播(7003)
  -> 所有客户端更新聊天窗口
```

## 6.4 战斗时序

```text
AttackButton
  -> SendAttack(5001)
  -> game-server 计算伤害
  -> 返回攻击结果(5002)
  -> 广播伤害事件(5003)
  -> 客户端播放受击/飘字/死亡动画
```

---

# 7. 玩法系统时序

## 7.1 掉落与拾取

```text
Monster Defeated
  -> Server Spawn Drop(6001)
  -> Client Spawn Drop Actor
  -> Player Interact Drop
  -> Send Pickup Request(6002)
  -> Server Confirm Pickup(6003)
  -> Refresh Inventory UI
```

## 7.2 任务

```text
Accept Task -> 9001
Update Progress -> 9002
Submit Task -> 9003
Task Result -> 9004
```

## 7.3 队伍

```text
Create Party -> 11001
Invite / Join / Leave -> 11003 / 11004
Party State Update -> 11002 ~ 11005
```

## 7.4 副本

```text
Create Instance -> 12001
Start Wave -> 12002
Complete Instance -> 12004
Enter Party Instance -> 12005
```

## 7.5 宠物

```text
Summon Pet -> 13001
Recall / Follow -> 13003 / 13004 / 13006
Pet Assist Event -> 13005
```

## 7.6 强化

```text
Enhance Item -> 14001
Reinforce Item -> 14002
Upgrade Result -> 14003
Attribute Update -> 14004
```

---

# 8. 断线重连时序

## 8.1 断线检测

```text
No Heartbeat / Socket Closed / Read Fail
  -> 判定断线
  -> 显示重连 UI
  -> 进入 Reconnecting 状态
```

## 8.2 自动重连

```text
Reconnecting
  -> Close Old Socket
  -> Wait Backoff Delay
  -> Reconnect TCP
  -> Send Handshake
  -> Re-enter Character if needed
  -> Re-enter Scene if needed
  -> Restore UI / Gameplay State
```

### 重连时建议恢复内容
- Token
- UserId
- CharacterId
- SceneId
- PartyId
- InstanceId
- PetId
- UI 状态

---

# 9. 处理分层时序

## 9.1 C++ 负责
- 网络状态机
- 协议解析
- 消息分发
- 断线重连
- 状态恢复

## 9.2 蓝图负责
- UI 显示
- 动画播放
- 特效表现
- 场景切换表现
- 交互反馈

---

# 10. 时序图文字版

```text
启动
  -> 登录界面
    -> HTTP 登录
      -> 保存 token
        -> TCP 连接
          -> 握手
            -> 角色列表
              -> 创建/选择角色
                -> 进入场景
                  -> HUD 初始化
                    -> 心跳
                    -> 移动
                    -> 聊天
                    -> 战斗
                    -> 掉落
                    -> 任务
                    -> 组队
                    -> 副本
                    -> 宠物
                    -> 强化
                    -> 若断线
                        -> 自动重连
                        -> 恢复状态
```

---

# 11. 验收建议

客户端完成以下时序即可认为主链路跑通：

- [ ] 能登录
- [ ] 能建立 TCP
- [ ] 能握手
- [ ] 能请求角色列表
- [ ] 能创建/选择角色
- [ ] 能进入场景
- [ ] 能发心跳
- [ ] 能同步移动
- [ ] 能聊天
- [ ] 能战斗
- [ ] 能拾取掉落
- [ ] 能接任务
- [ ] 能组队
- [ ] 能进副本
- [ ] 能召唤宠物
- [ ] 能强化装备
- [ ] 能断线重连并恢复状态

---

# 12. 结论

这份时序说明可以帮助 UE 团队把整个客户端主流程串起来。建议先按“登录 -> 握手 -> 角色 -> 进场景 -> 心跳/移动”做最小闭环，再扩展到其他玩法系统。
