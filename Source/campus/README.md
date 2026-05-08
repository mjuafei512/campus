# MyCampusGame C++ Source

## 目录结构

```
Source/MyCampusGame/
├── Core/                      # 模块入口、全局基础类
├── Network/                   # 网络层 (TCP/HTTP/协议)
├── Protobuf/                  # Protobuf 相关
│   └── Generated/             # 生成的 Protobuf 代码
├── Player/                    # 玩家相关逻辑
├── Scene/                     # 场景管理
├── UI/                        # UI 控制器
│   ├── Login/
│   ├── CharacterSelect/
│   └── HUD/
├── World/                     # 游戏世界实体
│   ├── Monsters/
│   ├── Drops/
│   ├── Pets/
│   └── NPC/
├── Combat/                    # 战斗逻辑
├── Data/                      # 数据表和配置
│   └── Types/                 # 数据结构定义
├── Utils/                     # 工具函数
├── Subsystems/                # Subsystem 类
└── Components/                # 组件类
```

## 核心类

| 类 | 位置 | 职责 |
| --- | --- | --- |
| `UGameNetworkSubsystem` | Network/ | 全局网络管理 |
| `UTcpGameClient` | Network/ | TCP 长连接 |
| `UAuthService` | Network/ | HTTP 登录 |
| `UProtocolManager` | Network/ | 协议打包 |
| `UMessageDispatcher` | Network/ | 消息分发 |
| `USceneManagerSubsystem` | Scene/ | 场景管理 |
| `USceneEntityManager` | Scene/ | 场景实体管理 |
| `UPlayerStateSyncComponent` | Player/ | 玩家状态同步 |
| `UCombatComponent` | Combat/ | 战斗逻辑 |

## 开发指南

- C++ 负责：网络连接、协议编解码、消息分发、数据缓存
- 蓝图负责：UI 表现、动画、特效、交互流程