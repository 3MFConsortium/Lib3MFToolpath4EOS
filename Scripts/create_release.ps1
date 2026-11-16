<#
Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Builds LibEOS in Release and packages the binary release ZIP
          (build-release/LibEOS-<version>-win64.zip).

Usage:    powershell -ExecutionPolicy Bypass -File Scripts\create_release.ps1 [-BuildDir <dir>] [-Generator <cmake generator>]
#>

param(
	[string]$BuildDir = "",
	[string]$Generator = ""
)

$ErrorActionPreference = "Stop"

$RepositoryDir = Split-Path -Parent $PSScriptRoot
if ($BuildDir -eq "") {
	$BuildDir = Join-Path $RepositoryDir "build-release"
}

function Invoke-Checked([string]$Tool, [string[]]$Arguments) {
	Write-Host "> $Tool $($Arguments -join ' ')"
	& $Tool @Arguments
	if ($LASTEXITCODE -ne 0) {
		throw "$Tool failed with exit code $LASTEXITCODE"
	}
}

$ConfigureArguments = @("-S", $RepositoryDir, "-B", $BuildDir, "-DLIBEOS_BUILD_EXAMPLES=ON")
if ($Generator -ne "") {
	$ConfigureArguments += @("-G", $Generator)
}

Invoke-Checked "cmake" $ConfigureArguments
Invoke-Checked "cmake" @("--build", $BuildDir, "--config", "Release", "--parallel")

Push-Location $BuildDir
try {
	Invoke-Checked "cpack" @("-G", "ZIP", "-C", "Release", "--config", "CPackConfig.cmake")
}
finally {
	Pop-Location
}

$Package = Get-ChildItem -Path $BuildDir -Filter "LibEOS-*-win64.zip" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if ($null -eq $Package) {
	throw "cpack did not produce a LibEOS-*-win64.zip in $BuildDir"
}
Write-Host ""
Write-Host "Release package: $($Package.FullName)"
