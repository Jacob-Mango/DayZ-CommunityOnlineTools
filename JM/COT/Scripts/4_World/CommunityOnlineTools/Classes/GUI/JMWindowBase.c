#ifndef CF_WINDOWS
typedef JMWindowBase CF_Window;
class JMWindowBase extends ScriptedWidgetEventHandler  
{
	reference float m_DragXN;
	reference float m_DragYN;
	reference float m_DragXP;
	reference float m_DragYP;

	private ref Widget layoutRoot;

	private ButtonWidget m_CloseButton;
	private Widget m_TitleWrapper;
	private TextWidget m_TitleText;
	private Widget m_TitlePanel;
	private Widget m_Background;

	private Widget m_ResizeDragUp;
	private Widget m_ResizeDragDown;
	private Widget m_ResizeDragLeft;
	private Widget m_ResizeDragRight;

	private ref JMFormBase m_Form;
	private JMRenderableModuleBase m_Module;
	private JMConfirmationForm m_Confirmation;

	private autoptr array< Widget > m_BrokenWidgets;

	private float m_OffsetX;
	private float m_OffsetY;

	private int m_ResizeDirection;
	private float m_StartResizeSizeW;
	private float m_StartResizeSizeH;
	private int m_StartResizePositionX;
	private int m_StartResizePositionY;

	void JMWindowBase() 
	{
		GetCOTWindowManager().AddWindow( this );

		m_BrokenWidgets = new array< Widget >;
	}

	void ~JMWindowBase() 
	{
		#ifdef COT_DEBUGLOGS
		Print( "+JMWindowBase::~JMWindowBase" );
		#endif
		GetCOTWindowManager().RemoveWindow( this );

		Hide();

		//layoutRoot.Unlink();
		#ifdef COT_DEBUGLOGS
		Print( "-JMWindowBase::~JMWindowBase" );
		#endif
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

		m_ResizeDragUp = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_up" ) );
		m_ResizeDragDown = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_down" ) );
		m_ResizeDragLeft = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_left" ) );
		m_ResizeDragRight = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_right" ) );
	}

	void SetModule( JMRenderableModuleBase module )
	{
		m_Module = module;
		if ( Assert_Null( m_Module, "No valid RenderableModule supplied." ) )
			return; 

		Widget content_ctr = layoutRoot.FindAnyWidget( "content" );
		Widget menu = content_ctr;

		if ( m_Module.GetLayoutRoot() != "" )
		{
			menu = GetGame().GetWorkspace().CreateWidgets( m_Module.GetLayoutRoot(), content_ctr );
			if ( Assert_Null( menu, "No valid widget supplied." ) )
				return; 

			float width = -1;
			float height = -1;
			menu.GetSize( width, height );

			content_ctr.SetSize( width, height );
			SetSize( width, height );

			menu.GetScript( m_Form );
		}

		if ( !m_Form )
			m_Form = m_Module.InitForm( menu );
		
		if ( Assert_Null( m_Form, "No valid Form supplied." ) )
			return; 

		m_Form.Init( this, m_Module );
		m_TitleText.SetText( m_Module.GetTitle() );
		GetCOTWindowManager().BringFront( this );

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

	Widget GetWidgetRoot()
	{
		return layoutRoot;
	}

	JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne )
	{
		if ( !m_Confirmation )
			return NULL;

		m_Confirmation.CreateConfirmation_One( type, title, message, callBackOneName, callBackOne );

		return m_Confirmation;
	}

	JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo )
	{
		if ( !m_Confirmation )
			return NULL;

		m_Confirmation.CreateConfirmation_Two( type, title, message, callBackOneName, callBackTwoName, callBackOne, callBackTwo );

		return m_Confirmation;
	}

	JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree )
	{
		if ( !m_Confirmation )
			return NULL;

		m_Confirmation.CreateConfirmation_Three( type, title, message, callBackOneName, callBackTwoName, callBackThreeName, callBackOne, callBackTwo, callBackThree );

		return m_Confirmation;
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
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Show" );
		#endif

		if ( !layoutRoot || layoutRoot.IsVisible() )
			return;

		layoutRoot.Show( true );

		m_Form.OnShow();

		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( Update );

		GetGame().GetInput().ChangeGameFocus( 1 );
		GetGame().GetUIManager().ShowUICursor( true );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Show" );
		#endif
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Hide" );
		#endif
		
		if ( Assert_Null( layoutRoot ) )
			return;

		if ( Assert_Null( m_Form ) )
			return;

		if ( Assert_Null( this ) )
			return;

		layoutRoot.Show( false );

		m_Form.OnHide();

		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( Update );

		if ( !GetCommunityOnlineToolsBase().IsOpen() && GetCOTWindowManager().Count() == 0 )
		{
			GetGame().GetInput().ResetGameFocus();
			GetGame().GetUIManager().ShowUICursor( false );
		}

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Hide" );
		#endif
	}

	void Focus()
	{
		SetBackgroundColour( 0.9995, 0.06, 0.08, 0.11 );
		SetTitleColour( 1.0, 0.04, 0.04, 0.12 );

		//foreach ( Widget widget : m_BrokenWidgets )
		//{
		//	widget.Show( true );
		//}

		m_Form.OnFocus();
	}

	void Unfocus()
	{
		SetBackgroundColour( 0.95, 0.042, 0.056, 0.077 );
		SetTitleColour( 1.0, 0.02, 0.02, 0.06 );

		//foreach ( Widget widget : m_BrokenWidgets )
		//{
		//	widget.Show( false );
		//	if ( widget.GetChildren() )
		//	{
		//		widget.GetChildren().Show( true );
		//	}
		//}

		m_Form.OnUnfocus();
	}

	void OnFormLoaded()
	{
		if ( !m_Form || !m_Form.GetLayoutRoot() )
			return;
/*
		Widget w = m_Form.GetLayoutRoot();
		while ( w != NULL )
		{
			Widget c = NULL;
			if ( w.GetTypeName() == "ScrollWidget" )
			{
				m_BrokenWidgets.Insert( w ); 
			} else if ( w.GetTypeName() == "ItemPreviewWidget" )
			{
				m_BrokenWidgets.Insert( w );
			} else if ( w.GetTypeName() == "PlayerPreviewWidget" )
			{
				m_BrokenWidgets.Insert( w );
			} else if ( w.GetTypeName() == "HtmlWidget" )
			{
				m_BrokenWidgets.Insert( w );
			} else if ( w.GetTypeName() == "MapWidget" )
			{
				m_BrokenWidgets.Insert( w );
			} else
			{
				c = w.GetChildren();
				if ( c != NULL )
				{
					w = c;
					continue;
				}
			}

			c = w.GetSibling();
			while ( c == NULL )
			{
				c = w.GetParent();
				c = c.GetSibling();
			}
			
			if ( w == m_Form.GetLayoutRoot() )
				return;

			w = c;
		}
*/
	}

	void Update( float timeSlice )
	{
		if ( m_Form )
			m_Form.Update();

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
		if ( w == m_CloseButton )
		{
			m_Module.Hide();
			return true;
		}

		return false;
	}

	override bool OnDrag( Widget w, int x, int y )
	{
		if ( w == m_TitleWrapper )
		{
			layoutRoot.GetPos( m_OffsetX, m_OffsetY );

			m_OffsetX = x - m_OffsetX;
			m_OffsetY = y - m_OffsetY;

			m_TitleWrapper.SetPos( 0, 0, true );
			m_TitleWrapper.SetPos( 0, 0, false );
			
			return true;
		}

		if ( w == m_ResizeDragUp || w == m_ResizeDragDown || w == m_ResizeDragLeft || w == m_ResizeDragRight )
		{
			w.SetPos( 0, 0, true );
			w.SetPos( 0, 0, false );

			if ( w == m_ResizeDragUp ) 
			{
				m_ResizeDirection = 0;
			} else if ( w == m_ResizeDragDown ) 
			{
				m_ResizeDirection = 1;
			} else if ( w == m_ResizeDragLeft ) 
			{
				m_ResizeDirection = 2;
			} else if ( w == m_ResizeDragRight )
			{
				m_ResizeDirection = 3;
			}

			m_StartResizePositionX = x;
			m_StartResizePositionY = y;

			layoutRoot.GetSize( m_StartResizeSizeW, m_StartResizeSizeH );

			return true;
		}
		
		return false;
	}

	override bool OnDragging( Widget w, int x, int y, Widget reciever )
	{
		if ( w == m_TitleWrapper )
		{
			SetPosition( x - m_OffsetX, y - m_OffsetY );

			return true;
		}

		if ( w == m_ResizeDragUp || w == m_ResizeDragDown || w == m_ResizeDragLeft || w == m_ResizeDragRight )
		{
			Resize( w, x, y );

			return true;
		}

		return false;
	}

	override bool OnDrop( Widget w, int x, int y, Widget reciever )
	{
		if ( w == m_TitleWrapper )
		{
			SetPosition( x - m_OffsetX, y - m_OffsetY );
			
			return true;
		}

		if ( w == m_ResizeDragUp || w == m_ResizeDragDown || w == m_ResizeDragLeft || w == m_ResizeDragRight )
		{
			Resize( w, x, y );
			
			return true;
		}

		return false;
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

	private void Resize( Widget w, int x, int y )
	{
		w.SetPos( 0, 0, true );

		float newWidth = m_StartResizeSizeW;
		float newHeight = m_StartResizeSizeH;

		switch ( m_ResizeDirection )
		{
			case 0:
				newHeight += m_StartResizePositionX - x;
				break;
			case 1:
				newHeight += m_StartResizePositionX - x;
				break;
			case 2:
				newWidth += m_StartResizePositionY - y;
				break;
			case 3:
				newWidth += m_StartResizePositionY - y;
				break;
		}

		// layoutRoot.SetSize( newWidth, newHeight );
	}
};
#endif