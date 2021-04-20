modded class ChatInputMenu
{
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (!finished) return false;
		
		string text = m_edit_box.GetText();
		if (text != "" && (text.Substring(0, 1)) == "/")
		{
			text = text.Substring(1, text.Length() - 1);

			JMCommandModule mod;
			if ( Class.CastTo( mod, GetModuleManager().GetModule( JMCommandModule ) ) )
			{
				mod.PerformCommand(text);
			}
		
			m_close_timer.Run(0.1, this, "Close");
			
			GetUApi().GetInputByName("UAPersonView").Supress();	

			return true;
		}
		
		return super.OnChange(w, x, y, finished);
	}
};