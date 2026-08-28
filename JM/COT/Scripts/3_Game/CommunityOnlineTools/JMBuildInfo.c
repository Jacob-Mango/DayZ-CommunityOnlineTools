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
	static const string VERSION = "0.8.100451";

	//! VERSION plus the channel suffix - what the UI shows.
	static const string DISPLAY_VERSION = "0.8.100451-internal";

	//! One of the CHANNEL_* ids above.
	static const string CHANNEL = "internal";

	//! Short commit the build came from, or "unknown" outside a git checkout.
	static const string COMMIT = "5207366c";

	//! Branch the build came from.
	static const string BRANCH = "lt-experiments";

	//! True when the working tree had uncommitted changes at build time.
	static const bool DIRTY = true;

	//! UTC build timestamp, "YYYY-MM-DD HH:MM".
	static const string BUILT_AT = "2026-08-28 01:35";

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
