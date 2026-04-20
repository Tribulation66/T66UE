param(
    [Parameter(Mandatory = $true)]
    [string]$PodIp,

    [Parameter(Mandatory = $true)]
    [int]$Port,

    [string]$User = "root",
    [string]$KeyPath = "C:\Users\DoPra\.ssh\id_ed25519",
    [string]$AccessFile = "C:\UE\T66\Model Generation\LOCAL_ACCESS.env"
)

$tokenLine = Get-Content -LiteralPath $AccessFile |
    Where-Object { $_ -match '^HF_TOKEN=' } |
    Select-Object -First 1

if (-not $tokenLine) {
    throw "HF_TOKEN entry not found in $AccessFile"
}

$token = $tokenLine.Substring("HF_TOKEN=".Length).Trim()

$tempPy = [System.IO.Path]::GetTempFileName() + ".py"
@"
from huggingface_hub import login
login(token="$token")
"@ | Set-Content -LiteralPath $tempPy -Encoding Ascii

& scp.exe `
    -o BatchMode=yes `
    -o StrictHostKeyChecking=no `
    -o UserKnownHostsFile=NUL `
    -i $KeyPath `
    -P $Port `
    $tempPy `
    "${User}@${PodIp}:/tmp/hf_login.py"

& ssh.exe `
    -o BatchMode=yes `
    -o StrictHostKeyChecking=no `
    -o UserKnownHostsFile=NUL `
    -i $KeyPath `
    -p $Port `
    "$User@$PodIp" `
    "/opt/conda/envs/trellis2/bin/python /tmp/hf_login.py"

Remove-Item -LiteralPath $tempPy -Force
