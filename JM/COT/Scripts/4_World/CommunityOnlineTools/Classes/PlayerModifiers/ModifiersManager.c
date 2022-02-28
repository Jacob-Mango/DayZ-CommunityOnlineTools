modded class ModifiersManager
{
	override void ActivateModifier(int modifier_id, bool triggerEvent = EActivationType.TRIGGER_EVENT_ON_ACTIVATION)
	{
        if ( !m_Player.COTHasGodMode() )
		    super.ActivateModifier(modifier_id, triggerEvent);
	}
};