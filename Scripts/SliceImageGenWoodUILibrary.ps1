param(
    [string]$LibrarySheet = "SourceAssets/UI/MasterLibrary/Concepts/20260501_imagegen_wood_library_v2_no_alpha/main_menu_wood_asset_library_imagegen_sheet_v2_no_alpha.png",
    [string]$SquareButtonSheet = "SourceAssets/UI/MasterLibrary/Concepts/20260501_imagegen_wood_library_v2_no_alpha/topbar_square_button_imagegen_sheet_v1.png",
    [string]$ReferenceButtonSlotSheet = "SourceAssets/UI/MasterLibrary/Concepts/20260501_imagegen_wood_library_v6_reference_buttons_slots/wood_buttons_slots_reference_sheet_v1.png",
    [string]$PanelSheet = "SourceAssets/UI/MasterLibrary/Concepts/20260501_imagegen_wood_panel_9slice_v1/wood_panel_9slice_crop_v1.png",
    [string]$InnerPanelSheet = "SourceAssets/UI/MasterLibrary/Concepts/20260501_imagegen_wood_panel_9slice_v1/wood_inner_panel_crop_v1.png",
    [string]$DropdownFieldSheet = "SourceAssets/UI/MasterLibrary/Concepts/20260501_imagegen_wood_library_v3_white_controls/leaderboard_dropdown_fields_dark_imagegen_sheet_v1.png",
    [string]$OutRoot = "SourceAssets/UI/MasterLibrary/Slices",
    [string]$Manifest = "SourceAssets/UI/MasterLibrary/imagegen_wood_library_v2_no_alpha_slice_manifest.json"
)

$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

function Resolve-RepoPath {
    param([string]$Path)
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return $Path
    }
    return (Join-Path $Root $Path)
}

function Copy-ImageCrop {
    param(
        [System.Drawing.Bitmap]$SourceImage,
        [int]$X,
        [int]$Y,
        [int]$W,
        [int]$H,
        [string]$RelativeOutput
    )

    $OutputPath = Resolve-RepoPath (Join-Path $OutRoot $RelativeOutput)
    $OutputDir = Split-Path -Parent $OutputPath
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

    $Rect = [System.Drawing.Rectangle]::new($X, $Y, $W, $H)
    $Crop = $SourceImage.Clone($Rect, $SourceImage.PixelFormat)
    try {
        $Crop.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $Crop.Dispose()
    }

    return [ordered]@{
        output = ($OutputPath.Substring($Root.Length + 1) -replace "\\", "/")
        source_box = @($X, $Y, $X + $W, $Y + $H)
        size = @($W, $H)
    }
}

function Copy-ImageCropWithChromaKey {
    param(
        [System.Drawing.Bitmap]$SourceImage,
        [int]$X,
        [int]$Y,
        [int]$W,
        [int]$H,
        [string]$RelativeOutput
    )

    $OutputPath = Resolve-RepoPath (Join-Path $OutRoot $RelativeOutput)
    $OutputDir = Split-Path -Parent $OutputPath
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

    $Crop = New-Object System.Drawing.Bitmap $W, $H, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    try {
        for ($Py = 0; $Py -lt $H; $Py++) {
            for ($Px = 0; $Px -lt $W; $Px++) {
                $Color = $SourceImage.GetPixel($X + $Px, $Y + $Py)
                $IsKeyGreen = $Color.G -gt 40 `
                    -and $Color.R -lt 145 `
                    -and $Color.B -lt 145 `
                    -and $Color.G -gt ($Color.R * 1.55) `
                    -and $Color.G -gt ($Color.B * 1.55)

                if ($IsKeyGreen) {
                    $Crop.SetPixel($Px, $Py, [System.Drawing.Color]::FromArgb(0, 0, 0, 0))
                }
                else {
                    $Crop.SetPixel($Px, $Py, [System.Drawing.Color]::FromArgb(255, $Color.R, $Color.G, $Color.B))
                }
            }
        }

        $Crop.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $Crop.Dispose()
    }

    return [ordered]@{
        output = ($OutputPath.Substring($Root.Length + 1) -replace "\\", "/")
        source_box = @($X, $Y, $X + $W, $Y + $H)
        size = @($W, $H)
        chroma_key = "#00ff00"
    }
}

function Copy-ExistingPng {
    param(
        [string]$SourceRelative,
        [string]$TargetRelative
    )

    $SourcePath = Resolve-RepoPath (Join-Path $OutRoot $SourceRelative)
    $TargetPath = Resolve-RepoPath (Join-Path $OutRoot $TargetRelative)
    $TargetDir = Split-Path -Parent $TargetPath
    New-Item -ItemType Directory -Force -Path $TargetDir | Out-Null
    Copy-Item -LiteralPath $SourcePath -Destination $TargetPath -Force

    return [ordered]@{
        output = ($TargetPath.Substring($Root.Length + 1) -replace "\\", "/")
        source = $SourceRelative
    }
}

