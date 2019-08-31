class JMSelectedWidget extends ScriptedWidgetEventHandler 
{
	protected Widget layoutRoot;

	protected TextWidget m_WName;

    protected string m_Name;

    protected Object m_Object;

    protected JMSelectedForm m_Menu;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
	}

	void Init( JMSelectedForm menu ) 
	{
        m_Menu = menu;

		m_WName = TextWidget.Cast( layoutRoot.FindAnyWidget( "obj_sel_name" ) );
	}

	void Show()
	{
		layoutRoot.Show( true );
		m_WName.Show( true );

		OnShow();
	}

	void Hide()
	{
		OnHide();

		m_WName.Show( false );
		layoutRoot.Show( false );
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	void Update() 
	{
		
	}

	ref Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	void SetObject( Object obj )
	{
		m_Object = obj;
		
		if ( m_Object == NULL ) 
		{
			Hide();
		} else 
		{
			Show();
            
            m_Name = m_Object.GetDisplayName();
			if ( m_Name == "" )
				m_Name = m_Object.GetType();

			m_WName.SetText( m_Name );
			m_WName.SetColor( 0xFFFFFFFF );
		}
	}

	string GetName()
	{
		return m_Name;
	}

	Object GetObject()
	{
		return m_Object;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		return false;
	}
}