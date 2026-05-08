param(
    [string[]]$Only = @()
)

$ErrorActionPreference = "Stop"

$RepoRoot = "C:\UE\T66"
$RunRoot = Join-Path $RepoRoot "Model Generation\Runs\EnemyBosses\EnemyBossBatch01"
$ManifestPath = Join-Path $RunRoot "Reports\Stage01_Enemies_TrellisManifest.json"
$Wrapper = Join-Path $RepoRoot "Model Generation\Scripts\RunQuadRetroCharacterPipeline.ps1"
$RunLog = Join-Path $RunRoot "Reports\Stage02_Enemies_QuadRetro_RunLog.jsonl"

if (!(Test-Path -LiteralPath $ManifestPath)) {
    throw "Enemy Trellis manifest not found: $ManifestPath"
}
if (!(Test-Path -LiteralPath $Wrapper)) {
    throw "Quad Retro wrapper not found: $Wrapper"
}

$Manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
$Rows = @($Manifest.rows)

function Write-RunLog {
    param([hashtable]$Payload)

    $Payload.timestamp = (Get-Date).ToString("o")
    ($Payload | ConvertTo-Json -Compress) | Add-Content -LiteralPath $RunLog -Encoding UTF8
}

function Test-ReportSettings {
    param(
        [string]$ReportPath,
        [string]$OutputModel
    )

    if (!(Test-Path -LiteralPath $ReportPath)) {
        return $false
    }

    $Report = Get-Content -LiteralPath $ReportPath -Raw | ConvertFrom-Json
    return (
        (Test-Path -LiteralPath $OutputModel) -and
        $Report.output_glb -and
        (Test-Path -LiteralPath ([string]$Report.output_glb)) -and
        $Report.retopo_quads -and
        $Report.retopo_triangles -and
        $Report.adjustable_values.texture_size -eq 512 -and
        $Report.adjustable_values.bake_size -eq 1024 -and
        $Report.adjustable_values.palette_mode -eq "none" -and
        $Report.adjustable_values.dither_type -eq "none" -and
        [double]$Report.adjustable_values.dither_strength -eq 0 -and
        $Report.qremesh_report.last_progress -eq 2
    )
}

foreach ($Row in $Rows) {
    $EnemyID = [string]$Row.row_id
    if ($Only.Count -gt 0 -and $EnemyID -notin $Only) {
        continue
    }
    $InputModel = Join-Path $RunRoot ([string]$Row.raw_trellis_glb)
    $OutputDir = Join-Path $RunRoot ("QuadRetro\Medium\Enemies\" + $EnemyID)
    $OutputModel = Join-Path $OutputDir ("Models\" + $EnemyID + "_QuadRetro.glb")
    $ReportPath = Join-Path $OutputDir ("Reports\" + $EnemyID + "_QuadRetro_report.json")
    $FrontRender = Join-Path $OutputDir ("Renders\" + $EnemyID + "_QuadRetro_front.png")

    if (!(Test-Path -LiteralPath $InputModel)) {
        Write-Warning "[QuadRetro] missing raw Trellis input $EnemyID"
        Write-RunLog @{ enemy_id = $EnemyID; status = "MissingInput"; input_model = $InputModel }
        continue
    }

    if ((Test-Path -LiteralPath $FrontRender) -and (Test-ReportSettings -ReportPath $ReportPath -OutputModel $OutputModel)) {
        Write-Host "[QuadRetro] skip complete $EnemyID"
        Write-RunLog @{ enemy_id = $EnemyID; status = "SkippedComplete"; output_model = $OutputModel; report = $ReportPath; front_render = $FrontRender }
        continue
    }

    $ExistingProcesses = @(Get-Process blender,xremesh -ErrorAction SilentlyContinue)
    if ($ExistingProcesses.Count -gt 0) {
        $Names = ($ExistingProcesses | ForEach-Object { "$($_.ProcessName)#$($_.Id)" }) -join ", "
        throw "Refusing to start $EnemyID because Blender/Quad Remesher is already running: $Names"
    }

    Write-Host "[QuadRetro] start $EnemyID"
    Write-RunLog @{ enemy_id = $EnemyID; status = "Started"; input_model = $InputModel; output_dir = $OutputDir }

    try {
        powershell -ExecutionPolicy Bypass -File $Wrapper `
            -InputModel $InputModel `
            -OutputDir $OutputDir `
            -Label $EnemyID `
            -TargetQuads 12000 `
            -AdaptiveSize 50 `
            -QRemeshSourceTargetTris 8000 `
            -TextureSize 512 `
            -PaletteMode "none" `
            -DitherType "none" `
            -DitherStrength 0 `
            -BakeSize 1024 `
            -RenderQA:$true `
            -Background:$false `
            -TimeoutSeconds 900

        if ($LASTEXITCODE -ne 0) {
            throw "Wrapper failed with exit code $LASTEXITCODE"
        }

        if (!(Test-Path -LiteralPath $FrontRender) -or !(Test-ReportSettings -ReportPath $ReportPath -OutputModel $OutputModel)) {
            throw "Expected output, report settings, or front render missing after wrapper exit."
        }

        $Report = Get-Content -LiteralPath $ReportPath -Raw | ConvertFrom-Json
        Write-Host "[QuadRetro] done $EnemyID"
        Write-RunLog @{
            enemy_id = $EnemyID
            status = "Completed"
            output_model = $OutputModel
            report = $ReportPath
            front_render = $FrontRender
            retopo_quads = $Report.retopo_quads
            retopo_triangles = $Report.retopo_triangles
        }
    }
    catch {
        Write-Warning "[QuadRetro] failed $EnemyID : $($_.Exception.Message)"
        Write-RunLog @{
            enemy_id = $EnemyID
            status = "Failed"
            error = $_.Exception.Message
            output_model = $OutputModel
            report = $ReportPath
            front_render = $FrontRender
        }
        break
    }
}
