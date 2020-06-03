class ESPRenderBox extends ScriptedWidgetEventHandler 
{
	static ref ESPModule espModule;
	static ref ESPMenu espMenu;
	static ref PlayerMenu playerMenu;

	static bool ShowJustName = false;
	static bool UseClassName = false;

	protected ref Widget 			layoutRoot;

	protected ref Widget			m_CheckboxStyle;
	protected ref Widget			m_JustName;

	protected ref TextWidget		m_Name1;

	protected ref TextWidget		m_Name2;
	protected ref ButtonWidget		m_Button;
	ref CheckBoxWidget				Checkbox;

	bool ShowOnScreen;

	int Width;
	int Height;
	float FOV;
	vector ScreenPos;

	ref ESPInfo Info;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}

	void ~ESPRenderBox()
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

	vector GetPosition()
	{
		if ( Info.target )
		{
			if ( Info.type == ESPType.PLAYER )
			{
				Human man = Human.Cast( Info.target );

				if ( !man )
				{
					return Info.target.GetPosition();
				}

				if ( man.IsInTransport() )
				{
					return man.GetPosition() + "0 1.1 0";
				} else 
				{
					vector position = man.GetPosition() + "0 1.85 0";

					int bone = man.GetBoneIndexByName( "Head" );

					if ( bone != -1 )
					{
						position = man.GetBonePositionWS( bone ) + "0 0.2 0";
					}

					return position;
				}
			}

			return Info.target.GetPosition();
		}
		return "0 0 0";
	}

	void Update() 
	{
		m_CheckboxStyle.Show( !ShowJustName );
		m_JustName.Show( ShowJustName );
			
		vector position = GetPosition();
		ScreenPos = GetGame().GetScreenPos( position );

		GetScreenSize( Width, Height );

		if ( ScreenPos[0] <= 0 || ScreenPos[1] <= 0 )
		{
			ShowOnScreen = false;
		} else {
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

			float distance = Math.Round( vector.Distance( position, GetGame().GetPlayer().GetPosition() ) * 10.0 ) / 10.0;

			string text = "";

			if ( Info.type != ESPType.PLAYER && UseClassName )
			{
				text = Info.target.GetType() + " (" + distance + "m)";
			} else 
			{
				text = Info.name + " (" + distance + "m)";
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

	void SetInfo( ref ESPInfo info )
	{
		Info = info;
		
		if ( Info == NULL || Info.target == NULL ) 
		{
			ShowOnScreen = false;
			Hide();
			GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );
			return;
		}

		int colour = 0xFFFFFFFF;

		switch ( Info.type )
		{
			case ESPType.PLAYER:
			{
				colour = 0xFF00FFFF;
				break;
			}
			case ESPType.VEHICLE:
			{
				colour = 0xFF00FFDD;
				break;
			}
			case ESPType.BASEBUILDING:
			{
				colour = 0xFFF75B18;
				break;
			}
			case ESPType.ITEM:
			{
				colour = 0xFFF7F71D;
				break;
			}
			case ESPType.INFECTED:
			{
				colour = 0xFF23FF44;
				break;
			}
			case ESPType.CREATURE:
			{
				colour = 0xFF54F7BB;
				break;
			}
			case ESPType.ALL:
			{
				colour = 0xFFFFFFFF;
				break;
			}
		}

		m_Name1.SetColor( colour );
		m_Name2.SetColor( colour );
			
		ShowOnScreen = true;
			
		GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert( this.Update );
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{		
		if ( Info.type == ESPType.PLAYER )
		{
			if ( Info.player && playerMenu ) 
			{
				if ( w == Checkbox && Checkbox != NULL )
				{
					playerMenu.OnPlayer_Checked_ESP( this );
				}

				if ( w == m_Button && Checkbox != NULL )
				{
					playerMenu.OnPlayer_Button_ESP( this );
				}
			}
		}
		
		if ( espModule )
		{
			if ( w == Checkbox && Checkbox != NULL )
			{
				espModule.SelectBox( this, Checkbox.IsChecked(), false );
			}

			if ( w == m_Button && m_Button != NULL )
			{
				espModule.SelectBox( this, true, true );
			}

			if ( espMenu )
			{
				espMenu.OnSelect();
			}
		}

		return true;
	}
}