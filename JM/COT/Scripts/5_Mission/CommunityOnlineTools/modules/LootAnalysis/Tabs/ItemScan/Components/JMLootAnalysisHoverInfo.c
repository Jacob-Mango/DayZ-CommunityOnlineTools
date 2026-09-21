// =============================================================================
//  JMLootAnalysisHoverInfo
//
//  The floating readout the Item Scan map shows while the cursor is over a
//  marker: a model preview plus health, quantity, remaining lifetime and
//  attachments - modelled directly on JMVehiclesHoverInfo, minus the parts
//  that only make sense for a live-polled vehicle (fuel/coolant/speed/lock/
//  crew, and the periodic re-fetch that keeps those current). A scanned
//  item's numbers are a snapshot from the scan itself, not something worth
//  polling the server for on every hover.
//
//  Not parented to the map panel, for the same reason JMVehiclesHoverInfo
//  is not: the map clips its children, and a marker near the edge would clip
//  the panel with it. Anchored to the window root instead.
// =============================================================================
class JMLootAnalysisHoverInfo
{
	protected Widget m_Anchor;
	protected Widget m_Root;
	protected TextWidget m_Title;
	protected TextWidget m_HealthValue;
	protected TextWidget m_QuantityLabel;
	protected TextWidget m_QuantityValue;
	protected TextWidget m_LifetimeValue;
	protected Widget     m_AttachmentsRow;
	protected ItemPreviewWidget m_Preview;

	//! Local, client-only entity built purely to feed the preview widget (see
	//! JMLocalPreview), plus the key it was built from - class, attachments and the
	//! state of each - so re-hovering the same result is free but a different
	//! health/quantity/food stage is never served a stale model.
	protected EntityAI      m_PreviewItem;
	protected string        m_PreviewKey;

	//! One throwaway entity per attachment icon in m_AttachmentsRow - unlike
	//! m_PreviewItem these are never reused across hovers (a fresh set is
	//! cheap: rebuilt only on hover-enter, not per frame), so ShowAt always
	//! clears and rebuilds this list rather than diffing it.
	protected ref array<EntityAI> m_AttachmentPreviewItems;
	protected string m_ShownMarkerId;
	protected bool   m_Open;
	static const float PANEL_W = 300;
	static const float PANEL_H = 288;
	static const string ATTACHMENT_ICON_LAYOUT = "JM/COT/GUI/layouts/loot_analysis_attachment_icon.layout";

	//! Same bounds as JMVehiclesHoverInfo's PANEL_SORT, and the same reason:
	//! above the map but below the ItemPreviewWidget's own fixed priority.
	static const int PANEL_SORT = 120;
	static const float CURSOR_OFFSET_X = 18;
	static const float CURSOR_OFFSET_Y = 18;
	static const float LEVEL_WARN = 0.5;
	static const float LEVEL_BAD  = 0.2;

	void JMLootAnalysisHoverInfo(notnull Widget anchor)
	{
		m_Anchor = anchor;
		m_AttachmentPreviewItems = new array<EntityAI>;

		m_Root = g_Game.GetWorkspace().CreateWidgets("JM/COT/GUI/layouts/loot_analysis_hover_info.layout", m_Anchor);

		if (!m_Root)
			return;

		Class.CastTo(m_Preview, m_Root.FindAnyWidget("lahi_preview"));

		m_Title           = TextWidget.Cast(m_Root.FindAnyWidget("lahi_title"));
		m_HealthValue     = TextWidget.Cast(m_Root.FindAnyWidget("lahi_health_value"));
		m_QuantityLabel   = TextWidget.Cast(m_Root.FindAnyWidget("lahi_quantity_label"));
		m_QuantityValue   = TextWidget.Cast(m_Root.FindAnyWidget("lahi_quantity_value"));
		m_LifetimeValue   = TextWidget.Cast(m_Root.FindAnyWidget("lahi_lifetime_value"));
		m_AttachmentsRow  = m_Root.FindAnyWidget("lahi_attachments_row");

		SetTextColor(m_Title, JMTheme.TEXT_PRIMARY);
		SetTextColor(m_HealthValue, JMTheme.TEXT_PRIMARY);
		SetTextColor(m_QuantityValue, JMTheme.TEXT_PRIMARY);
		SetTextColor(m_LifetimeValue, JMTheme.TEXT_PRIMARY);

		m_Root.Show(false);
	}

	void ~JMLootAnalysisHoverInfo()
	{
		DestroyPreviewItem();
		ClearAttachmentIcons();

		if (g_Game && m_Root)
			m_Root.Unlink();
	}

	bool IsShowing(string markerId)
	{
		return m_Open && m_ShownMarkerId == markerId;
	}

