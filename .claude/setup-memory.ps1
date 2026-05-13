# Link Claude Code's per-user memory directory to this repo's .claude\memory\
#
# Claude Code reads and writes memory from
#   %USERPROFILE%\.claude\projects\<encoded-repo-path>\memory\
# which is per-user and per-machine. To share memory via git we keep the real
# files in <repo>\.claude\memory\ and create a directory junction at the
# per-user path so Claude transparently reads/writes the repo copy.
#
# Run once after cloning (Windows, no admin required):
#     powershell -ExecutionPolicy Bypass -File .claude\setup-memory.ps1
#
# Re-run after moving the repo to a different absolute path.

$ErrorActionPreference = "Stop"

$repoRoot   = (Get-Item (Join-Path $PSScriptRoot "..")).FullName
$repoMemory = Join-Path $repoRoot ".claude\memory"

if (-not (Test-Path $repoMemory)) {
    Write-Error "Repo memory directory missing: $repoMemory"
    exit 1
}

# Claude Code encodes the absolute repo path by replacing : / \ with -
# Drive letter is stored lowercase (e.g. d--Users-son-SEngine, not D--...).
# Example: d:\Users\son\SEngine -> d--Users-son-SEngine
$encoded = $repoRoot -replace '[:\\/]', '-'
if ($encoded.Length -ge 1) {
    $encoded = $encoded.Substring(0,1).ToLower() + $encoded.Substring(1)
}
$claudeProjectDir = Join-Path $env:USERPROFILE ".claude\projects\$encoded"
$claudeMemory     = Join-Path $claudeProjectDir "memory"

Write-Host "Repo path:    $repoRoot"
Write-Host "Encoded:      $encoded"
Write-Host "Junction at:  $claudeMemory"
Write-Host "Target:       $repoMemory"

if (-not (Test-Path $claudeProjectDir)) {
    New-Item -ItemType Directory -Path $claudeProjectDir -Force | Out-Null
}

if (Test-Path $claudeMemory) {
    $item = Get-Item $claudeMemory -Force
    $isReparse = ($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0
    if ($isReparse) {
        Write-Host "Removing existing junction/symlink."
        & cmd /c rmdir "`"$claudeMemory`""
    } else {
        $existing = @(Get-ChildItem $claudeMemory -Force -ErrorAction SilentlyContinue)
        if ($existing.Count -gt 0) {
            $stamp  = Get-Date -Format 'yyyyMMdd-HHmmss'
            $backup = "$claudeMemory.backup-$stamp"
            Write-Warning "Existing memory has files; backing up to $backup"
            Move-Item -LiteralPath $claudeMemory -Destination $backup
        } else {
            Remove-Item -LiteralPath $claudeMemory -Force
        }
    }
}

& cmd /c mklink /J "`"$claudeMemory`"" "`"$repoMemory`"" | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Error "mklink failed (exit $LASTEXITCODE)."
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "Done. Files visible via junction:"
Get-ChildItem $claudeMemory -Force | Select-Object Name | Format-Table -HideTableHeaders
