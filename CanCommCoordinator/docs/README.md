# CanCommCoordinator

## 模块用途

`CanCommCoordinator` 是项目级 **CAN 通信策略协调模块**。它根据各通信保持源（如 KL15、PNC、Diag、OTA 等）的激活状态，计算目标通信策略，并通过标准 BSW 接口输出决策。

该模块不是 `CanServer` 的复刻，而是一个可配置、可测试、可迁移、BSW 优先的通信策略协调模块模板。核心框架通过 **SourceMask（multi-word bitset）** 统一管理输入源，不耦合任何项目具体的 source 语义。

## 适用场景

- 需要基于多源输入（KL15、PNC、Diag、OTA、LocalKeep 等）协调通信模式的系统
- 需要处理 ADAS-only PN / Global PN / PNC 策略的系统
- 需要从旧 `CanServer` 迁移到标准 AUTOSAR BSW 架构的项目
- 需要 host 单元测试验证通信策略的项目
- 需要支持超过 32 个通信保持源的系统（通过 `CANCOMM_SOURCE_WORD_NUM` 配置）

## 限制

- 不替代标准 BSW（CanNm、Nm、ComM、BswM、CanSM、CanIf、CanTrcv、Com、EcuM）
- Legacy NM Parse 兼容模式 **默认关闭（STD_OFF）**，仅用于迁移期，长期应由 CanNm/ComM/BswM 接管
- 750ms hold timer 是项目兼容逻辑，非 AUTOSAR 标准 timer
- Adapter 实现由构建系统选择（GenericMock / VectorAdapter），**不使用 weak symbol** 以保证跨编译器可移植性
- 核心框架**不定义**具体 source 名称（IGNITION/PNC/DIAG 等），仅项目配置层（`Cfg.c`）定义
- `SourceConfigTable` 和 `debugName` 默认通过条件编译关闭（`STD_OFF`），以节省 ROM

## 测试方案说明

当前采用 **Unity-only** 轻量测试框架（内嵌于 `tests/support/`），无需外部依赖即可在 host 环境编译运行。

### 为什么不使用完整 CMock/Ceedling？

- CMock 需要 Ruby 运行时代码生成器，增加 CI/CD 环境依赖。
- 本模块的 Adapter 抽象层已通过 `project/generic/CanCommCoordinator_GenericMock.c` 实现手写 Mock，覆盖全部外部依赖。
- 如果项目后续需要 CMock 的自动化 mock 生成能力，扩展路径如下：
  1. 引入 CMock + Unity 源码到 `tests/support/cmock/`
  2. 使用 Ruby 脚本生成 ComM/Nm/BswM 的 mock 头文件到 `tests/mocks/`
  3. 替换 `GenericMock.c` 为 CMock 生成的 mock，保留测试用例不变

## 快速开始

```bash
cd tests
make test
```

## 目录结构

```
CanCommCoordinator/
├── include/        公共头文件
├── src/            核心源码
├── project/        项目绑定层（Vector / GenericMock）
├── tests/          单元测试 + Makefile
├── stub/BSW/       最小 BSW stub，支持脱离 DaVinci 编译
└── docs/           文档
```
