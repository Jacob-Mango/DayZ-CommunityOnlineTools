modded class JMMapForm
{
	/*
	override void OnInit()
	{
		super.OnInit();
		
		// As usual... not this simple. Resizing map fucks up the scrolling in-game, map scale doesn't update automatically?
		// FUCK ME. I regret buying three 4k monitors. If you're reading this and haven't got one yet - don't.
		
		int screenWidth, screenHeight;
		GetScreenSize(screenWidth, screenHeight);

		if (screenWidth > 3800) // 4k enhance map size so it's not tiny
		{
			float width, height;
			
			layoutRoot.GetSize(width, height);
			layoutRoot.SetSize(width * 2, height * 2);
			
			m_MapWidget.GetSize(width, height);
			m_MapWidget.SetSize(width * 2, height * 2);
		}
	}
	*/

	override void UpdateMapMarkers()
	{
		super.UpdateMapMarkers();

		auto temp = m_Module.GetTempItemMarks();
		if (!temp)
			return;

		foreach (JMMapTempMark m : temp)
		{
			m_MapWidget.AddUserMark(m.m_Pos, m.m_Label, m.m_Color, m.m_Icon);
		}
	}
}