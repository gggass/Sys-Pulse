# 🚀 Sys-Pulse: 高性能系统遥测引擎

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/Language-C++17-blue.svg)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Language-Python3.12+-blue.svg)](https://www.python.org/)

**Sys-Pulse** 是一款轻量级、高性能的系统监控工具。它巧妙地结合了 **C++** 的底层采集能力与 **Python** 的优雅界面展示，专为追求极致性能和美观终端体验的开发者设计。

---

## ✨ 核心特性

- **🏎️ 极低开销**：C++ 采集核心直接读取 Linux `/proc` 文件系统或 Windows PDH 接口，资源占用几乎可以忽略。
- **🎨 现代终端 UI**：基于 Python `rich` 库构建，提供响应式、美观的命令行仪表盘。
- **📊 实时指标**：高频采集 CPU 使用率、内存分布、磁盘 IO 以及网络吞吐量。
- **🧩 解耦架构**：后端采集与前端展示通过轻量级 JSON 管道通信，稳定且易于扩展。

## 🛠️ 工作原理

Sys-Pulse 采用生产-消费模型：
1. **采集器 (C++)**：独立运行的二进制程序，负责高频获取系统各项指标，并将结果以 JSON 格式输出到标准输出（stdout）。
2. **仪表盘 (Python)**：负责管理采集器进程、解析数据流，并在终端实时渲染动态 UI。

## 🚀 快速开始

### 环境要求
- **Linux**: GCC/G++ (支持 C++17)
- **Windows**: Visual Studio (MSVC) 或 MinGW
- Python 3.12+

### 安装与运行

1. **克隆仓库**:
   ```bash
   git clone https://github.com/gggass/Sys-Pulse.git
   cd Sys-Pulse
   ```

2. **编译核心**:
   - **Linux**: `make`
   - **Windows (MSVC)**: `cl /EHsc /O2 sys_collector_win.cpp /link pdh.lib /out:sys_collector_win.exe`
   - **Windows (MinGW)**: `g++ -O2 sys_collector_win.cpp -o sys_collector_win.exe -lpdh`

3. **安装 Python 依赖**:
   ```bash
   pip install -r requirements.txt
   ```

4. **启动**:
   ```bash
   python sys_pulse.py
   ```

## 📜 文件说明
- `sys_collector.cpp`: Linux 采集引擎源码 (C++)。
- `sys_collector_win.cpp`: Windows 采集引擎源码 (C++)。
- `sys_pulse.py`: 终端仪表盘脚本 (Python)。
- `Makefile`: Linux 编译脚本。
- `requirements.txt`: Python 依赖库。

## 🤝 贡献指南
欢迎提交 Issue 或 Pull Request 来增加更多监控项或优化 UI 体验。

## 📄 开源协议
本项目采用 MIT 协议开源。

---
由 **gggass** 精心打造。
