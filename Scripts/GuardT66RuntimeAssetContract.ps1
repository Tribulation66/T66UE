param(
    [switch]$FailOnDuplicateLooseRuntimeDependencies
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Push-Location $RepoRoot

try {
    if (-not (Get-Command rg -ErrorAction SilentlyContinue)) {
        throw "ripgrep (rg) is required for this guardrail."
    }

    function Normalize-RelativePath {
        param([string]$Path)

        if ($null -eq $Path) {
            return ""
        }
        return $Path.Replace('\', '/').Trim()
    }

    function Get-RootPathForTest {
        param([string]$RelativePath)

        $normalized = Normalize-RelativePath $RelativePath
        if ($normalized.EndsWith('/...')) {
            return $normalized.Substring(0, $normalized.Length - 4)
        }
        return $normalized
    }

    function Read-LooseRuntimeContentPolicy {
        param([string]$ConfigPath)

        if (-not (Test-Path -LiteralPath $ConfigPath)) {
            throw "Missing content policy config: $ConfigPath"
        }

        $entries = @()
        $inSection = $false
        foreach ($line in Get-Content -LiteralPath $ConfigPath) {
            if ($line -match '^\s*\[(.+)\]\s*$') {
                $inSection = ($Matches[1] -eq '/Script/T66.T66GameContentSettings')
                continue
            }

            if (-not $inSection) {
                continue
            }

            if ($line -notmatch '^\s*\+LooseRuntimeContentRoots=\((.+)\)\s*$') {
                continue
            }

            $rawFields = $Matches[1]
            $fields = @{}
            foreach ($match in [regex]::Matches($rawFields, '([A-Za-z0-9_]+)="([^"]*)"')) {
                $fields[$match.Groups[1].Value] = $match.Groups[2].Value
            }

            $relativePath = if ($fields.ContainsKey('RelativePath')) { $fields['RelativePath'] } else { '' }
            $owner = if ($fields.ContainsKey('Owner')) { $fields['Owner'] } else { '' }
            $classification = if ($fields.ContainsKey('Classification')) { $fields['Classification'] } else { '' }
            $rationale = if ($fields.ContainsKey('Rationale')) { $fields['Rationale'] } else { '' }

            $entries += [pscustomobject]@{
                RelativePath = Normalize-RelativePath $relativePath
                Owner = $owner
                Classification = $classification
                Rationale = $rationale
            }
        }

        return @($entries)
    }

    $syncArgs = @(
        "-n",
        "LoadSynchronous|\.TryLoad\(|LoadClass|LoadObject|StaticLoadObject",
        "Source/T66/Core",
        "-g", "*.cpp",
        "-g", "*.h",
        "-g", "!Backend/**",
        "-g", "!T66WebImageCache*",
        "-g", "!T66UITexturePoolSubsystem.*"
    )
    $syncLines = @(& rg @syncArgs)
    if ($LASTEXITCODE -gt 1) {
        throw "rg failed while scanning owned Core sync-load exposure."
    }

    Write-Host "Owned Core sync-load exposure lines: $($syncLines.Count)"
    foreach ($line in $syncLines) {
        Write-Host "  $line"
    }

    $dependencyLines = @(& rg -n "AddLooseRuntimeDependency" Source/T66/T66.Build.cs)
    if ($LASTEXITCODE -gt 1) {
        throw "rg failed while scanning loose runtime dependencies."
    }

    $dependencies = @()
    foreach ($line in $dependencyLines) {
        if ($line -match 'AddLooseRuntimeDependency\("([^"]+)"\)') {
            $dependencies += $Matches[1].Replace('\', '/')
        }
    }

    Write-Host "Loose runtime dependency declarations: $($dependencies.Count)"
    foreach ($dependency in $dependencies) {
        Write-Host "  $dependency"
    }

    $policyEntries = Read-LooseRuntimeContentPolicy (Join-Path $RepoRoot "Config/DefaultGame.ini")
    Write-Host "Loose runtime content policy entries: $($policyEntries.Count)"
    foreach ($entry in $policyEntries) {
        Write-Host "  $($entry.RelativePath) [$($entry.Classification)] owner=$($entry.Owner)"
    }

    $errors = @()

    $dependencyCounts = @{}
    foreach ($dependency in $dependencies) {
        if (-not $dependencyCounts.ContainsKey($dependency)) {
            $dependencyCounts[$dependency] = 0
        }
        $dependencyCounts[$dependency] += 1
    }
    foreach ($dependency in $dependencyCounts.Keys | Sort-Object) {
        if ($dependencyCounts[$dependency] -gt 1) {
            $errors += "Duplicate loose runtime dependency declaration: $dependency"
        }
    }

    $policyCounts = @{}
    foreach ($entry in $policyEntries) {
        if (-not $policyCounts.ContainsKey($entry.RelativePath)) {
            $policyCounts[$entry.RelativePath] = 0
        }
        $policyCounts[$entry.RelativePath] += 1

        if ([string]::IsNullOrWhiteSpace($entry.RelativePath) -or
            [string]::IsNullOrWhiteSpace($entry.Owner) -or
            [string]::IsNullOrWhiteSpace($entry.Classification) -or
            [string]::IsNullOrWhiteSpace($entry.Rationale)) {
            $errors += "Incomplete loose runtime content policy entry: $($entry.RelativePath)"
        }
    }
    foreach ($policyPath in $policyCounts.Keys | Sort-Object) {
        if ($policyCounts[$policyPath] -gt 1) {
            $errors += "Duplicate loose runtime content policy entry: $policyPath"
        }
    }

    $policyPaths = @($policyEntries | ForEach-Object { $_.RelativePath })
    foreach ($dependency in $dependencies) {
        if ($policyPaths -notcontains $dependency) {
            $errors += "Loose runtime dependency has no UT66GameContentSettings policy entry: $dependency"
        }

        $rootPath = Get-RootPathForTest $dependency
        if (-not (Test-Path -LiteralPath (Join-Path $RepoRoot $rootPath))) {
            $errors += "Loose runtime dependency path does not exist: $dependency"
        }
    }

    foreach ($entry in $policyEntries) {
        if ($dependencies -notcontains $entry.RelativePath) {
            $errors += "UT66GameContentSettings policy entry is not staged in T66.Build.cs: $($entry.RelativePath)"
        }
    }

    $duplicateChildren = @()
    foreach ($dependency in $dependencies) {
        if (-not $dependency.EndsWith('/...')) {
            continue
        }

        $parent = $dependency.Substring(0, $dependency.Length - 4)
        foreach ($candidate in $dependencies) {
            if ($candidate -ne $dependency -and $candidate.StartsWith("$parent/")) {
                $duplicateChildren += "$candidate is already covered by $dependency"
            }
        }
    }

    if ($duplicateChildren.Count -gt 0) {
        Write-Host "Duplicate loose runtime dependency coverage:"
        foreach ($duplicate in $duplicateChildren) {
            Write-Host "  $duplicate"
        }

        if ($FailOnDuplicateLooseRuntimeDependencies) {
            $errors += $duplicateChildren
        }
    }

    if ($errors.Count -gt 0) {
        Write-Host "Runtime asset contract guard failures:"
        foreach ($guardError in $errors) {
            Write-Host "  $guardError"
        }
        exit 1
    }
} finally {
    Pop-Location
}
