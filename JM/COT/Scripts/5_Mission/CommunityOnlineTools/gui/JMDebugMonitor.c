//! Legacy, not used, only kept for compatibility with 3rd party mods
class JMDebugMonitor
{
	protected Widget layoutRoot;
	protected TextWidget WindowLabelText;
	protected TextListboxWidget NamesListboxWidget;
	protected TextListboxWidget ValuesListboxWidget;
	protected MultilineTextWidget ModifiersMultiTextWidget;
	
	void Init();

	void SetHealth(float value);

	void SetBlood(float value);
	
	void SetLastDamage(string value);
	
	void SetPosition(vector value);

	void Hide();

	void Show();
}