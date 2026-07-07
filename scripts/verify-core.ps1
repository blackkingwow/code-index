$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

$forbidden = @(
    "graph-ui",
    "src/ui",
    "pkg",
    "install.sh",
    "install.ps1"
)

foreach ($item in $forbidden) {
    $path = Join-Path $Root $item
    if (Test-Path -LiteralPath $path) {
        throw "forbidden item still exists: $item"
    }
}

$required = @(
    "README.md",
    "src/main.c",
    "src/mcp/mcp.c",
    "src/pipeline/pipeline.c",
    "src/store/store.c",
    "src/cypher/cypher.c"
)

foreach ($item in $required) {
    $path = Join-Path $Root $item
    if (-not (Test-Path -LiteralPath $path)) {
        throw "required file is missing: $item"
    }
}

Write-Host "structure check passed"

$mainPath = Join-Path $Root "src/main.c"
$main = Get-Content -LiteralPath $mainPath -Raw
if ($main -match 'ui/|cbm_ui_|http_server|--ui|--port') {
    throw "src/main.c still references ui entrypoints"
}

$makefilePath = Join-Path $Root "Makefile.cbm"
$makefile = Get-Content -LiteralPath $makefilePath -Raw
if ($makefile -match 'graph-ui|src/ui|UI_SRCS|embedded_assets') {
    throw "Makefile.cbm still references ui build inputs"
}

Write-Host "ui reference check passed"

$cliPath = Join-Path $Root "src/cli/cli.c"
$cli = Get-Content -LiteralPath $cliPath -Raw
if ($cli -match 'install|uninstall|self-update|download|agent configuration|Claude|Codex CLI|Gemini') {
    throw "src/cli/cli.c still contains installer or distribution logic"
}

Write-Host "cli trim check passed"

$make = Get-Command make -ErrorAction SilentlyContinue
if ($make) {
    Push-Location $Root
    try {
        make -f Makefile.cbm CC=gcc CXX=g++ build/c/codebase-memory-mcp
        if ($LASTEXITCODE -ne 0) {
            throw "build failed with exit code $LASTEXITCODE"
        }
    } finally {
        Pop-Location
    }

    $bin = Join-Path $Root "build/c/codebase-memory-mcp.exe"
    if (-not (Test-Path -LiteralPath $bin)) {
        $bin = Join-Path $Root "build/c/codebase-memory-mcp"
    }
    if (-not (Test-Path -LiteralPath $bin)) {
        throw "build did not produce binary"
    }
    & $bin --version | Out-Host

    $verifyDir = Join-Path $Root "verification"
    New-Item -ItemType Directory -Path $verifyDir -Force | Out-Null
    Get-ChildItem -LiteralPath $verifyDir -Directory -Filter "verify-smoke-repo-*" -ErrorAction SilentlyContinue |
        Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
    $smokeRepo = Join-Path $verifyDir ("verify-smoke-repo-" + [guid]::NewGuid().ToString("N"))
    New-Item -ItemType Directory -Path $smokeRepo -Force | Out-Null
    @'
#include <stdio.h>

static int add_one(int value) {
    return value + 1;
}

int main(void) {
    int result = add_one(41);
    printf("%d\n", result);
    return 0;
}
'@ | Set-Content -LiteralPath (Join-Path $smokeRepo "main.c") -Encoding ascii

    $git = Get-Command git -ErrorAction SilentlyContinue
    if ($git) {
        Push-Location $smokeRepo
        try {
            git init | Out-Null
            git add main.c | Out-Null
            git -c user.name='Codex' -c user.email='codex@example.invalid' commit -m 'init smoke repo' | Out-Null
        } finally {
            Pop-Location
        }
    }

    $indexArgs = Join-Path $verifyDir "verify-index-args.json"
    @{ repo_path = $smokeRepo; persistence = $true } | ConvertTo-Json | Set-Content -LiteralPath $indexArgs -Encoding utf8
    $indexOut = & $bin cli index_repository --args-file $indexArgs
    if ($LASTEXITCODE -ne 0) {
        throw "index_repository smoke check failed: $indexOut"
    }
    $indexJson = $indexOut | ConvertFrom-Json
    $indexResult = if ($indexJson.result) { $indexJson.result } else { $indexJson }
    if ($indexResult.status -ne "indexed" -or -not $indexResult.artifact_present) {
        throw "index_repository smoke check returned unexpected result: $indexOut"
    }

    $graphPath = Join-Path $smokeRepo ".codebase-memory/graph.db.zst"
    if (-not (Test-Path -LiteralPath $graphPath)) {
        throw "index_repository smoke check did not create graph artifact"
    }

    $archArgs = Join-Path $verifyDir "verify-arch-args.json"
    @{ project = $indexResult.project; aspects = @("all") } | ConvertTo-Json | Set-Content -LiteralPath $archArgs -Encoding utf8
    $archOut = & $bin cli get_architecture --args-file $archArgs
    if ($LASTEXITCODE -ne 0) {
        throw "get_architecture smoke check failed: $archOut"
    }
    $archJson = $archOut | ConvertFrom-Json
    $archResult = if ($archJson.result) { $archJson.result } else { $archJson }
    if ($archResult.total_nodes -lt 1 -or $archResult.total_edges -lt 1) {
        throw "get_architecture smoke check returned an empty graph: $archOut"
    }
    Write-Host "smoke index check passed"
} else {
    Write-Host "make not found; build check skipped"
}

Write-Host "build check passed"