	// -------------------------------------------------------------------------
	//  Attachment icons - one small ItemPreviewWidget per attachment, instead
	//  of a comma-joined classname line. Rebuilt every ShowAt call: hovers are
	//  driven by mouse enter/leave, not per-frame, so this never runs hot.
	// -------------------------------------------------------------------------

	protected void SetAttachmentIcons(JMLootScanItemDetail detail)
	{
		if (!m_AttachmentsRow)
			return;

		ClearAttachmentIcons();

		TStringArray attachments = detail.m_Attachments;
		if (!attachments || attachments.Count() == 0)
			return;

		Widget row = UIActionManager.CreateWrapSpacerCompact(m_AttachmentsRow, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_TOP);

		for (int i = 0; i < attachments.Count(); i++)
		{
			Widget icon = g_Game.GetWorkspace().CreateWidgets(ATTACHMENT_ICON_LAYOUT, row);
			if (!icon)
				continue;

			ItemPreviewWidget preview;
			Class.CastTo(preview, icon.FindAnyWidget("lai_preview"));
			if (!preview)
				continue;

			//! Drawn in the state the attachment was actually found in.
			JMLootPreviewState state = AttachmentState(detail, i);
			EntityAI previewItem = JMLocalPreview.Spawn(attachments[i], state.Health, state.Quantity, state.Stage);
			if (!previewItem)
				continue;

			JMLocalPreview.Freeze(previewItem);
			JMLocalPreview.Show(preview, previewItem);

			m_AttachmentPreviewItems.Insert(previewItem);
		}
	}

	// -------------------------------------------------------------------------
	//  Fields
	// -------------------------------------------------------------------------

	protected void SetFields(JMLootScanItemDetail detail)
	{
		if (m_Title)
			m_Title.SetText(detail.m_ClassName);

		SetLevelField(m_HealthValue, detail.m_HealthPct);

		//! Firearms report m_QuantityPct == -1 (see JMLootScanItemDetail) - the
		//! row relabels itself to Ammo and shows a plain round count instead of
		//! a percentage, since "quantity" means nothing for a gun.
		if (detail.m_AmmoCount >= 0)
		{
			if (m_QuantityLabel)
				m_QuantityLabel.SetText("Ammo");

			if (m_QuantityValue)
			{
				m_QuantityValue.SetText(detail.m_AmmoCount.ToString());

				if (detail.m_AmmoCount <= 0)
					m_QuantityValue.SetColor(JMTheme.DANGER);
				else
					m_QuantityValue.SetColor(JMTheme.TEXT_PRIMARY);
			}
		}
		else
		{
			if (m_QuantityLabel)
				m_QuantityLabel.SetText("Quantity");

			SetLevelField(m_QuantityValue, detail.m_QuantityPct);
		}

		if (m_LifetimeValue)
		{
			if (detail.m_LifetimeSeconds < 0)
			{
				m_LifetimeValue.SetText("N/A");
				m_LifetimeValue.SetColor(JMTheme.TEXT_DISABLED);
			}
			else
			{
				m_LifetimeValue.SetText(FormatDuration(detail.m_LifetimeSeconds));
				m_LifetimeValue.SetColor(JMTheme.TEXT_PRIMARY);
			}
		}

		SetAttachmentIcons(detail);
	}

	//! Shared by health and quantity: a 0..1 fraction as a percentage,
	//! coloured by how low it is. A negative fraction means the item has no
	//! such stat (no quantity, or a class GetMaxHealth() never reports one).
	protected void SetLevelField(TextWidget widget, float fraction)
	{
		if (!widget)
			return;

		if (fraction < 0)
		{
			widget.SetText("N/A");
			widget.SetColor(JMTheme.TEXT_DISABLED);
			return;
		}

		int percent = Math.Round(fraction * 100);
		widget.SetText(percent.ToString() + "%");

		if (fraction <= LEVEL_BAD)
			widget.SetColor(JMTheme.DANGER);
		else if (fraction <= LEVEL_WARN)
			widget.SetColor(JMTheme.WARNING);
		else
			widget.SetColor(JMTheme.SUCCESS);
	}

	// -------------------------------------------------------------------------
	//  Model preview
	// -------------------------------------------------------------------------

