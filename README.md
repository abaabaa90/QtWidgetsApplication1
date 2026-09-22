# QtWidgetsApplication1 — 仿 ParaView 三维可视化平台

> 一个以学习 VTK 为目的的桌面可视化项目：用 Qt 搭建类似 ParaView 的窗口框架，用 VTK 做三维渲染与交互。

基于 **C++20 + Qt 5.14.2 + VTK 9.5.2** 开发的科研三维可视化桌面程序。整体骨架参照 ParaView 的分层设计：无边框窗口外壳、可拖拽停靠面板、中央多视图分屏、类 Office 的功能区（Ribbon），并在此基础上实现了几何建模、交互式草图绘制与多种格式的数据读取。

## 功能一览

### 三维渲染与交互

- **基本体素建模**：正方体、球体、圆柱、圆锥、平面
- **拾取与高亮**：`vtkCellPicker` 点选模型，选中项以边缘描边高亮
- **变换 gizmo**：`vtkBoxWidget` 拖动实现平移 / 旋转 / 缩放，也可在属性面板直接输入数值
- **交互式草图绘制**：直线、多段线（Enter 结束）、圆（圆心+半径点）、圆弧（三点定弧），绘制过程带橡皮筋实时预览，Esc 退出
- **点云显示**：支持多个数据集同屏，按数据集单独控制显隐
- **视图控制**：相机重置、背景色、坐标轴显示、交互模式切换（旋转 / 平移 / 缩放）

### 数据读取

- **Tecplot ASCII**：自研 `Reader` 手写词法分析器，解析 `Title` / `Varibles` / 数据块
- **高速文本读取**：`TextDataReader` 用 `mio` 内存映射 + OpenMP 分块并行解析，避免大文件整体载入内存
- **线程池分块读文件**：`ThreadPool` / `ThreadPoolPlus` + `ReaderHighSpeedDemo` 演示多线程分块读取
- **DICOM 序列**：`ReadDICOMSeries` 基于 `vtkDICOMImageReader` + `vtkImageViewer2`，鼠标滚轮翻片

### 界面框架

- **工程树**（左停靠）：工程 → 数据集 → 变量 三级结构，支持筛选、重命名、删除、显隐勾选与右键菜单
- **属性面板**（右停靠）：`QStackedWidget` 双页，分别显示数据集元信息与选中模型属性
- **输出日志**（下停靠）：操作与错误日志
- **多视图分屏**：任意视图可水平/垂直切分，可最大化、可关闭，活动视图高亮边框
- **视图浮动**：中央渲染视图可浮出为独立顶层窗口
- **亮 / 暗主题**：20 个语义化颜色角色 + QSS 模板运行时替换
- **布局持久化**：停靠布局与主题记忆到 `QSettings`

### 功能区（Ribbon）

顶部为类 Office 的页签式工具栏。目前开放「几何」一个页签：

| 工具组 | 工具 | 状态 |
| --- | --- | --- |
| 创建 | 正方体 / 球体 / 圆柱 / 圆锥 / 平面 | ✅ 已实现 |
| 编辑 | 删除选中 / 清除全部 | ✅ 已实现 |
| 绘制 | 直线 / 多段线 / 圆 / 圆弧 | ✅ 已实现 |
| 修改 | 移动 / 旋转 / 缩放 / 复制 / 镜像 / 修剪 / 圆角 / 拉伸 | ⚠️ 仅 UI 占位 |
| 注释 | 文字 / 标注 | ⚠️ 仅 UI 占位 |
| 图层 | 图层 | ⚠️ 仅 UI 占位 |
| 选择 | 选择 | ⚠️ 仅 UI 占位 |

> 占位按钮点击后只在输出面板打印「功能待实现」，工具表是数据驱动的（见 `MainWindow.cpp` 的 `kGeometryTools`），**加一个工具只需加一行**。

## 技术栈

| 层次 | 选型 |
| --- | --- |
| 语言标准 | C++20（`/std:c++20`） |
| GUI | Qt 5.14.2 — `core`、`gui`、`widgets`、`printsupport` |
| 三维渲染 | VTK 9.5.2（含 `vtkGUISupportQt`，通过 `QVTKOpenGLNativeWidget` 嵌入） |
| 构建 | Visual Studio 2022（工具集 `v143`）+ Qt VS Tools，MSBuild 工程（`.vcxproj` / `.sln`） |
| 二维绘图 | QCustomPlot（源码内置，暂未接入主界面） |
| 并行 | OpenMP、自研线程池、`mio` 内存映射 |

## 目录结构

