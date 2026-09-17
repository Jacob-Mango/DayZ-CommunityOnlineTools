#ifdef JM_COT_AUTOTEST

//! RPC surface check for JMServerStatsModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/ServerStats/JMServerStatsModule.c).
//!
//! sender=NULL would be the identity ceiling for this boot-time test if there
//! were anything to call - there is no live PlayerIdentity to attach at this
//! point in the boot sequence. This file documents CURRENT behavior as a
//! regression baseline: JMServerStatsModuleRPC (RPC.c) has a single member,
//! Stats, and RPC.c documents it as "Server -> Client. No client -> server
//! direction: nothing is requested". The OnRPC switch does route Stats to a
//! handler (RPC_Stats), but that handler opens with `if (!IsMissionClient())
//! return;` - on the server side (where this suite runs) it is a no-op by
//! construction, not a real client -> server entry point. There is genuinely
//! no client -> server RPC surface on this module, so there is nothing to
//! sweep and the canary machinery is skipped entirely.

static void JMAutoTest_JMServerStatsModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMServerStatsModule RPC surface check");
	PrintFormat("================================================================");

	JMServerStatsModule mod;
	if (!CF_Modules<JMServerStatsModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMServerStatsModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Pass("JMServerStatsModule has no client->server RPC surface - Stats is server-to-client only, nothing to sweep");
}

#endif
