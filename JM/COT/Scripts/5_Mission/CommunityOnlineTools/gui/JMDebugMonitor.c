//! Legacy, not used, only kept for compatibility with 3rd party mods
class JMDebugMonitor
{
	protected Widget layoutRoot;
	protected TextWidget WindowLabelText;
	protected TextListboxWidget NamesListboxWidget;
	protected TextListboxWidget ValuesListboxWidget;
	protected MultilineTextWidget ModifiersMultiTextWidget;

	void SetBlood(float value);

	void SetHealth(float value);

	void SetLastDamage(string value);

	void SetPosition(vector value);

	void Init();

	void Hide();

	void Show();
}