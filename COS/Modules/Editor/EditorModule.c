class EditorModule : Module
{
    void EditorModule()
    {
        GetRPCManager().AddRPC( "COS", "SetPosition", this, SingeplayerExecutionType.Server );
    }

    void ~EditorModule()
    {

    }

	override void RegisterKeyMouseBindings() 
	{
		KeyMouseBinding toggleCOMEditor = new KeyMouseBinding( GetModuleType() , "ShowCOMEditor" , "[Y]"    , "Opens the COM Editor."        );
    
		toggleCOMEditor.AddKeyBind( KeyCode.KC_Y,    KeyMouseBinding.KB_EVENT_RELEASE );

		RegisterKeyMouseBinding( toggleCOMEditor );
    }

    void ShowCOMEditor()
    {
        GetGame().GetUIManager().ShowScriptedMenu( new EditorMenu() , NULL );
    }

    void SetPosition( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
		Param1< vector > data;
		if ( !ctx.Read( data ) ) return;
		
		if( type == CallType.Server )
		{	
            target.SetPosition( data.param1 );
		}
    }
}