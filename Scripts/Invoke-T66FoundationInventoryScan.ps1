[CmdletBinding()]
param(
    [string]$Root,
    [string]$JsonPath,
    [string]$MarkdownPath
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($Root)) {
    $Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
}

function Get-RelativePath {
    param(
        [Parameter(Mandatory = $true)][string]$BasePath,
        [Parameter(Mandatory = $true)][string]$FullPath
    )

    $base = [System.IO.Path]::GetFullPath($BasePath).TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar
    $full = [System.IO.Path]::GetFullPath($FullPath)
    return $full.Substring($base.Length).Replace('\', '/')
}

function Test-CommentLine {
    param([string]$Line)
    return $Line -match '^\s*(//|/\*|\*)'
}

function Get-RunResetKind {
    param([string]$RelativePath, [string]$Line)

    if (Test-CommentLine $Line) { return 'CommentContract' }
    if ($Line -match '^\s*void\s+.*ResetForNewRun\s*\(') { return 'Definition' }
    if ($RelativePath.EndsWith('.h') -and $Line -match '\bResetForNewRun\s*\([^)]*\)\s*;') { return 'Declaration' }
    return 'CallSite'
}

function Get-RequestQuitKind {
    param([string]$RelativePath, [string]$Line)

    if ($RelativePath -match 'Core/Shutdown/T66ShutdownSubsystem\.(h|cpp)$') { return 'ShutdownApi' }
    if (Test-CommentLine $Line) { return 'Comment' }
    return 'PlayerQuitCall'
}

function Get-RegisterParticipantKind {
    param([string]$RelativePath, [string]$Line)

    if ($RelativePath -match 'Core/Shutdown/T66ShutdownSubsystem\.(h|cpp)$') { return 'ShutdownApi' }
    if (Test-CommentLine $Line) { return 'Comment' }
    return 'ParticipantRegistration'
}

function Get-StatusExitKind {
    param([string]$RelativePath, [string]$Line)

    if ($RelativePath -match 'Core/Shutdown/T66ShutdownSubsystem\.cpp$') { return 'ShutdownFinalExit' }
    if ($Line -match 'Proof|Smoke|QA|AutoDump|Complete|Capture|Automation|Perf') { return 'ProofOrAutomationExit' }
    return 'DirectStatusExit'
}

function Get-WorldCleanupKind {
    param([string]$Line)

    if ($Line -match '^\s*void\s+.*::Deinitialize\s*\(') { return 'DeinitializeDefinition' }
    if ($Line -match 'ClearManagedTrapActors\s*\(') { return 'OwnerCleanupHelper' }
    if ($Line -match '\bEndPlay\s*\(') { return 'EndPlayReference' }
    return 'CleanupReference'
}

$sourceRoot = Join-Path $Root 'Source/T66'
if (-not (Test-Path -LiteralPath $sourceRoot)) {
    throw "Source root not found: $sourceRoot"
}

$rules = @(
    [pscustomobject]@{
        Surface = 'WorldTransition.RawOpenLevel'
        Pattern = 'UGameplayStatics::OpenLevel\s*\('
        IntendedOwner = 'LifecycleSystem / future world-transition coordinator'
        MigrationPass = 'Pass 3'
        Kind = { param($relativePath, $line) if (Test-CommentLine $line) { 'Comment' } else { 'CallSite' } }
    },
    [pscustomobject]@{
        Surface = 'RunBoundary.ResetForNewRun'
        Pattern = '\bResetForNewRun\s*\('
        IntendedOwner = 'UT66RunStateSubsystem run boundary'
        MigrationPass = 'Pass 2'
        Kind = { param($relativePath, $line) Get-RunResetKind $relativePath $line }
    },
    [pscustomobject]@{
        Surface = 'DurableState.SyncSaveGameToSlot'
        Pattern = 'UGameplayStatics::SaveGameToSlot\s*\('
        IntendedOwner = 'Durable-state owner plus future flush coordinator'
        MigrationPass = 'Pass 4'
        Kind = { param($relativePath, $line) if (Test-CommentLine $line) { 'Comment' } else { 'DirectSaveCall' } }
    },
    [pscustomobject]@{
        Surface = 'DurableState.AsyncSaveGameToSlot'
        Pattern = 'UGameplayStatics::AsyncSaveGameToSlot\s*\('
        IntendedOwner = 'Durable-state owner plus future flush coordinator'
        MigrationPass = 'Pass 4'
        Kind = { param($relativePath, $line) if (Test-CommentLine $line) { 'Comment' } else { 'DirectAsyncSaveCall' } }
    },
    [pscustomobject]@{
        Surface = 'Shutdown.RegisterParticipant'
        Pattern = '\bRegisterParticipant\s*\('
        IntendedOwner = 'ShutdownSystem participant registry'
        MigrationPass = 'Pass 6'
        Kind = { param($relativePath, $line) Get-RegisterParticipantKind $relativePath $line }
    },
    [pscustomobject]@{
        Surface = 'Quit.RequestQuitGame'
        Pattern = '\bRequestQuitGame\s*\('
        IntendedOwner = 'ShutdownSystem / UT66ShutdownSubsystem'
        MigrationPass = 'Pass 6'
        Kind = { param($relativePath, $line) Get-RequestQuitKind $relativePath $line }
    },
    [pscustomobject]@{
        Surface = 'Quit.DirectQuitGameFallback'
        Pattern = 'UKismetSystemLibrary::QuitGame\s*\('
        IntendedOwner = 'ShutdownSystem, unless documented emergency fallback'
        MigrationPass = 'Pass 6'
        Kind = { param($relativePath, $line) if (Test-CommentLine $line) { 'Comment' } else { 'DirectQuitFallback' } }
    },
    [pscustomobject]@{
        Surface = 'Exit.DirectRequestExitWithStatus'
        Pattern = 'FPlatformMisc::RequestExitWithStatus\s*\('
        IntendedOwner = 'Shutdown final exit or proof/fatal owner by classification'
        MigrationPass = 'Pass 6 / proof exception'
        Kind = { param($relativePath, $line) Get-StatusExitKind $relativePath $line }
    },
    [pscustomobject]@{
        Surface = 'WorldRuntime.CleanupHooks'
        Pattern = '\bDeinitialize\s*\(|\bEndPlay\s*\(|ClearManagedTrapActors\s*\('
        IntendedOwner = 'World subsystem or actor owner-local cleanup'
        MigrationPass = 'Pass 5'
        Kind = { param($relativePath, $line) Get-WorldCleanupKind $line }
    }
)

$rows = New-Object System.Collections.Generic.List[object]
$files = Get-ChildItem -LiteralPath $sourceRoot -Recurse -File -Include '*.cpp', '*.h' | Sort-Object FullName

foreach ($file in $files) {
    $relativePath = Get-RelativePath -BasePath $Root -FullPath $file.FullName
    $lines = Get-Content -LiteralPath $file.FullName
    for ($i = 0; $i -lt $lines.Count; ++$i) {
        $line = $lines[$i]
        foreach ($rule in $rules) {
            if ($line -match $rule.Pattern) {
                $kind = & $rule.Kind $relativePath $line
                $rows.Add([pscustomobject]@{
                    Surface = $rule.Surface
                    Kind = $kind
                    Path = $relativePath
                    Line = $i + 1
                    Text = $line.Trim()
                    IntendedOwner = $rule.IntendedOwner
                    MigrationPass = $rule.MigrationPass
                })
            }
        }
    }
}

$orderedRows = @($rows | Sort-Object Surface, Kind, Path, Line)
$summary = @(
    $orderedRows |
        Group-Object Surface, Kind |
        ForEach-Object {
            $parts = $_.Name -split ', '
            [pscustomobject]@{
                Surface = $parts[0]
                Kind = $parts[1]
                Count = $_.Count
            }
        } |
        Sort-Object Surface, Kind
)

$payload = [pscustomobject]@{
    GeneratedAtUtc = (Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ')
    Root = (Resolve-Path $Root).Path
    SourceRoot = (Resolve-Path $sourceRoot).Path
    SourceFileCount = $files.Count
    Summary = $summary
    Rows = $orderedRows
}

if ($JsonPath) {
    $jsonFullPath = if ([System.IO.Path]::IsPathRooted($JsonPath)) { $JsonPath } else { Join-Path $Root $JsonPath }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $jsonFullPath) | Out-Null
    $payload | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $jsonFullPath -Encoding UTF8
}

if ($MarkdownPath) {
    $markdownFullPath = if ([System.IO.Path]::IsPathRooted($MarkdownPath)) { $MarkdownPath } else { Join-Path $Root $MarkdownPath }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $markdownFullPath) | Out-Null
    $md = New-Object System.Collections.Generic.List[string]
    $bt = [char]96
    $md.Add('# Foundation Inventory Scan')
    $md.Add('')
    $md.Add("- Generated UTC: $($payload.GeneratedAtUtc)")
    $md.Add("- Source files scanned: $($payload.SourceFileCount)")
    $md.Add('- Counts are classified rows, not raw grep totals.')
    $md.Add('')
    $md.Add('## Summary')
    $md.Add('')
    $md.Add('| Surface | Kind | Count |')
    $md.Add('|---|---|---:|')
    foreach ($entry in $summary) {
        $md.Add("| $bt$($entry.Surface)$bt | $bt$($entry.Kind)$bt | $($entry.Count) |")
    }
    $md.Add('')
    $md.Add('## Rows')
    $md.Add('')
    $md.Add('| Surface | Kind | Path | Line | Intended Owner | Migration Pass |')
    $md.Add('|---|---|---|---:|---|---|')
    foreach ($row in $orderedRows) {
        $md.Add("| $bt$($row.Surface)$bt | $bt$($row.Kind)$bt | $bt$($row.Path)$bt | $($row.Line) | $($row.IntendedOwner) | $($row.MigrationPass) |")
    }
    $md | Set-Content -LiteralPath $markdownFullPath -Encoding UTF8
}

$summary | Format-Table -AutoSize
