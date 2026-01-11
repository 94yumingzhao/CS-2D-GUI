# CS-2D-GUI: 二维下料求解器图形界面

> CS-2D-BP-Arc 分支定价求解器的 Qt 6 图形前端
>
> 功能: 求解控制 | 实例生成 | 切割可视化

---

## 目录

### 第一部分: 功能概述

- 1 [项目简介](#1-项目简介)
- 2 [功能模块](#2-功能模块)

### 第二部分: 界面说明

- 3 [求解 Tab](#3-求解-tab)
- 4 [生成 Tab](#4-生成-tab)
- 5 [可视化 Tab](#5-可视化-tab)

### 第三部分: 代码实现

- 6 [程序架构](#6-程序架构)
- 7 [核心组件](#7-核心组件)
- 8 [构建与运行](#8-构建与运行)

---

# 第一部分: 功能概述

## 1. 项目简介

### 1.1 功能定位

CS-2D-GUI 是 CS-2D-BP-Arc 求解器的图形用户界面，提供:

- 可视化的参数配置和子问题方法选择
- 实时的五阶段求解进度监控
- 测试算例的难度控制生成
- 切割方案的图形化展示

### 1.2 核心求解器

调用 CS-2D-BP-Arc.exe 进行实际求解:

- 算法框架: Branch and Price = Column Generation + Branch and Bound
- 子问题方法: CPLEX IP / Arc Flow / DP

### 1.3 关联项目

| 项目 | 说明 |
|:-----|:-----|
| [CS-2D-BP-Arc](https://github.com/94yumingzhao/CS-2D-BP-Arc) | 核心求解器 |
| [CS-2D-Data](https://github.com/94yumingzhao/CS-2D-Data) | 算例生成器 |

---

## 2. 功能模块

### 2.1 三大功能 Tab

| Tab | 功能 | 说明 |
|:---:|:-----|:-----|
| 求解 | 运行求解器 | 加载算例、配置参数、启动求解、监控进度 |
| 生成 | 创建测试算例 | 难度控制、批量生成 |
| 可视化 | 切割方案展示 | 加载结果、翻页浏览、导出图片 |

### 2.2 技术栈

| 项目 | 说明 |
|:----:|:-----|
| 编程语言 | C++17 |
| GUI 框架 | Qt 6.10 |
| 编译器 | MSVC 2022 |
| 构建系统 | CMake 3.24+ |

---

# 第二部分: 界面说明

## 3. 求解 Tab

### 3.1 左侧控制面板

**文件选择区**:
- 浏览按钮: 选择 CSV/TXT 算例文件
- 文件信息: 显示子板类型数、母板尺寸、总需求量

**参数配置**:
- 时间限制 (秒)
- SP1 子问题方法: CPLEX IP / Arc Flow / DP
- SP2 子问题方法: CPLEX IP / Arc Flow / DP

**结果摘要**:
- 最优母板数
- 根节点下界
- 最优性 Gap
- 分支节点数
- 总利用率

### 3.2 右侧面板

**CPLEX 设置**:
- 线程数
- 内存限制

**实时日志**:
- 五阶段进度 (数据读取 / 启发式 / 根节点CG / 整数性检查 / 分支定价)
- 列生成迭代信息
- 分支节点信息

---

## 4. 生成 Tab

### 4.1 难度控制

| 难度 | 范围 | 策略 | 适用场景 |
|:----:|:----:|:----:|:---------|
| 简单 | 0.0-0.3 | 逆向生成 | 算法验证 |
| 中等 | 0.3-0.8 | 参数化随机 | 性能测试 |
| 困难 | 0.8-1.0 | 残差算例 | 压力测试 |

### 4.2 规模配置

| 参数 | 范围 | 说明 |
|:----:|:----:|:-----|
| 母板宽度 | 100-5000 | X 方向 |
| 母板高度 | 100-5000 | Y 方向 |
| 难度系数 | 0.0-1.0 | 自动派生其他参数 |
| 生成数量 | 1-100 | 批量生成 |

### 4.3 输出

- 预览生成配置
- 输出目录选择
- 实时生成进度

---

## 5. 可视化 Tab

### 5.1 结果加载

- 加载求解器输出的 JSON 结果文件
- 解析切割方案数据

### 5.2 切割方案浏览

**导航控制**:
- 上一块/下一块母板
- 母板下拉选择
- 当前页/总页数显示

**方案展示**:
- 母板轮廓
- 子板位置和尺寸
- 类型编号标注
- 颜色区分不同类型

**指标显示**:
- 当前母板利用率
- 子板数量统计

### 5.3 导出功能

- 导出当前方案为 PNG 图片
- 导出全部方案

---

# 第三部分: 代码实现

## 6. 程序架构

### 6.1 目录结构

```
CS-2D-GUI/
+-- CMakeLists.txt
+-- CMakePresets.json
+-- README.md
+-- src/
    +-- main.cpp                    # 程序入口
    +-- main_window.h/cpp           # 主窗口
    +-- parameter_widget.h/cpp      # 参数配置
    +-- cplex_param_widget.h/cpp    # CPLEX 设置
    +-- results_widget.h/cpp        # 结果显示
    +-- log_widget.h/cpp            # 日志输出
    +-- solver_worker.h/cpp         # 求解器后台线程
    +-- generator_widget.h/cpp      # 实例生成控件
    +-- generator_worker.h/cpp      # 生成器后台线程
    +-- cutting_view_widget.h/cpp   # 切割方案绘制
    +-- difficulty_mapper.h/cpp     # 难度参数映射
```

---

## 7. 核心组件

### 7.1 组件职责

| 组件 | 文件 | 职责 |
|:-----|:-----|:-----|
| MainWindow | main_window.cpp | 主窗口，管理 Tab 切换 |
| ParameterWidget | parameter_widget.cpp | 求解参数配置 |
| SolverWorker | solver_worker.cpp | 后台调用求解器 |
| GeneratorWidget | generator_widget.cpp | 算例生成界面 |
| GeneratorWorker | generator_worker.cpp | 后台生成算例 |
| CuttingViewWidget | cutting_view_widget.cpp | 切割方案绘制 |
| LogWidget | log_widget.cpp | 实时日志显示 |

### 7.2 切割绘制

CuttingViewWidget 负责:

- 解析 JSON 中的切割方案
- 计算子板在母板中的位置
- 使用 QPainter 绘制矩形
- 支持缩放和翻页

### 7.3 求解器信号

| 信号 | 参数 | 时机 |
|:-----|:-----|:-----|
| DataLoaded | 类型数, 宽, 高, 总需求 | 数据加载完成 |
| ResultsReady | 最优值, 下界, Gap, 节点数, 利用率 | 求解完成 |
| SolutionReady | JSON 路径 | 结果文件生成 |
| SolverLogMessage | 消息 | 日志输出 |

---

## 8. 构建与运行

### 8.1 环境要求

| 软件 | 版本 |
|:-----|:-----|
| Windows | 10/11 x64 |
| Visual Studio | 2022 |
| Qt | 6.10+ |
| CMake | 3.24+ |

### 8.2 Qt 配置

```cmake
set(CMAKE_PREFIX_PATH "D:/Tools-DV/Qt/6.10.1/msvc2022_64")
```

### 8.3 构建命令

```bash
cmake --preset vs2022-release
cmake --build build/vs2022 --config Release
```

### 8.4 部署

```bash
D:\Tools-DV\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe build/vs2022/bin/Release/CS-2D-GUI.exe
```

### 8.5 运行要求

- CS-2D-BP-Arc.exe 在同目录或 PATH 中
- Qt 运行时库已部署

---

**文档版本**: 1.0
**更新日期**: 2026-01-11
