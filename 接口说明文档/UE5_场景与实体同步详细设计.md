# UE5 场景与实体同步详细设计

本文档用于说明 UE5 客户端在进入场景后，如何同步玩家、怪物、掉落、宠物、队伍成员等实体状态，并保持与服务端一致。

---

# 1. 设计目标

场景同步的目标是：

- 客户端展示的场景实体与服务端状态一致
- 玩家位置、朝向、战斗目标及时同步
- 怪物、掉落、宠物、队伍成员等实体可增删改
- 支持视野内实体刷新与离开
- 支持场景切换后的快速重建

---

# 2. 场景同步的核心思想

建议采用：

- **服务端权威**：服务端决定实体状态
- **客户端预测**：玩家本地移动可先行表现
- **事件驱动更新**：服务端推送变化，客户端刷新实体
- **分层缓存**：场景实体缓存、玩家状态缓存、UI 状态缓存

---

# 3. 客户端场景数据结构

建议在 UE5 中维护以下结构：

```cpp
USTRUCT(BlueprintType)
struct FSceneEntityData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int64 EntityId = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 EntityType = 0; // 1=玩家 2=怪物 3=掉落 4=宠物 5=NPC

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float RotationYaw = 0.f;

    UPROPERTY(BlueprintReadWrite)
    int32 Hp = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxHp = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bVisible = true;
};
```

建议再维护一个场景缓存：

```cpp
USTRUCT(BlueprintType)
struct FSceneSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 SceneId = 0;

    UPROPERTY(BlueprintReadWrite)
    TMap<int64, FSceneEntityData> Entities;
};
```

---

# 4. 实体类型约定

建议统一以下类型编码：

| 类型 | 编码 | 说明 |
| --- | --- | --- |
| 玩家 | `1` | 其他玩家 |
| 怪物 | `2` | 可战斗怪物 |
| 掉落 | `3` | 地面掉落物 |
| 宠物 | `4` | 召唤兽/伙伴 |
| NPC | `5` | 非玩家角色 |

---

# 5. 进入场景后客户端做什么

当收到 `4002` 进入场景响应后，客户端应：

1. 记录当前 `SceneId`
2. 清理旧场景实体
3. 加载地图
4. 创建本地玩家角色
5. 根据响应中的 nearby entities 创建其他实体
6. 建立场景缓存
7. 打开 HUD
8. 启动位置同步定时器

### 节点流程
```text
OnGameMessage(MsgId=4002)
  -> Parse EnterSceneResponse
  -> Set Current SceneId
  -> Clear Old Scene Entities
  -> Load Map
  -> Spawn Local Player
  -> Spawn Nearby Entities
  -> Start Sync Timers
  -> Open HUD
```

---

# 6. 玩家实体同步

## 6.1 本地玩家位置
玩家本地位置由 Character Movement 控制。

### 同步建议
- 移动时本地先表现
- 每隔固定时间或阈值变化后发送移动包
- 服务端回包后再修正位置

## 6.2 本地玩家状态
建议同步：
- 位置
- 朝向
- 当前动作
- 战斗状态
- 当前目标
- 当前任务状态

### 节点流程
```text
OnCharacterMovementUpdated
  -> Compare Last Location
  -> If Threshold Exceeded
       -> SendMove
       -> Update Last Location
```

---

# 7. 怪物实体同步

## 7.1 怪物生成
服务端下发怪物生成事件后：
- 客户端 Spawn 对应 `BP_Monster`
- 填充 ID、名称、HP、位置
- 加入场景缓存

## 7.2 怪物刷新
当服务端推送怪物 HP、位置、死亡事件时：
- 更新怪物血条
- 更新位置
- 播放受击/死亡动画
- 从缓存中移除或标记死亡

### 节点流程
```text
OnGameMessage(MsgId=8002 / 8003 / 5003)
  -> Parse Monster / Damage Event
  -> Find Monster Actor
  -> Update HP / Play Hit / Play Death
```

---

# 8. 掉落实体同步

## 8.1 掉落生成
当服务端下发掉落生成信息时：
- 客户端 Spawn `BP_DropItem`
- 显示掉落名字和图标
- 加入场景缓存

## 8.2 掉落拾取
当服务端确认拾取成功后：
- 删除掉落实体
- 播放拾取特效
- 刷新背包 UI

