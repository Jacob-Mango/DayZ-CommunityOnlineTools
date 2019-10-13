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
		JMItemSetSpawnerModule iss = JMItemSetSpawnerModule.Cast( module );

		if ( iss == NULL ) return;

		string name;
		ref UIActionButton button;

		for ( int j = 0; j < iss.GetItemSets().Count(); j++ )
		{
			name = iss.GetItemSets()[j];
			
			button = UIActionManager.CreateButton( m_ActionsWrapper, "Spawn " + name + " on Selected Player(s)", this, "SpawnOnPlayers" );
			button.SetData( new JMItemSpawnerButtonData( name ) );
			m_ItemSetButtons.Insert( button );

			button = UIActionManager.CreateButton( m_ActionsWrapper, "Spawn " + name + " at Cursor", this, "SpawnOnCursor" );
			button.SetData( new JMItemSpawnerButtonData( name ) );
			m_ItemSetButtons.Insert( button );
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