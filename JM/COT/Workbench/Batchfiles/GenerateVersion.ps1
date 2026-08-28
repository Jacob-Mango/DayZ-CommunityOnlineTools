<#
.SYNOPSIS
    Stamp the COT build version into the sources that ship inside the PBO.

.DESCRIPTION
    Enforce Script cannot read a file at compile time and the engine does not
    expose CfgMods' `versionPath` back to script, so the only way the running
    mod can know its own version is for that version to be a compiled-in
    constant. This script generates that constant.

    It writes two files, both generated - never hand-edit either:

      Scripts/Data/Version.hpp                      the plain version string,
                                                    which CfgMods.versionPath
                                                    points at (launcher/mod list)

      Scripts/3_Game/CommunityOnlineTools/JMBuildInfo.c
                                                    the same values as Enforce
                                                    constants, which is what the
                                                    sidebar footer reads

    The build number in Workbench/version is incremented on every run, so two
    builds of the same source are still distinguishable.

.PARAMETER Channel
    stable | beta | experimental | internal. Omit to auto-detect (see below).

.PARAMETER NoBump
    Regenerate from the current version without incrementing the build number.

.NOTES
    Channel auto-detection, most specific first:
      COT_BUILD_CHANNEL environment variable, if set
      clean tree on an annotated/lightweight tag        -> stable
      clean tree on main / master                       -> beta
      branch named exp*, experimental*, or 1.*-exp      -> experimental
      anything else (feature branch, or a dirty tree)   -> internal
#>
[CmdletBinding()]
param(
    [ValidateSet('stable', 'beta', 'experimental', 'internal')]
    [string] $Channel,

    [switch] $NoBump
)

$ErrorActionPreference = 'Stop'

$repoRoot    = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$versionFile = Join-Path $repoRoot 'Workbench\version'
$hppFile     = Join-Path $repoRoot 'Scripts\Data\Version.hpp'
$buildInfoC  = Join-Path $repoRoot 'Scripts\3_Game\CommunityOnlineTools\JMBuildInfo.c'

# --- version number -----------------------------------------------------------

if (Test-Path $versionFile) {
    $raw = (Get-Content $versionFile -Raw).Trim()
} else {
    $raw = '0.0.0'
}

$parts = $raw -split '\.'
if ($parts.Count -lt 3) { $parts = @('0', '0', '0') }

$major = [int] $parts[0]
$minor = [int] $parts[1]
$build = [int] $parts[2]

if (-not $NoBump) { $build++ }

$version = "$major.$minor.$build"

# --- git facts ----------------------------------------------------------------

function Get-GitValue {
    param([string[]] $GitArgs)
    try {
        $out = & git @GitArgs 2>$null
        if ($LASTEXITCODE -ne 0) { return '' }
        if ($null -eq $out) { return '' }
        return ($out | Select-Object -First 1).ToString().Trim()
    } catch {
        return ''
    }
}

$commit = Get-GitValue @('rev-parse', '--short=8', 'HEAD')
$branch = Get-GitValue @('rev-parse', '--abbrev-ref', 'HEAD')
$tag    = Get-GitValue @('describe', '--tags', '--exact-match', 'HEAD')
$status = Get-GitValue @('status', '--porcelain')
$dirty  = -not [string]::IsNullOrWhiteSpace($status)

if ($commit -eq '') { $commit = 'unknown' }
if ($branch -eq '') { $branch = 'unknown' }

# --- channel ------------------------------------------------------------------

if (-not $Channel) {
    if ($env:COT_BUILD_CHANNEL) {
        $Channel = $env:COT_BUILD_CHANNEL.ToLower()
    } elseif ($dirty) {
        # A dirty tree is by definition not a build anyone else can reproduce.
        $Channel = 'internal'
    } elseif ($tag -ne '') {
        $Channel = 'stable'
    } elseif ($branch -in @('main', 'master')) {
        $Channel = 'beta'
    } elseif ($branch -match '^(exp|experimental)') {
        $Channel = 'experimental'
    } else {
        $Channel = 'internal'
    }
}

if ($Channel -notin @('stable', 'beta', 'experimental', 'internal')) {
    $Channel = 'internal'
}

# Short suffix appended to the version everywhere it is shown. Stable carries
# none - a release should read as a bare version number.
$suffix = switch ($Channel) {
    'stable'       { '' }
    'beta'         { '-beta' }
    'experimental' { '-exp' }
    default        { '-internal' }
}

$displayVersion = "$version$suffix"
$builtAt        = (Get-Date).ToUniversalTime().ToString('yyyy-MM-dd HH:mm')

# --- write --------------------------------------------------------------------

if (-not $NoBump) {
    Set-Content -Path $versionFile -Value $version -Encoding ascii -NoNewline
}

New-Item -ItemType Directory -Force -Path (Split-Path $hppFile) | Out-Null
Set-Content -Path $hppFile -Value $displayVersion -Encoding ascii -NoNewline

$enforce = @"
// =============================================================================
//  JMBuildInfo.c - GENERATED, DO NOT EDIT
//
//  Written by Workbench/Batchfiles/GenerateVersion.ps1 on every build. Any
//  hand edit is overwritten by the next one. Change the generator instead.
//
//  Enforce Script has no compile-time file access and the engine never hands
//  CfgMods' versionPath back to script, so a compiled-in constant is the only
//  way the running mod can report its own build.
// =============================================================================

class JMBuildInfo
{
	//! Channel ids. Compared against CHANNEL rather than parsed out of VERSION.
	static const string CHANNEL_STABLE       = "stable";
	static const string CHANNEL_BETA         = "beta";
	static const string CHANNEL_EXPERIMENTAL = "experimental";
	static const string CHANNEL_INTERNAL     = "internal";

	//! Bare MAJOR.MINOR.BUILD, no channel suffix.
	static const string VERSION = "$version";

	//! VERSION plus the channel suffix - what the UI shows.
	static const string DISPLAY_VERSION = "$displayVersion";

	//! One of the CHANNEL_* ids above.
	static const string CHANNEL = "$Channel";

	//! Short commit the build came from, or "unknown" outside a git checkout.
	static const string COMMIT = "$commit";

	//! Branch the build came from.
	static const string BRANCH = "$branch";

	//! True when the working tree had uncommitted changes at build time.
	static const bool DIRTY = $($dirty.ToString().ToLower());

	//! UTC build timestamp, "YYYY-MM-DD HH:MM".
	static const string BUILT_AT = "$builtAt";

	//! "COT 1.4.2" / "COT 1.4.2-beta". What the sidebar footer renders.
	static string GetFooterText()
	{
		return "COT " + DISPLAY_VERSION;
	}

	//! Full provenance for bug reports and the server info module.
	static string GetLongVersion()
	{
		return "COT " + DISPLAY_VERSION + " (" + COMMIT + " on " + BRANCH + ", built " + BUILT_AT + " UTC)";
	}

	static bool IsStable()
	{
		return CHANNEL == CHANNEL_STABLE;
	}
}
"@

New-Item -ItemType Directory -Force -Path (Split-Path $buildInfoC) | Out-Null
Set-Content -Path $buildInfoC -Value $enforce -Encoding ascii

Write-Host "[version] $displayVersion  channel=$Channel  commit=$commit  branch=$branch  dirty=$dirty"
Write-Host "[version] wrote $hppFile"
Write-Host "[version] wrote $buildInfoC"
