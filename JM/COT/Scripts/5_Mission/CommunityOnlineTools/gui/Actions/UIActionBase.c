class UIActionBase extends ScriptedWidgetEventHandler 
{
	protected Widget layoutRoot;

	protected Widget m_Disable;

	protected Class m_Instance;
	protected string m_FuncName;

	protected bool m_HasCallback;

	protected bool m_WasFocused;

	protected bool m_LeftMouseDown;

	protected ref UIActionData m_Data;

	void ~UIActionBase()
	{
		delete m_Data;
	}

	void GetUserData( out Class data )
	{
		layoutRoot.GetUserData( data );
	}

	void SetUserData( Class data )
	{
		layoutRoot.SetUserData( data );
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		OnInit();

		m_Disable = layoutRoot.FindAnyWidget( "action_wrapper_disable" );

		Show();
	}

	void OnInit() 
	{
	}

	void Show()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Show" );
		#endif

		layoutRoot.Show( true );
		OnShow();

		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( Update );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Show" );
		#endif
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Hide" );
		#endif
		//Error("test");

		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( Update );

		OnHide();
		
		if ( layoutRoot )
			layoutRoot.Show( false );

		if (GetGame().GetMission().IsInputExcludeActive("menu"))
			GetGame().GetMission().RemoveActiveInputExcludes({"menu"});

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Hide" );
		#endif
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	void Update( float timeSlice )
	{
		bool isFocused = IsFocusWidget( GetFocus() );

		if (isFocused && !m_WasFocused)
		{
			if (!GetGame().GetMission().IsInputExcludeActive("menu"))
				GetGame().GetMission().AddActiveInputExcludes({"menu"});
		}
		else if (!isFocused && m_WasFocused)
		{
			if (GetGame().GetMission().IsInputExcludeActive("menu"))
				GetGame().GetMission().RemoveActiveInputExcludes({"menu"});
		}

		m_WasFocused = isFocused;

		if ( m_LeftMouseDown )
		{
			if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) == 0 )
			{
				m_LeftMouseDown = false;

				if ( m_WasFocused && !IsFocusWidget( GetWidgetUnderCursor() ) )
				{
					SetFocus( NULL );
				}
			}
		} else
		{
			if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) != 0 )
			{
				m_LeftMouseDown = true;
			}
		}
	}

	bool IsFocused()
	{
		return m_WasFocused;
	}

	bool IsFocusWidget( Widget widget )
	{
		return false;
	}

	void UpdatePermission( string permission )
	{
		SetEnabled( GetPermissionsManager().HasPermission( permission ) );
	}

	void SetEnabled( bool enable )
	{
		if ( enable )
		{
			Enable();
		} else
		{
			Disable();
		}
	}

	void Disable()
	{
		layoutRoot.Enable( false );

		m_Disable.Show( true );
	}

	void Enable()
	{
		layoutRoot.Enable( true );

		m_Disable.Show( false );
	}

	void SetWidth( float width )
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		layoutRoot.SetSize( width, h );
		layoutRoot.Update();
	}

/*
	void SetHeight( float height )
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		layoutRoot.SetSize( w, height );
		layoutRoot.Update();
	}

	float GetHeight()
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		return h;
	}

	void SetYPosition( float ypos )
	{
		float w;
		float h;
		layoutRoot.GetPos( w, h );
		layoutRoot.SetPos( w, ypos );
		layoutRoot.Update();
	}
*/
	void SetFixedSize( float width, float height )
	{
		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetFlags( WidgetFlags.HEXACTSIZE, true );
		layoutRoot.SetSize( width, height );
		layoutRoot.Update();
	}

	void SetFixedHeight( float height )
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetSize( w, height );
		layoutRoot.Update();
	}

	void SetPosition( float xpos )
	{
		float w;
		float h;
		layoutRoot.GetPos( w, h );
		layoutRoot.SetPos( xpos, h );
		layoutRoot.Update();
	}

	void SetWidgetWidth( Widget widget, float width )
	{
		float w;
		float h;
		widget.GetSize( w, h );
		widget.SetSize( width, h );
		layoutRoot.Update();
		widget.Update();
	}

	void SetWidgetPosition( Widget widget, float xpos )
	{
		float x;
		float y;
		widget.GetPos( x, y );
		widget.SetPos( xpos, y );
		layoutRoot.Update();
		widget.Update();
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	bool IsVisible()
	{
		if ( !layoutRoot )
			return false;
			
		return layoutRoot.IsVisible();
	}

	void SetCallback( Class instance, string funcname )
	{
		if ( instance == NULL || funcname == "" ) return;

		m_Instance = instance;
		m_FuncName = funcname;

		m_HasCallback = true;
	}

	bool CallEvent( UIEvent eid )
	{
		return false;
	}

	void SetData( UIActionData data )
	{
		m_Data = data;
	}

	UIActionData GetData()
	{
		return m_Data;
	}

	void SetButton( string text )
	{
	}

	string GetButton()
	{
		return "";
	}

	void SetLabel( string text )
	{
	}

	void SetChecked( bool checked )
	{
	}
	
	bool IsChecked()
	{
		return false;
	}
	
	void SetText( string text )
	{
	}

	string GetText()
	{
		return "";
	}

	float GetCurrent()
	{
		return 0.0;
	}

	void SetCurrent( float value )
	{
		
	}

	void SetValue( vector v )
	{
	}

	vector GetValue()
	{
		return "0 0 0";
	}

	void SetSelection( int i, bool sendEvent = true )
	{
	}

	int GetSelection()
	{
		return -1;
	}
}