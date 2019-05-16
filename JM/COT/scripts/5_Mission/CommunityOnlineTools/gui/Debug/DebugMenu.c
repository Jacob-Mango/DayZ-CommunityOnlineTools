class DebugMenu extends Form
{
	protected Widget m_ActionsWrapper;

	protected autoptr map<string, UIActionCheckbox> m_CheckBoxes;

	void DebugMenu()
	{
		m_CheckBoxes = new map<string, UIActionCheckbox>;
	}

	void ~DebugMenu()
	{
	}

	override string GetTitle()
	{
		return "Debug Menu";
	}
	
	override string GetIconName()
	{
		return "D";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnInit( bool fromMenu )
	{
		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

		ref Widget settings = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 2, 1 );
		
		int debugCount = GetDebugging().Type().GetVariableCount();
		
		for ( int i = 0; i < debugCount; i++)
		{
			string name = GetDebugging().Type().GetVariableName( i );
			if ( name.Contains( "FileName" ) )
				continue;
			
			m_CheckBoxes.Insert( name, UIActionManager.CreateCheckbox( settings, name ) );
		}

		if ( GetPermissionsManager().HasPermission( "COT.Apply" ) )
		{
			UIActionManager.CreateButton( settings, "Apply Settings", this, "Click_Set" );
		}
	}

	override void OnShow()
	{
		super.OnShow();

		for ( int i = 0; i < m_CheckBoxes.Count(); i++)
		{
			bool result = false;
			
			EnScript.GetClassVar( GetDebugging(), m_CheckBoxes.GetKey( i ), 0, result );
			
			m_CheckBoxes.GetElement( i ).SetChecked( result );
		}
	}

	void Click_Set( UIEvent eid, ref UIActionButton action )
	{
		for ( int i = 0; i < m_CheckBoxes.Count(); i++)
		{
			EnScript.SetClassVar( GetDebugging(), m_CheckBoxes.GetKey( i ), 0, m_CheckBoxes.GetElement( i ).IsChecked() );
		}

		//GetRPCManager().SendRPC( "COT_Manager", "LoadData", new Param1< string >( JsonFileLoader< DebugSettings >.JsonMakeData( settings ) ), false );
	}

	override void OnHide()
	{
		super.OnHide();
	}
}