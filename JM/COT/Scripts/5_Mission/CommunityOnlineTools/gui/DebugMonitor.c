modded class DebugMonitor
{
	override void Show()
	{
		if (!GetCommunityOnlineToolsBase().IsOpen())
			super.Show();
	}
}
