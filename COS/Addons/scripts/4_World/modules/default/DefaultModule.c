class DefaultModule : Module
{
    void DefaultModule()
    {
    }

    void ~DefaultModule()
    {
    }

    override void RegisterKeyMouseBindings() 
    {
        KeyMouseBinding toggleCOMEditor = new KeyMouseBinding( GetModuleType(), "ShowCOMEditor", "[Y]", "Opens the COM Editor." );
    
        toggleCOMEditor.AddKeyBind( KeyCode.KC_Y, KeyMouseBinding.KB_EVENT_RELEASE );

        RegisterKeyMouseBinding( toggleCOMEditor );
    }

    override void onUpdate( float timeslice )
    {
    }

    void ShowCOMEditor()
    {
        GetGame().GetUIManager().ShowScriptedMenu( new EditorMenu() , NULL );
    }
}