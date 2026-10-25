#Requires -Version 7.0
<#
.SYNOPSIS
递归合并当前目录下所有代码文件到一个文本文件。

.DESCRIPTION
支持按扩展名白名单筛选（涵盖 C/C++/C#/Python/JS/Go/Rust/Shell/Markdown/配置 等常见语言），
自动跳过 build / bin / obj / node_modules / .git 等构建产物和缓存目录，
输出带清晰的文件分隔头，方便喂给 LLM 或做代码审阅。

.PARAMETER OutputFile
输出文件名（相对于当前目录），默认 merged_code.txt。

.PARAMETER Extensions
自定义扩展名白名单。传入后会**替换**默认列表，而不是追加。传入空数组可包含所有文件。

.PARAMETER ExcludeDirs
自定义排除目录名（按目录名匹配，不区分大小写）。

.PARAMETER MaxFileSizeBytes
单个文件最大字节数，超过则跳过，默认 2MB。

.PARAMETER IncludeLineNumbers
输出时给每行加上行号。

.PARAMETER Preview
只列出会被合并的文件，不实际写入。

.EXAMPLE
pwsh ./merge-code.ps1

.EXAMPLE
pwsh ./merge-code.ps1 -OutputFile all.txt -IncludeLineNumbers

.EXAMPLE
pwsh ./merge-code.ps1 -Preview
#>
[CmdletBinding()]
param(
    [string]$OutputFile = "merged_code.txt",

    [string[]]$Extensions = @(
        # C / C++
        '.c', '.h', '.cpp', '.cc', '.cxx', '.hpp', '.hxx', '.hh', '.inl', '.ipp', '.tpp',
        # C#
        '.cs', '.csproj', '.sln', '.props', '.targets',
        # Python
        '.py', '.pyi', '.pyx', '.pxd', '.ipynb',
        # PowerShell / Shell / Batch
        '.ps1', '.psm1', '.psd1', '.sh', '.bash', '.zsh', '.bat', '.cmd',
        # Web
        '.js', '.mjs', '.cjs', '.ts', '.jsx', '.tsx', '.vue', '.svelte',
        '.html', '.htm', '.css', '.scss', '.sass', '.less',
        # 其他语言
        '.java', '.kt', '.kts', '.go', '.rs', '.rb', '.php', '.lua', '.pl', '.pm',
        '.r', '.m', '.mm', '.swift', '.scala', '.dart', '.ex', '.exs',
        # 构建 / 配置
        '.cmake', '.pro', '.gradle', '.mk', '.make',
        '.json', '.jsonc', '.xml', '.yml', '.yaml', '.toml', '.ini', '.conf', '.cfg',
        '.properties', '.env', '.editorconfig', '.gitignore', '.gitattributes',
        # 文档 / 数据
        '.md', '.markdown', '.txt', '.rst', '.adoc', '.sql', '.proto', '.graphql',
        # 其他常见
        '.dockerfile', '.containerfile'
    ),

    [string[]]$ExcludeDirs = @(
        '.git', '.svn', '.hg', '.vs', '.vscode', '.idea',
        'node_modules', '__pycache__', '.pytest_cache', '.mypy_cache', '.ruff_cache', '.tox',
        'build', 'bin', 'obj', 'out', 'dist', 'lib', 'libs', 'target',
        '.cache', '.next', '.nuxt', '.svelte-kit',
        'venv', '.venv', 'env', '.env', 'site-packages',
        '.gradle', '.mvn', '.nuget', 'packages',
        'coverage', 'htmlcov', '.nyc_output'
    ),

    [long]$MaxFileSizeBytes = 2MB,

    [switch]$IncludeLineNumbers,

    [switch]$Preview
)

$ErrorActionPreference = 'Stop'
$root     = (Get-Location).Path
$outPath  = Join-Path $root $OutputFile
$utf8NoBom = [System.Text.UTF8Encoding]::new($false)

