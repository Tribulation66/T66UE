param(
    [Parameter(Mandatory = $true)]
    [string]$PodIp,

    [Parameter(Mandatory = $true)]
    [int]$Port,

    [string]$User = "root",
    [string]$KeyPath = "C:\Users\DoPra\.ssh\id_ed25519",
    [string]$AccessFile = "C:\UE\T66\Model Generation\LOCAL_ACCESS.env",
    [string]$PythonPath = "/opt/conda/envs/pixal3d/bin/python"
)

$tokenLine = Get-Content -LiteralPath $AccessFile |
    Where-Object { $_ -match '^HF_TOKEN=' } |
    Select-Object -First 1

if (-not $tokenLine) {
    throw "HF_TOKEN entry not found in $AccessFile"
}

$token = $tokenLine.Substring("HF_TOKEN=".Length).Trim()

$tempPy = [System.IO.Path]::GetTempFileName() + ".py"
$remotePy = "/tmp/pixal3d_hf_login.py"
@"
from huggingface_hub import login
login(token="$token")
"@ | Set-Content -LiteralPath $tempPy -Encoding Ascii

try {
    & scp.exe `
        -o BatchMode=yes `
        -o StrictHostKeyChecking=no `
        -o UserKnownHostsFile=NUL `
        -i $KeyPath `
        -P $Port `
        $tempPy `
        "${User}@${PodIp}:$remotePy"

    if ($LASTEXITCODE -ne 0) {
        throw "Failed to upload Hugging Face login helper."
    }

    & ssh.exe `
        -o BatchMode=yes `
        -o StrictHostKeyChecking=no `
        -o UserKnownHostsFile=NUL `
        -i $KeyPath `
        -p $Port `
        "$User@$PodIp" `
        "$PythonPath $remotePy"

    if ($LASTEXITCODE -ne 0) {
        throw "Remote Hugging Face login failed."
    }
}
finally {
    & ssh.exe `
        -o BatchMode=yes `
        -o StrictHostKeyChecking=no `
        -o UserKnownHostsFile=NUL `
        -i $KeyPath `
        -p $Port `
        "$User@$PodIp" `
        "rm -f $remotePy" | Out-Null

    Remove-Item -LiteralPath $tempPy -Force -ErrorAction SilentlyContinue
}
