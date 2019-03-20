class EditorModule extends Module
{
	ref Form form;
	ref ButtonWidget menuButton;

	string GetLayoutRoot()
	{
		return "";
	}

	bool HasAccess()
	{
		return true;
	}

	bool HasButton()
	{
		return true;
	}

	bool IsVisible()
	{
		return form.GetLayoutRoot().IsVisible();
	}

	void Show( bool reset = false )
	{
		if ( HasAccess() )
		{
			form.Show();

			if ( reset )
			{
				form.window.SetPosition( 0, 0 );
			}
		}
	}

	void Hide()
	{
		if ( IsVisible() )
		{
			form.Hide();
		}
	}

	void ToggleShow( bool reset = false )
	{
		if ( IsVisible() ) 
		{
			form.Hide();
		}
		else if ( HasAccess() )
		{
			form.Show();

			if ( reset )
			{
				form.window.SetPosition( 0, 0 );
			}
		}
	}
}