class JMDebugModule: JMRenderableModuleBase
{
	void JMDebugModule()
	{
		GetRPCManager().AddRPC( "COT_Manager", "LoadData", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "COT.Apply" );

		CFLogger.OnUpdate.Insert( this.ReloadSettings );
	}

	void ~JMDebugModule()
	{
		CFLogger.OnUpdate.Remove( this.ReloadSettings );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "COT.View" );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();
	}

	override void ReloadSettings()
	{
		if ( !GetGame().IsClient() || ( GetGame().IsServer() && GetGame().IsMultiplayer() ) )
			return;

		if ( form && form.IsVisible() )
			form.OnShow();
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( GetGame().IsServer() )
			GetLogger().Save();
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/debug_form.layout";
	}
	
	void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param1< ref CFLogger > data;
		if ( !ctx.Read( data ) ) return;

		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "COT.Apply", sender ) )
				return;

			GetLogger().Copy( data.param1 );
		}
	}
}