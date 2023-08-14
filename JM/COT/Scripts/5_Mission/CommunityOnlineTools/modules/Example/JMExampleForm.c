/*
 * This file is an example class for anyone looking to learn how to mod COT
*/
class JMExampleForm: JMFormBase
{
	protected UIActionScroller 			m_Scroller;

	protected UIActionText 				m_Text;
	protected UIActionEditableText 		m_EditableText;
	protected UIActionEditableVector 	m_EditableVector;

	protected UIActionButton 			m_Button;
	protected UIActionNavigateButton 	m_NavButton;

	protected UIActionDropdownList 		m_Dropdown;
	protected UIActionSlider 			m_Slider;
	protected UIActionCheckbox 			m_Checkbox;

	protected int m_NavButtonStateId;

	protected ref array< string > m_NavButtonWordList =
	{
		"Mid",
		"Extension",
		"When Train ?"
	};

	protected ref array< string > m_DropdownWordList =
	{
		"Jacob Mango",
		"Liquidrock",
		"Arkensor",
		"LieutenantMaster",
		"DannyDog"
	};

	private JMExampleModule m_Module;
	private JMPlayerModule m_PlayerModule;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_Scroller = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		Widget actions = m_Scroller.GetContentWidget();

		Widget gridActions = UIActionManager.CreateGridSpacer( actions, 8, 1 );

		m_Text = UIActionManager.CreateText( gridActions, "Left Texty", "Right Texto" );
		m_EditableText = UIActionManager.CreateEditableText( gridActions, "Wordy:", this, "OnChange_EditableText" );
		m_EditableVector = UIActionManager.CreateEditableVector( gridActions, "Position", this, "OnClick_EditableVector", "Set" );

		// You can use GridSpacers to organize your UI in a much more easy way than giving manually by hand the size of each elements
		// In this case we are assiging two buttons to this grid which will be spaced out evenly horizontaly
		Widget gridButtons = UIActionManager.CreateGridSpacer( gridActions, 1, 2 );
		m_Button = UIActionManager.CreateButton( gridButtons, "Dont Press that button", this, "OnClick_Button" );
		m_NavButton = UIActionManager.CreateNavButton( gridButtons, "Mid", JM_COT_ICON_ARROW_LEFT, JM_COT_ICON_ARROW_RIGHT, this, "OnClick_NavButton" );

		m_Dropdown = UIActionManager.CreateDropdownBox( gridActions, actions, "Bestie:", m_DropdownWordList, this, "OnClick_Dropdown" );
		m_Dropdown.SetPosition( 0.70 );
		m_Dropdown.SetWidth( 0.3 );

		m_Slider = UIActionManager.CreateSlider( gridActions, "Slidy:", 0, 1, this, "OnChange_Slider" );
		m_Slider.SetCurrent( 0 );
		m_Slider.SetStepValue( 0.1 );
		m_Slider.SetMin( 0.0 );
		m_Slider.SetMax( 100.0 );
		m_Slider.SetPosition( 0.5 );
		m_Slider.SetWidth( 0.3 );

		m_Checkbox =  UIActionManager.CreateCheckbox( gridActions, "Did My Homework", this, "OnClick_Checkbox" );

		Class.CastTo(m_PlayerModule, GetModuleManager().GetModule(JMPlayerModule));
	}

	override void OnShow()
	{
		super.OnShow();
	}

	override void OnHide()
	{
		super.OnHide();
	}

	override void OnSettingsUpdated()
	{		
		// Update everything settings related here
	}

	void OnClick_Checkbox( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( action.IsChecked() )
		{
			action.SetLabel("\"Did\" My Homework");

			COTCreateLocalAdminNotification( new StringLocaliser( "Liar" ) );
		}
		else
		{
			action.SetLabel("Did My Homework");
		}
	}

	void OnChange_Slider( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		float current = m_Slider.GetCurrent();
		float color;

		if ( current == 0 )
			color = 0;
		else
			color = (255 / 100) * current; 
		
		m_Slider.SetColor( ARGB( 255, color, color, color ) );
		m_Slider.SetAlpha( 1.0 );
	}

	void OnChange_EditableText( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_EditableText.Disable();
	}

	void OnClick_NavButton( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK || eid == UIEvent.CLICK_RIGHTSIDE )
		{
			m_NavButtonStateId++;
		}
		else if ( eid == UIEvent.CLICK_LEFTSIDE )
		{
			m_NavButtonStateId--;
		}
		else
		{
			return;
		}

		if ( m_NavButtonStateId < 0 )
			m_NavButtonStateId = 0;

		if ( m_NavButtonStateId > m_NavButtonWordList.Count() - 1 )
			m_NavButtonStateId = m_NavButtonWordList.Count() - 1;

		m_NavButton.SetButton(m_NavButtonWordList[m_NavButtonStateId]);
	}

	void OnClick_EditableVector( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if ( !player || !player.GetIdentity() )
			return;

		if( !m_PlayerModule )
		{
			if (!Class.CastTo(m_PlayerModule, GetModuleManager().GetModule(JMPlayerModule)))
				return;
		}

		m_PlayerModule.TeleportTo( action.GetValue(), { player.GetIdentity().GetId() } );
	}

	void OnClick_Dropdown( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Example.Dropdown" ) )
			return;

		//string word = action.GetText();

		//m_Module.WebHookExample( word );
	}

	void OnClick_Button( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Example.Button" ) )
			return;

		CreateConfirmation_Two( JMConfirmationType.INFO, "Are you sure ?", "Kick Selected Player(s)", "#STR_COT_GENERIC_YES", "OnFunnyButtons_01", "#STR_COT_GENERIC_YES", "OnFunnyButtons_01" );
	}

	void OnFunnyButtons_01()
	{
		CreateConfirmation_Two( JMConfirmationType.INFO, "Are you sure ?", "It will kick you included", "#STR_COT_GENERIC_YES", "OnFunnyButtons_02", "#STR_COT_GENERIC_YES", "OnFunnyButtons_02" );
	}

	void OnFunnyButtons_02()
	{
		CreateConfirmation_Two( JMConfirmationType.INFO, "Please think twice", "You cannot undo this action !", "#STR_COT_GENERIC_YES", "OnConfirmationButton", "#STR_COT_GENERIC_YES", "OnConfirmationButton" );
	}

	void OnConfirmationButton()
	{
		array< string > players = JM_GetSelected().GetPlayers();
		if ( players.Count() == 0 )
		{
			PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			if ( player.GetIdentity() )
				players.Insert(player.GetIdentity().GetId());
		}

		m_PlayerModule.Kick( players, "I am so sorry :c" );
	}
};
