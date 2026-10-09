# autosar-module-lab

面向嵌入式与 AUTOSAR 工程的独立模块集合。每个模块包含自己的接口、示例、测试与接入文档，可以单独阅读和构建。

| 模块 | 用途 | 文档入口 |
|---|---|---|
| [CommPolicyCoordinator](CommPolicyCoordinator/) | 跨总线通信策略协调，连接项目输入源与标准 BSW 接口 | [模块说明](CommPolicyCoordinator/docs/README.md) |
| [embedded-runtime-observer](embedded-runtime-observer/) | C99 Runtime Measurement：每核 Task、Idle、ISR、函数时间和固定窗口统计 | [中文说明](embedded-runtime-observer/README_zh-CN.md) · [English](embedded-runtime-observer/README.md) |

## Runtime Measurement 快速开始

Linux/Unix Host 环境需要 C99 编译器、GNU Make、Python 3，以及并发测试使用的 pthread。

```sh
git clone https://github.com/binkyle/autosar-module-lab.git
cd autosar-module-lab/embedded-runtime-observer
make test
./build/basic
```

基础示例输出：`Task=20 Idle=70 ISR=10 Total=100 Load=30.0%`。

模块采用静态内存和显式时间戳接口。Host 示例与测试已验证；TC397/MICROSAR 接入文档提供平台适配契约和 Mock 示例，真实目标编译与硬件验证由具体工程完成。

配套博客：[嵌入式运行时测量专题](https://binkyle.github.io/blog/Autosar/RuntimeMeasurement/)。

