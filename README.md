# 网页版俄罗斯方块 (Web Tetris)

![俄罗斯方块截图](animation.gif)

## 项目介绍

这是一个将命令行俄罗斯方块游戏(`tinytetris.cpp`)改造成网页版的项目。该项目展示了如何将C++实现的游戏核心逻辑与Web前端结合，创建一个可在浏览器中运行的游戏。

### 项目特点

- **跨语言架构**：C++核心逻辑、Python Flask后端、JavaScript前端
- **模块化设计**：核心游戏逻辑与界面展示分离
- **跨平台支持**：可在任何现代浏览器中运行

## 项目结构

```
.
├── CMakeLists.txt       - CMake构建配置文件
├── tetris_game.h        - C++游戏核心头文件
├── tetris_game.cpp      - C++游戏核心实现
├── app.py               - Flask后端服务器
├── requirements.txt     - Python依赖项
├── templates/           - HTML模板
│   └── index.html       - 游戏主页面
├── static/              - 静态资源
│   ├── script.js        - 前端JavaScript代码
│   └── style.css        - CSS样式表
├── tinytetris.cpp       - 原始命令行游戏代码
└── README.md            - 项目说明文档
```

## 技术架构

该项目采用了三层架构设计：

1. **C++核心层**：
   - 实现游戏的核心逻辑，如方块移动、旋转、碰撞检测、行消除等
   - 编译为动态链接库（.so/.dylib/.dll）供后端调用

2. **Python Flask后端**：
   - 使用CFFI（C Foreign Function Interface）加载C++库
   - 提供RESTful API接口与前端交互
   - 管理游戏状态和会话

3. **JavaScript前端**：
   - 使用HTML5 Canvas绘制游戏界面
   - 处理用户输入并发送到后端
   - 实时更新游戏状态和分数

## 安装指南

### 前提条件

- C++编译器（支持C++11）
- CMake (3.10或更高版本)
- Python 3.6+
- pip（Python包管理器）

### 安装步骤

1. **克隆仓库**:
   ```bash
   git clone [repository_url]
   cd tinytetris
   ```

2. **编译C++核心库**:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   cd ..
   ```
   
   这将生成`libtetris_core.so`（Linux）、`libtetris_core.dylib`（macOS）或`tetris_core.dll`（Windows）

3. **安装Python依赖**:
   ```bash
   pip install -r requirements.txt
   ```

## 运行游戏

1. **启动Flask服务器**:
   ```bash
   python app.py
   ```

2. **在浏览器中访问游戏**:
   打开浏览器并访问：`http://localhost:5001`

3. **游戏控制**:
   - W/上箭头: 旋转方块
   - A/左箭头: 向左移动
   - D/右箭头: 向右移动
   - S/下箭头: 加速下落（软降）
   - 空格键: 直接下落到底部（硬降）
   - Q: 重新开始游戏

## 代码解析

### C++核心 (tetris_game.h/cpp)

C++核心实现了所有游戏逻辑，包括：

- 方块形状定义和管理
- 棋盘状态维护
- 碰撞检测
- 行消除和计分
- 游戏循环控制

C++代码通过extern "C"导出C风格的API，便于其他语言调用。

### Flask后端 (app.py)

Flask后端主要做三件事：

1. 使用CFFI加载C++动态库
2. 提供API端点与前端交互
3. 管理游戏状态和会话

主要API端点：
- `/api/start` - 开始新游戏
- `/api/action` - 处理游戏动作（移动、旋转等）
- `/api/state` - 获取当前游戏状态

### JavaScript前端 (script.js)

前端负责：

1. 绘制游戏界面
2. 处理用户输入
3. 与后端API交互
4. 更新游戏状态和UI

## 项目学习要点

通过学习此项目，您可以了解：

1. **C++核心概念**:
   - 类和对象设计
   - 内存管理
   - 位操作和优化
   - 动态库创建

2. **跨语言集成**:
   - 如何通过FFI在不同语言间传递数据
   - C++与Python的集成方式

3. **Web开发**:
   - RESTful API设计
   - Canvas绘图
   - 前后端交互

4. **项目架构**:
   - 如何将一个单文件程序重构为模块化系统
   - 关注点分离原则的应用

## 故障排除

### 常见问题

1. **找不到动态库**
   - 确保库文件在正确位置（根目录或build目录）
   - 检查库文件名是否正确
   - 在macOS上可能需要设置DYLD_LIBRARY_PATH

2. **编译错误**
   - 确保使用支持C++11的编译器
   - 检查CMake版本是否至少为3.10

3. **后端启动失败**
   - 检查Python依赖是否正确安装
   - 检查端口5001是否被占用

## 扩展项目的想法

1. **多人模式**：添加WebSocket支持，实现多人对战
2. **难度级别**：实现不同难度的游戏速度
3. **主题切换**：添加多种颜色主题
4. **排行榜**：添加高分排行榜功能
5. **移动版本**：优化触摸屏控制

## 致谢

本项目基于原始的`tinytetris.cpp`命令行游戏，感谢原作者的精巧实现。

## 许可证

本项目采用MIT许可证，详情请参阅LICENSE文件。