$LibraryPath = Resolve-RepoPath $LibrarySheet
$SquareButtonPath = Resolve-RepoPath $SquareButtonSheet
$ReferenceButtonSlotPath = Resolve-RepoPath $ReferenceButtonSlotSheet
$PanelPath = Resolve-RepoPath $PanelSheet
$InnerPanelPath = Resolve-RepoPath $InnerPanelSheet
$DropdownFieldPath = Resolve-RepoPath $DropdownFieldSheet
$ManifestPath = Resolve-RepoPath $Manifest

if (!(Test-Path -LiteralPath $LibraryPath)) {
    throw "Missing imagegen library sheet: $LibraryPath"
}
if (!(Test-Path -LiteralPath $SquareButtonPath)) {
    throw "Missing imagegen square button sheet: $SquareButtonPath"
}
if (!(Test-Path -LiteralPath $ReferenceButtonSlotPath)) {
    throw "Missing imagegen reference button/slot sheet: $ReferenceButtonSlotPath"
}
if (!(Test-Path -LiteralPath $PanelPath)) {
    throw "Missing imagegen panel sheet: $PanelPath"
}
if (!(Test-Path -LiteralPath $InnerPanelPath)) {
    throw "Missing imagegen inner panel sheet: $InnerPanelPath"
}
if (!(Test-Path -LiteralPath $DropdownFieldPath)) {
    throw "Missing imagegen dropdown field sheet: $DropdownFieldPath"
}

$ManifestEntries = New-Object System.Collections.Generic.List[object]

$LibraryImage = [System.Drawing.Bitmap]::FromFile($LibraryPath)
try {
    $LibraryCrops = @(
        @{ key = "wide_normal"; box = @(26, 26, 462, 122); output = "Buttons/basic_button_normal.png" },
        @{ key = "wide_hover"; box = @(534, 25, 462, 124); output = "Buttons/basic_button_hover.png" },
        @{ key = "wide_pressed"; box = @(1046, 27, 452, 121); output = "Buttons/basic_button_pressed.png" },
        @{ key = "wide_disabled"; box = @(27, 185, 461, 120); output = "Buttons/basic_button_disabled.png" },
        @{ key = "wide_selected"; box = @(1046, 27, 452, 121); output = "Buttons/select_button_selected.png" },
        @{ key = "select_normal"; box = @(645, 458, 400, 91); output = "Buttons/select_button_normal.png" },
        @{ key = "select_selected"; box = @(645, 458, 400, 91); output = "Buttons/duo_button_left_selected.png" },
        @{ key = "fill_bar"; box = @(28, 587, 403, 41); output = "Misc/progress_fill_wood.png" },
        @{ key = "pfp_slot"; box = @(97, 660, 242, 209); output = "Slots/pfp_slot_normal.png" },
        @{ key = "paper"; box = @(1035, 575, 470, 291); output = "Panels/paper_background.png" },
        @{ key = "topbar_plate"; box = @(22, 902, 1491, 94); output = "TopBar/topbar_strip_normal.png" }
    )

    foreach ($Crop in $LibraryCrops) {
        $Box = $Crop.box
        $Entry = Copy-ImageCrop -SourceImage $LibraryImage -X $Box[0] -Y $Box[1] -W $Box[2] -H $Box[3] -RelativeOutput $Crop.output
        $Entry.key = $Crop.key
        $Entry.source = ($LibraryPath.Substring($Root.Length + 1) -replace "\\", "/")
        $ManifestEntries.Add($Entry)
    }
}
finally {
    $LibraryImage.Dispose()
}

$PanelImage = [System.Drawing.Bitmap]::FromFile($PanelPath)
try {
    $Entry = Copy-ImageCrop -SourceImage $PanelImage -X 0 -Y 0 -W $PanelImage.Width -H $PanelImage.Height -RelativeOutput "Panels/basic_panel_normal.png"
    $Entry.key = "panel_9slice_basic"
    $Entry.source = ($PanelPath.Substring($Root.Length + 1) -replace "\\", "/")
    $ManifestEntries.Add($Entry)
}
finally {
    $PanelImage.Dispose()
}

