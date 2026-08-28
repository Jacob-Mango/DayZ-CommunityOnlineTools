<#
.SYNOPSIS
    Stamp the build version, then hand off to dz-project-manager.exe build.

.DESCRIPTION
    dz-project-manager.exe has no pre-build hook, and the version constants the
    mod reports at runtime have to exist as source before the PBO is packed.
    This wrapper is the seam: it runs GenerateVersion.ps1 first, then forwards
    every argument it was given straight through to the real build action.

    Use this instead of calling dz-project-manager.exe build directly, or the
    sidebar footer will report whatever version the last stamped build left
    behind.

.EXAMPLE
    ./build.ps1
    ./build.ps1 --build-only
    ./build.ps1 -Channel stable
#>
# PositionalBinding is off on purpose: with it on, a pass-through flag like
# --build-only binds positionally to -Channel and fails its ValidateSet.
[CmdletBinding(PositionalBinding = $false)]
param(
    # Overrides channel auto-detection for this build.
    [ValidateSet('stable', 'beta', 'experimental', 'internal')]
    [string] $Channel,

    # Regenerate the version constants without incrementing the build number.
    [switch] $NoBump,

    # Everything else goes to dz-project-manager.exe build verbatim.
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]] $BuildArgs
)

$stampArgs = @{}
if ($Channel) { $stampArgs['Channel'] = $Channel }
if ($NoBump)  { $stampArgs['NoBump']  = $true }

& (Join-Path $PSScriptRoot 'Workbench\Batchfiles\GenerateVersion.ps1') @stampArgs
if ($LASTEXITCODE -ne 0 -and $null -ne $LASTEXITCODE) {
    # GenerateVersion only shells out to git, whose exit code is not ours to
    # propagate; reset it so the build verdict below is the one that counts.
    $global:LASTEXITCODE = 0
}

$exe = Join-Path $PSScriptRoot 'dz-project-manager.exe'

# dz-project-manager writes progress lines to stderr. Windows PowerShell wraps
# every native stderr line in an ErrorRecord, which under ErrorActionPreference
# = Stop is a TERMINATING error - the build would abort on its first heartbeat
# and swallow its own output. Continue is what lets the run finish; the exit
# code is the real verdict.
$ErrorActionPreference = 'Continue'

if ($BuildArgs) {
    & $exe build @BuildArgs
} else {
    & $exe build
}

exit $LASTEXITCODE

