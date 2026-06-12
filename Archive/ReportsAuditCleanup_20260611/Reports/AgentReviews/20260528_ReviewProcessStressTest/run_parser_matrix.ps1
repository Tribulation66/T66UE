[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$Root = "C:\UE\T66"
$RunRoot = Join-Path $Root "Reports\AgentReviews\20260528_ReviewProcessStressTest"
$FixtureRoot = Join-Path $RunRoot "parser_fixtures"
$ClaudeHelper = Join-Path $Root "Scripts\Invoke-ClaudePlanReview.ps1"
$CodexHelper = Join-Path $Root "Scripts\Invoke-CodexPlanReview.ps1"

$null = New-Item -ItemType Directory -Force -Path $FixtureRoot

$Cases = @(
    [pscustomobject]@{
        Name = "valid_approve"
        Content = "Verdict: APPROVE`n`nBlockers`n- None."
        ExpectedVerdict = "APPROVE"
        ExpectedGreenlit = $true
        ExpectedMalformed = $false
        Note = "APPROVE should be the only greenlight."
    },
    [pscustomobject]@{
        Name = "valid_revise"
        Content = "Verdict: REVISE`n`nMajor Issues`n- Missing repo inspection."
        ExpectedVerdict = "REVISE"
        ExpectedGreenlit = $false
        ExpectedMalformed = $false
        Note = "REVISE is valid but should not greenlight."
    },
    [pscustomobject]@{
        Name = "valid_needs_human_decision"
        Content = "Verdict: NEEDS_HUMAN_DECISION`n`nClarifying Questions`n- Pick direction A or B."
        ExpectedVerdict = "NEEDS_HUMAN_DECISION"
        ExpectedGreenlit = $false
        ExpectedMalformed = $false
        Note = "Human decision is valid but must pause."
    },
    [pscustomobject]@{
        Name = "valid_block"
        Content = "Verdict: BLOCK`n`nBlockers`n- Required credential is unavailable."
        ExpectedVerdict = "BLOCK"
        ExpectedGreenlit = $false
        ExpectedMalformed = $false
        Note = "Hard block is valid but must pause."
    },
    [pscustomobject]@{
        Name = "missing_verdict"
        Content = "Blockers`n- No first-line verdict."
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Missing verdict must fail closed."
    },
    [pscustomobject]@{
        Name = "verdict_not_first"
        Content = "Review complete.`nVerdict: APPROVE"
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Verdict after preface must fail closed."
    },
    [pscustomobject]@{
        Name = "unknown_verdict"
        Content = "Verdict: WAIT_FOR_USER`n`nClarifying Questions`n- Continue?"
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Unknown legacy/user-gate token must fail closed."
    },
    [pscustomobject]@{
        Name = "quoted_verdict"
        Content = "> Verdict: APPROVE`n`nBody."
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Quoted verdict must fail closed."
    },
    [pscustomobject]@{
        Name = "heading_verdict"
        Content = "# Verdict: APPROVE`n`nBody."
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Markdown heading verdict must fail closed."
    },
    [pscustomobject]@{
        Name = "indented_verdict"
        Content = " Verdict: APPROVE`n`nBody."
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Indented verdict must fail closed."
    },
    [pscustomobject]@{
        Name = "lowercase_token_approve"
        Content = "Verdict: approve`n`nBody."
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Lowercase token variant must fail closed."
    },
    [pscustomobject]@{
        Name = "lowercase_prefix_approve"
        Content = "verdict: APPROVE`n`nBody."
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Lowercase Verdict prefix must fail closed."
    },
    [pscustomobject]@{
        Name = "uppercase_prefix_approve"
        Content = "VERDICT: APPROVE`n`nBody."
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Uppercase Verdict prefix must fail closed."
    },
    [pscustomobject]@{
        Name = "stale_approved"
        Content = "Verdict: APPROVED`n`nBody."
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Stale APPROVED token must fail closed."
    },
    [pscustomobject]@{
        Name = "stale_needs_user_decision"
        Content = "Verdict: NEEDS_USER_DECISION`n`nBody."
        ExpectedVerdict = ""
        ExpectedGreenlit = $false
        ExpectedMalformed = $true
        Note = "Stale NEEDS_USER_DECISION token must fail closed."
    },
    [pscustomobject]@{
        Name = "trailing_space_approve"
        Content = "Verdict: APPROVE   `n`nBlockers`n- None."
        ExpectedVerdict = "APPROVE"
        ExpectedGreenlit = $true
        ExpectedMalformed = $false
        Note = "Trailing spaces are tolerated by TrimEnd."
    },
    [pscustomobject]@{
        Name = "duplicate_conflicting_after_valid"
        Content = "Verdict: APPROVE`nVerdict: BLOCK`n`nBlockers`n- Conflicting second verdict."
        ExpectedVerdict = "APPROVE"
        ExpectedGreenlit = $true
        ExpectedMalformed = $false
        Note = "Current parser is first-line-only; this exposes a possible stricter future guard."
    }
)

$Rows = @()
foreach ($Case in $Cases) {
    $FixturePath = Join-Path $FixtureRoot "$($Case.Name).md"
    Set-Content -LiteralPath $FixturePath -Value $Case.Content -Encoding UTF8

    foreach ($Helper in @(
        [pscustomobject]@{ Name = "Claude"; Path = $ClaudeHelper },
        [pscustomobject]@{ Name = "Codex"; Path = $CodexHelper }
    )) {
        $Result = & $Helper.Path -ParseReviewPathOnly $FixturePath
        $ActualVerdict = ""
        if ($Result.PSObject.Properties.Name -contains "Verdict" -and $null -ne $Result.Verdict) {
            $ActualVerdict = [string] $Result.Verdict
        }
        $ActualGreenlit = [bool] $Result.Greenlit
        $ActualMalformed = $ActualVerdict -ceq ""
        $Passed = ($ActualVerdict -ceq $Case.ExpectedVerdict) -and ($ActualGreenlit -eq $Case.ExpectedGreenlit) -and ($ActualMalformed -eq $Case.ExpectedMalformed)

        $Rows += [pscustomobject]@{
            Helper = $Helper.Name
            Case = $Case.Name
            ExpectedVerdict = $Case.ExpectedVerdict
            ActualVerdict = $ActualVerdict
            ExpectedGreenlit = $Case.ExpectedGreenlit
            ActualGreenlit = $ActualGreenlit
            ExpectedMalformed = $Case.ExpectedMalformed
            ActualMalformed = $ActualMalformed
            Passed = $Passed
            Note = $Case.Note
        }
    }
}

$CsvPath = Join-Path $RunRoot "parser_matrix_results.csv"
$JsonPath = Join-Path $RunRoot "parser_matrix_results.json"
$MdPath = Join-Path $RunRoot "parser_matrix_results.md"

$Rows | Export-Csv -NoTypeInformation -LiteralPath $CsvPath
$Rows | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $JsonPath -Encoding UTF8

$Lines = @()
$Lines += "# Parser Matrix Results"
$Lines += ""
$Lines += "| Helper | Case | Expected | Actual | Greenlit | Passed | Note |"
$Lines += "|---|---|---:|---:|---:|---:|---|"
foreach ($Row in $Rows) {
    $Expected = if ([string]::IsNullOrWhiteSpace($Row.ExpectedVerdict)) { "MALFORMED" } else { $Row.ExpectedVerdict }
    $Actual = if ([string]::IsNullOrWhiteSpace($Row.ActualVerdict)) { "MALFORMED" } else { $Row.ActualVerdict }
    $Lines += "| $($Row.Helper) | $($Row.Case) | $Expected | $Actual | $($Row.ActualGreenlit) | $($Row.Passed) | $($Row.Note) |"
}
$Lines | Set-Content -LiteralPath $MdPath -Encoding UTF8

$Failed = @($Rows | Where-Object { -not $_.Passed })
[pscustomobject]@{
    Result = if ($Failed.Count -eq 0) { "PASS" } else { "FAIL" }
    CaseCount = $Cases.Count
    RowCount = $Rows.Count
    FailedCount = $Failed.Count
    CsvPath = $CsvPath
    JsonPath = $JsonPath
    MarkdownPath = $MdPath
}