```
QtWidgetsApplication1/
├── QtWidgetsApplication1.sln / .vcxproj       VS 解决方案与工程文件
├── main.cpp                                   程序入口（设置应用名 / 字体 / Fusion 风格）
│
├── ── 窗口外壳层 ──
├── BaseWindow.*                               无边框自绘窗口：圆角、阴影、标题栏、拖动、最小/最大/关闭
├── FrameworkWindow.*                          ParaView 风格框架基类：内容区内嵌 QMainWindow 作宿主
├── MainWindow.*                               主窗口：装配菜单 / 工具栏 / 停靠面板 / 功能区
├── TitleBar/BaseTitleBar.*                    自定义标题栏
├── pch.h                                      预编译头
│
├── ── 停靠面板体系 ──
├── DockPanelBase.*                            可复用停靠面板基类（四边可停靠、可浮动）
├── DockTitleBar.*                             面板自定义标题栏
├── DockDragOverlay.*                          拖拽停靠时的覆盖提示层
├── ProjectTreePanel.*                         工程树面板
├── ModelProperties.*                          模型属性编辑面板
│
├── ── 中央多视图 ──
├── RenderViewContainer.*                      递归 QSplitter 容器，支持任意分屏 / 最大化 / 关闭
├── RenderViewFrame.*                          单个视图外框（标题栏 + 分屏 / 最大化 / 关闭按钮）
├── VtkRenderWidget.*                          VTK 渲染控件封装：建模、拾取、gizmo、草图绘制
├── DataSet.h / ModelObject.h                  数据集与模型对象的数据结构
│
├── ── 功能区 ──
├── RibbonBar.* / RibbonPage.* / RibbonGroup.* 类 Office 页签式工具栏
│
├── ── 主题 ──
├── ThemeManager.*                             主题管理器（单例，20 个 ColorRole，Light/Dark）
├── StyleManager.*                             样式辅助
├── theme.qss                                  QSS 模板（含 $token 占位符，运行时替换）
├── style.qss                                  早期样式表（保留）
│
├── ── 数据读取 ──
├── Reader.*                                   Tecplot ASCII 解析器
├── TextDataReader.*                           mmap + OpenMP 高速文本解析
├── ThreadPool.h / ThreadPoolPlus.h            线程池
├── ReaderHighSpeedDemo.cpp                    线程池分块读文件演示
├── SafeQueue.h                                线程安全队列
├── mio.h / mmpio.h                            内存映射库
├── ReadDICOMSeries.cpp                        DICOM 序列读取示例
├── GenerateRandom.cpp                         随机数工具
├── CustomLogTicker.* / WidgetStateController.* 辅助工具
├── file1.txt                                  小型 Tecplot 测试数据
│
├── ── 第三方源码（非本人编写）──
├── qcustomplot.*                              QCustomPlot 二维绘图库
├── TecPlotReader.*                            VTK 官方 vtkTecplotReader 源码拷贝
├── MultiProcessController.*                   VTK 官方 vtkMultiProcessController 源码拷贝
│
└── ── 未启用的试验代码（保留备查）──
    ├── main1.cpp / Serizala.cpp / LegendDraggablePlot.*   整文件被注释
    ├── Start.cpp                                          片段草稿
    ├── AdvancedCurveStyler.*                              配合 QCustomPlot 的曲线样式器
    └── CylinderExample.* / cube_of_pipeline.cpp / distancetotwopoint.cpp   VTK 示例片段
```

## 运行前提

1. **Visual Studio 2022**，安装「使用 C++ 的桌面开发」工作负载（工具集 `v143`）
2. **Qt 5.14.2**（MSVC 2019 64-bit 版本），并安装 **Qt VS Tools** 扩展（工程依赖 `$(QtMsBuild)`，没有这个扩展会报 `QtMsBuildNotFound`）
3. **VTK 9.5.2**，需自行编译并安装，且**必须包含 Qt 支持模块**（`vtkGUISupportQt`，否则找不到 `QVTKOpenGLNativeWidget.h`）

> ⚠️ 工程里 VTK 的路径是**写死的**，见 `QtWidgetsApplication1.vcxproj`：
> - 头文件：`E:\VTK\VTK-9.5.2-install\include\vtk-9.5`
> - 库文件：`E:\VTK\VTK-9.5.2-install\lib\*.lib`
>
> 如果你的 VTK 装在别处，改这两处即可。

**运行时还需要把 DLL 放到能找到的位置**（否则程序启动即闪退）：

- Qt 的 `Qt5Core.dll` / `Qt5Gui.dll` / `Qt5Widgets.dll` / `Qt5PrintSupport.dll`，以及 `platforms\qwindows.dll`
- VTK 的 `vtk*.dll`（`E:\VTK\VTK-9.5.2-install\bin` 下）

最省事的做法是把 VTK 的 `bin` 目录和 Qt 的 `bin` 目录加进系统 `PATH`。

## 构建步骤

