class ESPBox extends ScriptedWidgetEventHandler 
{
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

	bool UseSyncedPosition;

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
		if ( Player.PlayerObject )
		{
			if ( Player.PlayerObject.IsInTransport() )
			{
				return Player.PlayerObject.GetPosition() + "0 1.1 0";
			} else 
			{
				vector position = Player.PlayerObject.GetPosition() + "0 1.85 0";

				int bone = Player.PlayerObject.GetBoneIndexByName( "Head" );

				if ( bone != -1 )
				{
					position = Player.PlayerObject.GetBonePositionWS( bone ) + "0 0.2 0";
				}

				return position;
			}
		}
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

		if ( ShowOnScreen && Player )
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

	

	string GetName()
	{
		return Player.GetName();
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{		
		if ( w == Checkbox )
		{
			Menu.OnPlayer_Checked( Player, Checkbox.IsChecked() );
		}

		if ( w == Button )
		{
			Checkbox.SetChecked( Menu.OnPlayer_Button( Player ) );
		}

		return true;
	}
}