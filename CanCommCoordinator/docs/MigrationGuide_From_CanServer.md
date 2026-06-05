# Migration Guide: From CanServer to CanCommCoordinator

## 迁移概述

本指南描述如何将旧 `CanServer` 模块逐步替换为 `CanCommCoordinator`。

## 迁移步骤

### Phase 1：并行开发（不替换）

1. 在项目中引入 `CanCommCoordinator` 源码和头文件
2. 保留旧 `CanServer` 运行
3. 配置 `CANCOMM_ENABLE_LEGACY_NM_PARSE = STD_ON`
4. 将旧 `CanServer` 的 NM 解析逻辑映射到 `CanCommCoordinator_OnNmRxIndication`

### Phase 2：Adapter 实现

1. 实现 `CanCommCoordinator_VectorAdapter.c` 中的 TODO：
   - `CanComm_Adapter_GetIgnition`（读取 KL15）
   - `CanComm_Adapter_GetLocalKeepAwake`（Diag/OTA/Run-app）
2. 在 VectorCfg 中映射 ComM User、Nm Channel、CanIf RxPduId
3. 验证 `CanComm_Adapter_BuildInputSnapshot` 能正确组装输入

### Phase 3：策略对齐

1. 对照旧 `CanServer` 的业务逻辑，确认 `CanComm_PolicyRuleTable` 覆盖所有场景
2. 调整 `CanComm_DecisionAllFull`、`CanComm_DecisionAdasOnly`、`CanComm_DecisionAllNo`
3. 确认 fallback 策略与旧模块的安全行为一致

### Phase 4：BswM 迁移

1. 将旧 `CanServer` 中直接调用的 `Com_IpduGroupStart/Stop` 迁移到 BswM ActionList
2. 将 `Nm_EnableCommunication` / `Nm_DisableCommunication` 迁移到 BswM ActionList
3. 在 BswM Rule 中引用 `CanCommCoordinator` 的 `bswmRequest` 输出

### Phase 5：切换与回滚准备

1. 在 DaVinci 中停用旧 `CanServer` 的调度（从 Task 中移除 MainFunction）
2. 启用 `CanCommCoordinator_MainFunction` 调度
3. 保留旧源码至少一个版本，确保可回滚

### Phase 6：验证

- [ ] KL15 ON -> all FULL
- [ ] KL15 OFF + ADAS1 PN -> only ADAS1
- [ ] KL15 OFF + no PN + no keep -> all NO / sleep candidate
- [ ] KL15 OFF + invalid NM -> fallback ALL_FULL
- [ ] Wakeup from TJA1145 -> normal restoration
- [ ] DLT/Trace 无异常 Error 计数

## 兼容性开关使用

```c
#define CANCOMM_ENABLE_LEGACY_NM_PARSE STD_ON
```

- 迁移期：保持 `STD_ON`，允许模块解析旧 NM PDU 格式
- 稳定期：改为 `STD_OFF`，由 CanNm/ComM/BswM 接管 PNC 处理
- 最终：移除 `CanCommCoordinator_OnNmRxIndication` 相关代码

## 旧代码清理清单

- [ ] 删除旧 `CanServer.c/.h`
- [ ] 删除旧 `CanServer` 相关的 ComM User 硬编码引用
- [ ] 删除旧 `CanServer` 中的 `Byte5 bit3` 等硬编码解析
- [ ] 更新 BswM ActionList，移除对旧模块的引用
- [ ] 更新 RTE 接口（如果旧模块有 RTE 端口）
