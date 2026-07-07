# code-index

`code-index` 是一个面向本地代码仓库的索引与知识图谱服务。它可以解析代码结构、建立可查询的图谱索引，并通过模型上下文协议服务或本地命令行提供检索、架构分析和调用路径追踪能力。

## 功能

- 为本地仓库建立代码知识图谱索引。
- 通过模型上下文协议标准输入输出服务暴露核心工具。
- 查询项目列表、索引状态、图谱结构和架构概览。
- 搜索图谱、搜索代码、追踪调用路径、执行只读图查询。
- 检测代码变更影响。
- 管理架构决策记录。
- 导入运行时追踪数据。
- 读写共享图谱快照文件。
- 通过 `cli <工具名> <参数>` 做本地单次工具调用。

## 项目范围

`code-index` 专注于代码索引、图谱查询和模型上下文协议服务，不包含图形界面、网页前端、一键安装器或多平台分发包装。

## 构建

在项目根目录运行：

```powershell
make -f Makefile.cbm CC=gcc CXX=g++ cbm
```

生成的服务程序位于 `build/c/` 目录，具体文件名以构建输出为准。

下文用 `<服务程序>` 代指构建得到的可执行文件。

## 运行服务

直接启动二进制时，它默认作为模型上下文协议服务运行，通过标准输入输出通信：

```powershell
<服务程序>
```

## 命令行调用

查看版本：

```powershell
<服务程序> --version
```

查看帮助：

```powershell
<服务程序> --help
```

单次调用工具：

```powershell
Set-Content -LiteralPath args.json -Value '{"repo_path":"D:/my project/code-index","mode":"fast"}'
<服务程序> cli list_projects
<服务程序> cli index_repository --args-file args.json
```

## 核心工具

`code-index` 提供以下模型上下文协议工具：

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

启用持久化索引时，项目会在被索引仓库中写入压缩后的知识图谱快照。这个文件适合团队共享，其他使用者可以用它跳过完整重建索引。

## 验证

运行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify-core.ps1
```

脚本会检查：

- 核心源码是否存在。
- 图形界面引用是否残留。
- 安装和分发入口是否残留。
- 是否能完成构建、版本命令、索引烟测和架构查询烟测。
