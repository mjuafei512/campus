# UE5 蓝图流程实战清单

本文给出蓝图层的可执行实现顺序，适合直接按节点搭建。

---

# 1. 入口流程

## 1.1 启动游戏
- `BP_GameInstance::Init`
- 创建并初始化 `UGameNetworkSubsystem`
- 加载登录界面 `WBP_Login`

## 1.2 登录界面
- 输入账号密码
- 点击登录按钮
- 调用 `LoginAndConnect`
- 成功后保存 Token / UserId
- 打开角色选择界面

---

# 2. 角色选择流程

## 2.1 请求角色列表
- 登录成功后调用 `SendCharacterListRequest`
- 等待消息 `2002`
- 在 `WBP_CharacterSelect` 中展示角色卡片

## 2.2 创建角色
- 点击“创建角色”按钮
- 输入名称
- 调用 `SendCreateCharacter`
- 成功后刷新列表

## 2.3 进入角色
- 点击某角色“进入”按钮
- 调用 `SendEnterCharacter`
- 成功后进入世界

---

# 3. 进入世界流程

## 3.1 进入场景
- 进入角色后立刻调用 `SendEnterScene(SceneId)`
- 收到 `4002` 后：
  - 加载场景地图
  - 生成玩家角色
  - 生成 NPC / 怪物 / 掉落
  - 创建 HUD

## 3.2 初始化 HUD
- `WBP_HUD`
- 角色血条
- 背包按钮
- 任务按钮
- 组队按钮
- 聊天按钮
- 宠物按钮
- 强化按钮

---

# 4. 网络状态同步

## 4.1 心跳
- `SetTimer` 每 10~30 秒发一次 `SendHeartbeat`
- 心跳失败累计超过阈值，显示断线

## 4.2 移动
- 角色移动时，位置变化超过阈值则发 `SendMove`
- 如果角色持续移动，建议每 0.2~0.5 秒同步一次

## 4.3 战斗
- 攻击按钮 -> `SendAttack`
- 服务端回 `DamageEvent` -> 播放受击表现
- 服务端回 `MonsterDefeatEvent` -> 播放死亡、掉落表现

---

# 5. UI 功能模块

## 5.1 背包
- 打开 `WBP_Inventory`
- 从服务端刷新物品列表
- 点击使用道具 -> `SendUseItem`
- 点击装备 -> `SendEquipItem`

## 5.2 任务
- 打开 `WBP_Task`
- 接任务 -> `SendAcceptTask`
- 战斗/拾取/交互触发进度更新 -> `SendUpdateTaskProgress`
- 完成后 -> `SendSubmitTask`

## 5.3 聊天
- `WBP_Chat` 接收 `7003`
- 玩家输入后调用 `SendChat`
- 将消息加入聊天频道 UI

## 5.4 组队
- `WBP_Party`
- 创建队伍
- 邀请队友
- 加入 / 退出
- 收到 `11002~11005` 时刷新成员列表

## 5.5 副本
- `WBP_Instance`
- 创建实例
- 进入队伍副本
- 收到 `12002~12005` 时刷新副本波次与状态

## 5.6 宠物
- `WBP_Pet`
- 召唤宠物
- 跟随 / 召回
- 收到 `13005` 时播放协战特效

## 5.7 强化
- `WBP_Upgrade`
- 选择装备
- 点击强化
- 收到 `14002~14004` 时刷新等级与属性

---

# 6. 推荐节点风格

## 6.1 网络成功回调
```text
OnLoginSuccess
  -> Save Token
  -> Save UserId
  -> Close Login UI
  -> Open CharacterSelect UI
```

## 6.2 场景加载
```text
OnEnterSceneSuccess
  -> Load Stream Level
  -> Spawn Player
  -> Spawn NPCs
  -> Spawn Monsters
  -> Spawn Drops
  -> Create HUD
```

## 6.3 任务完成
```text
OnTaskProgressUpdated
  -> Update Task UI
  -> If Completed
       -> Show Reward Toast
       -> Enable Submit Button
```

---

# 7. 调试建议

## 7.1 先做单线闭环
推荐按顺序做单线测试：
1. 登录
2. 握手
3. 角色选择
4. 进入场景
5. 心跳
6. 移动
7. 聊天

## 7.2 再扩展系统
然后逐步接入：
1. 战斗
2. 掉落
3. 背包
4. 任务
5. 组队
6. 副本
7. 宠物
8. 强化

---

# 8. 最佳实践

- 网络层全部 C++ 实现
- 蓝图只做调用和表现
- 消息收到后通过事件总线分发
- 所有关键信息放到 `GameInstance`
- 关卡切换不丢 token / userId / characterId

---

# 9. 结论

这份清单适合 UE 团队从 0 开始照着搭蓝图。建议先完成“登录 -> 握手 -> 角色 -> 进场景”的最小闭环，再逐步加功能。
