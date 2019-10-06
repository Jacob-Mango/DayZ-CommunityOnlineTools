class JMESPWidget extends ScriptedWidgetEventHandler 
{
	static JMESPForm espMenu;
	static JMESPModule espModule;

	static bool ShowJustName = false;
	static bool UseClassName = false;

	protected Widget 			layoutRoot;

	protected Widget			m_CheckboxStyle;
	protected Widget			m_JustName;

	protected TextWidget		m_Name1;
	protected TextWidget		m_Name2;

	protected ButtonWidget		m_Button;
	CheckBoxWidget				Checkbox;

	bool ShowOnScreen;

	int Width;
	int Height;
	float FOV;
	vector ScreenPos;

	ref JMESPMeta Info;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}

	void ~JMESPWidget()
	{
		Unlink();
	}

	void Init() 
	{
		m_CheckboxStyle = layoutRoot.FindAnyWidget("CheckboxStyle");
		m_JustName = layoutRoot.FindAnyWidget("JustName");
		
		m_Name1 = TextWidget.Cast( m_JustName.FindAnyWidget("text_name") );

		m_Name2 = TextWidget.Cast( m_CheckboxStyle.FindAnyWidget("text_name") );

		m_Button = ButtonWidget.Cast( m_CheckboxStyle.FindAnyWidget("button") );
		Checkbox = CheckBoxWidget.Cast( m_CheckboxStyle.FindAnyWidget("checkbox") );
	}

	void Show()
	{
		layoutRoot.Show( true );
		OnShow();
	}

	void Hide()
	{
		OnHide();
		layoutRoot.Show( false );
	}

	void Unlink()
	{
		GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );

		if ( layoutRoot ) layoutRoot.Unlink();

		delete m_Name1;
		delete m_Name2;
		delete m_Button;
		delete Checkbox;
		delete layoutRoot;
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
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

		return "0 0 0";
	}

	void Update() 
	{
		m_CheckboxStyle.Show( !ShowJustName );
		m_JustName.Show( ShowJustName );
			
		ScreenPos = GetGame().GetScreenPos( GetPosition() );

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

		if ( ShowOnScreen && ( ScreenPos[2] > ( espModule.ESPRadius  * 2 ) || ScreenPos[2] < 0 ) )
		{
			ShowOnScreen = false;
		}

		if ( ShowOnScreen && Info )
		{
			layoutRoot.SetPos( ScreenPos[0], ScreenPos[1] - ( Height / 2 ), true );

			float distance = Math.Round( ScreenPos[2] * 10.0 ) / 10.0;

			string text = "";

			if ( Info.type.IsInherited( JMESPViewTypePlayer ) )
			{
				text = Info.name + " (" + distance + "m)";
			} else
			{
				if ( UseClassName )
				{
					text = Info.target.GetType() + " (" + distance + "m)";
				} else
				{
					text = Info.name + " (" + distance + "m)";
				}
			}

			m_Name1.SetText( text );
			m_Name2.SetText( text );
			
			Show();
		} else 
		{
			Hide();
		}
	}

	ref Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	void SetInfo( ref JMESPMeta info )
	{
		Info = info;
		
		if ( Info == NULL || Info.target == NULL ) 
		{
			ShowOnScreen = false;
			Hide();
			GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );
			return;
		}

		m_Name1.SetColor( Info.colour );
		m_Name2.SetColor( Info.colour );

		Checkbox.Show( Info.type.IsInherited( JMESPViewTypePlayer ) );
		m_Button.Enable( Info.type.IsInherited( JMESPViewTypePlayer ) );
			
		ShowOnScreen = true;
			
		GetGame().GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( this.Update );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		if ( !Info.player )
		{
			return false;
		}
		
		if ( w == Checkbox )
		{
			JMScriptInvokers.MENU_PLAYER_CHECKBOX.Invoke( Info.player.GetGUID(), Checkbox.IsChecked() );

			return true;
		}

		if ( w == m_Button )
		{
			JMScriptInvokers.MENU_PLAYER_BUTTON.Invoke( Info.player.GetGUID(), !Checkbox.IsChecked() );

			return true;
		}

		return false;
	}
}