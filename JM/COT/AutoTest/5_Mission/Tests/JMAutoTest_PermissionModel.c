#ifdef JM_COT_AUTOTEST

//! Target: JMPlayerInstance.HasPermission/HasOwnPermission (JMPlayerInstance.c),
//! JMPermission.Check (JMPermission.c) - the tri-state (INHERIT/DISALLOW/ALLOW)
//! logic every module's HasAccess() and every RPC's permission check ultimately
//! runs on. Uses a dedicated scratch permission, registered here, so this suite
//! never depends on - or interferes with - any real module's permission tree.

static void JMAutoTest_PermissionModel_RegisterScratchPermission()
{
	//! RegisterPermission is idempotent per the existing pattern (every module
	//! calls it unconditionally in its own constructor) - safe to call once here.
	GetPermissionsManager().RegisterPermission("JMAutoTest.Scratch.Permission");
}

//! This suite runs from CommunityOnlineTools.OnStart(), which is not
//! guaranteed to happen after COTModule.OnMissionLoaded() (the module
//! lifecycle callback that normally creates the "everyone"/"admin" roles) -
//! confirmed this session by GetRole("everyone") coming back null here.
//! Force creation deterministically rather than depend on that ordering.
static void JMAutoTest_PermissionModel_EnsureDefaultRoles()
{
	COTModule.EnsureDefaultRoles();
}

static void JMAutoTest_PermissionModel_DefaultIsNotGranted()
{
	JMAutoTest_Group("A fresh JMPlayerInstance with no explicit rule does not have the scratch permission");

	JMPlayerInstance p = JMAutoTest_NewFixturePlayer();
	JMAutoTest_AssertFalse("HasOwnPermission - no rule added yet", p.HasOwnPermission("JMAutoTest.Scratch.Permission"));
}

static void JMAutoTest_PermissionModel_AllowGrants()
{
	JMAutoTest_Group("ALLOW grants the permission");

	JMAutoTest_AssertPermissionGrantable("scratch permission", "JMAutoTest.Scratch.Permission");
}

static void JMAutoTest_PermissionModel_DisallowBlocks()
{
	JMAutoTest_Group("DISALLOW blocks the permission");

	JMAutoTest_AssertPermissionDenyable("scratch permission", "JMAutoTest.Scratch.Permission");
}

//! The two role-based scenarios below deliberately use a REAL, already-
//! registered permission ("Admin.Ban.View") instead of the scratch one.
//! Reason, found empirically this session: JMRole's own permission tree is a
//! snapshot COPY taken at role-CONSTRUCTION time (JMRole.c - CopyPermissions
//! in the constructor), which for "everyone" happens once at boot
//! (COTModule.OnMissionLoaded), before this suite's scratch permission is
//! ever registered. JMPermission.AddPermission defaults requireRegistered
//! to true, so granting an unregistered-in-that-tree permission on an
//! already-existing role silently no-ops - not a bug, just means a role's
//! tree does not retroactively pick up permissions registered after the
//! role itself was created. A real permission, registered at boot before
//! any role is constructed, does not hit this ordering edge case.

static void JMAutoTest_PermissionModel_RoleGrantsWhenOwnTreeIsSilent()
{
	JMAutoTest_Group("A role's ALLOW grants the permission when the player's own tree has no explicit rule (INHERIT)");

	//! JMPlayerInstance's raw constructor does NOT auto-join "everyone" -
	//! that only happens via ClearRoles() (called elsewhere, e.g. on a real
	//! player connecting). A fixture player starts with zero roles, so it
	//! must join explicitly to exercise the role-fallback path at all.
	JMPlayerInstance p = JMAutoTest_NewFixturePlayer();
	p.AddRole("everyone");

	//! "everyone" is a role every real connected JMPlayerInstance carries -
	//! use it directly rather than registering a throwaway role, since role
	//! registration/removal is a GetPermissionsManager()-wide side effect this
	//! suite should not leave behind.
	JMRole everyone = GetPermissionsManager().GetRole("everyone");
	JMAutoTest_AssertNotNull("default 'everyone' role exists", everyone);
	if (!everyone)
		return;

	everyone.AddPermission("Admin.Ban.View", JMPermissionType.ALLOW);
	JMAutoTest_Assert("HasPermission (via role) is true when own tree is silent (INHERIT)", p.HasPermission("Admin.Ban.View"));
	JMAutoTest_AssertFalse("HasOwnPermission (own tree only) is still false - role grant is not an own-tree grant", p.HasOwnPermission("Admin.Ban.View"));

	//! Clean up: remove the ALLOW so this suite does not leak a permanent
	//! grant into the shared "everyone" role for the rest of the boot pass.
	everyone.AddPermission("Admin.Ban.View", JMPermissionType.INHERIT);
}

static void JMAutoTest_PermissionModel_OwnDisallowOverridesRoleAllow()
{
	JMAutoTest_Group("Own-tree DISALLOW wins over a role's ALLOW");

	JMPlayerInstance p = JMAutoTest_NewFixturePlayer();
	p.AddRole("everyone");
	p.AddPermission("Admin.Ban.View", JMPermissionType.DISALLOW);

	JMRole everyone = GetPermissionsManager().GetRole("everyone");
	if (!everyone)
		return;

	everyone.AddPermission("Admin.Ban.View", JMPermissionType.ALLOW);

	JMAutoTest_AssertFalse("own DISALLOW blocks even though the role ALLOWs", p.HasPermission("Admin.Ban.View"));

	everyone.AddPermission("Admin.Ban.View", JMPermissionType.INHERIT); // cleanup
}

static void JMAutoTest_PermissionModel()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: Permission model core tree logic");
	PrintFormat("================================================================");

	JMAutoTest_PermissionModel_EnsureDefaultRoles();
	JMAutoTest_PermissionModel_RegisterScratchPermission();
	JMAutoTest_PermissionModel_DefaultIsNotGranted();
	JMAutoTest_PermissionModel_AllowGrants();
	JMAutoTest_PermissionModel_DisallowBlocks();
	JMAutoTest_PermissionModel_RoleGrantsWhenOwnTreeIsSilent();
	JMAutoTest_PermissionModel_OwnDisallowOverridesRoleAllow();
}

#endif