	//! Build (or reuse) a local copy of the scanned item in the state it was found in - health,
	//! quantity, food stage - with the attachments it actually has, each in its own state. The
	//! same construction the Player Manager's inventory preview uses (JMLocalPreview), so a ruined
	//! rifle or a rotten apple is drawn as it really is, and a backpack shows the pouches it was
	//! found with rather than a bare, pristine model.
	protected void SetPreview(JMLootScanItemDetail detail)
	{
		if (!m_Preview)
			return;

		if (detail.m_ClassName == "")
		{
			ClearPreview();
			return;
		}

		string key = PreviewKey(detail);

		if (m_PreviewItem && m_PreviewKey == key)
		{
			m_Preview.Show(true);
			return;
		}

		DestroyPreviewItem();

		m_PreviewItem = JMLocalPreview.Spawn(detail.m_ClassName, detail.m_State.Health, detail.m_State.Quantity, detail.m_State.Stage);
		m_PreviewKey  = key;

		if (!m_PreviewItem)
		{
			ClearPreview();
			return;
		}

		for (int i = 0; i < detail.m_Attachments.Count(); i++)
		{
			JMLootPreviewState state = AttachmentState(detail, i);
			JMLocalPreview.Attach(m_PreviewItem, detail.m_Attachments[i], state.Health, state.Quantity, state.Stage);
		}

		//! Only now: a weapon with its simulation already off never seats the magazine that was
		//! just handed to it.
		JMLocalPreview.Freeze(m_PreviewItem);

		JMLocalPreview.Show(m_Preview, m_PreviewItem);
	}

	protected void SetTextColor(TextWidget widget, int color)
	{
		if (widget)
			widget.SetColor(color);
	}

	//! Open the panel for one scanned item at a screen-space cursor position.
	void ShowAt(string markerId, JMLootScanItemDetail detail, float screenX, float screenY)
	{
		if (!m_Root || !detail)
			return;

		m_ShownMarkerId = markerId;
		m_Open = true;

		SetFields(detail);
		SetPreview(detail);

		m_Root.Show(true);
		m_Root.SetSort(PANEL_SORT, true);

		Place(screenX, screenY);
	}

	void Hide()
	{
		m_Open = false;
		m_ShownMarkerId = "";

		if (m_Root)
			m_Root.Show(false);
	}

	// -------------------------------------------------------------------------
	//  Placement - identical scheme to JMVehiclesHoverInfo.Place: flip rather
	//  than clamp when the panel would run off the anchor.
	// -------------------------------------------------------------------------
	protected void Place(float screenX, float screenY)
	{
		if (!m_Root || !m_Anchor)
			return;

		float ax, ay, aw, ah;
		m_Anchor.GetScreenPos(ax, ay);
		m_Anchor.GetScreenSize(aw, ah);

		float px = screenX + CURSOR_OFFSET_X - ax;
		float py = screenY + CURSOR_OFFSET_Y - ay;

		if (screenX + CURSOR_OFFSET_X + PANEL_W > ax + aw)
			px = screenX - CURSOR_OFFSET_X - PANEL_W - ax;

		if (screenY + CURSOR_OFFSET_Y + PANEL_H > ay + ah)
			py = screenY - CURSOR_OFFSET_Y - PANEL_H - ay;

		if (px < 0)
			px = 0;
		if (py < 0)
			py = 0;

		m_Root.SetPos(px, py);
	}

	protected JMLootPreviewState AttachmentState(JMLootScanItemDetail detail, int index)
	{
		if (index < detail.m_AttachmentStates.Count())
			return detail.m_AttachmentStates[index];

		return new JMLootPreviewState();
	}

	protected void ClearAttachmentIcons()
	{
		foreach (EntityAI previewItem : m_AttachmentPreviewItems)
			JMLocalPreview.Destroy(previewItem);

		m_AttachmentPreviewItems.Clear();

		if (!m_AttachmentsRow)
			return;

		UIActionManager.ClearChildren( m_AttachmentsRow );
	}

	protected string FormatDuration(int seconds)
	{
		if (seconds >= 86400)
			return Math.Round(seconds / 86400.0).ToString() + "d";

		if (seconds >= 3600)
			return Math.Round(seconds / 3600.0).ToString() + "h";

		if (seconds >= 60)
			return Math.Round(seconds / 60.0).ToString() + "m";

		return seconds.ToString() + "s";
	}

	protected string PreviewKey(JMLootScanItemDetail detail)
	{
		string key = detail.m_ClassName + "|" + detail.m_State.Key();

		for (int i = 0; i < detail.m_Attachments.Count(); i++)
			key = key + "|" + detail.m_Attachments[i] + ":" + AttachmentState(detail, i).Key();

		return key;
	}

	protected void ClearPreview()
	{
		if (m_Preview)
			m_Preview.Show(false);
	}

	protected void DestroyPreviewItem()
	{
		ClearPreview();

		JMLocalPreview.Destroy(m_PreviewItem);

		m_PreviewItem = NULL;
		m_PreviewKey  = "";
	}
}
