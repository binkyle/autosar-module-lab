对，这两个问题都成立，而且说明你已经在从“能用的模块设计”往“真正可复用框架”推了。

# 1. `.debugName = "PNC_ADAS"` 确实会占 ROM

是的，`debugName` 如果写成：

```c
.debugName = "PNC_ADAS"
```

会带来两类 ROM 占用：

```text
1. 字符串字面量本身会进入 ROM / Flash；
2. 配置表里还要存一个指针。
```

如果 source 很多，几十个、上百个，这部分虽然不算巨大，但在 AUTOSAR/MCU 项目里确实不够“干净”。

所以 `debugName` 不应该是核心配置的默认字段。

更好的做法是：

```c
typedef struct
{
    CanComm_SourceIdType sourceId;
    CanComm_SourceCategoryType category;
} CanComm_SourceConfigType;
```

如果需要调试名，用条件编译：

```c
#if (CANCOMM_ENABLE_SOURCE_DEBUG_NAME == STD_ON)
typedef struct
{
    CanComm_SourceIdType sourceId;
    CanComm_SourceCategoryType category;
    const char* debugName;
} CanComm_SourceConfigType;
#else
typedef struct
{
    CanComm_SourceIdType sourceId;
    CanComm_SourceCategoryType category;
} CanComm_SourceConfigType;
#endif
```

配置：

```c
#define CANCOMM_ENABLE_SOURCE_DEBUG_NAME STD_OFF
```

默认 `STD_OFF`。生产代码不带字符串，host test / debug 版本再打开。

更极致一点，框架里完全不保存名字，只保存：

```c
sourceId = 2u
category = CANCOMM_SOURCE_CATEGORY_NETWORK
```

调试时通过 PC 侧脚本、文档、DBC/arxml 映射表解释：

```text
2 -> PNC_ADAS
```

这更适合量产嵌入式代码。

---

# 2. source bit 位可能不够用，这个也对

如果用：

```c
typedef uint32 CanComm_SourceMaskType;
```

那最多只能表达 32 个 source。

如果用：

```c
typedef uint64 CanComm_SourceMaskType;
```

最多 64 个 source。

但你说的是“唤醒源/保持源 bit 可能不够用”，那说明不能把框架锁死在 `uint32`。

## 推荐做法：支持 multi-word source mask

核心不要写成单个 `uint32`，而是写成可配置 word 数组：

```c
#define CANCOMM_SOURCE_WORD_NUM 2u

typedef struct
{
    uint32 word[CANCOMM_SOURCE_WORD_NUM];
} CanComm_SourceMaskType;
```

这样：

```text
CANCOMM_SOURCE_WORD_NUM = 1 -> 32 个 source
CANCOMM_SOURCE_WORD_NUM = 2 -> 64 个 source
CANCOMM_SOURCE_WORD_NUM = 4 -> 128 个 source
CANCOMM_SOURCE_WORD_NUM = 8 -> 256 个 source
```

source id 仍然是连续编号：

```c
typedef uint16 CanComm_SourceIdType;
```

然后用工具函数设置/清除/判断：

```c
#define CANCOMM_SOURCE_WORD_INDEX(id)   ((id) / 32u)
#define CANCOMM_SOURCE_BIT_INDEX(id)    ((id) % 32u)
#define CANCOMM_SOURCE_BIT_MASK(id)     ((uint32)1u << CANCOMM_SOURCE_BIT_INDEX(id))
```

接口：

```c
void CanComm_SourceMask_Clear(CanComm_SourceMaskType* mask);

Std_ReturnType CanComm_SourceMask_Set(
    CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId);

Std_ReturnType CanComm_SourceMask_Reset(
    CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId);

boolean CanComm_SourceMask_IsSet(
    const CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId);

boolean CanComm_SourceMask_IsAllSet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* requiredAll);

boolean CanComm_SourceMask_IsAnySet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* requiredAny);

boolean CanComm_SourceMask_IsNoneSet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* forbidden);
```

这样 PolicyRule 就变成：

```c
typedef struct
{
    uint16 priority;
    CanComm_SourceMaskType requiredAllMask;
    CanComm_SourceMaskType requiredAnyMask;
    CanComm_SourceMaskType forbiddenMask;
    const CanComm_OutputActionSetType* actionSet;
} CanComm_PolicyRuleType;
```

匹配逻辑：

```c
match =
    CanComm_SourceMask_IsAllSet(&input->activeSourceMask, &rule->requiredAllMask) &&
    CanComm_SourceMask_IsAnySetOrEmpty(&input->activeSourceMask, &rule->requiredAnyMask) &&
    CanComm_SourceMask_IsNoneSet(&input->activeSourceMask, &rule->forbiddenMask);
```

