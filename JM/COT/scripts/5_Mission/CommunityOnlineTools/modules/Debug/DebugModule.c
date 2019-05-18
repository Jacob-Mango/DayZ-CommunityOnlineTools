class DebugModule: EditorModule
{
	void DebugModule()
	{
		GetRPCManager().AddRPC( "COT_Manager", "LoadData", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "COT.Apply" );

		Debugging.OnUpdate.Insert( this.ReloadSettings );
	}

	void ~DebugModule()
	{
		Debugging.OnUpdate.Remove( this.ReloadSettings );
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
			GetDebugging().Save();
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/Debug/DebugMenu.layout";
	}
	
	void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param1< ref Debugging > data;
		if ( !ctx.Read( data ) ) return;

		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "COT.Apply", sender ) )
				return;

			GetDebugging().Copy( data.param1 );
		}
	}
}