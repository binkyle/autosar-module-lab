# Integration Guide for Vector / MICROSAR

## DaVinci Configurator 检查清单

### ComM
- [ ] 确认所有通信通道已配置 ComM User
- [ ] 确认 `ComM_RequestComMode` 可用于请求的 User
- [ ] 检查 PNC 映射（如果已启用 ComM PNC）
- [ ] 在 `project/vector/CommPolicy_VectorCfg.c` 中配置 `CommPolicy_VectorChannelBindingTable`，将每个逻辑通道映射到对应的 ComM User 和 Network Handle

### Nm / CanNm / LinNm / EthNm
- [ ] 确认对应总线的 PN 功能已正确配置（如使用 Partial Network）
- [ ] 检查 EIRA/ERA 聚合配置

### BswM
- [ ] 创建 ModeRequestPort 用于 CommPolicyCoordinator 的睡眠候选状态
- [ ] 创建 Rule：当所有通道为 NO_COMM 且无本地保持源时，允许进入 Sleep
- [ ] 创建 ActionList：
  - [ ] 调用 `Com_IpduGroupStop`（替代框架直接调用）
  - [ ] 调用 `Nm_DisableCommunication`（替代框架直接调用）
  - [ ] 调用 EcuM 相关 Action

### CanIf / CanTrcv / LinIf / EthIf
- [ ] 确认选择性唤醒配置正确（如 TJA1145 等 transceiver）
- [ ] 确认 Wakeup 恢复后对应 SM 能正常恢复通道

### SchM
- [ ] 确认 `SchM_CommPolicyCoordinator.h` 已生成（由 DaVinci 生成，**不是** stub/BSW 中的版本）
- [ ] 确认 ExclusiveArea 配置正确，避免与 CanIf/ComM 中断冲突
- [ ] 旧 `SchM_CanCommCoordinator.h` 不应继续被新框架引用

### stub/BSW 使用限制

`CommPolicyCoordinator/stub/BSW/` 目录下的头文件**仅允许用于 host 单元测试编译**。
集成到 Vector/MICROSAR 工程时，必须：
- 从 include path 中移除 `stub/BSW/`
- 使用 DaVinci Configurator 生成的标准 BSW 头文件
- 禁止将 stub 中的 `ComMConf_xxx` 等符号用于目标编译

## 编译集成

1. 将 `CommPolicyCoordinator/src/*.c` 加入编译
2. 将 `CommPolicyCoordinator/include` 加入 Include Path
3. 将 `CommPolicyCoordinator/project/vector/CommPolicy_VectorAdapter.c` 加入编译
4. 移除 `CommPolicyCoordinator/project/generic/CommPolicy_GenericMock.c`
5. **必须移除** `CommPolicyCoordinator/stub/BSW/` 的 include path（stub 仅用于 host 单元测试）
6. **禁止**在 `src/` 或 `include/` 中引用任何 DaVinci 生成符号；唯一允许引用 ComM 类型和符号的位置是 `project/vector/CommPolicy_VectorAdapter.c` 和 `project/vector/CommPolicy_VectorCfg.c`（通过 `CommPolicy_VectorChannelBindingTable`）

### Adapter 选择机制（无 weak symbol）

`CommPolicy_Adapter.c` **不提供**默认弱符号实现。构建系统必须通过链接以下文件之一来提供具体的 Adapter 实现：
- **Host 测试**：链接 `project/generic/CommPolicy_GenericMock.c`
- **Vector 目标**：链接 `project/vector/CommPolicy_VectorAdapter.c`

如果两者都未链接，链接器将报告未定义符号错误。

## 验证步骤

1. PowerRun ON -> 所有通道请求 FULL_COMMUNICATION
2. PowerRun OFF + Partial Network active -> 仅对应分组通道保持 FULL
3. PowerRun OFF + 无 active source -> 所有通道保持 FULL（fallback safe）
4. PowerRun OFF + Global Network active -> 所有通道保持 FULL
5. PowerRun OFF + 无 keep source -> 所有通道 NO_COMM，sleep candidate 置位
6. 选择性唤醒 -> 对应 SM/ComM/Nm 自动恢复
