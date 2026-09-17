#ifdef JM_COT_AUTOTEST

//! Target: JMCommand/JMSubCommand.Execute (Scripts/4_World/CommunityOnlineTools/
//! Classes/Commands/JMCommand.c) - the single dispatch point every registered
//! chat command funnels through - and the "Actions.QuickActions" permission
//! string (Scripts/4_World/CommunityOnlineTools/Classes/PermissionsOld/
//! JMPermissionManager.c - HasQuickActionAccess).
//!
//! Only two modules register chat commands in this codebase
//! (JMCommandConstructor discovers them dynamically, there is no static
//! list): JMTeleportModule ("teleport"/"tp" -> position/pos/get) and
//! JMObjectSpawnerModule ("object" -> spawn). JMCommandModule's own
//! JMCommandModuleRPC.PerformCommand dispatch (which parses chat input and
//! looks these up by name) is already covered by
//! JMAutoTest_JMCommandModule.c; this suite instead constructs a
//! JMSubCommand identical to what AddSubCommand() would build for each real
//! registration and calls Execute() directly, so the actual permission
//! check + null-identity path gets exercised without needing to round-trip
//! through chat-string parsing.
//!
//! Actions.QuickActions itself (Scripts/4_World/CommunityOnlineTools/
//! CommunityOnlineTools.c constructor - RegisterPermission) is not an RPC or
//! a module gate, it is a single permission string checked identically on
//! client and server by HasQuickActionAccess(PlayerBase) inside every
//! CAContinuous* action component. HasQuickActionAccess needs a real
//! PlayerBase to call GetIdentity() on, which does not exist at this
//! boot-time stage (no player has spawned yet) - so this suite tests the
//! permission string itself via the same grant/deny round trip the
//! permission matrix suite uses, which is the actual decision
//! HasQuickActionAccess reduces to once past its own player-identity lookup.
//!
//! sender=NULL is the identity ceiling for this boot-time test - no
//! PlayerIdentity mock exists anywhere in this engine or mod.

static void JMAutoTest_ChatCommands_TeleportPositionSurvivesNullSender()
{
	JMAutoTest_Group("teleport/tp 'position' subcommand (JMTeleportModule.Command_Position) survives NULL sender");

	JMTeleportModule mod;
	if (!CF_Modules<JMTeleportModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMTeleportModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	//! Mirrors JMModuleBase.AddSubCommand's exact construction for the real
	//! "position" registration (JMTeleportModule.c - GetSubCommands).
	JMSubCommand cmd = new JMSubCommand(mod, "position", "Command_Position", "Admin.Player.Teleport.Position");

	JMAutoTest_ExpectCanary();
	cmd.Execute(NULL, new array<string>());
	JMAutoTest_Canary("teleport 'position' Execute(NULL sender)");

	JMAutoTest_CanaryCheck("Chat command - teleport position");
}

static void JMAutoTest_ChatCommands_ObjectSpawnSurvivesNullSender()
{
	JMAutoTest_Group("object 'spawn' subcommand (JMObjectSpawnerModule.Command_Spawn) survives NULL sender");

	JMObjectSpawnerModule mod;
	if (!CF_Modules<JMObjectSpawnerModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMObjectSpawnerModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	//! Mirrors JMModuleBase.AddSubCommand's exact construction for the real
	//! "spawn" registration (JMObjectSpawnerModule.c - GetSubCommands).
	JMSubCommand cmd = new JMSubCommand(mod, "spawn", "Command_Spawn", "Entity.Spawn.Position");

	JMAutoTest_ExpectCanary();
	cmd.Execute(NULL, new array<string>());
	JMAutoTest_Canary("object 'spawn' Execute(NULL sender)");

	JMAutoTest_CanaryCheck("Chat command - object spawn");
}

static void JMAutoTest_QuickActions_PermissionGrantDeny()
{
	JMAutoTest_Group("Actions.QuickActions permission string - ALLOW/DISALLOW round trip");

	//! Already registered by CommunityOnlineTools.c's constructor at boot -
	//! a real permission, not a scratch one, so no registration call needed.
	JMAutoTest_AssertPermissionGrantable("Actions.QuickActions", "Actions.QuickActions");
	JMAutoTest_AssertPermissionDenyable("Actions.QuickActions", "Actions.QuickActions");
}

static void JMAutoTest_ChatCommandsAndQuickActions()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: Chat commands (JMCommand.Execute) + Actions.QuickActions");
	PrintFormat("================================================================");

	JMAutoTest_ChatCommands_TeleportPositionSurvivesNullSender();
	JMAutoTest_ChatCommands_ObjectSpawnSurvivesNullSender();
	JMAutoTest_QuickActions_PermissionGrantDeny();
}

#endif
