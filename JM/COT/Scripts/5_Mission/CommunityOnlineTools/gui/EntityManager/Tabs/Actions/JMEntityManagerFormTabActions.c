//! "Actions" tab of JMEntityManagerForm - one button per per-entity action the
//! adapter exposes, for whichever entity is currently selected. Back-reference
//! to the owning form, same shape as JMPlayerRowWidget.Menu.
//!
//! The action descriptors (JMEntityAction) are owned by the form: the roster's
//! bulk buttons hold weak UserData refs into the same array.
class JMEntityManagerFormTabActions: JMFormTab
{
	protected JMEntityManagerForm m_Form;
	protected UIActionScroller m_OptionsScroller;
	protected ref map<string, UIActionBase> m_ActionButtons;
	protected UIActionButton m_ReturnButton;

	void JMEntityManagerFormTabActions( JMEntityManagerForm form )
	{
		m_Form = form;

		m_ActionButtons = new map<string, UIActionBase>;
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		// The form's action list was filled in its OnCreate and the bulk buttons
		// hold weak UserData refs into it - do not clear or refetch it here.
		array<ref JMEntityAction> actions = m_Form.GetActions();

		m_OptionsScroller = UIActionManager.CreateScroller( panel );
		Widget optContent = m_OptionsScroller.GetContentWidget();

		int perEntityCount = 0;
		foreach ( JMEntityAction ca: actions )
		{
			if ( !ca.m_IsBulk ) perEntityCount++;
		}

		UIActionCard optCard = UIActionManager.CreateCard( optContent, "#STR_COT_VEHICLE_OPTIONS" );
		Widget optGrid = UIActionManager.CreateGridSpacer( optCard.GetContent(), perEntityCount + 1, 1 );
		foreach ( JMEntityAction act: actions )
		{
			if ( act.m_IsBulk )
				continue;

			//! Every adapter's actions carry their own permission key, and the
			//! server already refuses one the caller does not hold
			//! (JMEntityManagerModule). Binding here means the button reflects
			//! that instead of failing silently on click. An empty key means
			//! the action is deliberately ungated - do not bind it, or
			//! HasPermission("") would disable it for everyone.
			if ( act.m_IsDestructive )
			{
				UIActionConfirmInline btn = UIActionManager.CreateConfirmInline( optGrid, act.m_Label, this, "OnEntityClick" );
				UIActionIconGrid.ApplyDeletePreset( btn );
				btn.SetUserData( act );
				if ( act.m_Tooltip != "" )
					btn.SetTooltip( act.m_Tooltip );
				if ( act.m_Permission != "" )
					m_Form.UpdatePermission( btn, act.m_Permission );
				m_ActionButtons.Set( act.m_Id, btn );
			}
			else
			{
				UIActionButton btn2 = UIActionManager.CreateButton( optGrid, act.m_Label, this, "OnEntityClick" );
				btn2.SetUserData( act );
				if ( act.m_Tooltip != "" )
					btn2.SetTooltip( act.m_Tooltip );
				if ( act.m_Permission != "" )
					m_Form.UpdatePermission( btn2, act.m_Permission );
				m_ActionButtons.Set( act.m_Id, btn2 );
			}
		}

		Widget retGrid = UIActionManager.CreateGridSpacer( optCard.GetContent(), 1, 1 );
		m_ReturnButton = UIActionManager.CreateButton( retGrid, "#STR_COT_ENTITYMANAGER_RETURN", this, "" );
		if ( m_ReturnButton ) m_ReturnButton.SetOnClick( this, "OnClick_Return" );
		m_ReturnButton.SetTooltip( "#STR_COT_ENTITYMANAGER_GO_BACK_TO_THE_MAP" );

		m_OptionsScroller.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		UpdateScroller();
	}

	void UpdateScroller()
	{
		if ( m_OptionsScroller )
			m_OptionsScroller.UpdateScroller();
	}

	void OnClick_Return( UIActionBase action )
	{
		m_Form.BackToList();
	}

	// Dispatched from every per-entity action button.
	void OnEntityClick( UIEvent eid, UIActionBase action )
	{
		Class data;
		action.GetUserData( data );

		JMEntityAction act;
		if ( !Class.CastTo( act, data ) )
			return;

		// Destructive = ConfirmInline fires CHANGE on confirm; others fire CLICK.
		if ( act.m_IsDestructive && eid != UIEvent.CHANGE )
			return;
		if ( !act.m_IsDestructive && eid != UIEvent.CLICK )
			return;

		if ( !m_Form.GetCurrentEntity() )
			return;

		m_Form.GetModule().RequestAction( act.m_Id, m_Form.GetCurrentEntity() );

		// Destructive actions return to list by default.
		if ( act.m_IsDestructive )
			m_Form.BackToList();
	}
}
