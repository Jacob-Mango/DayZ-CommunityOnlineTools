class JMWindowBase extends ScriptedWidgetEventHandler  
{
	private Widget layoutRoot;

	private ButtonWidget m_CloseButton;
	private Widget m_TitleWrapper;
	private TextWidget m_TitleText;
	private Widget m_TitlePanel;
	private Widget m_Background;

	private JMFormBase m_Form;
	private JMRenderableModuleBase m_Module;
	private JMConfirmation m_Confirmation;

	private float offsetX;
	private float offsetY;

	void JMWindowBase() 
	{
        GetCOTWindowManager().AddWindow( this );
	}

	void ~JMWindowBase() 
	{
        GetCOTWindowManager().RemoveWindow( this );

		Hide();

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
		m_TitlePanel = layoutRoot.FindAnyWidget( "title_wrapper" );
		m_TitleText = TextWidget.Cast( layoutRoot.FindAnyWidget( "title_text" ) );
		m_Background = Widget.Cast( layoutRoot.FindAnyWidget( "background" ) );
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

			m_TitleText.SetText( module.GetTitle() );
			
        	GetCOTWindowManager().BringFront( this );
		}

		layoutRoot.FindAnyWidget( "confirmation_panel" ).GetScript( m_Confirmation );

		if ( m_Confirmation )
		{
			m_Confirmation.Init( this );
		}
	}

	JMRenderableModuleBase GetModule()
	{
		return m_Module;
	}

	JMFormBase GetForm()
	{
		return m_Form;
	}

	Widget GetLayoutRoot()
	{
		return layoutRoot;
	}

	void CreateConfirmation_One( string title, string message, string callBackOneName, string callBackOne )
	{
		if ( !m_Confirmation )
			return;

		m_Confirmation.CreateConfirmation_One( title, message, callBackOneName, callBackOne );
	}

	void CreateConfirmation_Two( string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo )
	{
		if ( !m_Confirmation )
			return;

		m_Confirmation.CreateConfirmation_Two( title, message, callBackOneName, callBackTwoName, callBackOne, callBackTwo );
	}

	void CreateConfirmation_Three( string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree )
	{
		if ( !m_Confirmation )
			return;

		m_Confirmation.CreateConfirmation_Three( title, message, callBackOneName, callBackTwoName, callBackThreeName, callBackOne, callBackTwo, callBackThree );
	}

	void SetTitleColour( float alpha, float r, float g, float b )
	{
		m_TitlePanel.SetColor( ARGB( alpha * 255, r * 255, g * 255, b * 255 ) );
	}

	void SetBackgroundColour( float alpha, float r, float g, float b )
	{
		m_Background.SetColor( ARGB( alpha * 255, r * 255, g * 255, b * 255 ) );
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

		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( Update );

		GetGame().GetInput().ChangeGameFocus( 1 );
		GetGame().GetUIManager().ShowUICursor( true );

		GetDayZGame().COTPreventMenuOpen = true;
	}

	void Hide()
	{
		if ( !layoutRoot )
			return;

		layoutRoot.Show( false );

		m_Form.OnHide();

		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( Update );

		if ( !GetCommunityOnlineToolsBase().IsOpen() && GetCOTWindowManager().Count() == 0 )
		{
			GetGame().GetInput().ResetGameFocus();
			GetGame().GetUIManager().ShowUICursor( false );
			
			GetDayZGame().COTPreventMenuOpen = false;
		}
	}

	void Update( float timeSlice )
	{
		if ( !m_TitleWrapper )
			return;

		m_TitleWrapper.SetPos( 0, 0, true );
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		Widget parentWidget = w;
		while ( parentWidget != NULL )
		{
			if ( parentWidget == layoutRoot )
			{
				GetCOTWindowManager().BringFront( this );
				break;
			}
			parentWidget = parentWidget.GetParent();
		}

		return super.OnMouseButtonDown( w, x, y, button );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{

		if ( super.OnClick( w, x, y, button ) )
		{
		//	return true;
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
		//	return true;
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
		//	return true;
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
		//	return true;
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