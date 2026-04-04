param(
    [int]$Port = 9222,
    [string]$UserDataDir = "$env:USERPROFILE\\codex-chatgpt-debug",
    [string]$StartUrl = "https://chatgpt.com/"
)

$chromePaths = @(
    "$env:ProgramFiles\\Google\\Chrome\\Application\\chrome.exe",
    "${env:ProgramFiles(x86)}\\Google\\Chrome\\Application\\chrome.exe",
    "$env:LocalAppData\\Google\\Chrome\\Application\\chrome.exe"
)

$chromeExe = $null
foreach ($candidate in $chromePaths) {
    if (Test-Path $candidate) {
        $chromeExe = $candidate
        break
    }
}

if (-not $chromeExe) {
    throw "Could not find chrome.exe. Update Tools\\ChatGPTBridge\\launch_debug_chrome.ps1 with the correct path."
}

New-Item -ItemType Directory -Force -Path $UserDataDir | Out-Null

Start-Process -FilePath $chromeExe -ArgumentList @(
    "--remote-debugging-port=$Port",
    "--user-data-dir=$UserDataDir",
    $StartUrl
)

Write-Host "Launched Chrome with remote debugging on port $Port"
Write-Host "User data dir: $UserDataDir"
Write-Host "Log into ChatGPT in that Chrome window, then start the bridge server."
