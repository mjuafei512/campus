# UE5 客户端接入编程指南

本文档面向 UE5 客户端程序员，说明如何对接当前游戏服务端的 HTTP 登录、TCP 长连接、Protobuf 协议、状态汇报与服务端状态接收。

## 1. 服务端概览

当前服务端分为两部分：

- `auth-server`
  - 提供 HTTP 登录/注册/JWT 验证
- `game-server`
  - 提供 TCP 长连接
  - 使用自定义 TCP 帧 + Protobuf 进行消息交互
  - 负责角色、场景、聊天、战斗、背包、任务、组队、副本、宠物、强化等逻辑

## 2. UE5 客户端分层建议

建议将 UE5 客户端拆成三层：

### 2.1 网络底层层（C++）
职责：
- HTTP 请求
- TCP 连接
- 收发帧
- Protobuf 编解码
- 心跳
- 重连

### 2.2 协议封装层（C++）
职责：
- 构造请求消息
- 解析响应消息
- 消息分发
- 事件广播

### 2.3 业务表现层（蓝图）
职责：
- UI
- 角色表现
- 动画/特效/音效
- 场景加载
- 战斗表现
- 任务/背包/社交界面

## 3. 推荐 UE 类设计

### 3.1 `UGameNetworkSubsystem`
建议继承 `UGameInstanceSubsystem`。

用途：
- 保存登录 token
- 管理 TCP 生命周期
- 提供蓝图调用入口

### 3.2 `UAuthService`
用途：
- 调用登录、注册、验证接口
- 返回 token 和 userId

### 3.3 `UTcpGameClient`
用途：
- 建立 TCP 连接
- 发送/接收游戏帧
- 处理粘包拆包

### 3.4 `UProtocolManager`
用途：
- 构造 protobuf 请求
- 反序列化响应
- 按 msgId 分发事件

### 3.5 `UPlayerStateSyncComponent`
用途：
- 玩家状态同步
- 位置、旋转、HP、场景、战斗状态

## 4. 连接流程

### 4.1 HTTP 登录
1. 输入账号密码
2. 调用 `auth-server`
3. 成功后拿到 JWT token
4. 保存 token 到 `GameInstance` 或 `Subsystem`

### 4.2 TCP 连接
1. 使用 token 连接 `game-server`
2. 连接成功后发握手包 `1001`
3. 收到 `1002` 后进入角色流程
4. 请求角色列表 `2001`
5. 选择角色后发 `2003`
6. 进入场景后发 `4001`

## 5. TCP 帧格式

服务端使用的帧格式：

```text
[Length: 4 bytes][MsgId: 4 bytes][Payload: N bytes]
```

其中：
- `Length = 4 + Payload.Length`
- `MsgId` 为消息号
- `Payload` 是 Protobuf 序列化结果

## 6. 状态汇报建议

UE 客户端建议汇报：
- 位置
- 朝向
- 当前场景
- 战斗状态
- 当前目标
- 心跳
- 聊天
- 拾取
- 任务进度
- 队伍/宠物/强化状态变化

推荐汇报频率：
- 心跳：10~30 秒
- 移动：位置变化时
- 战斗：状态变化时
- 任务/社交：事件发生时

## 7. 服务端状态接收建议

客户端要监听以下典型推送：
- `1002` 握手响应
- `2004` 进入角色响应
- `4002` 进入场景响应
- `5003` 伤害事件
- `7003` 聊天广播
- `8003` 怪物死亡事件
- `6003` 拾取结果
- `9004` 任务结果
- `11002`~`11005` 队伍事件
- `12002`~`12005` 副本事件
- `13005`~`13006` 宠物事件
- `14003`~`14004` 强化与属性事件

## 8. 蓝图组织建议

### UI 蓝图
- `WBP_Login`
- `WBP_CharacterSelect`
- `WBP_HUD`
- `WBP_Inventory`
- `WBP_Task`
- `WBP_Party`
- `WBP_Chat`

### 角色蓝图
- `BP_PlayerCharacter`
- `BP_PlayerController`
- `BP_PlayerState`
- `BP_GameState`

### 业务管理蓝图
- `BP_NetworkManager`
- `BP_ProtocolManager`

## 9. 蓝图调用建议

蓝图只负责：
- 发起登录
- 连接服务器
- 点击角色进入
- 触发战斗/聊天/拾取/任务交互
- 展示服务端事件

不要在蓝图中直接处理 socket 和 protobuf，底层建议由 C++ 封装。

## 10. 开发顺序建议

1. 先实现 HTTP 登录
2. 再实现 TCP 连接与握手
3. 再实现角色列表和进入场景
4. 再实现移动、聊天、战斗
5. 再接背包、任务、组队、副本、宠物、强化
6. 最后做重连、断线恢复和 UI 完整联动

## 11. 与当前服务端的适配结论

当前服务端已经非常适合 UE5 接入，因为它具备：
- HTTP 登录
- JWT
- TCP 长连接
- 自定义消息帧
- Protobuf 协议
- 模块化玩法逻辑

UE5 只需要完成：
- 登录
- TCP Socket
- 帧解析
- Protobuf 编解码
- 消息分发
- 蓝图事件绑定

即可对接。
