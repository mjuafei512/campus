# campus

```
/Content
├── _Core/                   # 核心系统资源（框架、管理器、通用工具）
            ├── Blueprints/              # 核心蓝图（如游戏模式、玩家控制器、游戏实例）
            ├── Materials/               # 通用材质（如标准PBR材质、UI材质）
            └── Maps/                    # 核心地图（如主菜单、登录关卡、持久化关卡）
├── Characters/              # 所有角色相关资源（玩家、NPC）
            ├── Player/                  # 玩家角色
            │   ├── Meshes/              # 玩家模型（身体、服装）
            │   ├── Animations/          # 玩家动画（待机、跑步、社交动作）
            │   ├── Blueprints/          # 玩家角色蓝图、属性组件
            │   └── Materials/           # 玩家专属材质
            └── NPCs/                    # NPC角色
                ├── Student/             # 学生NPC
                ├── Teacher/             # 教师NPC
                └── Vendor/              # 商店老板NPC
                    ├── Meshes/          # 对应NPC的模型
                    ├── Animations/      # 对应NPC的动画
                    └── Blueprints/      # NPC行为逻辑蓝图
├── Environments/            # 校园场景资源（建筑、道具、植被）
            ├── Maps/                    # 所有关卡地图
            │   ├── Main_Campus/         # 主校园场景
            │   ├── Classroom_01/        # 教室场景
            │   ├── Dormitory/           # 宿舍场景
            │   └── Library/             # 图书馆场景
            ├── Props/                   # 场景道具
            │   ├── Furniture/           # 家具（课桌、椅子、床）
            │   ├── Electronics/         # 电子设备（电脑、投影仪）
            │   └── Consumables/         # 消耗品（书本、饮料）
            ├── Buildings/               # 建筑模块（墙体、门窗、屋顶）
            ├── Vegetation/              # 植被（树木、花草、灌木）
            └── Materials/               # 场景专用材质（如墙面漆、木地板、草地）
├── UI/                      # 用户界面资源
            ├── Layouts/                 # UI布局（主界面、背包、聊天窗口）
            ├── Widgets/                 # 通用控件（按钮、图标、血条、聊天气泡）
            ├── Icons/                   # 图标资源（技能图标、物品图标）
            ├── Fonts/                   # 游戏使用的字体
            └── Materials/               # UI特效材质（如流光、高亮）
├── Audio/                   # 音频资源（背景音乐、音效、语音）
            ├── BGM/                     # 背景音乐（校园氛围、教室、宿舍）
            ├── SFX/                     # 音效（脚步声、交互声、UI点击声）
            └── Voice/                   # 语音（NPC对话、系统提示）
├── VFX/                     # 特效资源（技能、环境、UI特效）
└── Data/                    # 游戏数据（DataAssets, DataTables）
            ├── Items/                   # 物品数据（服装、道具属性）
            ├── NPCs/                    # NPC数据（对话、行为参数）
            ├── Quests/                  # 任务数据（任务目标、奖励）
            └── Social/                  # 社交数据（表情、动作列表）
```

```
/Source
└── MyCampusGame/            # 你的游戏模块
    ├── Public/              # 公开的头文件（.h）
    │   ├── Characters/      # 角色相关的C++类
    │   ├── Social/          # 社交系统相关的C++类
    │   └── Interaction/     # 交互系统相关的C++类
    └── Private/             # 私有实现文件（.cpp）
        ├── Characters/
        ├── Social/
        └── Interaction/
```


| 资产类型 | 推荐前缀 | 命名风格 | 示例 |
| :--- | :--- | :--- | :--- |
| 蓝图类 | `BP_` | PascalCase | `BP_PlayerCharacter` |
| 静态网格体 | `SM_` | PascalCase | `SM_Desk_01` |
| 材质 | `M_` | PascalCase | `M_Wood_Floor` |
| 纹理 | `T_` | PascalCase | `T_Brick_Wall_D` (D=漫反射) |
| 动画 | `A_` | PascalCase | `A_Player_Run` |
| 音效 | `S_` | PascalCase | `S_Footstep_Concrete` |
| UI控件 | `W_` | PascalCase | `W_MainMenuButton` |

重要原则：
避免空格和特殊字符：使用下划线 _ 或驼峰式命名。
保持简洁和描述性：名称应能清晰反映资产内容和用途。
团队统一：在项目开始前，团队必须就命名规范达成一致并严格遵守。