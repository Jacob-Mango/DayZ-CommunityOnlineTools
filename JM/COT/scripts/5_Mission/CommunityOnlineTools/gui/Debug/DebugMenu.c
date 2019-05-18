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

		int numOfModes = GetDebugging().Modes.Count() + 2;

		int numberOfPanels = Math.Ceil( numOfModes / 9.0 );

		ref Widget panel = UIActionManager.CreateGridSpacer( m_ActionsWrapper, numberOfPanels, 1 );

		array< ref Widget > settings = new array< ref Widget >;

		for ( int j = 0; j < numberOfPanels - 1; j++ )
		{
			settings.Insert( UIActionManager.CreateGridSpacer( panel, 9, 1 ) );
		}

		settings.Insert( UIActionManager.CreateGridSpacer( panel, numOfModes - ( ( numberOfPanels - 1 ) * 9 ), 1 ) );

		int parent = 0;

		m_CheckBoxes.Insert( "EnableDebug", UIActionManager.CreateCheckbox( settings[parent], "EnableDebug" ) );

		for ( int i = 0; i < numOfModes - 2; i++ )
		{
			parent = Math.Floor( i / 9.0 );

			string name = GetDebugging().Modes.GetKey( i );
			
			m_CheckBoxes.Insert( name, UIActionManager.CreateCheckbox( settings[parent], name ) );
		}

		UIActionManager.CreateButton( settings[parent], "Apply Settings", this, "OnClick_Apply" );
	}

	override void OnShow()
	{
		super.OnShow();

		m_CheckBoxes.Get( "EnableDebug" ).SetChecked( GetDebugging().EnableDebug );

		for ( int i = 0; i < m_CheckBoxes.Count(); i++)
		{
			if ( m_CheckBoxes.GetKey( i ).Contains( "EnableDebug" ) )
				continue;

			m_CheckBoxes.GetElement( i ).SetChecked( GetDebugging().Modes.Get( m_CheckBoxes.GetKey( i ) ) );
		}
	}

	void OnClick_Apply( UIEvent eid, ref UIActionButton action )
	{
		GetDebugging().EnableDebug = m_CheckBoxes.Get( "EnableDebug" ).IsChecked();

		for ( int i = 0; i < m_CheckBoxes.Count(); i++)
		{
			if ( m_CheckBoxes.GetKey( i ).Contains( "EnableDebug" ) )
				continue;

			GetDebugging().Modes.Set( m_CheckBoxes.GetKey( i ), m_CheckBoxes.GetElement( i ).IsChecked() );
		}

		GetRPCManager().SendRPC( "COT_Manager", "LoadData", new Param1< ref Debugging >( GetDebugging() ), false );
	}

	override void OnHide()
	{
		super.OnHide();
	}
}