param(
    [string]$Exe = "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe",
    [string]$OutputRoot,
    [int]$ResX = 1920,
    [int]$ResY = 1080,
    [double]$ClickDelaySeconds = 2.5,
    [double]$DumpDelaySeconds = 4.5,
    [double]$ScreenshotDelaySeconds = 5.0,
    [int]$TimeoutSeconds = 70,
    [int]$DisplayNumber = 1,
    [int]$WindowOffsetX = 0,
    [int]$WindowOffsetY = 0,
    [switch]$NoPrepareWindowedSettings,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$ProjectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..")).TrimEnd('\')
$CaptureScript = Join-Path $PSScriptRoot "CaptureT66UIScreen.ps1"

if (-not $OutputRoot) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputRoot = Join-Path $ProjectRoot "Saved\FrontendTagClickSmokeMatrix\$stamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)

function New-SmokeCase {
    param(
        [string]$Name,
        [string]$Screen,
        [string]$Modal,
        [string]$ClickTag,
        [string]$ExpectedPostClickScreen,
        [string[]]$RequiredLog = @(),
        [string[]]$ForbiddenLog = @(),
        [string[]]$RequiredDump = @(),
        [string[]]$ForbiddenDump = @()
    )

    [pscustomobject]@{
        Name = $Name
        StartScreen = $Screen
        Modal = $Modal
        ClickTag = $ClickTag
        ExpectedPostClickScreen = $ExpectedPostClickScreen
        RequiredLog = $RequiredLog
        ForbiddenLog = $ForbiddenLog
        RequiredDump = $RequiredDump
        ForbiddenDump = $ForbiddenDump
    }
}

function Assert-TextContains {
    param(
        [string]$Text,
        [string]$Needle,
        [string]$Path,
        [string]$Label
    )

    if ($Text.IndexOf($Needle, [System.StringComparison]::OrdinalIgnoreCase) -lt 0) {
        throw "$Label missing expected marker '$Needle' in $Path"
    }
}

function Assert-TextDoesNotContain {
    param(
        [string]$Text,
        [string]$Needle,
        [string]$Path,
        [string]$Label
    )

    if ($Text.IndexOf($Needle, [System.StringComparison]::OrdinalIgnoreCase) -ge 0) {
        throw "$Label contained forbidden marker '$Needle' in $Path"
    }
}

function Invoke-SmokeCase {
    param([pscustomobject]$Case)

    $caseRoot = Join-Path $OutputRoot $Case.Name
    New-Item -ItemType Directory -Force -Path $caseRoot | Out-Null

    $screenshotPath = Join-Path $caseRoot "screen.png"
    $dumpPath = Join-Path $caseRoot "dump.json"
    $logPath = Join-Path $caseRoot "run.log"

    foreach ($path in @($screenshotPath, $dumpPath, $logPath)) {
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force
        }
    }

    $extra = @(
        "-T66AutoDumpScreen=$dumpPath",
        "-T66AutoDumpScreenDelay=$DumpDelaySeconds",
        "-T66KeepAliveAfterScreenshot",
        "-abslog=$logPath",
        "-forcelogflush",
        "-nop4",
        "-nosplash"
    )

    $captureParams = @{
        Exe = $Exe
        Screen = $Case.StartScreen
        Output = $screenshotPath
        ResX = $ResX
        ResY = $ResY
        DelaySeconds = $ScreenshotDelaySeconds
        ClickTag = $Case.ClickTag
        ClickDelaySeconds = $ClickDelaySeconds
        TimeoutSeconds = $TimeoutSeconds
        DisplayNumber = $DisplayNumber
        WindowOffsetX = $WindowOffsetX
        WindowOffsetY = $WindowOffsetY
        ExtraArgs = $extra
    }
    if ($Case.Modal) {
        $captureParams.Modal = $Case.Modal
    }
    if ($NoPrepareWindowedSettings) {
        $captureParams.NoPrepareWindowedSettings = $true
    }
    if ($PrintOnly) {
        $captureParams.PrintOnly = $true
    }

    Write-Host "=== $($Case.Name) ==="
    & $CaptureScript @captureParams

    if ($PrintOnly) {
        return [pscustomobject]@{
            Name = $Case.Name
            Status = "PRINT_ONLY"
            Screenshot = $screenshotPath
            Dump = $dumpPath
            Log = $logPath
        }
    }

    if (-not (Test-Path -LiteralPath $screenshotPath)) {
        throw "Missing screenshot for $($Case.Name): $screenshotPath"
    }
    if (-not (Test-Path -LiteralPath $dumpPath)) {
        throw "Missing widget dump for $($Case.Name): $dumpPath"
    }
    if (-not (Test-Path -LiteralPath $logPath)) {
        throw "Missing log for $($Case.Name): $logPath"
    }

    $logText = Get-Content -LiteralPath $logPath -Raw
    $dumpText = Get-Content -LiteralPath $dumpPath -Raw

    foreach ($marker in $Case.RequiredLog) {
        Assert-TextContains -Text $logText -Needle $marker -Path $logPath -Label $Case.Name
    }
    foreach ($marker in $Case.ForbiddenLog) {
        Assert-TextDoesNotContain -Text $logText -Needle $marker -Path $logPath -Label $Case.Name
    }
    foreach ($marker in $Case.RequiredDump) {
        Assert-TextContains -Text $dumpText -Needle $marker -Path $dumpPath -Label $Case.Name
    }
    foreach ($marker in $Case.ForbiddenDump) {
        Assert-TextDoesNotContain -Text $dumpText -Needle $marker -Path $dumpPath -Label $Case.Name
    }

    [pscustomobject]@{
        Name = $Case.Name
        Status = "PASS"
        StartScreen = $Case.StartScreen
        Modal = $Case.Modal
        ClickTag = $Case.ClickTag
        ExpectedPostClickScreen = $Case.ExpectedPostClickScreen
        Screenshot = $screenshotPath
        Dump = $dumpPath
        Log = $logPath
    }
}

