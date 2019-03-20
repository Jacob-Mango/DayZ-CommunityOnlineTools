class ESPBox extends ScriptedWidgetEventHandler 
{
	static ref ESPModule espModule;
	static ref PlayerMenu playerMenu;

	protected ref Widget layoutRoot;

	ref TextWidget		Name;
	ref ButtonWidget	Button;
	ref CheckBoxWidget	Checkbox;

	bool ShowOnScreen;

	int Width;
	int Height;
	float FOV;
	vector ScreenPos;

	float BoxWidth;
	float BoxHeight;

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
		Name = TextWidget.Cast(layoutRoot.FindAnyWidget("text_name"));
		Button = ButtonWidget.Cast(layoutRoot.FindAnyWidget("button"));
		Checkbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("checkbox"));

		layoutRoot.GetScreenSize( BoxWidth, BoxHeight );
	}

	void Show()
	{
		layoutRoot.Show( true );
		Button.Show( true );
		Checkbox.Show( true );
		Name.Show( true );
		OnShow();
	}

	void Hide()
	{
		OnHide();
		Name.Show( false );
		Button.Show( false );
		Checkbox.Show( false );
		layoutRoot.Show( false );
	}

	void Unlink()
	{
		Name.Unlink();
		Button.Unlink();
		Checkbox.Unlink();
		layoutRoot.Unlink();
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
			if ( Info.isPlayer )
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

		if ( ShowOnScreen )
		{
			if ( ScreenPos[2] > 1000 || ScreenPos[2] < 0 )
			{
				ShowOnScreen = false;
			}
		}

		if ( ShowOnScreen && Info )
		{
			layoutRoot.SetPos( ScreenPos[0] - ( BoxWidth / 8 ), ScreenPos[1] - ( Height / 2 ) - ( BoxHeight / 2 ), true );
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
		
		if ( Info == NULL ) 
		{
			ShowOnScreen = false;
			Hide();
			GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );
		} else 
		{
			ShowOnScreen = true;

			if ( Info.target == NULL )
			{
				Hide();
				ShowOnScreen = false;
				GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );
				return;
			}

			Name.SetText( Info.name );

			if ( !GetGame().IsMultiplayer() )
			{
				ShowOnScreen = false;
				Name.SetColor( 0xFF4B77BE );
				Hide();
				return;
			}

/*
			PlayerBase controllingPlayer = PlayerBase.Cast( GetGame().GetPlayer() );

			if ( controllingPlayer && Player.GetGUID() == controllingPlayer.GetIdentity().GetId() )
			{
				Name.SetColor( 0xFF2ECC71 );

				bool cameraExists = controllingPlayer.GetCurrentCamera() != NULL;

				if ( cameraExists && controllingPlayer.GetCurrentCamera().IsInherited( DayZPlayerCamera3rdPerson ) )
				{
					ShowOnScreen = true;
				} else if ( cameraExists && controllingPlayer.GetCurrentCamera().IsInherited( DayZPlayerCamera3rdPersonVehicle ) )
				{
					ShowOnScreen = true;
				} else if ( CurrentActiveCamera )
				{
					ShowOnScreen = true;
				} else 
				{
					ShowOnScreen = false;
					Hide();
				}
			} 
*/

			ShowOnScreen = true;

			if ( ShowOnScreen )
			{
				Name.SetColor( 0xFFFFFFFF );
			
				ScreenPos = GetGame().GetScreenPos( GetPosition() );

				if ( ScreenPos[2] > espModule.ESPRadius || ScreenPos[2] < 0 )
				{
					Hide();
					ShowOnScreen = false;
				} else 
				{
					Show();

					ShowOnScreen = true;

					GetScreenSize( Width, Height );

					FOV = Camera.GetCurrentFOV() * ( Height * 1.0 ) / ( Width * 1.0 );

					GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert( this.Update );

					Update();
				}
			}
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{		
		if ( Info.isPlayer && Info.player )
		{
			if ( w == Checkbox )
			{
				playerMenu.OnPlayer_Checked_ESP( this );
			}

			if ( w == Button )
			{
				playerMenu.OnPlayer_Button_ESP( this );
			}
		} else
		{
			if ( w == Checkbox )
			{
				espModule.SelectBox( this, Checkbox.IsChecked(), false );
			}

			if ( w == Button )
			{
				espModule.SelectBox( this, true, true );
			}
		}

		return true;
	}
}