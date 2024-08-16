#ifdef CF_MODULE_PERMISSIONS
modded class CF_Permission_PlayerBase
{
	protected int m_DataLastUpdated;

	protected vector m_Position;
	protected vector m_Orientation;

	protected float m_Health;
	protected float m_Blood;
	protected float m_Shock;

	protected int m_BloodStatType;

	protected float m_Energy;
	protected float m_Water;

	protected float m_HeatComfort;
	protected float m_HeatBuffer;	

	protected float m_Wet;
	protected float m_Tremor;
	protected float m_Stamina;

	protected int m_LifeSpanState;
	protected bool m_BloodyHands;
	protected bool m_GodMode;
	protected bool m_Frozen;
	protected bool m_Invisibility;
	protected bool m_UnlimitedAmmo;
	protected bool m_ReceiveDmgDealt;
	protected bool m_CannotBeTargetedByAI;
	protected bool m_RemoveCollision;

	override void OnSend( ParamsWriteContext ctx )
	{
		super.OnSend( ctx );

		OnSendPosition( ctx );
		OnSendOrientation( ctx );
		OnSendHealth( ctx );
	}
	
	override void OnRecieve( ParamsReadContext ctx )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnRecieve");
		#endif

		super.OnRecieve( ctx );
	
		OnRecievePosition( ctx );
		OnRecieveOrientation( ctx );
		OnRecieveHealth( ctx );

		m_DataLastUpdated = GetGame().GetTime();
	}
	
	void OnSendPosition( ParamsWriteContext ctx )
	{
		ctx.Write( m_Position );
	}

	void OnRecievePosition( ParamsReadContext ctx )
	{
		ctx.Read( m_Position );
	}
	
	void OnSendOrientation( ParamsWriteContext ctx )
	{
		ctx.Write( m_Orientation );
	}

	void OnRecieveOrientation( ParamsReadContext ctx )
	{
		ctx.Read( m_Orientation );
	}
	
	void OnSendHealth( ParamsWriteContext ctx )
	{
		ctx.Write( m_Health );
		ctx.Write( m_Blood );
		ctx.Write( m_Shock );
		ctx.Write( m_BloodStatType );
		ctx.Write( m_Energy );
		ctx.Write( m_Water );
		ctx.Write( m_HeatComfort );
		ctx.Write( m_HeatBuffer );
		ctx.Write( m_Wet );
		ctx.Write( m_Tremor );
		ctx.Write( m_Stamina );
		ctx.Write( m_LifeSpanState );
		ctx.Write( m_BloodyHands );
		ctx.Write( m_GodMode );
		ctx.Write( m_Frozen );
		ctx.Write( m_Invisibility );
		ctx.Write( m_UnlimitedAmmo );
		ctx.Write( m_ReceiveDmgDealt );
		ctx.Write( m_CannotBeTargetedByAI );
		ctx.Write( m_RemoveCollision );
	}

	void OnRecieveHealth( ParamsReadContext ctx )
	{
		ctx.Read( m_Health );
		ctx.Read( m_Blood );
		ctx.Read( m_Shock );
		ctx.Read( m_BloodStatType );
		ctx.Read( m_Energy );
		ctx.Read( m_Water );
		ctx.Read( m_HeatComfort );
		ctx.Read( m_HeatBuffer );
		ctx.Read( m_Wet );
		ctx.Read( m_Tremor );
		ctx.Read( m_Stamina );
		ctx.Read( m_LifeSpanState );
		ctx.Read( m_BloodyHands );
		ctx.Read( m_GodMode );
		ctx.Read( m_Frozen );
		ctx.Read( m_Invisibility );
		ctx.Read( m_UnlimitedAmmo );
		ctx.Read( m_ReceiveDmgDealt );
		ctx.Read( m_CannotBeTargetedByAI );
		ctx.Read( m_RemoveCollision );
	}

	int GetDataLastUpdatedTime()
	{
		return m_DataLastUpdated;
	}

	vector GetPosition()
	{
		return m_Position;
	}

	vector GetOrientation()
	{
		return m_Orientation;
	}

	float GetHealth()
	{
		return m_Health;
	}

	float GetBlood()
	{
		return m_Blood;
	}

	float GetShock()
	{
		return m_Shock;
	}

	int GetBloodStatType()
	{
		return m_BloodStatType;
	}

	float GetEnergy()
	{
		return m_Energy;
	}

	float GetWater()
	{
		return m_Water;
	}

	float GetHeatComfort()
	{
		return m_HeatComfort;
	}

	float GetHeatBuffer()
	{
		return m_HeatBuffer;
	}

	float GetWet()
	{
		return m_Wet;
	}

	float GetTremor()
	{
		return m_Tremor;
	}

	float GetStamina()
	{
		return m_Stamina;
	}

	int GetLifeSpanState()
	{
		return m_LifeSpanState;
	}

	bool HasBloodyHands()
	{
		return m_BloodyHands;
	}

	bool HasGodMode()
	{
		return m_GodMode;
	}

	bool IsFrozen()
	{
		return m_Frozen;
	}

	bool HasInvisibility()
	{
		return m_Invisibility;
	}

	bool HasUnlimitedAmmo()
	{
		return m_UnlimitedAmmo;
	}

	bool GetReceiveDmgDealt()
	{
		return m_ReceiveDmgDealt;
	}

	bool GetCannotBeTargetedByAI()
	{
		return m_CannotBeTargetedByAI;
	}

	bool GetRemoveCollision()
	{
		return m_RemoveCollision;
	}
};
#endif