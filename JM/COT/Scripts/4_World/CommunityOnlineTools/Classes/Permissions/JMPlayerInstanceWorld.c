#ifdef CF_MODULE_PERMISSIONS
modded class CF_Permission_Player
{
	override bool OnUpdateData()
	{
		if ( !super.OnUpdateData() )
			return false;

		m_Position = PBObject.GetPosition();
		m_Orientation = PBObject.GetOrientation();
	
		m_Health = PBObject.GetHealth( "GlobalHealth","Health" );
		m_Blood = PBObject.GetHealth( "GlobalHealth", "Blood" );
		m_Shock = PBObject.GetHealth( "GlobalHealth", "Shock" );

		m_BloodStatType = PBObject.GetStatBloodType().Get();

		m_Energy = PBObject.GetStatEnergy().Get();
		m_Water = PBObject.GetStatWater().Get();
		m_HeatComfort = PBObject.GetStatHeatComfort().Get();
		m_HeatBuffer = PBObject.GetStatHeatBuffer().Get();
		m_Wet = PBObject.GetStatWet().Get();
		m_Tremor = PBObject.GetStatTremor().Get();
		m_Stamina = PBObject.GetStatStamina().Get();
		m_LifeSpanState = PBObject.GetLifeSpanState();
		m_BloodyHands = PBObject.HasBloodyHands();
		m_GodMode = PBObject.COTHasGodMode();
		m_Frozen = PBObject.COTIsFrozen();
		m_Invisibility = PBObject.COTIsInvisible();
		m_UnlimitedAmmo = PBObject.COTHasUnlimitedAmmo();
		m_ReceiveDmgDealt = PBObject.COTGetReceiveDamageDealt();
		m_CannotBeTargetedByAI = PBObject.COTGetCannotBeTargetedByAI();
		m_RemoveCollision = PBObject.COTGetRemoveCollision();

		return true;
	}
};
#endif