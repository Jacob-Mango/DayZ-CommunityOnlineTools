class JMDebugForm extends JMFormBase
{
	protected Widget m_ActionsWrapper;

	protected autoptr map<string, UIActionCheckbox> m_CheckBoxes;

	void JMDebugForm()
	{
		m_CheckBoxes = new map<string, UIActionCheckbox>;
	}

	void ~JMDebugForm()
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

		UIActionManager.CreateButton( m_ActionsWrapper, "Apply Settings", this, "OnClick_Apply" );

		m_CheckBoxes.Insert( "EnableDebug", UIActionManager.CreateCheckbox( m_ActionsWrapper, "EnableDebug" ) );

		for ( int i = 0; i < GetLogger().Modes.Count(); i++ )
		{
			string name = GetLogger().Modes.GetKey( i );
			
			m_CheckBoxes.Insert( name, UIActionManager.CreateCheckbox( m_ActionsWrapper, name ) );
		}
	}

	override void OnShow()
	{
		super.OnShow();

		m_CheckBoxes.Get( "EnableDebug" ).SetChecked( GetLogger().EnableDebug );

		for ( int i = 0; i < m_CheckBoxes.Count(); i++)
		{
			if ( m_CheckBoxes.GetKey( i ).Contains( "EnableDebug" ) )
				continue;

			m_CheckBoxes.GetElement( i ).SetChecked( GetLogger().Modes.Get( m_CheckBoxes.GetKey( i ) ) );
		}
	}

	void OnClick_Apply( UIEvent eid, ref UIActionButton action )
	{
		GetLogger().EnableDebug = m_CheckBoxes.Get( "EnableDebug" ).IsChecked();

		for ( int i = 0; i < m_CheckBoxes.Count(); i++)
		{
			if ( m_CheckBoxes.GetKey( i ).Contains( "EnableDebug" ) )
				continue;

			GetLogger().Modes.Set( m_CheckBoxes.GetKey( i ), m_CheckBoxes.GetElement( i ).IsChecked() );
		}

		GetRPCManager().SendRPC( "COT_Manager", "LoadData", new Param1< ref CFLogger >( GetLogger() ), false );
	}

	override void OnHide()
	{
		super.OnHide();
	}
}