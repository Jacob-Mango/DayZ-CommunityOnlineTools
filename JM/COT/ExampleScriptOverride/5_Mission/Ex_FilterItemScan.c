#ifdef JM_CommunityOnlineTools
// Example: hooking into a form's own init - JMItemStatsForm has no category
// filter menu to extend (see Ex_FilterObjectSpawner.c / Ex_FilterTeleportMenu.c
// for that pattern against a form that does), so this just shows the hook.
modded class JMItemStatsForm
{
	override void OnInit()
	{
		super.OnInit();
	}
}
#endif
