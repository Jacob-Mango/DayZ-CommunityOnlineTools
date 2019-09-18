class JMWindowBase extends UIScriptedMenu  
{
	private ButtonWidget m_CloseButton;
	private Widget m_TitleWrapper;
	private TextWidget m_Title;

	private JMFormBase m_Form;

	private float offsetX;
	private float offsetY;

	void JMWindowBase() 
	{
	}

	void ~JMWindowBase() 
	{
	}

	Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/windowbase.layout", NULL );

		m_CloseButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "close_button" ) );
		m_TitleWrapper = Widget.Cast( layoutRoot.FindAnyWidget( "title_bar_drag" ) );

		m_Title = TextWidget.Cast( GetLayoutRoot().FindAnyWidget( "title_text" ) );
	}

	void SetModule( JMRenderableModuleBase module )
	{
		Widget menu = GetGame().GetWorkspace().CreateWidgets( module.GetLayoutRoot(), GetLayoutRoot().FindAnyWidget( "content" ) );

		float width = -1;
		float height = -1;
		menu.GetSize( width, height );
		SetSize( width, height );

		menu.GetScript( m_Form );

		if ( m_Form )
		{
			m_Form.Init( this, module );

			m_Title.SetText( m_Form.GetTitle() );
		}
	}

	JMFormBase GetForm()
	{
		return m_Form;
	}

	override void OnShow()
	{
		super.OnShow();
	}

	override void OnHide() 
	{
		super.OnHide();
	}

	override void Update( float timeslice )
	{
		super.Update( timeslice );

		m_TitleWrapper.SetPos( 0, 0, true );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( super.OnClick( w, x, y, button ) )
		{
			return true;
		}

		if ( w == m_CloseButton )
		{
			m_Form.Hide();
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