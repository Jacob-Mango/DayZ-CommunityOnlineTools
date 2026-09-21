// =============================================================================
//  JMLootCEEditRow
//
//  Four editable CE fields - Nominal, Min as plain integer text, Lifetime and
//  Restock as HH:MM:SS time pickers - inside a card's content grid, next to
//  the read-only Markers/Tiers rows a UIActionKeyValueList already shows. Its
//  own small class rather than four more fields duplicated once per tab
//  (Item Scan and Distribution each have one), the way JMLootScanItemDetail
//  is its own class instead of four more parallel arrays.
//
//  Editing is local to the fields until the card's Save button (built by the
//  form, not here - it lives in the card header, not the content) reads
//  ReadValues() and sends it to the server. This class holds no permission
//  logic of its own; the form gates the Save button with RegisterPermission
//  the same way every other destructive action here is gated.
// =============================================================================
class JMLootCEEditRow
{
	protected Widget m_Wrapper;
	protected UIActionEditableText m_NominalEdit;
	protected UIActionEditableText m_MinEdit;
	protected UIActionTimePicker   m_LifetimeEdit;
	protected UIActionTimePicker   m_RestockEdit;

	void JMLootCEEditRow(notnull Widget parent)
	{
		m_Wrapper = UIActionManager.CreateGridSpacer(parent, 4, 1);

		m_NominalEdit  = UIActionManager.CreateEditableText(m_Wrapper, "#STR_COT_LOOTANALYSIS_NOMINAL", NULL, "", "");
		m_MinEdit      = UIActionManager.CreateEditableText(m_Wrapper, "#STR_COT_GENERIC_MIN",     NULL, "", "");

		//! HH:MM:SS instead of a raw seconds textbox - CE lifetime/restock are
		//! durations, and typing "259200" to mean 3 days is exactly what the
		//! ban-duration picker already exists to avoid.
		m_LifetimeEdit = UIActionManager.CreateTimePicker(m_Wrapper, "Lifetime");
		m_RestockEdit  = UIActionManager.CreateTimePicker(m_Wrapper, "Restock");

		//! Nothing to edit until a scan/lookup actually answers with CE data.
		m_Wrapper.Show(false);
	}

	//! Repaint from a fresh server answer - always overwrites whatever the
	//! admin was mid-typing, the same way picking a different row anywhere
	//! else in COT drops an unsaved edit. There is no "dirty" state to
	//! protect: Save reads the fields directly, so nothing is lost UNLESS a
	//! new item's data arrives before Save is pressed.
	void SetValues(JMLootItemTypeInfo info)
	{
		bool found = (info != NULL && info.m_Found);

		if (m_Wrapper)
			m_Wrapper.Show(found);

		if (!found)
			return;

		m_NominalEdit.SetText(info.m_Nominal.ToString());
		m_MinEdit.SetText(info.m_Min.ToString());
		m_LifetimeEdit.SetTotalSeconds(Math.Max(0, info.m_Lifetime));
		m_RestockEdit.SetTotalSeconds(Math.Max(0, info.m_Restock));
	}

	//! Values to send on Save. Nominal/Min: blank or non-numeric text falls
	//! back to `current`'s own value, rather than sending a zero a stray
	//! keystroke produced. Lifetime/Restock have no such failure mode - the
	//! time picker only ever holds a clamped non-negative total.
	void ReadValues(JMLootItemTypeInfo current, out int nominal, out int min, out int lifetime, out int restock)
	{
		nominal  = ParseOrFallback(m_NominalEdit.GetText(), current.m_Nominal);
		min      = ParseOrFallback(m_MinEdit.GetText(), current.m_Min);
		lifetime = m_LifetimeEdit.GetTotalSeconds();
		restock  = m_RestockEdit.GetTotalSeconds();
	}

	//! Enforce's string has no IsNumeric() - round-tripping through ToInt()
	//! is the cheap substitute: a string that was really an integer comes
	//! back identical, anything else (garbage, empty, "12abc") does not and
	//! falls back instead of silently becoming 0.
	protected int ParseOrFallback(string text, int fallback)
	{
		text.Trim();

		if (text == "")
			return fallback;

		int parsed = text.ToInt();
		if (parsed.ToString() != text)
			return fallback;

		return parsed;
	}
}
