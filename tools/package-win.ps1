$ErrorActionPreference = "Stop"

$PRODUCT = "jfduke3d"
$VERSION = Get-Date -UFormat "%Y%m%d"

$VCVARSALL = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"

if ($Args.Length -lt 2) {
    Write-Output "package-win.ps1 (amd64|x86) (build|finish)*"
    exit
}

if ($Args[0] -eq "amd64") {
    $ARCH = "amd64"
    $DIRARCH = "win"
} elseif ($Args[0] -eq "x86") {
    $ARCH = "x86"
    $DIRARCH = "win32"
} else {
    Write-Warning ("Unknown arch type {0}" -f $Args[0])
    exit
}

for ($arg = 1; $arg -lt $Args.Length; $arg++) {
    if ($Args[$arg] -eq "build") {
        Remove-Item "Makefile.msvcuser" -ErrorAction Si