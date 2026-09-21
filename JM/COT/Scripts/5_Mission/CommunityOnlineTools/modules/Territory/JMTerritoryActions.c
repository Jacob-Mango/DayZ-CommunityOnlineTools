#ifdef EXPANSIONMODBASEBUILDING
//! Admin sets a territory's level. See JMModuleAction.
class JMTerritorySetLevel: JMModuleAction
{
	int TerritoryID;
	int NewLevel;

	override int GetRPC() { return JMTerritoryModuleRPC.SetLevel; }

	override string GetPermission() { return JMConstants.PERM_EXPANSION_TERRITORY_SETLEVEL; }

	override string GetWebhookType() { return "SetLevel"; }

	override int GetWebhookColor() { return JMConstants.WEBHOOK_COLOR_WARNING; }

	override void Write( ParamsWriteContext ctx )
	{
		ctx.Write( TerritoryID );
		ctx.Write( NewLevel );
	}

	override bool Read( ParamsReadContext ctx )
	{
		return ctx.Read( TerritoryID ) && ctx.Read( NewLevel );
	}

	override bool Apply( JMModuleBase module, PlayerIdentity sender, JMPlayerInstance admin )
	{
		ExpansionTerritoryModule territoryModule = ExpansionTerritoryModule.Cast( CF_ModuleCoreManager.Get( ExpansionTerritoryModule ) );
		if ( territoryModule )
			territoryModule.Exec_AdminSetTerritoryLevel( TerritoryID, NewLevel, sender );

		return true;
	}

	override string Describe() { return "Set territory " + TerritoryID + " to level " + NewLevel; }
}
#endif