# ---------- 预处理过滤集合 ----------
$extSet = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($e in $Extensions) {
    if ([string]::IsNullOrWhiteSpace($e)) { continue }
    $e = $e.Trim()
    if (-not $e.StartsWith('.')) { $e = '.' + $e }
    [void]$extSet.Add($e)
}

$excludeSet = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($d in $ExcludeDirs) {
    if (-not [string]::IsNullOrWhiteSpace($d)) { [void]$excludeSet.Add($d.Trim()) }
}

# ---------- 收集文件 ----------
# 先拿到所有目录做排除判断，再枚举文件，比 Get-ChildItem -Recurse 后过滤更准确
$files = Get-ChildItem -LiteralPath $root -Recurse -File -Force -ErrorAction SilentlyContinue | Where-Object {
    $rel = $_.FullName.Substring($root.Length).TrimStart([char]'\', [char]'/')
    $parts = $rel -split '[\\/]'

    # 排除输出文件本身以及任何位于排除目录下的文件
    $isExcluded = $false
    for ($i = 0; $i -lt $parts.Length - 1; $i++) {
        if ($excludeSet.Contains($parts[$i])) { $isExcluded = $true; break }
    }
    if ($isExcluded) { return $false }
    if ($_.FullName -eq $outPath) { return $false }
    if ($_.Length -gt $MaxFileSizeBytes) { return $false }

    # 扩展名白名单
    if ($extSet.Count -gt 0) {
        if (-not $extSet.Contains($_.Extension)) { return $false }
    }
    return $true
} | Sort-Object FullName

if ($files.Count -eq 0) {
    Write-Warning "没有找到符合条件的文件，检查 -Extensions / -ExcludeDirs。"
    return
}

# ---------- Preview 模式 ----------
if ($Preview) {
    Write-Host "以下 $($files.Count) 个文件会被合并：" -ForegroundColor Cyan
    $files | ForEach-Object {
        $rel = $_.FullName.Substring($root.Length).TrimStart([char]'\', [char]'/')
        Write-Host ("  {0,10:N1} KB  {1}" -f ($_.Length / 1KB), $rel)
    }
    return
}

# ---------- 写入 ----------
if (Test-Path -LiteralPath $outPath) { Remove-Item -LiteralPath $outPath -Force }

$sw = [System.IO.StreamWriter]::new($outPath, $false, $utf8NoBom)
$fileCount = 0
$totalBytes = 0

try {
    $sw.WriteLine("# Merged Code")
    $sw.WriteLine("# Root:      $root")
    $sw.WriteLine("# Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
    $sw.WriteLine("# Files:     $($files.Count)")
    $sw.WriteLine()

    foreach ($f in $files) {
        $rel = $f.FullName.Substring($root.Length).TrimStart([char]'\', [char]'/')

        $sw.WriteLine(('=' * 100))
        $sw.WriteLine("FILE: $rel")
        $sw.WriteLine("SIZE: $($f.Length) bytes    LINES: ~$((($f.Length -split '').Count))")
        $sw.WriteLine(('=' * 100))

        try {
            $lineNo = 0
            foreach ($line in [System.IO.File]::ReadLines($f.FullName)) {
                $lineNo++
                if ($IncludeLineNumbers) {
                    $sw.WriteLine(('{0,6}: {1}' -f $lineNo, $line))
                } else {
                    $sw.WriteLine($line)
                }
            }
        } catch {
            $sw.WriteLine("<<< READ ERROR: $($_.Exception.Message) >>>")
        }

        $sw.WriteLine()
        $fileCount++
        $totalBytes += $f.Length
    }
} finally {
    $sw.Dispose()
}

# ---------- 汇总 ----------
$outSizeKB = [math]::Round((Get-Item -LiteralPath $outPath).Length / 1KB, 1)
$inSizeKB  = [math]::Round($totalBytes / 1KB, 1)

Write-Host ""
Write-Host "✔ 合并完成" -ForegroundColor Green
Write-Host "  文件数: $fileCount"
Write-Host "  输入:   $inSizeKB KB"
Write-Host "  输出:   $outPath ($outSizeKB KB)"