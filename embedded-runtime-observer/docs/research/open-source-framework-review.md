# 开源框架调研与选择

调研日期：2026-10-09。以下依据官方项目、文档和实际读取的公开头文件；未在 TC397 上对这些工具做横向性能测试。当前版本的代码独立编写，没有复制第三方实现。

## 1. 各方案解决的问题

| 方案 | 定位与许可核查 | 值得参考的机制 | 本项目的选择 |
|---|---|---|---|
| [Percepio TraceRecorderSource](https://github.com/percepio/TraceRecorderSource) | 目标端事件记录器；仓库标示 Apache-2.0 | 已读取 `include/trcISR.h`：每核 ISR 栈，以及 ISR 结束后可能发生任务切换的显式状态 | 参考每核所有权与嵌套模型；不捆绑 Tracealyzer，不移植完整 Recorder |
| [barectf](https://github.com/efficios/barectf) | 生成输出 CTF 的 C tracer；MIT | [Platform API](https://barectf.org/docs/barectf/3.1/platform/api.html) 分离时钟与数据流的存储/输出 | 后续编码组件候选；MVP 不需要 YAML 生成器作为构建依赖 |
| [Eclipse Trace Compass](https://github.com/eclipse-tracecompass/org.eclipse.tracecompass) | Host trace 分析与可视化；EPL-2.0 | 记录与离线分析分工；仓库含 CTF/BTF 模块 | 作为未来消费端，不嵌入 MCU 核心 |
| [Zephyr Tracing](https://docs.zephyrproject.org/latest/services/tracing/index.html) | RTOS 内的 tracing 子系统；Zephyr 根许可 Apache-2.0，逐文件还需核查 | 格式与 backend 分离，可复用既有 RTOS 插桩 | 自定义 port 参考；不把 Zephyr 内核带入 AUTOSAR 或裸机项目 |
| [Perfetto](https://perfetto.dev/docs/instrumentation/track-events) | Host 分析/显示及跨平台 tracing SDK；[根许可](https://github.com/google/perfetto/blob/main/LICENSE) 为 Apache-2.0 | Track、嵌套 slice、counter 的可视化语义 | 实现 Host instant-event JSON 导出；不把 SDK 作为 C99 MCU 依赖 |
| [SEGGER SystemView](https://www.segger.com/products/development-tools/systemview/) | 完整工具链；官网区分 Commercial-use 与 Friendly License | RTOS Hook 到目标事件再到 Host 的闭环 | 用于技术比较；不视为可随意复制分发的 MIT/Apache 组件 |
| [FreeRTOS 原生统计](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/08-Run-time-statistics) | RTOS 自带运行时间统计；[Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) 为 MIT | 优先使用系统已有统计接口，减少额外插桩 | 接入前仍需确定 IRQ 的归属，不能从 API 名称推导 exclusive 语义 |

许可只描述所查根许可/相关文件，不能代替任意版本、子目录、Host 工具及商业包的逐项核查。由于本项目只链接自己的 C 源码，选用 MIT；参考链接和设计理念没有作为第三方代码复制进来。

## 2. 关键维度的取舍

| 维度 | 本项目决策与需要核查的事实 |
|---|---|
| 采集与统计 | 内核接收执行事件，在线分类累计；Trace 可关闭，丢包不影响累计 |
| Task | 切换事件测 dispatch 区间；Activation、Runnable 计数靠独立测量点，不从 Hook 次数猜测 |
| ISR | 每核有界调用栈；exclusive 只加最上层，inclusive 加所有活动 ISR 帧 |
| 多核 | 每核单写者，复制后发布；不提供无保护远程读取，也不假设全局时间同步 |
| 时间源 | 低位计数器模减 + 明确采样间隔；60 秒窗口用 64 位累计，不延长低位计数器的可观测回绕周期 |
| 性能 | 仅提供可复现 Host 基准；成熟工具/本项目的目标 RAM、ROM 和 WCET 都需在同一硬件、配置下测量 |
| 实时性 | 核心没有动态分配、传输或锁回调；port 负责短临界区，全部循环由静态容量/采样契约界定 |
| 可移植性 | explicit timestamp 接口不依赖 OS；由 port 查询 Core ID、时钟并映射 Task/ISR ID |
| 输出 | 字段级 CSV→JSON 在 Host；CTF/BTF 留在可选导出层，避免 MCU 实现多个文件格式 |
| 测试 | Mock 事件流、逐 tick 独立 oracle、回绕、ISR/函数递归、窗口边界、受锁保护的并发快照 |

## 3. 为什么仍需要一个小核心？

这是针对本项目目标的设计判断：成熟工具提供的可视化和采集生态很有价值，但一个希望复用在裸机、不同 RTOS 和 AUTOSAR 中的在线统计内核，仍需要独立、明确的时间归属契约。函数对的 wall time、任务 Hook 区间、CPU exclusive time 是不同量。给事件命名并不能自动保证这三个量相等。

因此只自行实现时间归属、嵌套状态、窗口切分、静态指标和最小事件缓冲；格式编码、分析界面和已有 RTOS 事件来源应优先复用。对 TC397 而言，默认使用全局同步时钟、直接读取 64 位变量、在 ISR 中阻塞传输，以及不受控制的内存分配都不合适。该判断不是对上述项目的缺陷指控；实际成熟 port 也可能已经解决这些问题。

## 4. 技术调研边界

实际读取了 Percepio ISR 公共 API/结构及 barectf 的平台接口文档，核查了各项目官方定位和根许可。部分 AUTOSAR PDF 和源码 URL 的获取失败，不据此声称已经阅读完整实现。没有用项目主页推定特定 TC397 port 已存在、已编译或已通过硬件测试。现有 MCU/OS 的可接入边界仍以本地源码、配置和供应商文档核查为准。
