class JMRenderableModuleBase extends JMModuleBase
{
	ref JMFormBase form;
	ref ButtonWidget menuButton;

	JMFormBase GetForm()
	{
		return form;
	}

	void DeleteForm()
	{
		delete form;
	}

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
		if ( !form ) return false;
		if ( !form.GetLayoutRoot() ) return false;
		
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