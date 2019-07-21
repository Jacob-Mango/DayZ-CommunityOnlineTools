class JMFormBase extends ScriptedWidgetEventHandler 
{
	protected ref Widget layoutRoot;
	
	ref JMWindowBase window;

	ref JMRenderableModuleBase module;

	bool HasBeenInitialized;

	void JMFormBase() 
	{
		HasBeenInitialized = false;
	}

	void ~JMFormBase() 
	{
		delete window;

		delete layoutRoot;
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
	}

	void Init( bool fromMenu )
	{
		OnInit( fromMenu );
		HasBeenInitialized = true;
	}

	void OnInit( bool fromMenu )
	{
		
	}

	bool IsVisible()
	{
		return window.IsVisible();
	}

	void Show()
	{
		if ( HasBeenInitialized )
		{
			window.Show();
			layoutRoot.Show( true );

			OnShow();
		}
	}

	void SetSize( float w, float h )
	{
		window.SetSize( w, h );
	}

	void Hide()
	{
		OnHide();
		layoutRoot.Show( false );
		window.Hide();
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	override bool OnUpdate( Widget w )
	{
		Update();

		super.OnUpdate( w );

		return true;
	}

	void Update() 
	{
		
	}

	ref Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	string GetTitle()
	{
		return "";
	}

	string GetImageSet()
	{
		return "";
	}

	string GetIconName()
	{
		return "";
	}

	bool ImageIsIcon()
	{
		return false;
	}
}