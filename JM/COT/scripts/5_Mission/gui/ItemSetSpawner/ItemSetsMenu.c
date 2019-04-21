class ItemSetsMenu extends Form
{
	protected Widget m_ActionsWrapper;

	protected ref array< ref UIActionButton > m_ItemSetButtons;

	void ItemSetsMenu()
	{
		m_ItemSetButtons = new ref array< ref UIActionButton >;
	}

	void ~ItemSetsMenu()
	{
	}

	override string GetTitle()
	{
		return "Item Sets Spawner";
	}
	
	override string GetIconName()
	{
		return "IS";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnInit( bool fromMenu )
	{
		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );
	}

	override void OnShow()
	{
		ItemSetSpawnerModule iss = ItemSetSpawnerModule.Cast( module );

		if ( iss == NULL ) return;

		string name;
		ref UIActionButton button;

		for ( int j = 0; j < iss.GetItemSets().Count(); j++ )
		{
			name = iss.GetItemSets()[j];

			button = UIActionManager.CreateButton( m_ActionsWrapper, "Spawn " + name + " on Selected Player(s)", this, "SpawnBaseBuilding" );
			button.SetData( new IS_Button_Data( name ) );

			m_ItemSetButtons.Insert( button );
		}		
	}

	override void OnHide() 
	{
		for ( int k = 0; k < m_ItemSetButtons.Count(); k++ )
		{
			m_ActionsWrapper.RemoveChild( m_ItemSetButtons[k].GetLayoutRoot() );
			m_ItemSetButtons[k].GetLayoutRoot().Unlink();
		}

		m_ItemSetButtons.Clear();
	}

	void SpawnBaseBuilding( UIEvent eid, ref UIActionButton action ) 
	{
		ref IS_Button_Data data = IS_Button_Data.Cast( action.GetData() );
		if ( !data ) return;

		GetRPCManager().SendRPC( "COT_ItemSetSpawner", "SpawnSelectedPlayers", new Param2< string, ref array< string > >( data.ClassName, SerializePlayersID( GetSelectedPlayers() ) ) );
	}
}