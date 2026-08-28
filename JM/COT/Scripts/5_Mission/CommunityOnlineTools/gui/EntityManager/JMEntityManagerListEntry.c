// =============================================================================
//  JMEntityManagerListEntry
//
//  Generic left-panel list row for an entity. Click the row to open its info
//  panel on the right.
// =============================================================================
class JMEntityManagerListEntry: ScriptedWidgetEventHandler
{
	protected Widget       m_Root;
	protected TextWidget   m_Name;
	protected TextWidget   m_Id;
	protected int          m_Sort;
	protected string       m_Label;
	protected ButtonWidget m_EditButton;
	protected ImageWidget  m_StatusIcon;

	protected ref JMEntityMetaData m_Entity;
	protected ref JMEntityManagerForm m_Form;

	void JMEntityManagerListEntry( Widget parent, JMEntityManagerForm form, JMEntityMetaData entity )
	{
		m_Root       = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/entitymanager/EntityManager_List_Entry.layout", parent );
		m_Name       = TextWidget.Cast( m_Root.FindAnyWidget( "entity_name" ) );
		m_Id         = TextWidget.Cast( m_Root.FindAnyWidget( "entity_id" ) );
		m_EditButton = ButtonWidget.Cast( m_Root.FindAnyWidget( "edit_button" ) );
		m_StatusIcon = ImageWidget.Cast( m_Root.FindAnyWidget( "entity_status_icon" ) );

		m_Form   = form;
		m_Entity = entity;

		m_Root.SetHandler( this );

		SetEntry();
	}

	void ~JMEntityManagerListEntry()
	{
		if ( g_Game && m_Root )
			m_Root.Unlink();
	}

	void SetSort( int sort, bool immedUpdate = true )
	{
		m_Sort = sort;
		m_Root.SetSort( sort, immedUpdate );
		SetEntry();
	}

	string GetLabel()
	{
		return m_Label;
	}

	JMEntityMetaData GetEntity()
	{
		return m_Entity;
	}

	void SetEntry()
	{
		if ( !m_Entity )
			return;

		m_Name.SetText( (m_Sort + 1).ToString() + " - " + m_Entity.m_DisplayName );

		string id = "";
		if ( m_Entity.m_Id != "" )
			id = " - " + m_Entity.m_Id;
		else if ( m_Entity.m_NetworkIDHigh != 0 || m_Entity.m_NetworkIDLow != 0 )
			id = " - ID: " + m_Entity.m_NetworkIDHigh.ToString() + " " + m_Entity.m_NetworkIDLow.ToString();
		m_Id.SetText( id );

		m_Label = m_Entity.m_DisplayName + id;

		if ( m_StatusIcon )
		{
			int color = m_Entity.m_MarkerColor;
			if ( color == 0 )
				color = ARGB( 255, 0, 255, 0 );
			m_StatusIcon.SetColor( color );
		}
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( m_EditButton && w == m_EditButton && m_Form && m_Entity )
		{
			m_Form.SetEntityInfo( m_Entity );
			return true;
		}
		return false;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( m_EditButton && w == m_EditButton )
		{
			m_Name.SetColor( ARGB( 255, 0, 0, 0 ) );
			m_Id.SetColor( ARGB( 255, 0, 0, 0 ) );
		}
		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( m_EditButton && w == m_EditButton )
		{
			m_Name.SetColor( ARGB( 255, 255, 255, 255 ) );
			m_Id.SetColor( ARGB( 255, 255, 255, 255 ) );
		}
		return false;
	}
}