if ($DumpDelaySeconds -le $ClickDelaySeconds) {
    throw "DumpDelaySeconds ($DumpDelaySeconds) must be greater than ClickDelaySeconds ($ClickDelaySeconds)."
}
if ($ScreenshotDelaySeconds -le $DumpDelaySeconds) {
    throw "ScreenshotDelaySeconds ($ScreenshotDelaySeconds) must be greater than DumpDelaySeconds ($DumpDelaySeconds)."
}
if (-not (Test-Path -LiteralPath $CaptureScript)) {
    throw "Missing capture script: $CaptureScript"
}
if (-not (Test-Path -LiteralPath $Exe)) {
    throw "Missing executable: $Exe"
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$commonForbiddenLog = @(
    "[Shutdown] Begin",
    "LogExit: Exiting",
    "T66AutoClickTagResolveFailed",
    "T66AutoClickTagNoButton",
    "T66AutoClickTagNotClickable",
    "T66AutoClickTagInvalidGeometry",
    "T66AutoClickTagShipping",
    "T66AutoClickTagInvalid",
    "failed to resolve click tag"
)

$cases = @(
    New-SmokeCase `
        -Name "01_TopBarPowerOpensQuitModal" `
        -Screen "MainMenu" `
        -ClickTag "FrontendTopBar.PowerButton" `
        -ExpectedPostClickScreen "QuitConfirmation" `
        -RequiredLog @(
            "Frontend automation: widget dump wrote"
        ) `
        -ForbiddenLog $commonForbiddenLog `
        -RequiredDump @(
            "QuitConfirmation.Root",
            "QuitConfirmation.StayButton",
            "QuitConfirmation.QuitButton"
        ) `
        -ForbiddenDump @();

    New-SmokeCase `
        -Name "02_QuitStayClosesModal" `
        -Screen "MainMenu" `
        -Modal "QuitConfirmation" `
        -ClickTag "QuitConfirmation.StayButton" `
        -ExpectedPostClickScreen "MainMenu" `
        -RequiredLog @(
            "Frontend automation: widget dump wrote"
        ) `
        -ForbiddenLog $commonForbiddenLog `
        -RequiredDump @(
            "MainMenu.Root"
        ) `
        -ForbiddenDump @(
            "QuitConfirmation.Root"
        );

    New-SmokeCase `
        -Name "03_TopBarAchievementsNavigation" `
        -Screen "MainMenu" `
        -ClickTag "FrontendTopBar.AchievementsButton" `
        -ExpectedPostClickScreen "Achievements" `
        -RequiredLog @(
            "Frontend automation: widget dump wrote"
        ) `
        -ForbiddenLog $commonForbiddenLog `
        -RequiredDump @(
            "SteamAchievements.Root",
            "FrontendTopBar.AchievementsButton"
        ) `
        -ForbiddenDump @(
            "QuitConfirmation.Root"
        );

    New-SmokeCase `
        -Name "04_TopBarSettingsNavigation" `
        -Screen "MainMenu" `
        -ClickTag "FrontendTopBar.SettingsButton" `
        -ExpectedPostClickScreen "Settings" `
        -RequiredLog @(
            "Frontend automation: widget dump wrote"
        ) `
        -ForbiddenLog $commonForbiddenLog `
        -RequiredDump @(
            '"screen": "Settings"',
            "SettingsGameplay.Root",
            "FrontendTopBar.SettingsButton"
        ) `
        -ForbiddenDump @(
            "SettingsRetroFX.Root",
            "QuitConfirmation.Root"
        );

    New-SmokeCase `
        -Name "05_TopBarPowerUpNavigation" `
        -Screen "MainMenu" `
        -ClickTag "FrontendTopBar.PowerUpButton" `
        -ExpectedPostClickScreen "PowerUp" `
        -RequiredLog @(
            "Frontend automation: widget dump wrote"
        ) `
        -ForbiddenLog $commonForbiddenLog `
        -RequiredDump @(
            '"screen": "PowerUp"',
            "FrontendTopBar.PowerUpButton"
        ) `
        -ForbiddenDump @(
            "QuitConfirmation.Root"
        );

    New-SmokeCase `
        -Name "06_TopBarAccountNavigation" `
        -Screen "MainMenu" `
        -ClickTag "FrontendTopBar.AccountButton" `
        -ExpectedPostClickScreen "AccountStatus" `
        -RequiredLog @(
            "Frontend automation: widget dump wrote"
        ) `
        -ForbiddenLog $commonForbiddenLog `
        -RequiredDump @(
            '"screen": "AccountStatus"',
            "Overview.SubTabs",
            "Overview.AccountStatusPanel",
            "FrontendTopBar.AccountButton"
        ) `
        -ForbiddenDump @(
            "QuitConfirmation.Root"
        );

    New-SmokeCase `
        -Name "07_AccountHistorySubTabNavigation" `
        -Screen "AccountStatus" `
        -ClickTag "Overview.SubTabs.HistoryButton" `
        -ExpectedPostClickScreen "AccountStatusHistory" `
        -RequiredLog @(
            "Frontend automation: widget dump wrote"
        ) `
        -ForbiddenLog $commonForbiddenLog `
        -RequiredDump @(
            '"screen": "AccountStatus"',
            "History.SubTabs",
            "History.SubTabs.HistoryButton",
            "FrontendTopBar.AccountButton"
        ) `
        -ForbiddenDump @(
            "Overview.SubTabs",
            "QuitConfirmation.Root"
        )
)

$results = New-Object System.Collections.Generic.List[object]
try {
    foreach ($case in $cases) {
        $results.Add((Invoke-SmokeCase -Case $case))
    }

    $summary = [pscustomobject]@{
        Status = if ($PrintOnly) { "PRINT_ONLY" } else { "PASS" }
        CreatedAt = (Get-Date).ToString("o")
        Exe = [System.IO.Path]::GetFullPath($Exe)
        OutputRoot = $OutputRoot
        ClickDelaySeconds = $ClickDelaySeconds
        DumpDelaySeconds = $DumpDelaySeconds
        ScreenshotDelaySeconds = $ScreenshotDelaySeconds
        Cases = $results
    }

    $summaryJson = Join-Path $OutputRoot "summary.json"
    $summaryMd = Join-Path $OutputRoot "summary.md"
    $summary | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $summaryJson -Encoding UTF8

    $md = @()
    $md += "# Frontend Tag Click Smoke Matrix"
    $md += ""
    $md += "Status: $($summary.Status)"
    $md += "Executable: $($summary.Exe)"
    $md += "Click delay: $ClickDelaySeconds"
    $md += "Dump delay: $DumpDelaySeconds"
    $md += "Screenshot delay: $ScreenshotDelaySeconds"
    $md += ""
    foreach ($result in $results) {
        $md += "## $($result.Name)"
        $md += ""
        $md += "- Status: $($result.Status)"
        $md += "- Start screen: $($result.StartScreen)"
        $md += "- Expected post-click screen: $($result.ExpectedPostClickScreen)"
        $md += "- Click tag: $($result.ClickTag)"
        $md += "- Screenshot: $($result.Screenshot)"
        $md += "- Dump: $($result.Dump)"
        $md += "- Log: $($result.Log)"
        $md += ""
    }
    $md -join "`r`n" | Set-Content -LiteralPath $summaryMd -Encoding UTF8

    Write-Host "Frontend tag-click smoke matrix $($summary.Status): $OutputRoot"
    Write-Host "Summary: $summaryJson"
} catch {
    $failure = [pscustomobject]@{
        Status = "FAIL"
        CreatedAt = (Get-Date).ToString("o")
        Exe = [System.IO.Path]::GetFullPath($Exe)
        OutputRoot = $OutputRoot
        Error = $_.Exception.Message
        Cases = $results
    }
    $failure | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $OutputRoot "summary.json") -Encoding UTF8
    throw
}
