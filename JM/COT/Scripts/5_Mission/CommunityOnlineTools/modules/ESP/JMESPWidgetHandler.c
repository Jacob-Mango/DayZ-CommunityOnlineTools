class JMESPWidgetHandler: ScriptedWidgetEventHandler 
{
	static JMESPForm espMenu;
	static JMESPModule espModule;

	static bool UseClassName = false;

	private Widget layoutRoot;

	private CheckBoxWidget m_chbx_SelectedObject;
	private TextWidget m_txt_ObjectName;
	private ButtonWidget m_btn_ToggleActions;
	private ImageWidget m_img_ToggleActions;

	private Widget m_pnl_Actions;

	private UIActionScroller m_scrler_Actions;
	private Widget m_rows_Actions;

	private bool m_IsShown;
	private bool m_DidUnlink;

	private vector m_LastPosition;
	private string m_TargetType;
	private bool m_UseClassName;

	bool ShowOnScreen;

	int Width;
	int Height;
	float FOV;
	vector ScreenPos;

	JMESPMeta Info;

	void OnWidgetScriptInit( Widget w )
	{
		m_DidUnlink = false;

		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}

	void ~JMESPWidgetHandler()
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPWidgetHandler::~JMESPWidgetHandler();" );
		#endif
		#endif

		OnHide();

		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "-JMESPWidgetHandler::~JMESPWidgetHandler();" );
		#endif
		#endif
	}

	void Init() 
	{
		Widget header;
		Class.CastTo( header, layoutRoot.FindAnyWidget( "esp_object_header" ) );

		Class.CastTo( m_chbx_SelectedObject, header.FindAnyWidget( "esp_select_checkbox" ) );
		Class.CastTo( m_txt_ObjectName, header.FindAnyWidget( "esp_object_name" ) );
		Class.CastTo( m_btn_ToggleActions, header.FindAnyWidget( "esp_toggle_button" ) );

		if ( Class.CastTo( m_img_ToggleActions, header.FindAnyWidget( "esp_toggle_button_image" ) ) )
		{
			m_img_ToggleActions.LoadImageFile( 0, "set:dayz_gui image:icon_expand" );
			m_img_ToggleActions.LoadImageFile( 1, "set:dayz_gui image:icon_collapse" );
		}

		Class.CastTo( m_pnl_Actions, layoutRoot.FindAnyWidget( "esp_actions_container" ) );

		m_scrler_Actions = UIActionManager.CreateScroller( m_pnl_Actions );
		Class.CastTo( m_rows_Actions, m_scrler_Actions.GetContentWidget() );

		HideActions();

		JMScriptInvokers.ON_DELETE_ALL.Insert( OnDeleteAll );
	}

	void Show()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Show" );
		#endif

		layoutRoot.Show( true );
		OnShow();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Show" );
		#endif
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Hide" );
		#endif

		OnHide();
		layoutRoot.Show( false );

		JMScriptInvokers.ON_DELETE_ALL.Remove( OnDeleteAll );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Hide" );
		#endif
	}

	void ToggleESPActions()
	{
		if ( m_pnl_Actions.IsVisible() )
		{
			HideActions();
		} else
		{
			ShowActions();
		}
	}

	void ShowActions()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::ShowActions" );
		#endif

		if (!Info.m_ActionsInitialized)
			Info.InitActions();

		m_pnl_Actions.Show( true );

		m_img_ToggleActions.SetImage( 0 );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::ShowActions" );
		#endif
	}

	void HideActions()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::HideActions" );
		#endif

		m_pnl_Actions.Show( false );

		m_img_ToggleActions.SetImage( 1 );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::HideActions" );
		#endif
	}

	void OnShow()
	{
		if (m_IsShown)
			return;

		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( this.Update );

		m_IsShown = true;
	}

	void OnHide() 
	{
		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( this.Update );

		Deselect();

		m_IsShown = false;
	}

	void OnDeleteAll()
	{
		if ( !Info.type.IsInherited( JMESPViewTypePlayer ) && m_chbx_SelectedObject.IsChecked() )
		{
			Hide();
		}
	}

	float ATan( float a )
	{
		return Math.Asin( a ) / Math.Acos( a );
	}

	private vector Player_GetPosition()
	{
		Human man;
		if ( !Class.CastTo( man, Info.target ) )
		{
			return Info.target.GetPosition();
		}

		if ( man.IsInTransport() )
		{
			return man.GetPosition() + "0 1.1 0";
		}

		vector position = man.GetPosition() + "0 1.85 0";

		int bone = man.GetBoneIndexByName( "Head" );

		if ( bone != -1 )
		{
			position = man.GetBonePositionWS( bone ) + "0 0.2 0";
		}

		return position;
	}

	vector GetPosition()
	{
		if ( Info.target )
		{
			if ( Info.type.IsInherited( JMESPViewTypePlayer ) )
			{
				return Player_GetPosition();
			} else
			{
				return Info.target.GetPosition();
			}
		}

		return m_LastPosition;
	}

	void Update() 
	{
		if ( Info == NULL ) 
		{
			ShowOnScreen = false;
			Hide();
			return;
		}

		m_LastPosition = GetPosition();

		ScreenPos = GetGame().GetScreenPos( m_LastPosition );

		float distance = vector.Distance(GetCurrentPosition(), m_LastPosition);

		//if (distance > 10)
			distance = Math.Round(distance * 10.0) / 10.0;

		GetScreenSize( Width, Height );

		if ( ScreenPos[0] <= 0 || ScreenPos[1] <= 0 )
		{
			ShowOnScreen = false;
		} else 
		{
			ShowOnScreen = true;
		}

		if ( ShowOnScreen && ( ScreenPos[0] >= Width || ScreenPos[1] >= Height ) )
		{
			ShowOnScreen = false;
		}

		if ( ShowOnScreen && ScreenPos[2] < 0 )
		{
			ShowOnScreen = false;
		}

		if ( ShowOnScreen && Info )
		{
			layoutRoot.SetPos( ScreenPos[0], ScreenPos[1], true );

			string text = " ";

			if ( Info.type.IsInherited( JMESPViewTypePlayer ) )
			{
				text += Info.name + " (" + distance + "m)";
			} else
			{
				if ( UseClassName )
				{
					text += m_TargetType + " (" + distance + "m)";
				} else
				{
					text += Info.name + " (" + distance + "m)";
				}
			}

			text += " ";

			m_txt_ObjectName.SetText( text );

			float w, h;
			layoutRoot.GetScreenSize(w, h);

			float tw, th;
			m_txt_ObjectName.GetScreenSize(tw, th);

			if (m_UseClassName != UseClassName)
			{
				w = 300;
				layoutRoot.SetScreenSize(w, h);
				m_UseClassName = UseClassName;
			}

			w = Math.Max(tw + 44, w);
			layoutRoot.SetScreenSize(w, h);

			Info.Update();

			m_scrler_Actions.UpdateScroller();
			
			layoutRoot.Show( true );
		} else 
		{
			layoutRoot.Show( false );
		}
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	void SetInfo( JMESPMeta info, out Widget action_rows )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::SetInfo" );
		#endif

		action_rows = m_rows_Actions;

		Info = info;
		
		if ( Info == NULL || Info.target == NULL ) 
		{
			ShowOnScreen = false;
			Hide();
			return;
		}

		m_chbx_SelectedObject.SetChecked( JM_GetSelected().IsObjectSelected( Info.target ) );

		m_TargetType = Info.GetType();

		m_txt_ObjectName.SetColor( Info.colour );

		m_chbx_SelectedObject.Show( true );
			
		ShowOnScreen = true;
		
		Show();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::SetInfo" );
		#endif
	}

	void Select()
	{
		if ( m_chbx_SelectedObject )
			m_chbx_SelectedObject.SetChecked( true );

		SetSelected( true );
	}

	void Deselect()
	{
		if ( m_chbx_SelectedObject )
			m_chbx_SelectedObject.SetChecked( false );

		SetSelected( false );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}
		
		if ( w == m_chbx_SelectedObject )
		{
			SetSelected( m_chbx_SelectedObject.IsChecked() );

			return true;
		}

		if ( w == m_btn_ToggleActions )
		{
			ToggleESPActions();

			return true;
		}

		return false;
	}

	void SetSelected( bool selected )
	{
		if ( Info.type.IsInherited( JMESPViewTypePlayer ) )
		{
			if (Info.player)
				JMScriptInvokers.MENU_PLAYER_CHECKBOX.Invoke( Info.player.GetGUID(), selected );
		} else if ( Info.target )
		{
			if ( selected )
			{
				JMScriptInvokers.ADD_OBJECT.Invoke( Info.target );
			} else
			{
				JMScriptInvokers.REMOVE_OBJECT.Invoke( Info.target );
			}
		}
	}
};
