class JMSelectedForm extends JMFormBase
{
	protected Widget m_ActionsWrapper;

	protected Widget m_WObjectList;
	protected TextWidget m_WObjectCount;

	protected autoptr array< GridSpacerWidget > m_ObjectListContainers;

	protected int m_TotalNumObjects;
	protected autoptr array< JMSelectedWidget > m_SelectedWidgets;

	void JMSelectedForm()
	{
		m_ObjectListContainers = new array< GridSpacerWidget >;

		m_SelectedWidgets = new array< JMSelectedWidget >;
	}

	void ~JMSelectedForm()
	{
	}

	override string GetTitle()
	{
		return "Selected Objects";
	}
	
	override string GetIconName()
	{
		return "S";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnInit( bool fromMenu )
	{	
		m_WObjectList		= layoutRoot.FindAnyWidget( "object_selected_list_wrapper" );
		m_WObjectCount		= TextWidget.Cast( layoutRoot.FindAnyWidget( "object_selected_list_count" ) );

		ref Widget rwWidget = NULL;
		ref JMSelectedWidget rwScript = NULL;

		int numSpacers = 10;
		int numRows = 100;

		for ( int i = 0; i < numSpacers; i++ )
		{
			GridSpacerWidget gsw = GridSpacerWidget.Cast( m_WObjectList.FindAnyWidget( "object_selected_list_0" + i ) );
			
			if ( gsw )
			{
				m_ObjectListContainers.Insert( gsw );

				for ( int j = 0; j < numRows; j++ )
				{
					rwWidget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/selected_widget.layout", gsw );
					
					if ( rwWidget == NULL )
					{
						continue;
					}

					rwWidget.GetScript( rwScript );

					if ( rwScript == NULL )
					{
						continue;
					}

					rwScript.Init( this );

					rwScript.SetObject( NULL );

					m_SelectedWidgets.Insert( rwScript );
				}
			}
		}

		m_TotalNumObjects = numSpacers * numRows;

		m_ActionsWrapper = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 4, 1 );

		UIActionManager.CreateButton( m_ActionsWrapper, "Delete All", this, "Click_Delete" )
	}

	override void OnShow()
	{
		super.OnShow();

		JMSelectedModule smod;
		if ( Class.CastTo( smod, module ) )
		{
			RefreshSelectedList( smod.GetObjects() );
		}
	}

	void RefreshSelectedList( set< Object > objects )
	{
		for ( int i = 0; i < m_SelectedWidgets.Count(); i++ )
		{
			if ( i < objects.Count() )
			{
				if ( objects[i] == NULL )
				{
					Print( "Removing selected because NULL" );
					objects.Remove( i );
					i--;
					continue;
				}

				m_SelectedWidgets[i].SetObject( objects[i] );
			} else
			{
				m_SelectedWidgets[i].SetObject( NULL );
			}
		}

		m_WObjectCount.SetText( "" + objects.Count() + " Selected" );
	}

	void Click_Delete( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMSelectedModule smod;
		if ( Class.CastTo( smod, module ) )
		{
			smod.Delete();
		}
	}
}