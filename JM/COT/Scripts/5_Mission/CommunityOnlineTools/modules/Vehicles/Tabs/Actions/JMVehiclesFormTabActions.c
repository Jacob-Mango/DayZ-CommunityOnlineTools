//! "Actions" tab of JMVehiclesForm - the per-vehicle command buttons (repair,
//! refuel, teleport, delete ...) for whichever vehicle is currently selected.
//! Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
//!
//! The selection (m_CurrentVehicle) and the JMConfirmation callbacks stay on
//! the form: a confirmation calls its callback on the form, and the roster's
//! bulk deletes and the map's marker menu share the same delete flow.
class JMVehiclesFormTabActions: JMFormTab
{
	protected JMVehiclesForm m_Form;
	protected UIActionScroller m_ScrollerActions;
	protected UIActionConfirmInline m_DeleteVehicleButton;
	protected UIActionButton m_RepairVehicleButton;
	protected UIActionButton m_RefuelVehicleButton;
	protected UIActionButton m_UnstuckVehicleButton;
	protected UIActionButton m_CoverVehicleButton;
	protected UIActionButton m_LockVehicleButton;
	protected UIActionButton m_UnPairVehicleButton;
	protected UIActionButton m_TeleportVehicleButton;
	protected UIActionButton m_TeleportMeButton;

	void JMVehiclesFormTabActions( JMVehiclesForm form )
	{
		m_Form = form;
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ScrollerActions = UIActionManager.CreateScroller( panel );
		Widget optContent = m_ScrollerActions.GetContentWidget();

		UIActionCard card = UIActionManager.CreateCard( optContent, "#STR_COT_VEHICLE_OPTIONS" );
		Widget gridOptA = UIActionManager.CreateGridSpacer( card.GetContent(), 9, 1 );
			m_DeleteVehicleButton = UIActionManager.CreateConfirmInline( gridOptA, "#STR_COT_GENERIC_DELETE", this, "OnClick_DeleteVehicle" );
			UIActionIconGrid.ApplyDeletePreset( m_DeleteVehicleButton );
			m_DeleteVehicleButton.SetTooltip( "#STR_COT_VEHICLES_REMOVE_THIS_VEHICLE_FROM_THE_WORLD" );
			m_RepairVehicleButton   = UIActionManager.CreateButton( gridOptA, "#STR_COT_VEHICLES_ACTION_REPAIR", this, "" );
			if ( m_RepairVehicleButton ) m_RepairVehicleButton.SetOnClick( this, "OnClick_RepairVehicle" );
			m_RepairVehicleButton.SetTooltip( "#STR_COT_VEHICLES_RESTORE_HEALTH_AND_REPLACE_MISSING_ATTAC" );
			m_RefuelVehicleButton   = UIActionManager.CreateButton( gridOptA, "#STR_COT_VEHICLES_ACTION_REFUEL", this, "" );
			if ( m_RefuelVehicleButton ) m_RefuelVehicleButton.SetOnClick( this, "OnClick_RefuelVehicle" );
			m_RefuelVehicleButton.SetTooltip( "#STR_COT_VEHICLES_FILL_FUEL_OIL_BRAKE_AND_COOLANT" );
			m_UnstuckVehicleButton  = UIActionManager.CreateButton( gridOptA, "#STR_COT_VEHICLES_ACTION_UNSTUCK", this, "" );
			if ( m_UnstuckVehicleButton ) m_UnstuckVehicleButton.SetOnClick( this, "OnClick_UnstuckVehicle" );
			m_UnstuckVehicleButton.SetTooltip( "#STR_COT_VEHICLES_LIFT_1_5_M_AND_DROP" );
			m_CoverVehicleButton    = UIActionManager.CreateButton( gridOptA, "#STR_COT_VEHICLES_ACTION_COVER_UNCOVER", this, "" );
			if ( m_CoverVehicleButton ) m_CoverVehicleButton.SetOnClick( this, "OnClick_CoverVehicle" );
			m_CoverVehicleButton.SetTooltip( "#STR_COT_VEHICLES_TOGGLE_EXPANSION_VEHICLE_COVER" );
			m_LockVehicleButton     = UIActionManager.CreateButton( gridOptA, "#STR_COT_VEHICLES_ACTION_LOCK_UNLOCK", this, "" );
			if ( m_LockVehicleButton ) m_LockVehicleButton.SetOnClick( this, "OnClick_LockVehicle" );
			m_LockVehicleButton.SetTooltip( "#STR_COT_VEHICLES_TOGGLE_THE_KEY_LOCK_STATE" );
			m_UnPairVehicleButton   = UIActionManager.CreateButton( gridOptA, "#STR_COT_VEHICLES_ACTION_UNPAIR_KEYS", this, "" );
			if ( m_UnPairVehicleButton ) m_UnPairVehicleButton.SetOnClick( this, "OnClick_UnPairVehicle" );
			m_UnPairVehicleButton.SetTooltip( "#STR_COT_VEHICLES_DETACH_ALL_PAIRED_KEYS_FROM_THIS" );
			m_TeleportVehicleButton = UIActionManager.CreateButton( gridOptA, "#STR_COT_VEHICLES_ACTION_TELEPORT_TO_VEHICLE", this, "" );
			if ( m_TeleportVehicleButton ) m_TeleportVehicleButton.SetOnClick( this, "OnClick_TeleportToVehicle" );
			m_TeleportVehicleButton.SetTooltip( "#STR_COT_VEHICLES_TELEPORT_YOURSELF_TO_THIS_VEHICLE_S" );
			m_TeleportMeButton      = UIActionManager.CreateButton( gridOptA, "#STR_COT_VEHICLES_ACTION_TELEPORT_VEHICLE_TO_ME", this, "" );
			if ( m_TeleportMeButton ) m_TeleportMeButton.SetOnClick( this, "OnClick_TeleportVehicleToMe" );
			m_TeleportMeButton.SetTooltip( "#STR_COT_VEHICLES_MOVE_THIS_VEHICLE_TO_YOUR_CURRENT" );

		m_Form.BindPermission( m_DeleteVehicleButton,   JMConstants.PERM_VEHICLES_DELETE );
		m_Form.BindPermission( m_RepairVehicleButton,   JMConstants.PERM_VEHICLES_REPAIR );
		m_Form.BindPermission( m_RefuelVehicleButton,   JMConstants.PERM_VEHICLES_REFUEL );
		m_Form.BindPermission( m_UnstuckVehicleButton,  JMConstants.PERM_VEHICLES_UNSTUCK );
		m_Form.BindPermission( m_CoverVehicleButton,    JMConstants.PERM_VEHICLES_COVER );
		m_Form.BindPermission( m_LockVehicleButton,     JMConstants.PERM_VEHICLES_LOCK );
		m_Form.BindPermission( m_UnPairVehicleButton,   JMConstants.PERM_VEHICLES_UNPAIR );
		m_Form.BindPermission( m_TeleportVehicleButton, JMConstants.PERM_VEHICLES_TELEPORT );
		m_Form.BindPermission( m_TeleportMeButton,      JMConstants.PERM_VEHICLES_TELEPORT );

	#ifndef EXPANSIONMODVEHICLE
		// Cover, Lock, UnPair are Expansion-only - disable them when Expansion is not loaded
		if ( m_CoverVehicleButton   ) m_CoverVehicleButton.SetEnabled( false );
		if ( m_LockVehicleButton    ) m_LockVehicleButton.SetEnabled( false );
		if ( m_UnPairVehicleButton  ) m_UnPairVehicleButton.SetEnabled( false );
	#endif

		m_ScrollerActions.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		UpdateScroller();
	}