$InnerPanelImage = [System.Drawing.Bitmap]::FromFile($InnerPanelPath)
try {
    $Entry = Copy-ImageCrop -SourceImage $InnerPanelImage -X 0 -Y 0 -W $InnerPanelImage.Width -H $InnerPanelImage.Height -RelativeOutput "Panels/inner_panel_normal.png"
    $Entry.key = "panel_9slice_inner"
    $Entry.source = ($InnerPanelPath.Substring($Root.Length + 1) -replace "\\", "/")
    $ManifestEntries.Add($Entry)
}
finally {
    $InnerPanelImage.Dispose()
}

$DropdownImage = [System.Drawing.Bitmap]::FromFile($DropdownFieldPath)
try {
    $DropdownCrops = @(
        @{ key = "dropdown_normal"; box = @(34, 318, 815, 214); output = "Controls/dropdown_field_normal.png" },
        @{ key = "dropdown_hover"; box = @(923, 318, 818, 214); output = "Controls/dropdown_field_hover.png" }
    )

    foreach ($Crop in $DropdownCrops) {
        $Box = $Crop.box
        $Entry = Copy-ImageCrop -SourceImage $DropdownImage -X $Box[0] -Y $Box[1] -W $Box[2] -H $Box[3] -RelativeOutput $Crop.output
        $Entry.key = $Crop.key
        $Entry.source = ($DropdownFieldPath.Substring($Root.Length + 1) -replace "\\", "/")
        $ManifestEntries.Add($Entry)
    }
}
finally {
    $DropdownImage.Dispose()
}

$ReferenceButtonSlotImage = [System.Drawing.Bitmap]::FromFile($ReferenceButtonSlotPath)
try {
    $ReferenceButtonSlotCrops = @(
        @{ key = "reference_wide_normal"; box = @(32, 149, 285, 200); output = "Buttons/basic_button_normal.png" },
        @{ key = "reference_wide_hover"; box = @(348, 149, 285, 200); output = "Buttons/basic_button_hover.png" },
        @{ key = "reference_wide_pressed"; box = @(660, 149, 285, 200); output = "Buttons/basic_button_pressed.png" },
        @{ key = "reference_wide_disabled"; box = @(976, 149, 240, 200); output = "Buttons/basic_button_disabled.png" },
        @{ key = "reference_round_normal"; box = @(48, 437, 230, 116); output = "Buttons/round_button_normal.png" },
        @{ key = "reference_round_hover"; box = @(369, 437, 230, 116); output = "Buttons/round_button_hover.png" },
        @{ key = "reference_round_pressed"; box = @(658, 437, 232, 116); output = "Buttons/round_button_pressed.png" },
        @{ key = "reference_round_disabled"; box = @(968, 437, 235, 116); output = "Buttons/round_button_disabled.png" },
        @{ key = "reference_pfp_slot"; box = @(55, 654, 263, 246); output = "Slots/pfp_slot_normal.png" },
        @{ key = "reference_party_slot"; box = @(55, 654, 263, 246); output = "Slots/basic_slot_normal.png" },
        @{ key = "reference_dropdown_field"; box = @(64, 990, 548, 102); output = "Controls/dropdown_field_normal.png" },
        @{ key = "reference_select_button"; box = @(684, 990, 222, 102); output = "Buttons/select_button_normal.png" },
        @{ key = "reference_select_selected"; box = @(348, 149, 285, 200); output = "Buttons/select_button_selected.png" }
    )

    foreach ($Crop in $ReferenceButtonSlotCrops) {
        $Box = $Crop.box
        $Entry = Copy-ImageCropWithChromaKey -SourceImage $ReferenceButtonSlotImage -X $Box[0] -Y $Box[1] -W $Box[2] -H $Box[3] -RelativeOutput $Crop.output
        $Entry.key = $Crop.key
        $Entry.source = ($ReferenceButtonSlotPath.Substring($Root.Length + 1) -replace "\\", "/")
        $ManifestEntries.Add($Entry)
    }
}
finally {
    $ReferenceButtonSlotImage.Dispose()
}

$SquareImage = [System.Drawing.Bitmap]::FromFile($SquareButtonPath)
try {
    $SquareCrops = @(
        @{ key = "square_normal"; box = @(23, 140, 417, 417); output = "Buttons/borderless_icon_button_normal.png" },
        @{ key = "square_hover"; box = @(452, 140, 417, 417); output = "Buttons/borderless_icon_button_hover.png" },
        @{ key = "square_pressed"; box = @(881, 140, 417, 417); output = "Buttons/borderless_icon_button_pressed.png" },
        @{ key = "square_disabled"; box = @(1309, 140, 417, 417); output = "Buttons/borderless_icon_button_disabled.png" },
        @{ key = "square_selected"; box = @(881, 140, 417, 417); output = "Buttons/borderless_icon_button_selected.png" }
    )

    foreach ($Crop in $SquareCrops) {
        $Box = $Crop.box
        $Entry = Copy-ImageCrop -SourceImage $SquareImage -X $Box[0] -Y $Box[1] -W $Box[2] -H $Box[3] -RelativeOutput $Crop.output
        $Entry.key = $Crop.key
        $Entry.source = ($SquareButtonPath.Substring($Root.Length + 1) -replace "\\", "/")
        $ManifestEntries.Add($Entry)
    }
}
finally {
    $SquareImage.Dispose()
}

