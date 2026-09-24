//! The four admin actions of the Compensations module. Tiny subclasses of one base, kept in one
//! file - see JMModuleAction for how an action is sent, gated and applied.

class JMCompensationSpawnCursor: JMModuleAction
{
	string SteamID;
	string Timestamp;
	vector Position;

	override int GetRPC() { return JMCompensationsModuleRPC.SpawnCursor; }

	override string GetPermission() { return JMConstants.PERM_COMPENSATIONS_SPAWN; }

	override string GetWebhookType() { return "Spawn"; }

	override int GetWebhookColor() { return JMConstants.WEBHOOK_COLOR_SPAWN; }

	override void Write( ParamsWriteContext ctx )
	{
		ctx.Write( SteamID );
		ctx.Write( Timestamp );
		ctx.Write( Position );
	}

	override bool Read( ParamsReadContext ctx )
	{
		if ( !ctx.Read( SteamID ) || !ctx.Read( Timestamp ) || !ctx.Read( Position ) )
			return false;

		return COT.IsValidWorldPosition( Position );
	}

	override bool Apply( JMModuleBase module, PlayerIdentity sender, JMPlayerInstance admin )
	{
		JMCompensationsModule compensations = JMCompensationsModule.Cast( module );

		return compensations && compensations.SpawnAtPosition( SteamID, Timestamp, Position );
	}

	override string Describe() { return "Spawned compensation for " + SteamID + " (" + Timestamp + ") at " + Position; }

	override string DescribeWebhook() { return "Spawned compensation for **" + SteamID + "** (" + Timestamp + ") at " + Position.ToString(); }
}

class JMCompensationSpawnTarget: JMModuleAction
{
	string SteamID;
	string Timestamp;
	EntityAI Target;

	override int GetRPC() { return JMCompensationsModuleRPC.SpawnTarget; }

	override string GetPermission() { return JMConstants.PERM_COMPENSATIONS_SPAWN; }

	override string GetWebhookType() { return "Spawn"; }

	override int GetWebhookColor() { return JMConstants.WEBHOOK_COLOR_SPAWN; }

	override void Write( ParamsWriteContext ctx )
	{
		ctx.Write( SteamID );
		ctx.Write( Timestamp );
		ctx.Write( Target );
	}

	override bool Read( ParamsReadContext ctx )
	{
		return ctx.Read( SteamID ) && ctx.Read( Timestamp ) && ctx.Read( Target );
	}

	override bool Apply( JMModuleBase module, PlayerIdentity sender, JMPlayerInstance admin )
	{
		JMCompensationsModule compensations = JMCompensationsModule.Cast( module );

		return compensations && compensations.SpawnOnEntity( SteamID, Timestamp, Target );
	}

	override string Describe() { return "Spawned compensation for " + SteamID + " (" + Timestamp + ") on " + Target.GetType(); }

	override string DescribeWebhook() { return "Spawned compensation for **" + SteamID + "** (" + Timestamp + ") on " + Target.GetType(); }
}

class JMCompensationSpawnPlayers: JMModuleAction
{
	string SteamID;
	string Timestamp;
	ref array< string > GUIDs;

	override int GetRPC() { return JMCompensationsModuleRPC.SpawnPlayers; }

	override string GetPermission() { return JMConstants.PERM_COMPENSATIONS_SPAWN; }

	override string GetWebhookType() { return "Spawn"; }

	override int GetWebhookColor() { return JMConstants.WEBHOOK_COLOR_SPAWN; }

	override void Write( ParamsWriteContext ctx )
	{
		ctx.Write( SteamID );
		ctx.Write( Timestamp );
		ctx.Write( GUIDs );
	}

	override bool Read( ParamsReadContext ctx )
	{
		if ( !ctx.Read( SteamID ) || !ctx.Read( Timestamp ) || !ctx.Read( GUIDs ) )
			return false;

		return GUIDs.Count() <= JMConstants.RPC_MAX_GUIDS;
	}

	override bool Apply( JMModuleBase module, PlayerIdentity sender, JMPlayerInstance admin )
	{
		JMCompensationsModule compensations = JMCompensationsModule.Cast( module );

		return compensations && compensations.SpawnOnPlayers( SteamID, Timestamp, GUIDs );
	}

	override string Describe() { return "Spawned compensation for " + SteamID + " (" + Timestamp + ") on " + GUIDs.Count() + " player(s)"; }

	override string DescribeWebhook() { return "Spawned compensation for **" + SteamID + "** (" + Timestamp + ") on " + GUIDs.Count() + " player(s)"; }
}

class JMCompensationDelete: JMModuleAction
{
	string SteamID;
	string Timestamp;

	override int GetRPC() { return JMCompensationsModuleRPC.Delete; }

	override string GetPermission() { return JMConstants.PERM_COMPENSATIONS_DELETE; }

	override string GetWebhookType() { return "Delete"; }

	override int GetWebhookColor() { return JMConstants.WEBHOOK_COLOR_DANGER; }

	override void Write( ParamsWriteContext ctx )
	{
		ctx.Write( SteamID );
		ctx.Write( Timestamp );
	}

	override bool Read( ParamsReadContext ctx )
	{
		return ctx.Read( SteamID ) && ctx.Read( Timestamp );
	}

	override bool Apply( JMModuleBase module, PlayerIdentity sender, JMPlayerInstance admin )
	{
		JMCompensationsModule compensations = JMCompensationsModule.Cast( module );

		return compensations && compensations.DeleteCompensation( SteamID, Timestamp );
	}

	override string Describe() { return "Deleted compensation for " + SteamID + " (" + Timestamp + ")"; }

	override string DescribeWebhook() { return "Deleted compensation for **" + SteamID + "** (" + Timestamp + ")"; }
}
