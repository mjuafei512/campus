# UE5 客户端整体目录结构建议

本文档建议 UE5 客户端工程的目录组织方式，目标是方便网络层、协议层、UI 层、角色层、场景层、战斗层独立维护。

---

# 1. 总体目标

建议将 UE5 工程按照“职责分层 + 功能模块”拆分，避免所有逻辑堆在蓝图或单个类中。

推荐原则：
- **网络层 C++ 化**
- **协议层 C++ 化**
- **表现层蓝图化**
- **系统层模块化**
- **UI 与场景逻辑分离**

---

# 2. 推荐工程目录

建议在 `Source/YourGame/` 下建立如下结构：

```text
Source/YourGame/
  Core/
    YourGame.Build.cs
    YourGameModule.cpp
    YourGameModule.h

  Network/
    GameNetworkSubsystem.h
    GameNetworkSubsystem.cpp
    TcpGameClient.h
    TcpGameClient.cpp
    AuthService.h
    AuthService.cpp
    ProtocolManager.h
    ProtocolManager.cpp
    MessageDispatcher.h
    MessageDispatcher.cpp
    NetworkTypes.h

  Protobuf/
    Generated/
    ProtobufBridge.h
    ProtobufBridge.cpp

  Player/
    PlayerCharacter.h
    PlayerCharacter.cpp
    PlayerController.h
    PlayerController.cpp
    PlayerState.h
    PlayerState.cpp
    PlayerStateSyncComponent.h
    PlayerStateSyncComponent.cpp

  Scene/
    SceneManagerSubsystem.h
    SceneManagerSubsystem.cpp
    SceneEntityManager.h
    SceneEntityManager.cpp
    SceneEntityData.h
    SceneSnapshot.h

  UI/
    Login/
      WBP_Login
      WBP_LoginController.cpp
    CharacterSelect/
      WBP_CharacterSelect
      WBP_CharacterEntry
    HUD/
      WBP_HUD
      WBP_Chat
      WBP_Inventory
      WBP_Task
      WBP_Party
      WBP_Instance
      WBP_Pet
      WBP_Upgrade

  World/
    Monsters/
      BP_Monster
      MonsterComponent.h
      MonsterComponent.cpp
    Drops/
      BP_DropItem
    Pets/
      BP_Pet
    NPC/
      BP_NPC

  Combat/
    CombatComponent.h
    CombatComponent.cpp
    CombatTypes.h

  Data/
    DataTables/
    GameConfigTypes.h
    RuntimeCache.h

  Utils/
    JsonUtils.h
    StringUtils.h
    TimeUtils.h
```

---

# 3. 目录职责说明

## 3.1 `Core/`
放引擎入口、模块初始化、全局基础类。

建议包含：
- 模块启动/关闭
- 全局单例或注册器
- 通用宏和基础定义

---

## 3.2 `Network/`
放所有网络相关逻辑。

包含：
- HTTP 登录
- TCP 连接
- 收发包
- 消息分发
- 协议管理
- 网络状态管理

这是最重要的 C++ 层之一。

---

## 3.3 `Protobuf/`
放 protobuf 生成代码和桥接层。

包含：
- 生成目录
- `ProtobufBridge`
- 数据转蓝图结构
- 蓝图和 C++ 之间的对象转换

---

## 3.4 `Player/`
放玩家相关逻辑。

包含：
- `PlayerCharacter`
- `PlayerController`
- `PlayerState`
- 角色状态同步组件

---

## 3.5 `Scene/`
放场景管理。

包含：
- 场景缓存
- 实体管理
- 进入/退出场景
- 实体生成和销毁

---

## 3.6 `UI/`
放所有 UI。

建议按功能再拆子目录：
- 登录
- 角色选择
- HUD
- 背包
- 聊天
- 队伍
- 副本
- 宠物
- 强化

---

## 3.7 `World/`
放游戏世界实体蓝图。

包含：
- 怪物
- 掉落
- 宠物
- NPC

---

## 3.8 `Combat/`
放战斗相关逻辑。

包含：
- 攻击判定
- 技能释放
- 受击反馈
- 血条刷新

---

## 3.9 `Data/`
放数据表和运行时配置。

建议用于：
- 场景配置
- 怪物配置
- 技能配置
- 背包配置
- 任务配置

---

## 3.10 `Utils/`
放通用工具函数。

包含：
- JSON 处理
- 字符串处理
- 时间处理
- 协议辅助函数

---

# 4. 蓝图与 C++ 的边界建议

## 4.1 建议 C++ 负责
- TCP 连接
- HTTP 登录
- 消息分发
- protobuf 编解码
- 心跳和超时
- 场景缓存
- 实体缓存
- 自动重连

## 4.2 建议蓝图负责
- UI 呈现
- 动画播放
- 特效表现
- 交互按钮
- 关卡切换
- 角色移动表现

---

# 5. 推荐核心类位置

| 类 | 建议位置 | 职责 |
| --- | --- | --- |
| `UGameNetworkSubsystem` | `Network/` | 全局网络管理 |
| `UTcpGameClient` | `Network/` | TCP 长连接 |
| `UAuthService` | `Network/` | HTTP 登录 |
| `UProtocolManager` | `Network/` 或 `Protobuf/` | 协议打包 |
| `UMessageDispatcher` | `Network/` | 消息分发 |
| `UPlayerStateSyncComponent` | `Player/` | 玩家状态同步 |
| `USceneEntityManager` | `Scene/` | 场景实体管理 |

---

# 6. 建议的数据结构目录

可以单独放：

```text
Source/YourGame/Data/Types/
  GameTypes.h
  NetworkTypes.h
  SceneTypes.h
  UIDataTypes.h
```

这样便于复用：
- 玩家结构
- 场景实体结构
- 任务结构
- 掉落结构
- 队伍结构

---

# 7. 资源目录建议

```text
Content/
  Blueprints/
  UI/
  Characters/
  Monsters/
  Pets/
  Drops/
  NPC/
  Maps/
  Materials/
  Niagara/
  Sounds/
  DataTables/
```

---

# 8. 版本控制建议

建议按模块提交：
- 第一批：网络层
- 第二批：登录和角色选择
- 第三批：场景和实体
- 第四批：战斗和掉落
- 第五批：背包/任务/队伍
- 第六批：副本/宠物/强化

这样回滚和排查更容易。

---

# 9. 典型的工程调用链

```text
GameInstance
  -> NetworkSubsystem
       -> AuthService
       -> TcpGameClient
       -> ProtocolManager
       -> MessageDispatcher
  -> SceneManagerSubsystem
  -> UI Widgets
  -> PlayerController
  -> PlayerCharacter
```

---

# 10. 新人开发建议

如果有新的 UE 程序员加入，建议先让他熟悉以下部分：
1. `Network/`
2. `UI/Login`
3. `UI/CharacterSelect`
4. `Player/`
5. `Scene/`

这是最容易跑通闭环的部分。

---

# 11. 最佳实践总结

- **网络必须独立**
- **协议必须独立**
- **场景管理必须独立**
- **UI 必须只调用接口**
- **实体数据必须缓存**
- **所有关键状态必须存 GameInstance 或 Subsystem**

---

# 12. 结论

这套目录结构适合一个中大型 MMO / 在线动作 RPG UE5 客户端。按这个结构组织后，后续增加玩法时不会把工程搞乱，也利于多人协作。
