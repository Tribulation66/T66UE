<#
.SYNOPSIS
Verifies Claude review result parsing and helper separation.

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
    [pscustomobject]@{ Name = "explicit_ok"; Content = "Result: OK`n`n## Issues`nNone."; Greenlit = $true; Result = "OK"; OutcomeKind = "ClaudeValidResult"; FailureKind = $null },
    [pscustomobject]@{ Name = "explicit_needs_user"; Content = "Result: NEEDS_USER`n`n## Questions`nOne."; Greenlit = $false; Result = "NEEDS_USER"; OutcomeKind = "ClaudeValidResult"; FailureKind = $null },
    [pscustomobject]@{ Name = "prefaced_ok"; Content = "I reviewed it.`nResult: OK"; Greenlit = $true; Result = "OK"; OutcomeKind = "ClaudeValidResult"; FailureKind = $null },
    [pscustomobject]@{ Name = "lowercase_ok"; Content = "result: ok`n`nBody."; Greenlit = $true; Result = "OK"; OutcomeKind = "ClaudeValidResult"; FailureKind = $null },
    [pscustomobject]@{ Name = "inferred_ok"; Content = "No blockers. Safe to proceed."; Greenlit = $true; Result = "OK"; OutcomeKind = "ClaudeValidResult"; FailureKind = $null },
    [pscustomobject]@{ Name = "inferred_needs_user"; Content = "This requires approval because only the user can decide."; Greenlit = $false; Result = "NEEDS_USER"; OutcomeKind = "ClaudeValidResult"; FailureKind = $null },
    [pscustomobject]@{ Name = "first_result_wins"; Content = "Result: NEEDS_USER`nResult: OK"; Greenlit = $false; Result = "NEEDS_USER"; OutcomeKind = "ClaudeValidResult"; FailureKind = $null },
    [pscustomobject]@{ Name = "stale_approved"; Content = "Verdict: APPROVED`n`nBody."; Greenlit = $false; Result = $null; OutcomeKind = "ClaudeMalformedResult"; FailureKind = "ClaudeMalformedResult" },
    [pscustomobject]@{ Name = "no_result"; Content = "No result here."; Greenlit = $false; Result = $null; OutcomeKind = "ClaudeMalformedResult"; FailureKind = "ClaudeMalformedResult" },
    [pscustomobject]@{ Name = "empty"; Content = ""; Greenlit = $false; Result = $null; OutcomeKind = "ClaudeMalformedResult"; FailureKind = "ClaudeMalformedResult" },
    [pscustomobject]@{ Name = "whitespace"; Content = "   `r`n`t "; Greenlit = $false; Result = $null; OutcomeKind = "ClaudeMalformedResult"; FailureKind = "ClaudeMalformedResult" }
)

foreach ($Case in $Cases) {
    $Path = New-Fixture -Name $Case.Name -Content $Case.Content
    $Result = Invoke-Parse -Path $Path
    Assert-Equal -Actual $Result.Greenlit -Expected $Case.Greenlit -Label "$($Case.Name) Greenlit"
    Assert-Equal -Actual $Result.Result -Expected $Case.Result -Label "$($Case.Name) Result"
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