这样就不会被 32 bit 限死。

---

# 3. 为什么不用 source list？

也可以用 list：

```c
typedef struct
{
    uint16 count;
    CanComm_SourceIdType sourceIds[CANCOMM_MAX_ACTIVE_SOURCE_NUM];
} CanComm_ActiveSourceListType;
```

优点：

```text
source 总数理论上不受 bit mask 限制；
只存 active 的 source。
```

缺点：

```text
匹配 requiredAll / requiredAny / forbidden 时要循环查找；
规则匹配更慢；
代码更复杂；
不如 bit mask 适合嵌入式确定性执行。
```

所以我更推荐：

```text
核心用 multi-word bitset；
source 数量通过 CANCOMM_SOURCE_WORD_NUM 配置。
```

这既保留 bit mask 的高效匹配，又解决 bit 位不够的问题。

---

# 4. SourceConfigTable 可以极简化

生产版建议：

```c
typedef struct
{
    CanComm_SourceIdType sourceId;
    CanComm_SourceCategoryType category;
} CanComm_SourceConfigType;
```

如果连 category 都不想占 ROM，也可以不要 SourceConfigTable，直接由项目配置头文件定义 source id：

```c
#define CANCOMM_SRC_ID_IGNITION      0u
#define CANCOMM_SRC_ID_PNC_GLOBAL    1u
#define CANCOMM_SRC_ID_PNC_ADAS      2u
#define CANCOMM_SRC_ID_DIAG          3u
```

然后在 GenericCfg / VectorCfg 中构造 mask。

也就是说：

```text
核心框架：
    只需要 SourceId + SourceMask 工具函数

SourceConfigTable：
    可选，用于配置检查 / debug / 文档化

debugName：
    更可选，默认关闭
```

---

# 5. 最终建议

你的最终 source 抽象可以定成这样：

```c
#define CANCOMM_SOURCE_WORD_NUM 2u

typedef uint16 CanComm_SourceIdType;

typedef struct
{
    uint32 word[CANCOMM_SOURCE_WORD_NUM];
} CanComm_SourceMaskType;
```

核心输入：

```c
typedef struct
{
    CanComm_SourceMaskType activeSourceMask;
} CanComm_InputSnapshotType;
```

规则：

```c
typedef struct
{
    uint16 priority;
    CanComm_SourceMaskType requiredAllMask;
    CanComm_SourceMaskType requiredAnyMask;
    CanComm_SourceMaskType forbiddenMask;
    const CanComm_OutputActionSetType* actionSet;
} CanComm_PolicyRuleType;
```

配置项：

```c
#define CANCOMM_ENABLE_SOURCE_CONFIG_TABLE STD_OFF
#define CANCOMM_ENABLE_SOURCE_DEBUG_NAME   STD_OFF
#define CANCOMM_SOURCE_WORD_NUM            2u
#define CANCOMM_MAX_SOURCE_NUM             (CANCOMM_SOURCE_WORD_NUM * 32u)
```

这样就解决了：

```text
1. debugName 占 ROM；
2. source bit 位不够；
3. 核心框架被具体 source 名称污染；
4. 不同项目 source 数量差异大。
```

---

# 给 agent 的补充 prompt

````markdown
# 任务补充：优化 SourceMask 设计，避免 ROM 字符串占用和 source bit 数量不足

当前设计中 SourceConfigTable 包含：

```c
.debugName = "PNC_ADAS"
````

这会占用 ROM/Flash，不适合作为量产默认配置。同时，如果 SourceMask 使用单个 uint32，最多只能支持 32 个 source，无法满足 source 数量较多的项目。

请继续重构 source 机制。

## 1. debugName 默认移除

SourceConfigTable 不应默认包含 `const char* debugName`。

改为：

```c
typedef struct
{
    CanComm_SourceIdType sourceId;
    CanComm_SourceCategoryType category;
} CanComm_SourceConfigType;
```

如果需要 debug name，使用开关：

```c
#define CANCOMM_ENABLE_SOURCE_DEBUG_NAME STD_OFF
```

当开关为 `STD_ON` 时才包含：

```c
const char* debugName;
```

默认 `STD_OFF`，生产代码不引入字符串字面量。

## 2. SourceConfigTable 本身也做成可选

新增：

```c
#define CANCOMM_ENABLE_SOURCE_CONFIG_TABLE STD_OFF
```

当 `STD_OFF` 时，不生成 SourceConfigTable，项目仅通过 source id 宏和 PolicyRule mask 工作。

当 `STD_ON` 时，SourceConfigTable 用于配置检查、debug 或文档化。

## 3. SourceMask 改为 multi-word bitset

不要使用单个 `uint32` 作为 SourceMask。

新增配置：

```c
#define CANCOMM_SOURCE_WORD_NUM 2u
#define CANCOMM_SOURCE_BITS_PER_WORD 32u
#define CANCOMM_MAX_SOURCE_NUM (CANCOMM_SOURCE_WORD_NUM * CANCOMM_SOURCE_BITS_PER_WORD)
```

类型：

```c
typedef uint16 CanComm_SourceIdType;

