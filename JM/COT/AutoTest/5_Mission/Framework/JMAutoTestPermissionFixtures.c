#ifdef JM_COT_AUTOTEST

//! Reusable fixtures for permission-tree testing: a throwaway JMPlayerInstance
//! built the same way an offline/local player is (new JMPlayerInstance(NULL)),
//! and small helpers to grant/deny a single permission string in isolation.
//! See JMPlayerInstance.c (HasPermission/HasOwnPermission/AddPermission) and
//! JMPermission.c (Check - INHERIT/DISALLOW/ALLOW tri-state, DISALLOW poisons
//! descendants until a deeper ALLOW resets it) for the mechanics these wrap.
//!
//! Lives in AutoTest/5_Mission (not 3_Game): JMPlayerInstance is a 4_World
//! class, and Enforce compiles 3_Game before 4_World, so referencing it from
//! a 3_Game file is a "Bad type" compile error - 5_Mission compiles last and
//! can see both.
//!
//! IMPORTANT: AddPermission() requires the permission string to already be
//! registered by some module's constructor (requireRegistered=true by
//! default). By the time the autotest boot pass runs, every module has
//! already run its own constructor and registered its own permissions, so any
//! permission string taken from a real RegisterPermission(...) call site is
//! safe to use here. Never invent a permission string that no module
//! registers.

//! A fresh, isolated JMPlayerInstance with no roles beyond the implicit
//! default and no explicit permissions - a clean slate to grant/deny against.
static JMPlayerInstance JMAutoTest_NewFixturePlayer()
{
	return new JMPlayerInstance(NULL);
}

//! Grants `permission` ALLOW directly on the player's own tree (not via a
//! role) and asserts HasPermission() now reports true.
static void JMAutoTest_AssertPermissionGrantable(string label, string permission)
{
	JMPlayerInstance p = JMAutoTest_NewFixturePlayer();
	p.AddPermission(permission, JMPermissionType.ALLOW);
	JMAutoTest_Assert(label + " - ALLOW grants HasPermission('" + permission + "')", p.HasPermission(permission));
}

//! Explicitly DISALLOWs `permission` on the player's own tree and asserts
//! HasPermission() now reports false.
static void JMAutoTest_AssertPermissionDenyable(string label, string permission)
{
	JMPlayerInstance p = JMAutoTest_NewFixturePlayer();
	p.AddPermission(permission, JMPermissionType.DISALLOW);
	JMAutoTest_AssertFalse(label + " - DISALLOW blocks HasPermission('" + permission + "')", p.HasPermission(permission));
}

#endif
