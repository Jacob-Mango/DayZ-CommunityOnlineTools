modded class PluginDeveloper
{
	override void TeleportAtCursor()
	{
#ifdef DEVELOPER
		super.TeleportAtCursor();
#endif
	}
	
	override void ToggleFreeCameraBackPos()
	{
#ifdef DEVELOPER
		auto mission = MissionBaseWorld.Cast(GetGame().GetMission());
		if (mission)
			mission.COT_LeaveFreeCam();

		super.ToggleFreeCameraBackPos();
#endif
	}
	
	override void ToggleFreeCamera()
	{
#ifdef DEVELOPER
		auto mission = MissionBaseWorld.Cast(GetGame().GetMission());
		if (mission)
			mission.COT_LeaveFreeCam();

		super.ToggleFreeCamera();
#endif
	}
};
