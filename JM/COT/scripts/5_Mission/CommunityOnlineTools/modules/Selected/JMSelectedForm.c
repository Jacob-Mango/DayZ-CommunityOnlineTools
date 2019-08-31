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
		m_ActionsWrapper    = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 4, 1 );
	
		m_WObjectList		= layoutRoot.FindAnyWidget( "object_list_wrapper" );
		m_WObjectCount		= TextWidget.Cast( layoutRoot.FindAnyWidget( "object_count" ) );

		ref Widget rwWidget = NULL;
		ref JMSelectedWidget rwScript = NULL;

		for ( int i = 0; i < 10; i++ )
		{
			GridSpacerWidget gsw = GridSpacerWidget.Cast( m_WObjectList.FindAnyWidget( "object_list_0" + i ) );
			
			if ( gsw )
			{
				m_ObjectListContainers.Insert( gsw );

				for ( int j = 0; j < 100; j++ )
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
	}

	void OnAddObject( Object obj )
	{
		for ( int i = 0; i < m_SelectedWidgets.Count(); i++ )
		{
			if ( m_SelectedWidgets[i].GetObject() == NULL )
			{
				m_SelectedWidgets[i].SetObject( obj );
				break;
			}
		}
	}

	void OnRemoveObject( Object obj )
	{
		for ( int i = 0; i < m_SelectedWidgets.Count(); i++ )
		{
			if ( m_SelectedWidgets[i].GetObject() == obj )
			{
				m_SelectedWidgets[i].SetObject( NULL );
				break;
			}
		}
	}

	void OnClear()
	{
		for ( int i = 0; i < m_SelectedWidgets.Count(); i++ )
		{
			m_SelectedWidgets[i].SetObject( NULL );
		}
	}
}