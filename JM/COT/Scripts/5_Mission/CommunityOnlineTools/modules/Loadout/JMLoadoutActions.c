//! Admin actions of the Loadout module that fit the JMModuleAction shape. The spawn actions do
//! not: they log and post a webhook once per player, which one action cannot express.

class JMLoadoutDelete: JMModuleAction
{
	string Loadout;

	override int GetRPC() { return JMLoadoutModuleRPC.Delete; }

	override string GetPermission() { return JMConstants.PERM_LOADOUTS_DELETE; }

	override string GetWebhookType() { return "Delete"; }

	override int GetWebhookColor() { return JMConstants.WEBHOOK_COLOR_DANGER; }

	override void Write( ParamsWriteContext ctx )
	{
		ctx.Write( Loadout );
	}

	override bool Read( ParamsReadContext ctx )
	{
		return ctx.Read( Loadout );
	}

	override bool Apply( JMModuleBase module, PlayerIdentity sender, JMPlayerInstance admin )
	{
		JMLoadoutSettings.Delete( Loadout );

		return true;
	}

	override string Describe() { return "Deleted '" + Loadout + "'"; }
}
