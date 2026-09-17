#ifdef JM_COT_AUTOTEST

//! Target: JMConstants.Lucide (Scripts/3_Game/CommunityOnlineTools/JMConstants.c)
//!
//! Lucide() is the single seam every dynamically-named lucide icon path goes
//! through (2034 icons addressed by name rather than one constant each). If
//! its prefix or extension ever drifts, every dynamic icon lookup silently
//! starts pointing at a texture that doesn't exist - no compile error, just a
//! blank icon at runtime.

static void JMAutoTest_Lucide_KnownName()
{
	JMAutoTest_Group("Lucide() builds the expected path for a real icon name");

	string got = JMConstants.Lucide("shield-check");
	JMAutoTest_AssertEqualStr("Lucide('shield-check') path", got, "JM/COT/GUI/textures/icons/lucide/shield-check.edds");
}

static void JMAutoTest_Lucide_EmptyName()
{
	JMAutoTest_Group("Lucide() with an empty name still returns the bare prefix+extension");

	// Documents current behaviour: Lucide() does no validation of `name`, so
	// an empty string still round-trips into a syntactically well-formed (if
	// non-existent) path rather than throwing. If validation is ever added,
	// this assertion is the one to update.
	string got = JMConstants.Lucide("");
	JMAutoTest_AssertEqualStr("Lucide('') path", got, "JM/COT/GUI/textures/icons/lucide/.edds");
}

static void JMAutoTest_Lucide_PrefixIsStable()
{
	JMAutoTest_Group("Lucide() output always starts under GUI/textures/icons/lucide/");

	string got = JMConstants.Lucide("x");
	JMAutoTest_Assert("Lucide('x') starts with the lucide icon prefix", got.IndexOf("JM/COT/GUI/textures/icons/lucide/") == 0);
}

static void JMAutoTest_Lucide_ExtensionIsEdds()
{
	JMAutoTest_Group("Lucide() always appends the .edds extension");

	string got = JMConstants.Lucide("settings");
	int extIdx = got.Length() - 5;
	JMAutoTest_AssertEqualStr("Lucide('settings') ends in .edds", got.Substring(extIdx, 5), ".edds");
}

static void JMAutoTest_JMConstants()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMConstants.Lucide() path building");
	PrintFormat("================================================================");

	JMAutoTest_Lucide_KnownName();
	JMAutoTest_Lucide_EmptyName();
	JMAutoTest_Lucide_PrefixIsStable();
	JMAutoTest_Lucide_ExtensionIsEdds();
}

#endif
