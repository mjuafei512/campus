# UE5 断线重连与超时机制文档

本文档用于指导 UE5 客户端在接入当前游戏服务端时，实现稳定的 TCP 连接维护、心跳监测、断线重连和超时处理。

---

# 1. 目标

客户端需要具备以下能力：

- TCP 连接成功后稳定维持会话
- 定时发送心跳包
- 判断服务端是否长时间无响应
- 网络中断后自动重连
- 重连后自动恢复登录态和场景态
- 防止重复创建连接、重复发送消息

---

# 2. 基本原则

## 2.1 状态驱动
建议在 UE5 中维护一个明确的网络状态机：

- `Disconnected`
- `Connecting`
- `Connected`
- `Handshaking`
- `Authenticated`
- `InGame`
- `Reconnecting`
- `Closing`

## 2.2 单连接原则
同一时刻只允许存在一个有效 TCP 连接。

## 2.3 心跳原则
客户端应定时向 `game-server` 发送心跳，防止连接因空闲被中断。

---

# 3. 网络状态机设计

建议在 `UGameNetworkSubsystem` 或独立网络管理器中定义如下状态：

```cpp
enum class EGameNetworkState : uint8
{
    Disconnected,
    Connecting,
    Connected,
    Handshaking,
    Authenticated,
    InGame,
    Reconnecting,
    Closing
};
```

---

# 4. 连接建立流程

## 4.1 首次登录流程
1. 调用 `auth-server` 登录
2. 拿到 `token` 和 `userId`
3. 建立 TCP 连接
4. 状态切换到 `Connecting`
5. 连接成功后切换到 `Connected`
6. 发送握手消息 `1001`
7. 收到 `1002` 后切换到 `Authenticated`
8. 请求角色列表、进入角色、进入场景
9. 状态切换到 `InGame`

## 4.2 状态流转
```text
Disconnected -> Connecting -> Connected -> Handshaking -> Authenticated -> InGame
```

---

# 5. 心跳机制

## 5.1 发送频率
建议每 10~30 秒发送一次心跳。

## 5.2 心跳包
当前服务端中建议使用：
- `3001` 作为客户端心跳请求

## 5.3 心跳超时
如果超过阈值时间未收到服务端响应，应视为连接异常。

建议阈值：
- 3 次连续超时
- 或 1 次超时超过 60 秒

## 5.4 心跳流程
```text
Timer Tick
  -> If NetworkState == InGame
       -> SendHeartbeat
       -> Start Response Timeout Timer
```

---

# 6. 超时检测机制

## 6.1 发送超时
如果调用 `SendPacket` 后长期未收到响应，记录为发送超时。

## 6.2 接收超时
如果服务端长时间没有任何消息返回，记录为接收超时。

## 6.3 断线判定
满足以下任一条件，判定为断线：
- Socket 关闭
- 读写失败
- 连续心跳无响应
- 收包解析异常导致连接终止

---

# 7. 自动重连策略

## 7.1 触发条件
检测到断线后，应自动进入重连流程。

## 7.2 重连步骤
1. 保存当前会话信息
   - `Token`
   - `UserId`
   - `CharacterId`
   - `SceneId`
   - `PartyId`
   - `InstanceId`
2. 断开旧连接
3. 等待短暂延时
4. 重新建立 TCP 连接
5. 重新发送握手 `1001`
6. 重新进入角色 / 场景
7. 恢复 UI 状态

## 7.3 重连间隔
建议采用指数退避：
- 第 1 次：1 秒
- 第 2 次：2 秒
- 第 3 次：4 秒
- 第 4 次：8 秒
- 上限：30 秒

---

# 8. 重连后的恢复逻辑

客户端断线后重连，建议恢复以下内容：

## 8.1 会话态
- Token
- UserId
- CharacterId
- SceneId
- PartyId
- InstanceId
- PetId

## 8.2 场景态
- 当前地图
- 玩家位置
- 当前目标
- 当前任务进度
- 背包 UI 状态
- 组队成员列表
- 宠物状态

## 8.3 恢复流程
```text
Reconnect Success
  -> SendHandshake
  -> Re-request Character List if needed
  -> Re-enter Character
  -> Re-enter Scene
  -> Refresh UI State
```

---

# 9. 服务端异常处理建议

当服务端返回错误或断开时，建议客户端分类处理：

## 9.1 认证失败
- token 失效
- token 被拒绝
- 账号禁用

处理方式：
- 回到登录页
- 清除本地 token

## 9.2 场景错误
- 场景不存在
- 进入角色失败
- 副本失效

处理方式：
- 提示错误
- 回到角色选择或大厅

## 9.3 网络错误
- socket 断开
- 收包异常
- 心跳超时

处理方式：
- 显示“网络断开，正在重连”
- 自动重连

---

# 10. UE5 蓝图中的处理建议

## 10.1 在 `BP_GameInstance` 保存状态
建议保存：
- `CurrentNetworkState`
- `LastHeartbeatTime`
- `ReconnectCount`
- `SessionToken`
- `UserId`
- `CharacterId`

## 10.2 蓝图事件
建议 C++ 暴露以下事件：
- `OnConnected`
- `OnDisconnected`
- `OnReconnectStarted`
- `OnReconnectSuccess`
- `OnReconnectFailed`
- `OnHeartbeatTimeout`

## 10.3 UI 表现
建议在 HUD 中显示：
- 当前网络状态
- 当前连接服务器
- 重连提示
- 心跳状态

---

# 11. 推荐的实现方式

## 11.1 C++ 负责
- Socket 创建/关闭
- 心跳定时器
- 网络状态机
- 超时判断
- 自动重连
- 消息恢复

## 11.2 蓝图负责
- UI 提示
- 重连动画
- 错误弹窗
- 重新进入场景后的 UI 刷新

---

# 12. 重连示意流程

```text
[网络正常]
  -> 定时心跳
  -> 接收消息
  -> 正常游戏

[网络异常]
  -> 检测断线
  -> 提示重连中
  -> 关闭旧连接
  -> 延时重试
  -> 重连成功
  -> 重新握手
  -> 恢复场景

[重连失败]
  -> 提示无法连接
  -> 返回登录页或大厅
```

---

# 13. 建议的超时阈值

| 项目 | 建议值 |
| --- | --- |
| 心跳间隔 | 10~30 秒 |
| 心跳超时 | 60 秒以内 |
| 重连首次等待 | 1 秒 |
| 重连最大等待 | 30 秒 |
| 连续失败次数 | 5 次以上提示用户 |

---

# 14. 与当前服务端的关系

当前服务端已经支持：
- TCP 长连接
- 握手
- 心跳
- 场景进入
- 角色进入
- 游戏状态消息

因此 UE5 客户端只要做好：
- 心跳发送
- 超时判断
- 断线重连
- 状态恢复

就可以稳定对接。

---

# 15. 最佳实践总结

1. **不要把 socket 逻辑写到蓝图里**
2. **所有网络状态变化都要有统一状态机**
3. **断线重连后不要直接假设状态有效，要重新握手**
4. **恢复 UI 前先恢复网络会话**
5. **心跳和重连必须做定时器**
6. **异常处理要区分认证失败和网络失败**

---

# 16. 结论

这份文档的目标是让 UE5 客户端在真实网络环境下更稳定地接入服务端。建议先实现“心跳 + 断线检测 + 自动重连 + 会话恢复”四件事，再逐步扩展到战斗和副本等高频交互功能。
