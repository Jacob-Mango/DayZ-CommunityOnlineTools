class JMItemSetForm extends JMFormBase
{
	protected Widget m_ActionsWrapper;

	protected ref array< ref UIActionButton > m_ItemSetButtons;

	void JMItemSetForm()
	{
		m_ItemSetButtons = new array< ref UIActionButton >;
	}

	void ~JMItemSetForm()
	{
	}

	override void OnInit()
	{
		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );
	}

	override void OnShow()
	{
		JMItemSetSpawnerModule iss;
		if ( !Class.CastTo( iss, module ) )
			return;

		for ( int j = 0; j < iss.GetItemSets().Count(); j++ )
		{
			Widget wrapper = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 3 );
			string name = iss.GetItemSets()[j];

			UIActionManager.CreateText( wrapper, name );
			
			UIActionButton selButton = UIActionManager.CreateButton( wrapper, "Selected Player(s)", this, "SpawnOnPlayers" );
			selButton.SetData( new JMItemSpawnerButtonData( name ) );
			m_ItemSetButtons.Insert( selButton );

			UIActionButton curButton = UIActionManager.CreateButton( wrapper, "Cursor", this, "SpawnOnCursor" );
			curButton.SetData( new JMItemSpawnerButtonData( name ) );
			m_ItemSetButtons.Insert( curButton );
		}
	}

	override void OnHide() 
	{
		for ( int k = 0; k < m_ItemSetButtons.Count(); k++ )
		{
			if ( m_ItemSetButtons[k].GetLayoutRoot() )
			{
				m_ActionsWrapper.RemoveChild( m_ItemSetButtons[k].GetLayoutRoot() );
			}
		}

		m_ItemSetButtons.Clear();
	}

	void SpawnOnPlayers( UIEvent eid, ref UIActionBase action ) 
	{
		JMItemSpawnerButtonData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		JMItemSetSpawnerModule mod;
		if ( !Class.CastTo( mod, module ) )
			return;

		mod.SpawnPlayers( data.ClassName, GetSelectedPlayers() );
	}

	void SpawnOnCursor( UIEvent eid, ref UIActionBase action ) 
	{
		JMItemSpawnerButtonData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		JMItemSetSpawnerModule mod;
		if ( !Class.CastTo( mod, module ) )
			return;

		mod.SpawnPosition( data.ClassName, GetCursorPos() );
	}
}