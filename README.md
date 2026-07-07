# code-index

`code-index` 是从 `codebase-memory-mcp` 裁剪出来的精简核心版。它保留代码索引、知识图谱存储、图谱查询、模型上下文协议服务、后台监听、运行追踪导入和共享图谱文件能力，去掉图形界面、自动安装配置和多平台分发包装。

## 保留功能

- 为本地仓库建立代码知识图谱索引。
- 通过模型上下文协议标准输入输出服务暴露核心工具。
- 查询项目、索引状态、图谱结构和架构概览。
- 搜索图谱、搜索代码、追踪调用路径、执行只读图查询。
- 检测代码变更影响。
- 管理架构决策记录。
- 导入运行时追踪数据。
- 读写 `.codebase-memory/graph.db.zst` 共享图谱文件。
- 通过 `cli <工具名> <参数>` 做本地单次工具调用。

## 已移除功能

- 图形界面和三维图谱可视化。
- 前端工程和网页资源嵌入构建。
- 一键安装脚本。
- 自动配置各种编程助手。
- 包管理器分发目录。
- 只服务发布、安装、前端和分发的脚本与测试。

## 构建

在项目根目录运行：

```powershell
make -f Makefile.cbm CC=gcc CXX=g++ build/c/codebase-memory-mcp
```

生成的二进制通常位于：

```text
build/c/codebase-memory-mcp
```

## 运行服务

直接启动二进制时，它默认作为模型上下文协议服务运行，通过标准输入输出通信：

```powershell
build/c/codebase-memory-mcp.exe
```

## 命令行调用

查看版本：

```powershell
build/c/codebase-memory-mcp.exe --version
```

查看帮助：

```powershell
build/c/codebase-memory-mcp.exe --help
```

单次调用工具：

```powershell
Set-Content -LiteralPath args.json -Value '{"repo_path":"D:/my project/code-index","mode":"fast"}'
build/c/codebase-memory-mcp cli list_projects
build/c/codebase-memory-mcp cli index_repository --args-file args.json
```

## 核心工具

精简版保留以下模型上下文协议工具：

- `index_repository`
- `index_status`
- `list_projects`
- `delete_project`
- `search_graph`
- `search_code`
- `trace_path`
- `detect_changes`
- `query_graph`
- `get_graph_schema`
- `get_code_snippet`
- `get_architecture`
- `manage_adr`
- `ingest_traces`

## 共享图谱文件

启用持久化索引时，项目会写入：

```text
.codebase-memory/graph.db.zst
```

这个文件是压缩后的知识图谱快照，适合团队共享，其他使用者可以用它跳过完整重建索引。

## 验证

运行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify-core.ps1
```

脚本会检查：

- 禁止目录和安装分发入口是否已移除。
- 核心源码是否存在。
- 图形界面引用是否残留。
- 命令行安装分发逻辑是否残留。
- 如果本机有 `make`，会尝试构建并运行版本命令。
