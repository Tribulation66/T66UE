[CmdletBinding()]
param(
    [string]$Prompt,
    [string]$PromptFile,
    [string]$OutDir = "SourceAssets\ToonStyle\ImageGen\Manual",
    [string]$Name,
    [ValidateSet("1024x1024", "1536x1024", "1024x1536", "2048x2048", "2048x1152", "3840x2160", "2160x3840", "auto")]
    [string]$Size = "1024x1024",
    [ValidateSet("low", "medium", "high", "auto")]
    [string]$Quality = "high",
    [string]$Model,
    [switch]$Force,
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($Prompt) -and [string]::IsNullOrWhiteSpace($PromptFile)) {
    throw "Provide -Prompt or -PromptFile."
}

if (-not [string]::IsNullOrWhiteSpace($Prompt) -and -not [string]::IsNullOrWhiteSpace($PromptFile)) {
    throw "Use either -Prompt or -PromptFile, not both."
}

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path

if ([string]::IsNullOrWhiteSpace($Name)) {
    $Name = "imagegen_{0}" -f (Get-Date -Format "yyyyMMdd_HHmmss")
}

$SafeName = ($Name -replace "[^A-Za-z0-9_.-]", "_").Trim("._-")
if ([string]::IsNullOrWhiteSpace($SafeName)) {
    throw "The supplied -Name does not contain any usable filename characters."
}

if (-not $SafeName.EndsWith(".png", [System.StringComparison]::OrdinalIgnoreCase)) {
    $SafeName = "$SafeName.png"
}

if ([System.IO.Path]::IsPathRooted($OutDir)) {
    $OutDirAbs = $OutDir
} else {
    $OutDirAbs = Join-Path $RepoRoot $OutDir
}

$OutDirAbs = [System.IO.Path]::GetFullPath($OutDirAbs)
$OutPath = [System.IO.Path]::GetFullPath((Join-Path $OutDirAbs $SafeName))

if ((Test-Path -LiteralPath $OutPath) -and -not $Force) {
    throw "Output already exists. Pass -Force to overwrite: $OutPath"
}

New-Item -ItemType Directory -Force -Path $OutDirAbs | Out-Null

if (-not [string]::IsNullOrWhiteSpace($PromptFile)) {
    $PromptPath = if ([System.IO.Path]::IsPathRooted($PromptFile)) {
        $PromptFile
    } else {
        Join-Path $RepoRoot $PromptFile
    }

    $Prompt = Get-Content -Raw -LiteralPath $PromptPath
}

$RunId = [System.IO.Path]::GetFileNameWithoutExtension($SafeName)
$SessionDir = Join-Path $RepoRoot ("tmp\codex_imagegen_cli\{0}" -f $RunId)
New-Item -ItemType Directory -Force -Path $SessionDir | Out-Null

$RequestPath = Join-Path $SessionDir "request.txt"
$LastMessagePath = Join-Path $SessionDir "last_message.txt"
$StdoutPath = Join-Path $SessionDir "codex_stdout.txt"

$ModelLine = if ([string]::IsNullOrWhiteSpace($Model)) {
    "Use the default image model available to this Codex CLI session."
} else {
    "Use image model preference: $Model."
}

$Request = @"
This is a trusted local Codex CLI image-generation run for the T66 workspace.

Use the built-in Codex image generation capability. Do not use OPENAI_API_KEY, the OpenAI API fallback script, web search, or external image URLs.

Generate exactly one raster image.

Image prompt:
$Prompt

Generation constraints:
- Target size: $Size
- Target quality: $Quality
- $ModelLine
- Do not embed the image in the final response.
- Save or copy the final selected PNG to this exact absolute path:
$OutPath

Implementation note:
- If the generated image is first written under CODEX_HOME/generated_images or the local .codex/generated_images directory, copy that generated PNG to the exact output path above.
- Create parent directories if needed.

Final response must be exactly one line:
IMAGE_SAVED: <absolute path>

If built-in image generation is unavailable in this CLI session, final response must be exactly one line:
IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE
"@

Set-Content -Path $RequestPath -Value $Request -Encoding UTF8

if ($DryRun) {
    Write-Output "DRY_RUN_REQUEST: $RequestPath"
    Write-Output "DRY_RUN_OUTPUT: $OutPath"
    Write-Output "DRY_RUN_COMMAND: codex exec --cd `"$RepoRoot`" --dangerously-bypass-approvals-and-sandbox --output-last-message `"$LastMessagePath`" -"
    exit 0
}

$CodexArgs = @(
    "exec",
    "--cd", $RepoRoot,
    "--dangerously-bypass-approvals-and-sandbox",
    "--output-last-message", $LastMessagePath,
    "-"
)

$PreviousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = "Continue"
try {
    $CodexOutput = $Request | & codex @CodexArgs 2>&1
    $ExitCode = $LASTEXITCODE
} finally {
    $ErrorActionPreference = $PreviousErrorActionPreference
}
$CodexOutput | Set-Content -Path $StdoutPath -Encoding UTF8

if ($ExitCode -ne 0) {
    throw "codex exec failed with exit code $ExitCode. See $StdoutPath"
}

if (-not (Test-Path -LiteralPath $OutPath)) {
    $LastMessage = if (Test-Path -LiteralPath $LastMessagePath) {
        Get-Content -Raw -LiteralPath $LastMessagePath
    } else {
        ""
    }

    throw "Codex completed but did not create $OutPath. Last message: $LastMessage"
}

Write-Output "IMAGE_SAVED: $OutPath"
