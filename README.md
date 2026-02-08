# EndFieldCheat

## 中文

`EndFieldCheat` 是一个基于 C++ 的 Windows DLL 项目，使用 **MinHook** 进行函数 Hook，使用 **Dear ImGui** 构建界面。

### 环境要求

- Windows 10/11 x64
- Visual Studio 2019/2022（含 C++ 工具链）
- CMake 3.14+

### 获取项目

本项目使用子模块（`imgui`、`minhook`）：

```bash
git clone https://github.com/a0yark/EndFieldCheat.git
cd EndFieldCheat
git submodule update --init --recursive
```

### 本地构建

**方式一：脚本构建（推荐）**

```bat
build.bat
```

默认输出目录：`C:\temp\EndfieldBuild\Release\`

**方式二：手动 CMake**

```bash
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
```

### 自动化构建（GitHub Actions）

- 工作流文件：`.github/workflows/build.yml`
- 触发：`push main`、`pull_request main`、手动触发（`workflow_dispatch`）
- 行为：自动拉取子模块、编译 `Release`、上传 `dll/pdb` 构建产物

### 自动发布（Tag -> GitHub Release）

- 工作流文件：`.github/workflows/release.yml`
- 触发：推送标签 `v*`（例如 `v1.0.0`）
- 行为：自动编译、打包为 zip、创建 GitHub Release 并上传附件

发布示例：

```bash
git tag v1.0.0
git push origin v1.0.0
```

### 项目结构

- `DllMain.cpp`：DLL 入口
- `Cheat.cpp`：主要逻辑
- `Hook.cpp` / `Hook.h`：Hook 实现
- `GameStructs.h`：游戏结构定义
- `imgui/`：Dear ImGui 子模块
- `minhook/`：MinHook 子模块

---

## English

`EndFieldCheat` is a Windows DLL project written in C++, using **MinHook** for function hooking and **Dear ImGui** for UI.

### Requirements

- Windows 10/11 x64
- Visual Studio 2019/2022 (with C++ toolchain)
- CMake 3.14+

### Clone

This repository uses submodules (`imgui`, `minhook`):

```bash
git clone https://github.com/a0yark/EndFieldCheat.git
cd EndFieldCheat
git submodule update --init --recursive
```

### Local Build

**Option 1: build script (recommended)**

```bat
build.bat
```

Default output path: `C:\temp\EndfieldBuild\Release\`

**Option 2: manual CMake**

```bash
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
```

### CI Build (GitHub Actions)

- Workflow: `.github/workflows/build.yml`
- Triggers: `push` on `main`, `pull_request` to `main`, and manual dispatch
- Behavior: checks out submodules, builds `Release`, uploads `dll/pdb` artifacts

### Release Automation (Tag -> GitHub Release)

- Workflow: `.github/workflows/release.yml`
- Trigger: push a tag matching `v*` (e.g. `v1.0.0`)
- Behavior: builds release, packages artifacts to zip, creates GitHub Release with asset upload

Release example:

```bash
git tag v1.0.0
git push origin v1.0.0
```

### Project Layout

- `DllMain.cpp`: DLL entry point
- `Cheat.cpp`: core logic
- `Hook.cpp` / `Hook.h`: hook implementation
- `GameStructs.h`: game structure definitions
- `imgui/`: Dear ImGui submodule
- `minhook/`: MinHook submodule
