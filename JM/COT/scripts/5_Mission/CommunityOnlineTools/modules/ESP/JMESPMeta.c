class JMESPMeta : Managed
{
	string name;
	JMESPViewType type;
	int colour;
	JMPlayerInstance player;
	Object target;

	JMESPWidget widget;

	void Create()
	{
		Print( "JMESPMeta::Create - " + target );

		Widget w = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp_widget.layout" );

		if ( w == NULL )
			return;

		w.GetScript( widget );

		if ( widget == NULL )
			return;

		widget.SetInfo( this );
	}

	bool IsValid()
	{
		if ( target == NULL )
			return false;

		return type.IsValid( target, this );
	}

	void Destroy()
	{
		Print( "JMESPMeta::Destroy - " + target );
		
		if ( widget )
			widget.Unlink();
	}
}