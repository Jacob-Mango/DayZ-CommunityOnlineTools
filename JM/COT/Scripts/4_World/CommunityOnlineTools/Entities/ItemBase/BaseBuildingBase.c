modded class BaseBuildingBase
{
	ref ScriptInvoker m_COT_ConstructionUpdate;

	void BaseBuildingBase()
	{
		m_COT_ConstructionUpdate = new ScriptInvoker();
	}

	override void OnPartBuiltClient( string part_name, int action_id )
	{
		super.OnPartBuiltClient( part_name, action_id );

		m_COT_ConstructionUpdate.Invoke();
	}

	override void OnPartDismantledClient( string part_name, int action_id )
	{
		super.OnPartDismantledClient( part_name, action_id );

		m_COT_ConstructionUpdate.Invoke();
	}

	override void OnPartDestroyedClient( string part_name, int action_id )
	{
		super.OnPartDestroyedClient( part_name, action_id );

		m_COT_ConstructionUpdate.Invoke();
	}

	override void EEItemAttached( EntityAI item, string slot_name )
	{
		super.EEItemAttached( item, slot_name );

		if ( IsMissionClient() )
			m_COT_ConstructionUpdate.Invoke();
	}

	override void EEItemDetached( EntityAI item, string slot_name )
	{
		super.EEItemDetached( item, slot_name );

		if ( IsMissionClient() )
			m_COT_ConstructionUpdate.Invoke();
	}
};