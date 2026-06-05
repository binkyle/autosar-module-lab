# Integration Guide for Vector / MICROSAR

## DaVinci Configurator 检查清单

### ComM
- [ ] 确认所有通信通道已配置 ComM User
- [ ] 确认 `ComM_RequestComMode` 可用于请求的 User
- [ ] 检查 PNC 映射（如果已启用 ComM PNC）

### CanNm / Nm
- [ ] 确认 CanNmPnEnable = TRUE（如果使用 PN）
- [ ] 确认 CanNmPnResetTime 与 `CANCOMM_PNC_HOLD_TIME_MS` 一致性
- [ ] 检查 EIRA/ERA 聚合配置

### BswM
- [ ] 创建 ModeRequestPort 用于 CanCommCoordinator 的睡眠候选状态
- [ ] 创建 Rule：当所有通道为 NO_COMM 且无本地保持源时，允许进入 Sleep
- [ ] 创建 ActionList：
  - [ ] 调用 `Com_IpduGroupStop`（替代模块直接调用）
  - [ ] 调用 `Nm_DisableCommunication`（替代模块直接调用）
  - [ ] 调用 EcuM 相关 Action

### CanIf / CanTrcv
- [ ] 确认 TJA1145 选择性唤醒配置正确
- [ ] 确认 Wakeup 恢复后 CanSM 能正常恢复通道

### RTE
- [ ] 如 `CanComm_Adapter_GetIgnition` 需要 RTE 接口，在 DaVinci Developer 中创建 SR/CS 接口

### SchM
- [ ] 确认 `SchM_CanCommCoordinator.h` 已生成（由 DaVinci 生成，**不是** stub/BSW 中的版本）
- [ ] 确认 ExclusiveArea 配置正确，避免与 CanIf/ComM 中断冲突

### stub/BSW 使用限制

`CanCommCoordinator/stub/BSW/` 目录下的头文件**仅允许用于 host 单元测试编译**。
集成到 Vector/MICROSAR 工程时，必须：
- 从 include path 中移除 `stub/BSW/`
- 使用 DaVinci Configurator 生成的标准 BSW 头文件
- 禁止将 stub 中的 `ComMConf_xxx` 等符号用于目标编译

## 编译集成

1. 将 `CanCommCoordinator/src/*.c` 加入编译
2. 将 `CanCommCoordinator/include` 加入 Include Path
3. 将 `CanCommCoordinator/project/vector/CanCommCoordinator_VectorAdapter.c` 加入编译
4. 移除 `CanCommCoordinator/project/generic/CanCommCoordinator_GenericMock.c`
5. **必须移除** `CanCommCoordinator/stub/BSW/` 的 include path（stub 仅用于 host 单元测试）
6. **禁止**在 `src/` 或 `include/` 中引用任何 DaVinci 生成符号；唯一允许的位置是 `project/vector/CanCommCoordinator_VectorAdapter.c` 和 `project/vector/CanCommCoordinator_VectorCfg.c`

### Adapter 选择机制（无 weak symbol）

`CanCommCoordinator_Adapter.c` **不再提供**默认弱符号实现。构建系统必须通过链接以下文件之一来提供具体的 Adapter 实现：
- **Host 测试**：链接 `project/generic/CanCommCoordinator_GenericMock.c`
- **Vector 目标**：链接 `project/vector/CanCommCoordinator_VectorAdapter.c`

如果两者都未链接，链接器将报告未定义符号错误。

## 验证步骤

1. KL15 ON -> 所有通道请求 FULL_COMMUNICATION
2. KL15 OFF + 0x501 PNI=1 Global=0 ADAS=1 -> 仅 ADAS1 保持 FULL
3. KL15 OFF + 0x501 PNI=0 -> 所有通道保持 FULL（fallback safe）
4. KL15 OFF + Global=1 -> 所有通道保持 FULL
5. KL15 OFF + 无 NM 超时 -> 所有通道 NO_COMM，sleep candidate 置位
6. TJA1145 选择性唤醒 -> CanSM/ComM/CanNm 自动恢复