### 节点流程
```text
OnGameMessage(MsgId=6002 / 6003)
  -> Parse Drop Event
  -> Spawn or Destroy Drop Actor
  -> Refresh Inventory
```

---

# 9. 宠物实体同步

宠物属于跟随实体，建议同步以下内容：
- 宠物是否激活
- 宠物位置
- 宠物朝向
- 宠物当前战斗状态

### 节点流程
```text
OnGameMessage(MsgId=13002 / 13004 / 13005 / 13006)
  -> Parse Pet Event
  -> Spawn / Update / Hide Pet Actor
```

---

# 10. 队伍成员同步

## 10.1 队伍成员显示
队伍成员通常出现在：
- 队伍 UI
- 场景角色列表
- 小地图/队伍标识

## 10.2 队伍成员实体
如果队伍成员进入同一场景，应：
- Spawn 玩家实体
- 显示队伍标签
- 显示血条/职业/等级

### 节点流程
```text
OnGameMessage(MsgId=11002~11005)
  -> Refresh Party UI
  -> Update Party Member Actors
```

---

# 11. 场景离开与切换

当玩家离开场景或切换地图时：

1. 停止同步定时器
2. 销毁当前场景实体
3. 清理场景缓存
4. 清理掉落和怪物引用
5. 加载新场景
6. 重新请求进入场景

### 节点流程
```text
OnSceneExit
  -> Stop Timers
  -> Destroy Entities
  -> Clear Scene Cache
  -> Load New Scene
```

---

# 12. 实体刷新机制

建议采用以下机制：

## 12.1 新增实体
服务端推送新增实体后：
- 如果本地不存在，创建
- 如果已存在，更新

## 12.2 删除实体
服务端推送删除实体后：
- 销毁 Actor
- 从缓存中移除

## 12.3 更新实体
服务端推送属性变化后：
- 更新 HP、位置、动画状态等

---

# 13. 视野同步建议

建议将实体分为两类：

- **常驻实体**：本场景始终存在，如 NPC、场景出生点
- **动态实体**：玩家、怪物、掉落、宠物

客户端只需重点同步动态实体。

---

# 14. 网络消息与场景实体映射

| 消息 | 映射内容 | 客户端动作 |
| --- | --- | --- |
| `4002` | 场景进入成功 | 创建场景缓存 |
| `5003` | 伤害事件 | 更新怪物/玩家 HP |
| `6003` | 拾取结果 | 删除掉落、刷新背包 |
| `7003` | 聊天广播 | 可选显示头顶消息 |
| `8003` | 怪物死亡 | 播放死亡动画、生成掉落 |
| `13005` | 宠物协战 | 播放宠物攻击表现 |
| `13006` | 宠物跟随 | 切换宠物状态 |
| `11002~11005` | 队伍变化 | 刷新队伍成员 UI |

---

# 15. 蓝图建议

## 15.1 场景实体管理蓝图
建议创建一个：
- `BP_SceneEntityManager`

职责：
- 管理当前场景所有实体 Actor
- 根据服务端消息创建/更新/销毁实体
- 维护本地 `SceneSnapshot`

## 15.2 HUD 刷新
建议 HUD 订阅：
- 场景变更
- 掉落变更
- 队伍变更
- 宠物变更

---

# 16. 推荐实现顺序

建议按以下顺序实现场景同步：

1. 进入场景时的实体生成
2. 本地玩家实体生成
3. 怪物实体生成和死亡
4. 掉落生成和拾取
5. 宠物显示
6. 队伍成员同步
7. 场景切换清理
8. 视野与缓存优化

---

# 17. 最佳实践

- 所有实体都应有唯一 `EntityId`
- 每个实体都应绑定一个类型 `EntityType`
- 客户端缓存应该可快速查找和删除
- 切场景时必须清空旧缓存
- 玩家位置同步应尽量轻量
- 怪物/掉落/宠物等动态实体通过服务端事件驱动

---

# 18. 结论

这份设计文档的目标是让 UE5 客户端在场景内做到“服务端权威 + 客户端表现流畅”。建议先实现“进入场景 -> 生成实体 -> 玩家移动 -> 怪物战斗 -> 掉落拾取”的闭环，再做宠物和队伍同步。
