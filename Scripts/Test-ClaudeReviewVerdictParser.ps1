<#
.SYNOPSIS
Verifies strict Claude review verdict parsing and helper separation.

.DESCRIPTION
Creates temporary review artifacts under Saved\AgentReviews and calls
Invoke-ClaudePlanReview.ps1 with -ParseReviewPathOnly. The test intentionally
does not call the Claude CLI.
#>

[CmdletBinding()]
param(
    [string] $OutputRoot = "C:\UE\T66\Saved\AgentReviews\ClaudeReviewParserFixtures"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$RepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$HelperPath = Join-Path $PSScriptRoot "Invoke-ClaudePlanReview.ps1"
$DirectReadHelperPath = Join-Path $PSScriptRoot "Invoke-ClaudeDirectRead.ps1"
$OutputRootPath = $OutputRoot
if (-not [System.IO.Path]::IsPathRooted($OutputRootPath)) {
    $OutputRootPath = Join-Path $RepoRoot $OutputRootPath
}

New-Item -ItemType Directory -Force -Path $OutputRootPath | Out-Null

function New-Fixture {
    param(
        [Parameter(Mandatory = $true)][string] $Name,
        [AllowEmptyString()][string] $Content
    )

    $Path = Join-Path $OutputRootPath "$Name.md"
    Set-Content -LiteralPath $Path -Value $Content -Encoding UTF8
    return $Path
}

function Assert-Equal {
    param(
        $Actual,
        $Expected,
        [Parameter(Mandatory = $true)][string] $Label
    )

    if ($Actual -cne $Expected) {
        throw "$Label expected '$Expected' but got '$Actual'"
    }
}

function Invoke-Parse {
    param([Parameter(Mandatory = $true)][string] $Path)
    return & $HelperPath -ParseReviewPathOnly $Path
}

function Invoke-ParseAuthStatus {
    param([AllowEmptyString()][string] $Json)
    return & $HelperPath -ParseAuthStatusJsonOnly $Json
}

$Cases = @(
    [pscustomobject]@{ Name = "valid_approve"; Content = "Verdict: APPROVE`n`n## Blockers`nNone."; Greenlit = $true; Verdict = "APPROVE"; OutcomeKind = "ClaudeValidVerdict"; FailureKind = $null },
    [pscustomobject]@{ Name = "valid_revise"; Content = "Verdict: REVISE`n`n## Major Issues`nOne."; Greenlit = $false; Verdict = "REVISE"; OutcomeKind = "ClaudeValidVerdict"; FailureKind = $null },
    [pscustomobject]@{ Name = "valid_needs_human_decision"; Content = "Verdict: NEEDS_HUMAN_DECISION`n`n## Clarifying Questions`nOne."; Greenlit = $false; Verdict = "NEEDS_HUMAN_DECISION"; OutcomeKind = "ClaudeValidVerdict"; FailureKind = $null },
    [pscustomobject]@{ Name = "valid_block"; Content = "Verdict: BLOCK`n`n## Blockers`nOne."; Greenlit = $false; Verdict = "BLOCK"; OutcomeKind = "ClaudeValidVerdict"; FailureKind = $null },
    [pscustomobject]@{ Name = "prefaced_approve"; Content = "I reviewed it.`nVerdict: APPROVE"; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "body_approve"; Content = "Review body first.`n`nVerdict: APPROVE"; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "quoted_approve"; Content = "> Verdict: APPROVE`n`nBody."; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "heading_approve"; Content = "# Verdict: APPROVE`n`nBody."; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "indented_approve"; Content = " Verdict: APPROVE`n`nBody."; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "lowercase_token_approve"; Content = "Verdict: approve`n`nBody."; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "lowercase_prefix_approve"; Content = "verdict: APPROVE`n`nBody."; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "uppercase_prefix_approve"; Content = "VERDICT: APPROVE`n`nBody."; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "stale_approved"; Content = "Verdict: APPROVED`n`nBody."; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "stale_needs_user_decision"; Content = "Verdict: NEEDS_USER_DECISION`n`nBody."; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "no_verdict"; Content = "No verdict here."; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "empty"; Content = ""; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "whitespace"; Content = "   `r`n`t "; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" },
    [pscustomobject]@{ Name = "conflicting_body"; Content = "Text first.`nVerdict: APPROVE`nVerdict: BLOCK"; Greenlit = $false; Verdict = $null; OutcomeKind = "ClaudeMalformedVerdict"; FailureKind = "ClaudeMalformedVerdict" }
)

foreach ($Case in $Cases) {
    $Path = New-Fixture -Name $Case.Name -Content $Case.Content
    $Result = Invoke-Parse -Path $Path
    Assert-Equal -Actual $Result.Greenlit -Expected $Case.Greenlit -Label "$($Case.Name) Greenlit"
    Assert-Equal -Actual $Result.Verdict -Expected $Case.Verdict -Label "$($Case.Name) Verdict"
    Assert-Equal -Actual $Result.OutcomeKind -Expected $Case.OutcomeKind -Label "$($Case.Name) OutcomeKind"
    Assert-Equal -Actual $Result.FailureKind -Expected $Case.FailureKind -Label "$($Case.Name) FailureKind"
}

$AuthCases = @(
    [pscustomobject]@{ Name = "auth_valid_first_party"; Json = '{"loggedIn":true,"authMethod":"claude.ai","apiProvider":"firstParty","email":"redacted@example.invalid","orgId":"redacted","orgName":"redacted","subscriptionType":"max"}'; Authenticated = $true; StatusReason = $null },
    [pscustomobject]@{ Name = "auth_logged_out"; Json = '{"loggedIn":false,"authMethod":"claude.ai","apiProvider":"firstParty","subscriptionType":"max"}'; Authenticated = $false; StatusReason = "NotLoggedIn" },
    [pscustomobject]@{ Name = "auth_api_key"; Json = '{"loggedIn":true,"authMethod":"apiKey","apiProvider":"firstParty","subscriptionType":"max"}'; Authenticated = $false; StatusReason = "UnexpectedAuthMethod" },
    [pscustomobject]@{ Name = "auth_wrong_provider"; Json = '{"loggedIn":true,"authMethod":"claude.ai","apiProvider":"anthropic","subscriptionType":"max"}'; Authenticated = $false; StatusReason = "UnexpectedApiProvider" },
    [pscustomobject]@{ Name = "auth_missing_provider"; Json = '{"loggedIn":true,"authMethod":"claude.ai","subscriptionType":"max"}'; Authenticated = $false; StatusReason = "UnexpectedApiProvider" },
    [pscustomobject]@{ Name = "auth_malformed_json"; Json = '{"loggedIn":true'; Authenticated = $false; StatusReason = "MalformedAuthStatusJson" }
)

foreach ($Case in $AuthCases) {
    $Result = Invoke-ParseAuthStatus -Json $Case.Json
    Assert-Equal -Actual $Result.Authenticated -Expected $Case.Authenticated -Label "$($Case.Name) Authenticated"
    Assert-Equal -Actual $Result.StatusReason -Expected $Case.StatusReason -Label "$($Case.Name) StatusReason"
    if (-not $Case.Authenticated) {
        Assert-Equal -Actual $Result.FailureKind -Expected "ClaudeUnavailable" -Label "$($Case.Name) FailureKind"
    }
}

$HelperText = Get-Content -LiteralPath $HelperPath -Raw
if ($HelperText -match "Invoke-CodexPlanReview\.ps1") {
    throw "Invoke-ClaudePlanReview.ps1 must not call or name Invoke-CodexPlanReview.ps1"
}

$DirectReadHelperText = Get-Content -LiteralPath $DirectReadHelperPath -Raw
if ($DirectReadHelperText -notmatch "OperatorArtifactNotGreenlight") {
    throw "Invoke-ClaudeDirectRead.ps1 must mark operator artifacts as OperatorArtifactNotGreenlight"
}
if ($DirectReadHelperText -notmatch 'Parameter\(Mandatory = \$true\)\]\s*\[ValidateSet\("Review", "Operator"\)\]') {
    throw "Invoke-ClaudeDirectRead.ps1 must require an explicit Review or Operator mode"
}

[pscustomobject]@{
    Result = "PASS"
    FixtureCount = $Cases.Count
    AuthFixtureCount = $AuthCases.Count
    OutputRoot = $OutputRootPath
}
