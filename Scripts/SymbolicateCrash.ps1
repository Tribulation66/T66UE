param([string]$CrashDir)
# Resolve T66 module+offset frames from CrashContext.runtime-xml using dbghelp + the build PDB.
$ErrorActionPreference = "Stop"
$xml = Get-Content (Join-Path $CrashDir "CrashContext.runtime-xml") -Raw
if ($xml -notmatch "(?s)<PCallStack>(.*?)</PCallStack>") { throw "no PCallStack" }
$frames = @()
foreach ($line in ($Matches[1] -split "`n")) {
    if ($line -match "^\s*T66\s+0x([0-9a-fA-F]+)\s*\+\s*([0-9a-fA-F]+)") {
        $frames += [uint64]("0x" + $Matches[2])
    }
}
"frames: $($frames.Count)"

$dbghelp = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TestWindow\VsTest\x64\dbghelp.dll"
Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public static class DbgHelp {
    [DllImport("$($dbghelp.Replace("\","\\"))", SetLastError=true)]
    public static extern bool SymInitialize(IntPtr hProcess, string UserSearchPath, bool fInvadeProcess);
    [DllImport("$($dbghelp.Replace("\","\\"))", SetLastError=true)]
    public static extern ulong SymLoadModuleEx(IntPtr hProcess, IntPtr hFile, string ImageName, string ModuleName, ulong BaseOfDll, uint DllSize, IntPtr Data, uint Flags);
    [DllImport("$($dbghelp.Replace("\","\\"))", SetLastError=true)]
    public static extern bool SymFromAddr(IntPtr hProcess, ulong Address, out ulong Displacement, IntPtr Symbol);
    [DllImport("$($dbghelp.Replace("\","\\"))", SetLastError=true, CharSet=CharSet.Ansi)]
    public static extern bool SymGetLineFromAddr64(IntPtr hProcess, ulong Address, out uint Displacement, ref IMAGEHLP_LINE64 Line);
    [DllImport("$($dbghelp.Replace("\","\\"))")]
    public static extern uint SymSetOptions(uint options);
    [StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
    public struct IMAGEHLP_LINE64 {
        public uint SizeOfStruct; public IntPtr Key; public uint LineNumber;
        public IntPtr FileName; public ulong Address;
    }
}
"@
$hProc = [System.Diagnostics.Process]::GetCurrentProcess().Handle
[DbgHelp]::SymSetOptions(0x12) | Out-Null   # UNDNAME | LOAD_LINES (no deferred -> fail loudly)
if (-not [DbgHelp]::SymInitialize($hProc, "C:\UE\T66\Binaries\Win64", $false)) { throw "SymInitialize failed" }
$base = [DbgHelp]::SymLoadModuleEx($hProc, [IntPtr]::Zero, "C:\UE\T66\Binaries\Win64\T66.exe", $null, 0x140000000, 0, [IntPtr]::Zero, 0)
if ($base -eq 0) { throw "SymLoadModuleEx failed: $([Runtime.InteropServices.Marshal]::GetLastWin32Error())" }
"module loaded at 0x{0:X}" -f $base

# SYMBOL_INFO: SizeOfStruct(4)+...+MaxNameLen at offset 0x50? Use buffer: struct size 0x58, name follows.
$maxName = 512
$bufSize = 0x58 + $maxName
$buf = [Runtime.InteropServices.Marshal]::AllocHGlobal($bufSize)
foreach ($off in $frames) {
    # zero buffer + set SizeOfStruct (0x58) at offset 0, MaxNameLen at offset 0x50
    for ($i = 0; $i -lt $bufSize; $i += 8) { [Runtime.InteropServices.Marshal]::WriteInt64($buf, $i, 0) }
    [Runtime.InteropServices.Marshal]::WriteInt32($buf, 0, 0x58)
    [Runtime.InteropServices.Marshal]::WriteInt32($buf, 0x50, $maxName)
    $disp = [uint64]0
    $addr = $base + $off
    $name = "<unresolved>"
    if ([DbgHelp]::SymFromAddr($hProc, $addr, [ref]$disp, $buf)) {
        $name = [Runtime.InteropServices.Marshal]::PtrToStringAnsi([IntPtr]::Add($buf, 0x54))
    } else {
        $name = "<unresolved err=$([Runtime.InteropServices.Marshal]::GetLastWin32Error())>"
    }
    $lineInfo = ""
    $line = New-Object DbgHelp+IMAGEHLP_LINE64
    $line.SizeOfStruct = [uint32][Runtime.InteropServices.Marshal]::SizeOf([type][DbgHelp+IMAGEHLP_LINE64])
    $ldisp = [uint32]0
    if ([DbgHelp]::SymGetLineFromAddr64($hProc, $addr, [ref]$ldisp, [ref]$line)) {
        $file = [Runtime.InteropServices.Marshal]::PtrToStringAnsi($line.FileName)
        $lineInfo = " [${file}:$($line.LineNumber)]"
    }
    "+0x{0:x}  {1}{2}" -f $off, $name, $lineInfo
}
[Runtime.InteropServices.Marshal]::FreeHGlobal($buf)
