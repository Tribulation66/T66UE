param(
    [int]$TimeoutSeconds = 1800
)

$ErrorActionPreference = "Stop"

$RepoRoot = "C:\UE\T66"
$RunRoot = Join-Path $RepoRoot "Model Generation\Runs\EnemyBosses\EnemyBossBatch01"
$Pipeline = Join-Path $RepoRoot "Model Generation\Scripts\RunQuadRetroCharacterPipeline.ps1"
$LogPath = Join-Path $RunRoot "Reports\Stage02_Bosses_QuadRetro_Retry15000_RunLog.jsonl"
$OutputRoot = Join-Path $RunRoot "QuadRetro\Medium\Bosses"
$BossIds = @(
    "Forest_ThornHive",
    "Forest_BuerVerdantChad"
)

function Quote-Arg {
    param([string]$Value)
    return '"' + ($Value -replace '"', '\"') + '"'
}

function Write-JsonLine {
    param([hashtable]$Record)
    $Record["timestamp_utc"] = (Get-Date).ToUniversalTime().ToString("o")
    $Record | ConvertTo-Json -Compress | Add-Content -LiteralPath $LogPath -Encoding UTF8
}

foreach ($BossId in $BossIds) {
    $InputModel = Join-Path $RunRoot "Raw\Trellis\Bosses\$BossId\$($BossId)_Trellis.glb"
    $OutputDir = Join-Path $OutputRoot $BossId
    $StdOut = Join-Path $RunRoot "Reports\$($BossId)_QuadRetro_retry15000_stdout.log"
    $StdErr = Join-Path $RunRoot "Reports\$($BossId)_QuadRetro_retry15000_stderr.log"

    Write-JsonLine @{
        boss_id = $BossId
        event = "start"
        qremesh_source_target_tris = 15000
        target_quads = 12000
        input_model = $InputModel
        output_dir = $OutputDir
    }

    $Args = @(
        "-ExecutionPolicy", "Bypass",
        "-File", $Pipeline,
        "-InputModel", $InputModel,
        "-OutputDir", $OutputDir,
        "-Label", $BossId,
        "-TargetQuads", "12000",
        "-AdaptiveSize", "50",
        "-QRemeshSourceTargetTris", "15000",
        "-TextureSize", "512",
        "-PaletteMode", "none",
        "-DitherType", "none",
        "-DitherStrength", "0",
        "-BakeSize", "1024",
        "-RenderQA", "true",
        "-Background", "false",
        "-TimeoutSeconds", "$TimeoutSeconds"
    )

    $ArgumentLine = ($Args | ForEach-Object {
        if ($_ -match '\s') { Quote-Arg $_ } else { $_ }
    }) -join " "

    $Process = Start-Process -FilePath "powershell" -ArgumentList $ArgumentLine -WindowStyle Hidden -Wait -PassThru -RedirectStandardOutput $StdOut -RedirectStandardError $StdErr
    $ExitCode = $Process.ExitCode
    $Model = Join-Path $OutputDir "Models\$($BossId)_QuadRetro.glb"
    $Report = Join-Path $OutputDir "Reports\$($BossId)_QuadRetro_report.json"
    $Front = Join-Path $OutputDir "Renders\$($BossId)_QuadRetro_front.png"

    Write-JsonLine @{
        boss_id = $BossId
        event = "finish"
        exit_code = $ExitCode
        model_exists = (Test-Path -LiteralPath $Model)
        report_exists = (Test-Path -LiteralPath $Report)
        front_render_exists = (Test-Path -LiteralPath $Front)
    }
}