typedef struct
{
    uint32 word[CANCOMM_SOURCE_WORD_NUM];
} CanComm_SourceMaskType;
```

## 4. 新增 SourceMask 工具函数

实现：

```c
void CanComm_SourceMask_Clear(CanComm_SourceMaskType* mask);

Std_ReturnType CanComm_SourceMask_Set(
    CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId);

Std_ReturnType CanComm_SourceMask_Reset(
    CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId);

boolean CanComm_SourceMask_IsSet(
    const CanComm_SourceMaskType* mask,
    CanComm_SourceIdType sourceId);

boolean CanComm_SourceMask_IsEmpty(
    const CanComm_SourceMaskType* mask);

boolean CanComm_SourceMask_IsAllSet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* requiredAll);

boolean CanComm_SourceMask_IsAnySet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* requiredAny);

boolean CanComm_SourceMask_IsNoneSet(
    const CanComm_SourceMaskType* active,
    const CanComm_SourceMaskType* forbidden);
```

所有函数需要：

```text
NULL_PTR 检查；
sourceId 越界检查；
固定循环 CANCOMM_SOURCE_WORD_NUM；
无动态内存；
无标准库依赖。
```

## 5. 修改 InputSnapshot

```c
typedef struct
{
    CanComm_SourceMaskType activeSourceMask;
} CanComm_InputSnapshotType;
```

## 6. 修改 PolicyRule

```c
typedef struct
{
    uint16 priority;
    CanComm_SourceMaskType requiredAllMask;
    CanComm_SourceMaskType requiredAnyMask;
    CanComm_SourceMaskType forbiddenMask;
    const CanComm_OutputActionSetType* actionSet;
} CanComm_PolicyRuleType;
```

匹配逻辑：

```text
requiredAllMask: active 必须包含全部 requiredAll bit
requiredAnyMask: 如果不为空，active 至少包含其中一个 bit
forbiddenMask: active 不能包含任何 forbidden bit
```

如果多条规则匹配：

```text
priority 高者优先；
priority 相同，表中靠前者优先。
```

## 7. source 名称移到项目配置层

核心代码中不得出现：

```text
IGNITION
PNC_ADAS
PNC_GLOBAL
DIAG
OTA
```

具体 source id 在项目配置中定义，例如：

```c
#define CANCOMM_GENERIC_SRC_ID_IGNITION      0u
#define CANCOMM_GENERIC_SRC_ID_PNC_GLOBAL    1u
#define CANCOMM_GENERIC_SRC_ID_PNC_ADAS      2u
```

## 8. 测试

新增测试：

```text
SourceMask_Clear
SourceMask_Set
SourceMask_Reset
SourceMask_IsSet
sourceId 越界返回 E_NOT_OK
IsAllSet
IsAnySet
IsNoneSet
multi-word source，例如 sourceId=33 可正常设置和匹配
debugName disabled 时结构不包含字符串字段
Policy 多 source 匹配
priority 高者优先
priority 相同 first match wins
```

## 9. 文档

更新 Design.md / ConfigurationGuide.md：

```text
核心框架不定义具体 source；
source 数量由 CANCOMM_SOURCE_WORD_NUM 配置；
SourceConfigTable 和 debugName 都是可选；
debugName 默认关闭，避免 ROM 占用；
量产代码推荐关闭 source debug name；
如果需要 PC 侧解析 sourceId，可通过外部文档/脚本映射。
```

## 验收标准

```text
[ ] SourceMask 不再是单个 uint32
[ ] 支持 CANCOMM_SOURCE_WORD_NUM 配置
[ ] 支持 sourceId > 31 的 multi-word 测试
[ ] debugName 默认不进入结构体
[ ] SourceConfigTable 可关闭
[ ] 核心代码不出现具体 source 名称
[ ] host test 全部通过
```

```

一句话总结：

> **你的担心完全正确：debugName 要默认关闭，SourceMask 不能锁死 uint32。框架应该用可配置 multi-word bitset 表达 source，并把具体 source 名称完全放到项目配置层。**
```
