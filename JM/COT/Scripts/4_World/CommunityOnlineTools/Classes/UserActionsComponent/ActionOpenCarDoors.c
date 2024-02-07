modded class ActionOpenCarDoors
{
	override void OnEnd(ActionData action_data)
	{
		super.OnEnd(action_data);

		if (CurrentActiveCamera && action_data.m_Player.COT_IsLeavingFreeCam())
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(action_data.m_Player.COT_GetOutVehicle);
	}
}
