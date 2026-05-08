$ErrorActionPreference = "Stop"

$RepoRoot = "C:\UE\T66"
$RunRoot = Join-Path $RepoRoot "Model Generation\Runs\EnemyBosses\EnemyBossBatch01"
$ManifestPath = Join-Path $RunRoot "Reports\Stage01_Bosses_TrellisManifest.json"
$Wrapper = Join-Path $RepoRoot "Model Generation\Scripts\RunQuadRetroCharacterPipeline.ps1"
$RunLog = Join-Path $RunRoot "Reports\Stage02_Bosses_QuadRetro_RunLog.jsonl"

$Manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
$Rows = @($Manifest.rows)

function Write-RunLog {
    param([hashtable]$Payload)
    $Payload.timestamp = (Get-Date).ToString("o")
    ($Payload | ConvertTo-Json -Compress) | Add-Content -LiteralPath $RunLog -Encoding UTF8
}

foreach ($Row in $Rows) {
    $BossID = [string]$Row.row_id
    $InputModel = Join-Path $RunRoot ([string]$Row.raw_trellis_glb)
    $OutputDir = Join-Path $RunRoot ("QuadRetro\Medium\Bosses\" + $BossID)
    $OutputModel = Join-Path $OutputDir ("Models\" + $BossID + "_QuadRetro.glb")
    $ReportPath = Join-Path $OutputDir ("Reports\" + $BossID + "_QuadRetro_report.json")
    $FrontRender = Join-Path $OutputDir ("Renders\" + $BossID + "_QuadRetro_front.png")

    if ((Test-Path -LiteralPath $OutputModel) -and
        (Test-Path -LiteralPath $ReportPath) -and
        (Test-Path -LiteralPath $FrontRender)) {
        Write-Host "[QuadRetro] skip complete $BossID"
        Write-RunLog @{ boss_id = $BossID; status = "SkippedComplete"; output_model = $OutputModel; report = $ReportPath; front_render = $FrontRender }
        continue
    }

    Write-Host "[QuadRetro] start $BossID"
    Write-RunLog @{ boss_id = $BossID; status = "Started"; input_model = $InputModel; output_dir = $OutputDir }

    try {
        powershell -ExecutionPolicy Bypass -File $Wrapper `
            -InputModel $InputModel `
            -OutputDir $OutputDir `
            -Label $BossID `
            -TargetQuads 12000 `
            -AdaptiveSize 50 `
            -QRemeshSourceTargetTris 30000 `
            -TextureSize 512 `
            -PaletteMode "none" `
            -DitherType "none" `
            -DitherStrength 0 `
            -BakeSize 1024 `
            -RenderQA:$true `
            -Background:$false `
            -TimeoutSeconds 1800

        if ($LASTEXITCODE -ne 0) {
            throw "Wrapper failed with exit code $LASTEXITCODE"
        }

        $Ok = (Test-Path -LiteralPath $OutputModel) -and
            (Test-Path -LiteralPath $ReportPath) -and
            (Test-Path -LiteralPath $FrontRender)
        if (-not $Ok) {
            throw "Expected output, report, or front render missing after successful wrapper exit."
        }

        Write-Host "[QuadRetro] done $BossID"
        Write-RunLog @{ boss_id = $BossID; status = "Completed"; output_model = $OutputModel; report = $ReportPath; front_render = $FrontRender }
    }
    catch {
        Write-Warning "[QuadRetro] failed $BossID : $($_.Exception.Message)"
        Write-RunLog @{ boss_id = $BossID; status = "Failed"; error = $_.Exception.Message; output_model = $OutputModel; report = $ReportPath; front_render = $FrontRender }
    }
}
