class JMESPWidgetHandler: ScriptedWidgetEventHandler 
{
	static JMESPForm espMenu;
	static JMESPModule espModule;

	static bool UseClassName = false;

	private Widget layoutRoot;

	private CheckBoxWidget m_chbx_SelectedObject;
	private ImageWidget m_img_HealthLevel;
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

		JMScriptInvokers.ON_DELETE_ALL.Remove( OnDeleteAll );

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
		Class.CastTo( m_img_HealthLevel, header.FindAnyWidget( "esp_health_level_icon" ) );
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
		else
			Info.UpdateActions();

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

		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( this.Update );

		m_IsShown = true;
	}

	void OnHide() 
	{
		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( this.Update );

		Deselect();

		m_IsShown = false;
	}

	void OnDeleteAll()
	{
		if (Info)
			Info.m_TargetDeleted = true;
	}

	float ATan( float a )
	{
		return Math.Asin( a ) / Math.Acos( a );
	}

	vector Player_GetTopOfHeadPosition()
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

		vector position = man.GetPosition() + "0 1.8 0";

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
			return Info.target.GetPosition();
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

		ScreenPos = g_Game.GetScreenPos( m_LastPosition );

		float distance = vector.Distance(GetCurrentPosition(), m_LastPosition);

		int zIndex;

		if (m_pnl_Actions.IsVisible())
			zIndex = 0;
		else
			zIndex = -distance * 100;

		layoutRoot.SetSort(zIndex);

		//if (distance > 10)
			distance = Math.Round(distance * 10.0) / 10.0;

		GetScreenSize( Width, Height );

		if (ScreenPos[0] <= 0 || ScreenPos[1] <= 0 || ScreenPos[0] >= Width || ScreenPos[1] >= Height || ScreenPos[2] < 0)
		{
			ShowOnScreen = false;
		}
		else if (g_Game.GetUIManager().GetMenu())
		{
			ShowOnScreen = false;
		}
	#ifdef DZ_Expansion_Core
		else if (GetDayZExpansion().GetExpansionUIManager().GetMenu())
		{
			ShowOnScreen = false;
		}
	#endif
		else
		{
			ShowOnScreen = true;
		}

		if ( ShowOnScreen && Info )
		{
			layoutRoot.SetPos( ScreenPos[0], ScreenPos[1], true );

			bool isHealthVisible = m_img_HealthLevel.IsVisible();

			if (Info.target)
			{
				if (!Info.target.IsPlainObject() && Info.target.GetNumberOfHealthLevels() > 0)
				{
					switch (Info.target.GetHealthLevel())
					{
						case GameConstants.STATE_WORN:
							m_img_HealthLevel.SetColor(Colors.COLOR_WORN | 0x7F000000);
							break;

						case GameConstants.STATE_DAMAGED:
							m_img_HealthLevel.SetColor(Colors.COLOR_DAMAGED | 0x7F000000);
							break;

						case GameConstants.STATE_BADLY_DAMAGED:
							m_img_HealthLevel.SetColor(Colors.COLOR_BADLY_DAMAGED | 0x7F000000);
							break;

						case GameConstants.STATE_RUINED:
							m_img_HealthLevel.SetColor(Colors.COLOR_RUINED | 0x7F000000);
							break;

						default:
							m_img_HealthLevel.SetColor(Colors.COLOR_PRISTINE | 0x7F000000);
							break;
					}
				}
				else if (isHealthVisible)
				{
					m_img_HealthLevel.Show(false);
					float x, y;
					m_img_HealthLevel.GetPos(x, y);
					m_txt_ObjectName.SetPos(x, y);
				}
			}

			string text;

			if ( Info.type.IsInherited( JMESPViewTypePlayer ) || !UseClassName )
				text += Info.name;
			else
				text += m_TargetType;

			text += " (" + distance + " m) ";

			m_txt_ObjectName.SetText( text );

			float w, h;
			layoutRoot.GetScreenSize(w, h);

			float iw, ih;
			m_img_HealthLevel.GetScreenSize(iw, ih);
			if (!isHealthVisible)
			{
				w -= iw + 2;
				iw = 0;
			}

			float tw, th;
			m_txt_ObjectName.GetScreenSize(tw, th);

			if (m_UseClassName != UseClassName)
			{
				w = 300;
				layoutRoot.SetScreenSize(w, h);
				m_UseClassName = UseClassName;
			}

			w = Math.Max(iw + tw + 48, w);
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
		if (!Info)
			return;

		if ( Info.type.IsInherited( JMESPViewTypePlayer ) )
		{
			if (Info.player)
				JMScriptInvokers.MENU_PLAYER_CHECKBOX.Invoke( Info.player.GetGUID(), selected );
		}

		if ( selected )
		{
			if ( Info.target )
				JMScriptInvokers.ADD_OBJECT.Invoke( Info.target );
		}
		else
		{
			JMScriptInvokers.REMOVE_OBJECT.Invoke( Info.target, Info.networkLow, Info.networkHigh );
		}
	}
}
