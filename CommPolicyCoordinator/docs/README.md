# CommPolicyCoordinator

## 模块用途

`CommPolicyCoordinator` 是项目级 **跨总线通用通信策略协调框架**。它根据各通信保持源（如 PowerRun、Network、Diag、OTA 等）的激活状态，计算目标通信策略，并通过标准 BSW 接口输出决策。

该框架支持 **CAN / LIN / ETH / FlexRay / User-defined** 等多种总线类型，不是 `CanServer` 的复刻，而是一个可配置、可测试、可迁移、BSW 优先的通信策略协调框架模板。核心框架通过 **SourceMask（multi-word bitset）** 统一管理输入源，通过 **ChannelMask（multi-word bitset）** 统一管理输出通道，不耦合任何项目具体的 source 语义或总线类型。

## 适用场景

- 需要基于多源输入协调通信模式的系统（不限于 CAN）
- 需要处理 Partial Network / Global Network 策略的系统
- 需要从旧 `CanServer` 迁移到标准 AUTOSAR BSW 架构的项目
- 需要 host 单元测试验证通信策略的项目
- 需要支持超过 32 个通信保持源的系统（通过 `COMM_POLICY_SOURCE_WORD_NUM` 配置）
- 需要支持超过 32 个通信通道的系统（通过 `COMM_POLICY_CHANNEL_WORD_NUM` 配置）

## 限制

- 不替代标准 BSW（Nm、ComM、BswM、CanSM、CanIf、CanTrcv、Com、EcuM、LinSM、EthSM 等）
- Adapter 实现由构建系统选择（GenericMock / VectorAdapter），**不使用 weak symbol** 以保证跨编译器可移植性
- 核心框架**不定义**具体 source 名称（POWER_RUN/NETWORK/DIAG 等），仅项目配置层定义
- `SourceConfigTable` 和 `debugName` 默认通过条件编译关闭（`STD_OFF`），以节省 ROM

## 测试方案说明

当前采用 **Unity-only** 轻量测试框架（内嵌于 `tests/support/`），无需外部依赖即可在 host 环境编译运行。

### 为什么不使用完整 CMock/Ceedling？

- CMock 需要 Ruby 运行时代码生成器，增加 CI/CD 环境依赖。
- 本框架的 Adapter 抽象层已通过 `project/generic/CommPolicy_GenericMock.c` 实现手写 Mock，覆盖全部外部依赖。
- 如果项目后续需要 CMock 的自动化 mock 生成能力，扩展路径如下：
  1. 引入 CMock + Unity 源码到 `tests/support/cmock/`
  2. 使用 Ruby 脚本生成 ComM/BswM 的 mock 头文件到 `tests/mocks/`
  3. 替换 `GenericMock.c` 为 CMock 生成的 mock，保留测试用例不变

## 快速开始

```bash
cd tests
mingw32-make test
```

## 目录结构

```
CommPolicyCoordinator/
├── include/        公共头文件
├── src/            核心源码
├── project/        项目绑定层（Vector / GenericMock）
│   ├── generic/    通用示例配置（host 测试 / 框架验证）
│   └── vector/     Vector/MICROSAR 绑定
├── examples/       业务场景参考示例
│   └── can_adas_pnc/   CAN+ADAS+PNC 业务示例
├── tests/          单元测试 + Makefile
├── stub/BSW/       最小 BSW stub，支持脱离 DaVinci 编译
└── docs/           文档
```