	void UpdateScroller()
	{
		if ( m_ScrollerActions )
			m_ScrollerActions.UpdateScroller();
	}

	void OnClick_TeleportToVehicle( UIActionBase action )
	{
		if ( m_Form.GetCurrentVehicle() )
			m_Form.GetModule().RequestTeleportToVehicle( m_Form.GetCurrentVehicle() );
	}

	void OnClick_TeleportVehicleToMe( UIActionBase action )
	{
		if ( m_Form.GetCurrentVehicle() )
			m_Form.GetModule().RequestTeleportVehicleToMe( m_Form.GetCurrentVehicle() );
	}

	void OnClick_RepairVehicle( UIActionBase action )
	{
		if ( m_Form.GetCurrentVehicle() )
			m_Form.GetModule().RequestRepairVehicle( m_Form.GetCurrentVehicle() );
	}

	void OnClick_RefuelVehicle( UIActionBase action )
	{
		if ( m_Form.GetCurrentVehicle() )
			m_Form.GetModule().RequestRefuelVehicle( m_Form.GetCurrentVehicle() );
	}

	void OnClick_UnstuckVehicle( UIActionBase action )
	{
		if ( m_Form.GetCurrentVehicle() )
			m_Form.GetModule().RequestUnstuckVehicle( m_Form.GetCurrentVehicle() );
	}

	void OnClick_CoverVehicle( UIActionBase action )
	{
		if ( m_Form.GetCurrentVehicle() )
			m_Form.GetModule().RequestCoverVehicle( m_Form.GetCurrentVehicle() );
	}

	void OnClick_LockVehicle( UIActionBase action )
	{
		if ( m_Form.GetCurrentVehicle() )
			m_Form.GetModule().RequestLockVehicle( m_Form.GetCurrentVehicle() );
	}

	void OnClick_UnPairVehicle( UIActionBase action )
	{
		if ( m_Form.GetCurrentVehicle() )
			m_Form.GetModule().RequestUnPairVehicle( m_Form.GetCurrentVehicle() );
	}

	void OnClick_DeleteVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_Form.GetCurrentVehicle() )
			m_Form.RequestDeleteVehicle( m_Form.GetCurrentVehicle() );
	}
}
