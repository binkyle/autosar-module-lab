# Migration Guide: From CanServer to CommPolicyCoordinator

## 迁移概述

本指南描述如何将旧 `CanServer` 模块逐步替换为 `CommPolicyCoordinator`。

## 迁移步骤

### Phase 1：并行开发（不替换）

1. 在项目中引入 `CommPolicyCoordinator` 源码和头文件
2. 保留旧 `CanServer` 运行
3. 将旧 `CanServer` 的 source 激活逻辑映射到 `CommPolicy_State_OnSourceActive/Inactive`

### Phase 2：Adapter 实现

1. 实现 `CommPolicy_VectorAdapter.c` 中的 TODO：
   - `CommPolicy_Adapter_BuildInputSnapshot`（组装输入 snapshot）
   - `CommPolicy_Adapter_RequestMode`（调用 BswM_RequestMode）
2. 在 VectorCfg 中映射 ComM User、Network Handle
3. 验证 `CommPolicy_Adapter_BuildInputSnapshot` 能正确组装输入

### Phase 3：策略对齐

1. 对照旧 `CanServer` 的业务逻辑，确认 `CommPolicy_PolicyRuleTable` 覆盖所有场景
2. 调整 `CommPolicy_OutputActionSetType` 定义（使用 `ChannelMask` 替代旧 `uint32` mask）
3. 确认 fallback 策略与旧模块的安全行为一致

### Phase 4：BswM 迁移

1. 将旧 `CanServer` 中直接调用的 `Com_IpduGroupStart/Stop` 迁移到 BswM ActionList
2. 将 `Nm_EnableCommunication` / `Nm_DisableCommunication` 迁移到 BswM ActionList
3. 在 BswM Rule 中引用 `CommPolicyCoordinator` 的 `modeRequestId` 输出

### Phase 5：切换与回滚准备

1. 在 DaVinci 中停用旧 `CanServer` 的调度（从 Task 中移除 MainFunction）
2. 启用 `CommPolicyCoordinator_MainFunction` 调度
3. 保留旧源码至少一个版本，确保可回滚

### Phase 6：验证

- [ ] PowerRun ON -> all FULL
- [ ] PowerRun OFF + Partial Network -> only对应分组通道
- [ ] PowerRun OFF + no active source -> all NO / sleep candidate
- [ ] PowerRun OFF + invalid source -> fallback ALL_FULL
- [ ] Wakeup from selective transceiver -> normal restoration
- [ ] DLT/Trace 无异常 Error 计数

## 旧代码清理清单

- [ ] 删除旧 `CanServer.c/.h`
- [ ] 删除旧 `CanServer` 相关的 ComM User 硬编码引用
- [ ] 删除旧 `CanServer` 中的 `Byte5 bit3` 等硬编码解析
- [ ] 更新 BswM ActionList，移除对旧模块的引用
- [ ] 更新 RTE 接口（如果旧模块有 RTE 端口）
- [ ] 更新 SchM 配置，生成 `SchM_CommPolicyCoordinator.h`
