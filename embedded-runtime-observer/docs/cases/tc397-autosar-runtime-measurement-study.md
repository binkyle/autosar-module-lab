# TC397 + AUTOSAR 测量案例：可复现模型与待核查工程

**状态：模型已在 Host 验证；真实业务源码未访问；硬件适配 Experimental / Unverified。**

本次输入是一份研究开发任务书，描述了 TC397、Vector MICROSAR OS、Task Hook、STM、RTM 测量点和约 60 秒的统计窗口。当前环境没有该 Windows 工程，也没有上传完整源码或先前调查报告。任务书中列出的潜在问题是调查线索，不能作为本次源码审查结果。

## 1. 已知背景与假设分开

| 输入线索 | 本次可证明的结论 | 真实工程仍需的证据 |
|---|---|---|
| ISR 时间可能计入 Idle | 若只结算跨越 ISR 的 Idle 区间，该算法会把中断误记为 Idle；数值模型已复现 | Idle/ISR 的实际 Hook 时序与扣减逻辑 |
| `ISR_PREEMPTED` 未启用 | 未核实，不能作事实发布 | 宏定义、构建选项、预处理结果 |
| ISR Hook 数组索引错误 | 未核实 | 数组声明、Core/Task/ISR ID 映射及每个读写表达式 |
| ISR 嵌套无状态保存 | 单一开始时间不能表示多个活动 ISR；通用栈模型已验证 | 实际状态字段、嵌套配置和入口顺序 |
| Idle 依赖 PostTaskHook | 仅延迟结算会把窗口截断前的活跃 Idle 漏记；模型已验证 | 周期结算实现、Hook 与窗口边界的关系 |
| 多核快照无同步 | 无保护读取 64 位变量在 C 中不能保证一致快照 | 所有写者、mask/锁协议、内存与发布方式 |
| RTM 覆盖有限 | 未核实 | RTM Point 表、每个调用点、Cat1/Cat2 与 OS 包装层边界 |

这些结论描述必要条件或反例，不确认某辆 ECU 已出现对应问题，也不估计该工程的实际误差。

## 2. 应还原的调用链

| 层 | 需识别的角色 | 可公开交付 |
|---|---|---|
| OS | Dispatch、Idle 路径、ISR 包装与调度边界 | 抽象状态转换与验证方法 |
| 生成配置 | Task/Core 映射、Counter、Hook、Measurement Point 配置 | 脱离业务名称的映射示意 |
| 应用 | Hook/RTM 回调到统计字段的读写 | 独立编写的 mock adapter |
| 硬件 | STM 配置、频率、访问权限、Core ID 编码 | BSP 输入契约与寄存器读取策略 |

`PreTaskHook/PostTaskHook` 是 OS 任务边界入口；项目的 ISR 回调命名和 `Rtm_Start/Stop` 不等价于对所有 OS 厂商都成立的标准测量边界。本项目 API 也不声称兼容 Vector RTM。具体生成宏与 ARTI 的对应关系需在使用版本中核查。

一般调度模型里，Idle 在没有可运行普通任务时成为执行背景，不靠周期激活模拟“空闲”。但 Vector 内部 Idle 是否经过应用 Task Hook 必须查实际实现。Extended Task 在 `WaitEvent` 真正进入等待时让出执行权；所等事件已经满足时不能默认发生一次切换。被抢占的 Task 恢复后需继续累计它自己的执行时间，而不是把激活次数加一。

## 3. 可复现数值案例

`examples/basic.c`：Task 0–20，Idle 20–50，ISR 50–60，Idle 60–100。

正确结果：Task=20，ISR=10，Idle=70，Total=100，Load=30%。如果把 Idle 的 20–100 区间直接记为 80，则 Load=20%，少计 10 个百分点。这是模型误差，不是实车测量值。

`examples/task_isr.c`：Idle→A→B→A→Task→Idle，得到 A inclusive=40、exclusive=30；B=10；物理 ISR=40。只打开一个抢占处理宏不能替代每层 ISR 的保存/恢复、执行点覆盖、正确索引、窗口切分和同步协议。

