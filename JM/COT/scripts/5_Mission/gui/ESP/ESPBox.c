class ESPBox extends ScriptedWidgetEventHandler 
{
	static ref ESPModule espModule;
	static ref ESPMenu espMenu;
	static ref PlayerMenu playerMenu;

	static bool ShowJustName = false;

	protected ref Widget layoutRoot;

	ref TextWidget		Name;
	ref ButtonWidget	Button;
	ref CheckBoxWidget	Checkbox;

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

	void ~ESPBox()
	{
	}

	void Init() 
	{
		Widget checkboxStyle = layoutRoot.FindAnyWidget("CheckboxStyle");
		Widget justName = layoutRoot.FindAnyWidget("JustName");
		if ( ShowJustName )
		{
			checkboxStyle.Show(false);
			justName.Show(true);

			Name = TextWidget.Cast(justName.FindAnyWidget("text_name"));
		} else 
		{
			checkboxStyle.Show(true);
			justName.Show(false);

			Name = TextWidget.Cast(checkboxStyle.FindAnyWidget("text_name"));
			Button = ButtonWidget.Cast(checkboxStyle.FindAnyWidget("button"));
			Checkbox = CheckBoxWidget.Cast(checkboxStyle.FindAnyWidget("checkbox"));
		}
	}

	void Show()
	{
		if (layoutRoot) layoutRoot.Show( true );
		if (Button) Button.Show( true );
		if (Checkbox) Checkbox.Show( true );
		if (Name) Name.Show( true );
		OnShow();
	}

	void Hide()
	{
		OnHide();
		if (Name) Name.Show( false );
		if (Button) Button.Show( false );
		if (Checkbox) Checkbox.Show( false );
		if (layoutRoot) layoutRoot.Show( false );
	}

	void Unlink()
	{
		if (Name) Name.Unlink();
		if (Button) Button.Unlink();
		if (Checkbox) Checkbox.Unlink();
		if (layoutRoot) layoutRoot.Unlink();
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
		vector position = GetPosition();

		vector normalize = ( position - GetGame().GetCurrentCameraPosition() );
		float dot = vector.Dot( normalize.Normalized(), GetGame().GetCurrentCameraDirection().Normalized() );
		
		float limit = FOV / 1.5;

		if ( dot < limit )
		{
			ShowOnScreen = false;
		} else
		{
			ShowOnScreen = true;
		}
			
		ScreenPos = GetGame().GetScreenPos( position );

		if ( ShowOnScreen && ( ScreenPos[2] > espModule.ESPRadius || ScreenPos[2] < 0 ) )
		{
			ShowOnScreen = false;
		}

		if ( ShowOnScreen && Info )
		{
			layoutRoot.SetPos( ScreenPos[0], ScreenPos[1] - ( Height / 2 ), true );
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
			return
		}

		Name.SetText( Info.name );

		switch ( Info.type )
		{
			case ESPType.PLAYER:
			{
				Name.SetColor( 0x00FFFFFF );
				break;
			}
			case ESPType.VEHICLE:
			{
				Name.SetColor( 0xA020F0FF );
				break;
			}
			case ESPType.BASEBUILDING:
			{
				Name.SetColor( 0xFF0000FF );
				break;
			}
			case ESPType.ITEM:
			{
				Name.SetColor( 0xFFA500FF );
				break;
			}
			case ESPType.INFECTED:
			{
				Name.SetColor( 0x0000FFFF );
				break;
			}
			case ESPType.CREATURE:
			{
				Name.SetColor( 0x00FF00FF );
				break;
			}
			case ESPType.ALL:
			{
				Name.SetColor( 0xFFFFFFFF );
				break;
			}
		}

		ScreenPos = GetGame().GetScreenPos( GetPosition() );

		Show();
			
		ShowOnScreen = true;
			
		GetScreenSize( Width, Height );
			
		FOV = Camera.GetCurrentFOV() * ( Height * 1.0 ) / ( Width * 1.0 );
			
		GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert( this.Update );
			
		Update();
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{		
		if ( Info.type == ESPType.PLAYER && Info.player && playerMenu )
		{
			if ( w == Checkbox && Checkbox != NULL )
			{
				playerMenu.OnPlayer_Checked_ESP( this );
			}

			if ( w == Button && Checkbox != NULL )
			{
				playerMenu.OnPlayer_Button_ESP( this );
			}
		} else if ( espModule )
		{
			if ( w == Checkbox && Checkbox != NULL )
			{
				espModule.SelectBox( this, Checkbox.IsChecked(), false );
			}

			if ( w == Button && Button != NULL )
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