1. 用 **Visual Studio 2022** 打开 `QtWidgetsApplication1.sln`
2. 确认 Qt VS Tools 已识别到 Qt 5.14.2（菜单 `Qt VS Tools` → `Qt Versions`）
3. 配置选择 **`Debug | x64`** 或 **`Release | x64`**（工程只提供这两个平台）
4. 生成解决方案（`Ctrl+Shift+B`）
5. 输出在 `x64\Debug\` 或 `x64\Release\`，运行 `QtWidgetsApplication1.exe`

> 工程已设置 `/utf-8` 与 `/wd4819`，但**历史文件里 GBK 与 UTF-8 混用**，部分旧注释在 VS 中会显示为乱码（不影响编译，见「已知问题」）。

## 快速上手

1. 启动后中央是默认的三维场景（一个演示用的圆锥/球/圆柱）
2. 顶部「几何」页签 → 「创建」组，点击正方体 / 球体等即可往场景里添加几何体
3. 鼠标左键点选模型 → 右侧属性面板切到「模型」页，可改颜色、位置、旋转、缩放
4. 拖动模型上的方框 gizmo 可以直接在视图里变换
5. 「绘制」组选一个工具（如「圆」），在视图里点击即可画草图，Esc 退出绘制
6. 菜单「文件 → 打开数据」可载入 Tecplot ASCII 文件（`.dat` / `.txt`），
   仓库自带的 `file1.txt` 就是一个最小示例
7. 视图右上角按钮可水平/垂直分屏；菜单「视图 → 浮动渲染视图」可把视图弹成独立窗口

## 代码架构要点

**三层窗口继承链**：

```
BaseWindow            无边框自绘外壳（圆角 / 阴影 / 标题栏 / 拖动）
   └── FrameworkWindow  在内容区里内嵌一个 QMainWindow 当「宿主 shell」，
                        由它承载菜单栏、工具栏、停靠面板、中央内容、状态栏
          └── MainWindow 具体业务装配
```

**主题机制**（`ThemeManager`）：

1. `qApp->setPalette(paletteFor(theme))` 设置全局调色板
2. 读取 `theme.qss` 模板，把 `$token` 占位符替换成对应主题的颜色，整体 `setStyleSheet()`
3. 发出 `themeChanged` 信号，让硬编码取色的控件（如 `RenderViewFrame`）自行重刷

颜色统一走 `ColorRole` 枚举，避免在控件里散落魔法色值。

**功能区扩展方式**：`MainWindow.cpp` 里的 `kGeometryTools` 是一张静态工具表，
每个工具声明 `{名称, 图标字符, 命令枚举}`。已实现的命令走 `GeoToolCmd` 分发，
未实现的只打日志——所以「加工具」和「实现工具」是解耦的。

## 已知问题与待办

**待补的功能**（按价值排序）

- [ ] **标量场着色**：把点云/网格的标量值映射到颜色（`vtkLookupTable` + Scalar Bar）——这是从「能看」到「能分析」的关键一步
- [ ] **等值面 / 切片 / 流线**：`vtkContourFilter`、`vtkCutter`、`vtkStreamTracer`
- [ ] **体绘制**：`vtkVolumeRayCastMapper`
- [ ] Ribbon「修改」组的移动 / 旋转 / 缩放 / 复制 / 镜像 / 修剪 / 圆角 / 拉伸
- [ ] 「注释」「图层」「选择」三组功能
- [ ] 数据格式扩展：VTK 原生格式（`.vtk` / `.vtu`）、CGNS、STL 等

**代码整洁度问题**

- **编码混乱**：老文件是 GBK，新文件是 UTF-8，同一文件内混用。建议统一转成 UTF-8 with BOM（MSVC 才认），否则乱码会一直存在
- **空文件残留**：以下 10 个文件是 0 字节，但仍被 `vcxproj` 引用，建议清理并从工程里移除
  `DimensionPlane.cpp`、`VTKWidget.cpp`、`VTKWidget.h`、`TitleBar.cpp`、`TitleBar.h`、
  `WindowControl.cpp`、`dicomReader.cpp`、`resource.h`、`vtkCharConvCompatibility.h`、`vtkStringScanner.h`
- **试验代码混在主工程里**：`main1.cpp`、`Serizala.cpp`、`LegendDraggablePlot.*`、`Start.cpp`
  要么整文件被注释，要么是片段草稿。它们目前仍参与编译，建议挪到 `sandbox/` 目录
- **源码目录当数据目录用**：DICOM 测试数据、大体积文本数据曾与源码混放（现已通过 `.gitignore` 排除，但本地目录仍然很大）

## 说明

- 本仓库**不包含** Qt、VTK 本体，也不包含编译产物与测试数据（`.vs/`、`x64/`、`node_modules/`、
  `dicom_demo/`、大体积数据文件等均由 `.gitignore` 排除）
- 仓库内的 `TecPlotReader.*`、`MultiProcessController.*` 是 VTK 官方源码拷贝，
  `qcustomplot.*` 是第三方库，均非本人编写，版权归各自作者
- 本项目为个人 VTK 学习项目，主要用于验证窗口框架与渲染交互的实现方式
