class JMWindowBase extends ScriptedWidgetEventHandler  
{
	private static ref Widget S_SCREEN;

	private Widget layoutRoot;

	private ButtonWidget m_CloseButton;
	private Widget m_TitleWrapper;
	private TextWidget m_Title;

	private JMFormBase m_Form;
	private JMRenderableModuleBase m_Module;

	private float offsetX;
	private float offsetY;

	void JMWindowBase() 
	{
		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( OnUpdate );
	}

	void ~JMWindowBase() 
	{
		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( OnUpdate );
		layoutRoot.Unlink();
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}

	void Init()
	{
		m_CloseButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "close_button" ) );
		m_TitleWrapper = Widget.Cast( layoutRoot.FindAnyWidget( "title_bar_drag" ) );
		m_Title = TextWidget.Cast( layoutRoot.FindAnyWidget( "title_text" ) );
	}

	static JMWindowBase Create()
	{
		if ( S_SCREEN == NULL )
		{
			S_SCREEN = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen.layout", NULL );
		}

		JMWindowBase window;
		GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/windowbase.layout", S_SCREEN ).GetScript( window );
		return window;
	}

	void SetModule( JMRenderableModuleBase module )
	{
		m_Module = module;

		Widget content_ctr = layoutRoot.FindAnyWidget( "content" );

		Widget menu = GetGame().GetWorkspace().CreateWidgets( module.GetLayoutRoot(), content_ctr );

		float width = -1;
		float height = -1;
		menu.GetSize( width, height );

		content_ctr.SetSize( width, height );
		SetSize( width, height );

		menu.GetScript( m_Form );

		if ( m_Form )
		{
			m_Form.Init( this, module );

			m_Title.SetText( module.GetTitle() );
		}
	}

	JMFormBase GetForm()
	{
		return m_Form;
	}

	Widget GetLayoutRoot()
	{
		return layoutRoot;
	}

	bool IsVisible()
	{
		return layoutRoot.IsVisible();
	}

	void Show()
	{
		if ( !layoutRoot )
			return;

		layoutRoot.Show( true );

		m_Form.OnShow();
	}

	void Hide()
	{
		if ( !layoutRoot )
			return;

		layoutRoot.Show( false );

		m_Form.OnHide();
	}

	void OnUpdate( float timeslice )
	{
		if ( !m_TitleWrapper )
			return;

		m_TitleWrapper.SetPos( 0, 0, true );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		Widget parentWidget = w;
		while ( parentWidget != NULL )
		{
			if ( parentWidget == layoutRoot )
			{
				Print( "" + m_Form + " is FOCUS!" );
				break;
			}
			parentWidget = parentWidget.GetParent();
		}

		if ( super.OnClick( w, x, y, button ) )
		{
			return true;
		}

		if ( w == m_CloseButton )
		{
			m_Module.Hide();
			return true;
		}

		return false;
	}

	override bool OnDrag( Widget w, int x, int y )
	{
		if ( super.OnDrag( w, x, y ) )
		{
			return true;
		}

		if ( w == m_TitleWrapper )
		{
			layoutRoot.GetPos( offsetX, offsetY );

			offsetX = x - offsetX;
			offsetY = y - offsetY;

			m_TitleWrapper.SetPos( 0, 0, true );
			m_TitleWrapper.SetPos( 0, 0, false );
			
			return true;
		}

		return false;
	}

	override bool OnDragging( Widget w, int x, int y, Widget reciever )
	{
		if ( super.OnDragging( w, x, y, reciever ) )
		{
			return true;
		}

		if ( w == m_TitleWrapper )
		{
			SetPosition( x - offsetX, y - offsetY );

			return true;
		}

		return false;
	}

	override bool OnDrop( Widget w, int x, int y, Widget reciever )
	{
		if ( super.OnDrop( w, x, y, reciever ) )
		{
			return true;
		}

		if ( w == m_TitleWrapper )
		{
			SetPosition( x - offsetX, y - offsetY );
			
			return true;
		}

		return false ;
	}

	void SetSize( float w, float h )
	{
		float tw, th;
		m_TitleWrapper.GetSize( tw, th );

		layoutRoot.SetSize( w, h + th );
	}

	void SetPosition( int x, int y )
	{
		layoutRoot.SetPos( x, y, true );
		
		m_TitleWrapper.SetPos( 0, 0, true );
	}
}