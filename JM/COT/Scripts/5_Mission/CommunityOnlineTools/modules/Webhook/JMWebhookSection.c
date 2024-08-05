class JMWebhookTypeData : UIActionData
{
	string Name;

	void JMWebhookTypeData( string name )
	{
		Name = name;
	}
};

class JMWebhookSection : Managed
{
	private UIActionText m_URL;
	private UIActionButton m_RemoveSectionButton;

	private UIActionScroller m_ScrollerActiveTypes;
	private Widget m_ActionsWrapper;

	private UIActionDropdownList m_DropDownList;
	private UIActionButton m_AddButton;

	private ref array< UIActionCheckbox > m_Types;

	private string m_Name;
	private string m_ContextURL;
	private string m_Address;

	private JMWebhookForm m_Form;
	private Widget m_Parent;

	void JMWebhookSection( Widget parent, JMWebhookForm form, JMWebhookConnectionGroup serialize, array< string > types )
	{
		m_Types = new array< UIActionCheckbox >;

		m_Form = form;
		m_Parent = parent;

		m_Name = serialize.Name;
		m_ContextURL = serialize.ContextURL;
		m_Address = serialize.Address;

		Init( serialize, types );
	}

	void Init( JMWebhookConnectionGroup serialize, array< string > types )
	{
		Widget spacer = UIActionManager.CreateGridSpacer( m_Parent, 2, 1 );

		m_URL = UIActionManager.CreateText( spacer, "URL:", m_ContextURL + m_Address );
		m_RemoveSectionButton = UIActionManager.CreateButton( spacer, "Remove Webhook", m_Form, "Action_RemoveWebhook" );
		m_RemoveSectionButton.SetData( new JMWebhookTypeData( serialize.Name ) );

		m_ScrollerActiveTypes = UIActionManager.CreateScroller( m_Parent );
		m_ScrollerActiveTypes.SetFixedHeight( 195 );
		m_ActionsWrapper = m_ScrollerActiveTypes.GetContentWidget();

		Widget dropDown = UIActionManager.CreateGridSpacer( m_Parent, 2, 1 );

		array< string > empty = new array< string >;
		m_DropDownList = UIActionManager.CreateDropdownBox( dropDown, m_Form.GetLayoutRoot(), "Types", empty );
		m_AddButton = UIActionManager.CreateButton( dropDown, "Add Type", m_Form, "Action_AddType" );

		UIActionManager.CreatePanel( m_Parent, 0xFF000000, 3 );
	}

	void UpdateState( JMWebhookConnectionGroup serialize, array< string > types )
	{
		m_Types.Clear();

		int r = 0;
		Widget parentTypeWidget = NULL;
		for ( int i = 0; i < serialize.Count(); ++i )
		{
			if ( m_Types.Count() % 100 == 0 )
			{
				parentTypeWidget = m_ActionsWrapper; // .FindAnyWidget( "Content_Row_0" + r );
				r++;
			}

			string name = serialize.Get( i ).Name;

			UIActionCheckbox checkbox = UIActionManager.CreateCheckbox( parentTypeWidget, name, m_Form, "Click_UpdateState", serialize.Get( i ).Enabled );
			checkbox.SetData( new JMWebhookTypeData( name ) );
			m_Types.Insert( checkbox );
		}

		array< string > showTypes = new array< string >;
		for ( i = 0; i < types.Count(); ++i )
		{
			if ( !serialize.Contains( types[i] ) )
				showTypes.Insert( types[i] );
		}

		m_DropDownList.SetItems( showTypes );
	}
};