$Aliases = @(
    @{ target = "Buttons/central_button_normal.png"; source = "Buttons/basic_button_normal.png" },
    @{ target = "Buttons/central_button_hover.png"; source = "Buttons/basic_button_hover.png" },
    @{ target = "Buttons/central_button_pressed.png"; source = "Buttons/basic_button_pressed.png" },
    @{ target = "Buttons/central_button_disabled.png"; source = "Buttons/basic_button_disabled.png" },
    @{ target = "Buttons/borderless_icon_button_selected.png"; source = "Buttons/borderless_icon_button_pressed.png" },
    @{ target = "Buttons/dropdown_option_button_normal.png"; source = "Buttons/basic_button_normal.png" },
    @{ target = "Buttons/dropdown_option_button_hover.png"; source = "Buttons/basic_button_hover.png" },
    @{ target = "Buttons/dropdown_option_button_pressed.png"; source = "Buttons/basic_button_pressed.png" },
    @{ target = "Buttons/dropdown_option_button_disabled.png"; source = "Buttons/basic_button_disabled.png" },
    @{ target = "Buttons/select_button_hover.png"; source = "Buttons/select_button_normal.png" },
    @{ target = "Buttons/select_button_pressed.png"; source = "Buttons/select_button_normal.png" },
    @{ target = "Buttons/select_button_disabled.png"; source = "Buttons/basic_button_disabled.png" },
    @{ target = "Buttons/duo_button_left_normal.png"; source = "Buttons/select_button_normal.png" },
    @{ target = "Buttons/duo_button_left_hover.png"; source = "Buttons/select_button_normal.png" },
    @{ target = "Buttons/duo_button_left_pressed.png"; source = "Buttons/select_button_normal.png" },
    @{ target = "Buttons/duo_button_left_disabled.png"; source = "Buttons/basic_button_disabled.png" },
    @{ target = "Buttons/duo_button_right_normal.png"; source = "Buttons/select_button_normal.png" },
    @{ target = "Buttons/duo_button_right_hover.png"; source = "Buttons/select_button_normal.png" },
    @{ target = "Buttons/duo_button_right_pressed.png"; source = "Buttons/select_button_normal.png" },
    @{ target = "Buttons/duo_button_right_disabled.png"; source = "Buttons/basic_button_disabled.png" },
    @{ target = "Buttons/duo_button_right_selected.png"; source = "Buttons/select_button_normal.png" },
    @{ target = "Controls/dropdown_field_hover.png"; source = "Controls/dropdown_field_normal.png" },
    @{ target = "Controls/radio_circle_selected.png"; source = "Controls/radio_circle_normal.png" },
    @{ target = "Fields/search_field_normal.png"; source = "Panels/paper_background.png" }
)

foreach ($Alias in $Aliases) {
    $Entry = Copy-ExistingPng -SourceRelative $Alias.source -TargetRelative $Alias.target
    $Entry.key = "alias:$($Alias.target)"
    $ManifestEntries.Add($Entry)
}

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $ManifestPath) | Out-Null
$ManifestData = [ordered]@{
    pipeline = "imagegen-source-rectangular-crop-copy-only"
    local_alpha_or_chroma_key = $true
    local_color_mutation = $false
    sources = @(
        ($LibraryPath.Substring($Root.Length + 1) -replace "\\", "/"),
        ($SquareButtonPath.Substring($Root.Length + 1) -replace "\\", "/"),
        ($ReferenceButtonSlotPath.Substring($Root.Length + 1) -replace "\\", "/"),
        ($PanelPath.Substring($Root.Length + 1) -replace "\\", "/"),
        ($InnerPanelPath.Substring($Root.Length + 1) -replace "\\", "/"),
        ($DropdownFieldPath.Substring($Root.Length + 1) -replace "\\", "/")
    )
    slices = $ManifestEntries
}
$ManifestData | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ManifestPath -Encoding UTF8

Write-Host "Wrote imagegen wood UI library slices; alpha-bearing source pixels were preserved."
Write-Host $ManifestPath
