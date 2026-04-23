# UE5 Protobuf 集成说明

本文说明如何在 UE5 客户端中接入与服务端 `shared-proto` 一致的 Protobuf 协议。

---

# 1. 目标

UE5 客户端需要实现：
- 请求对象序列化
- 响应对象反序列化
- 消息编号与协议对象映射
- 与 TCP 帧协议配合使用

服务端使用的消息结构是：

```text
[Length: 4 bytes][MsgId: 4 bytes][Payload: N bytes]
```

其中 Payload 是 protobuf 二进制数据。

---

# 2. 与服务端保持一致的原则

## 2.1 字段编号一致
Proto 文件中字段名可以不同，但字段编号必须一致。

例如：

```proto
message HandshakeRequest {
  int64 userId = 1;
  string token = 2;
}
```

UE5 端必须使用同样的字段号。

## 2.2 包名一致
建议 UE5 生成的 proto 类命名空间与服务端一致，便于维护。

例如：
- `com.game.proto.auth`
- `com.game.proto.character`
- `com.game.proto.scene`

## 2.3 消息号一致
例如：
- `1001`：握手请求
- `1002`：握手响应
- `2001`：角色列表请求
- `2002`：角色列表响应

---

# 3. UE5 中的 Protobuf 接入方案

## 方案 A：使用 protobuf 官方 C++ 库

### 优点
- 与服务端协议完全一致
- 维护成本低
- 可直接使用生成代码

### 缺点
- UE 构建环境需要额外配置
- 需要引入第三方库到 UE 工程

## 方案 B：手工编码/解码

### 优点
- 不依赖生成器
- 适合快速验证

### 缺点
- 开发成本高
- 容易出现字段错误
- 后期维护困难

## 推荐
正式项目建议使用 **方案 A**。

---

# 4. 生成 Protobuf C++ 代码

## 4.1 获取 proto 文件
把服务端 `shared-proto/src/main/proto/` 下的 proto 文件同步到 UE 客户端，例如：

```text
Content/Proto/
  auth.proto
  character.proto
  scene.proto
  chat.proto
  combat.proto
  drop.proto
  movement.proto
  monster.proto
  task.proto
  party.proto
  instance.proto
  pet.proto
  upgrade.proto
```

## 4.2 使用 protoc 生成 C++ 文件
示例命令：

```bash
protoc --cpp_out=./Generated ./Proto/*.proto
```

或者根据你的目录结构生成到 UE 的 Source/ThirdParty 目录。

## 4.3 在 UE Build.cs 中加入依赖
你需要把 protobuf 库加入 UE 模块：

```csharp
PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "protobuf", "include"));
PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty", "protobuf", "lib", "protobuf.lib"));
```

> 实际路径按你的工程调整。

---

# 5. 请求对象序列化

## 示例：握手请求

服务端 `HandshakeRequest` 示例：

```proto
message HandshakeRequest {
  int64 userId = 1;
  string token = 2;
}
```

UE5 C++ 代码：

```cpp
TArray<uint8> BuildHandshakePayload(const FString& Token, int64 UserId)
{
    game::auth::HandshakeRequest Request;
    Request.set_userid(UserId);
    Request.set_token(TCHAR_TO_UTF8(*Token));

    std::string Serialized;
    Request.SerializeToString(&Serialized);

    TArray<uint8> Bytes;
    Bytes.Append(reinterpret_cast<const uint8*>(Serialized.data()), Serialized.size());
    return Bytes;
}
```

---

# 6. 响应对象反序列化

## 示例：握手响应

```proto
message HandshakeResponse {
  bool success = 1;
  int32 code = 2;
  string message = 3;
}
```

UE5 代码：

```cpp
void ParseHandshakeResponse(const TArray<uint8>& Payload)
{
    game::auth::HandshakeResponse Response;
    Response.ParseFromArray(Payload.GetData(), Payload.Num());

    if (Response.success())
    {
        UE_LOG(LogTemp, Log, TEXT("Handshake success: %s"), UTF8_TO_TCHAR(Response.message().c_str()));
    }
}
```

---

# 7. 消息号与 proto 类型映射建议

建议在 UE5 中建立一个映射表：

```cpp
struct FProtoRoute
{
    int32 MsgId;
    FString Name;
};
```

例如：

- `1001` -> `HandshakeRequest`
- `1002` -> `HandshakeResponse`
- `2001` -> `CharacterListRequest`
- `2002` -> `CharacterListResponse`
- `4001` -> `EnterSceneRequest`
- `4002` -> `EnterSceneResponse`

---

# 8. 与 TCP 帧结合

发送时流程：

1. 构造 protobuf 对象
2. 序列化成字节数组
3. 写入 `Length`
4. 写入 `MsgId`
5. 写入 Payload
6. 发送给 socket

接收时流程：

1. 从 socket 读取字节流
2. 解析 `Length`
3. 解析 `MsgId`
4. 提取 Payload
5. protobuf 反序列化
6. 分发到蓝图事件

---

# 9. 蓝图桥接建议

建议在 C++ 层提供统一的桥接结构：

```cpp
USTRUCT(BlueprintType)
struct FCharacterInfoBP
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int64 CharacterId;

    UPROPERTY(BlueprintReadOnly)
    FString Name;

    UPROPERTY(BlueprintReadOnly)
    int32 Level;
};
```

把 protobuf 对象转换成蓝图能直接用的结构。

---

# 10. 实战顺序建议

建议 UE 团队按这个顺序接：

1. 先接 `auth.proto`
2. 再接 `character.proto`
3. 再接 `scene.proto`
4. 再接 `chat.proto`
5. 再接 `combat.proto`
6. 最后接 `party / instance / pet / upgrade`

这样可以先跑通最小闭环，再扩展玩法。

---

# 11. 注意事项

- protobuf 版本要和 UE 引入的库保持一致
- 字段编号不要随意改
- 新增字段尽量追加，不要复用旧编号
- 不要依赖字段名作为兼容依据，只认字段编号
- 业务扩展时优先兼容旧客户端

---

# 12. 结论

UE5 接 Protobuf 的最佳方式是：
- 服务端 proto 作为唯一协议源
- UE 端同步同一套 proto
- 通过 generated C++ class 完成序列化和反序列化
- C++ 转蓝图数据结构做 UI 和表现层消费