`examples/autosar_concept.c` 使用自编的 Hook 模型与 Mock STM：Idle=56，Task=30，ISR=10，Kernel gap=4，Load=44%。函数名 `mock_pre_taskhook/mock_post_taskhook` 是示意封装，不是供应商正式接口签名。`complete_coverage=0` 明确表示这些接口还不能证明真实系统的完整边界覆盖。

## 4. AUTOSAR 接入契约

1. 每个逻辑核配置独立 `rtm_core_t`，建立查表式 ID 映射，不用硬件 Core ID 直接索引假定连续的数组。
2. 在已核实的 dispatch 边界发 `rtm_task_switch`；若 Post/Pre 对覆盖调度空档，可映射 Post→Kernel、Pre→新 Task/Idle。
3. ISR enter 必须先于被测主体，exit 必须和 top ID 配对；最外层退出后，再记录实际任务切换。若供应商 Hook 顺序不同，先规范化事件，不能直接套用 mock 顺序。
4. 按安全且受支持的 OS/架构规则屏蔽本核所有观测写者；加保护后再读取 STM。Hook 中只采集，不打印、不传输、不分配。
5. 配置周期 poll，即使 Idle 不切换也能结束窗口。ISR 中发生 poll 时会结算当时的 ISR，而不是误记成被中断的任务。
6. 由拥有者核生成 snapshot 后通过已验证的 IOC/消息发布协议传给汇总核。各核副本带时间范围；未经校时和协调，不能称为同一时刻的全局快照。

## 5. STM 条件与可编译策略

`ports/stm_low32.c` 接收 BSP 提供的、有效且对齐的 TIM0 低 32 位寄存器指针，执行一次 volatile 读取。不含固定地址，不引用专有头文件。`examples/baremetal.c` 用 Mock 寄存器验证了该策略的 Host 可编译性；这不证明 TC397 上的 MMIO、权限或时序已验证。

必须核查真实 STM 频率、divider、睡眠/调试时是否停计、访问权限、寄存器宽度与读出方式。当前示例没有配置硬件。频率 100 MHz 仅用于计算示例：32 位完整回绕约 42.95 秒，半范围约 21.47 秒；60 秒窗口需分段采样并用 64 位累计。

Infineon 的[官方说明](https://community.infineon.com/t5/Knowledge-Base-Articles/How-to-synchronously-read-the-64-bit-counter-value-in-AURIX-STM/ta-p/772814)说明读取低部时 CAP 锁存高部。若采用 TIM0→CAP 的双读策略，还要证明另一核/嵌套读者不会在两次读取之间改写锁存结果。当前选择低 32 位采样避免依赖这种跨读取配对；仍不假设不同 STM 模块或不同核的 epoch 天然一致。以[设备手册和具体型号附录](https://documentation.infineon.com/aurixtc3xx/docs/qmd1702366622648)为最终依据。

## 6. 下一轮只读取证

在可访问工程的机器运行：

```powershell
python tools/collect_case_evidence.py "D:\work\ecu\host" --output "D:\reports\runtime-evidence.md"
```

路径是示意。脚本只读白名单源码及邻近调用文件，输出带 SHA-256 和行号的局部证据。输出必须位于业务工程外，已存在的输出会被拒绝。它不编译、不预处理、不修改业务工程，也不是完整审查器。沿每个调用点继续查宏、生成 RTM 表、ISR 包装、Idle 调度与全部统计写者后，才能将待核查项升级为源码事实。生成的专有源码摘录只用于本地审查，不直接发布。

硬件验证建议用可控制的 Task/IRQ 负载、GPIO/逻辑分析仪或独立计时进行对照，验证四种抢占链、窗口跨界、所有核、异常/溢出、最大 ISR 深度，以及 Hook/快照自身最坏开销。当前没有这些硬件结果。
