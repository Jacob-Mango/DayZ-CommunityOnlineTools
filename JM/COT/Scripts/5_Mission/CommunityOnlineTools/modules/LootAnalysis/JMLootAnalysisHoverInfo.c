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

	//! Local, client-only entity built purely to feed the preview widget, plus
	//! the class (and attachment set) it was built from, so re-hovering the
	//! same result is free.
	protected EntityAI      m_PreviewItem;
	protected string        m_PreviewClass;
	protected ref TStringArray m_PreviewAttachments;

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
		m_PreviewAttachments = new TStringArray;
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

	protected void SetTextColor(TextWidget widget, int color)
	{
		if (widget)
			widget.SetColor(color);
	}

	bool IsShowing(string markerId)
	{
		return m_Open && m_ShownMarkerId == markerId;
	}

	//! Open the panel for one scanned item at a screen-space cursor position.
	void ShowAt(string markerId, JMLootScanItemDetail detail, float screenX, float screenY)
	{
		if (!m_Root || !detail)
			return;

		m_ShownMarkerId = markerId;
		m_Open = true;

		SetFields(detail);
		SetPreview(detail.m_ClassName, detail.m_Attachments);

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

		SetAttachmentIcons(detail.m_Attachments);
	}

	// -------------------------------------------------------------------------
	//  Attachment icons - one small ItemPreviewWidget per attachment, instead
	//  of a comma-joined classname line. Rebuilt every ShowAt call: hovers are
	//  driven by mouse enter/leave, not per-frame, so this never runs hot.
	// -------------------------------------------------------------------------

	protected void SetAttachmentIcons(TStringArray attachments)
	{
		if (!m_AttachmentsRow)
			return;

		ClearAttachmentIcons();

		if (!attachments || attachments.Count() == 0)
			return;

		Widget row = UIActionManager.CreateWrapSpacerCompact(m_AttachmentsRow, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_TOP);

		foreach (string attachClass : attachments)
		{
			Widget icon = g_Game.GetWorkspace().CreateWidgets(ATTACHMENT_ICON_LAYOUT, row);
			if (!icon)
				continue;

			ItemPreviewWidget preview;
			Class.CastTo(preview, icon.FindAnyWidget("lai_preview"));
			if (!preview)
				continue;

			EntityAI previewItem = EntityAI.Cast(g_Game.CreateObject(attachClass, vector.Zero, true, false, false));
			if (!previewItem)
				continue;

			dBodyActive(previewItem, ActiveState.INACTIVE);
			dBodyDynamic(previewItem, false);
			previewItem.DisableSimulation(true);

			//! Exactly vanilla's own inventory-icon call sequence (see
			//! JMPlayerForm's inventory cell preview) - no SetModelPosition.
			//! The engine auto-frames from the config's bounding box and
			//! invView on its own; overriding the position is what used to
			//! push these previews back into a fraction of the cell.
			preview.SetForceFlipEnable(true);
			preview.SetForceFlip(false);
			preview.SetItem(previewItem);
			preview.SetModelOrientation(vector.Zero);
			preview.SetView(previewItem.GetViewIndex());
			preview.Show(true);

			m_AttachmentPreviewItems.Insert(previewItem);
		}
	}

	protected void ClearAttachmentIcons()
	{
		if (g_Game)
		{
			foreach (EntityAI previewItem : m_AttachmentPreviewItems)
			{
				if (previewItem)
					g_Game.ObjectDelete(previewItem);
			}
		}
		m_AttachmentPreviewItems.Clear();

		if (!m_AttachmentsRow)
			return;

		Widget child = m_AttachmentsRow.GetChildren();
		while (child)
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}
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

	// -------------------------------------------------------------------------
	//  Model preview
	// -------------------------------------------------------------------------

	//! Build (or reuse) a local entity for `className` with the same
	//! attachments the scanned item actually has, and frame it - matches
	//! JMObjectSpawnerForm's own preview pass, so a backpack shows the pouches
	//! it was actually found with rather than a bare model.
	protected void SetPreview(string className, TStringArray attachments)
	{
		if (!m_Preview)
			return;

		if (className == "")
		{
			ClearPreview();
			return;
		}

		if (m_PreviewItem && m_PreviewClass == className && SameAttachments(attachments))
		{
			m_Preview.Show(true);
			return;
		}

		DestroyPreviewItem();

		m_PreviewItem  = EntityAI.Cast(g_Game.CreateObject(className, vector.Zero, true, false, false));
		m_PreviewClass = className;

		m_PreviewAttachments.Clear();
		if (attachments)
			m_PreviewAttachments.Copy(attachments);

		if (!m_PreviewItem)
		{
			ClearPreview();
			return;
		}

		dBodyActive(m_PreviewItem, ActiveState.INACTIVE);
		dBodyDynamic(m_PreviewItem, false);
		m_PreviewItem.DisableSimulation(true);

		if (attachments)
		{
			foreach (string attachmentClass : attachments)
			{
				EntityAI attachment = EntityAI.Cast(m_PreviewItem.GetInventory().CreateInInventory(attachmentClass));
				if (attachment)
					dBodyActive(attachment, ActiveState.INACTIVE);
			}
		}

		//! Exactly vanilla's own inventory-icon call sequence (see
		//! JMPlayerForm's inventory cell preview) - no SetModelPosition. The
		//! engine auto-frames from the config's bounding box and invView on
		//! its own; overriding the position is what used to push these
		//! previews back into a fraction of the panel.
		m_Preview.SetForceFlipEnable(true);
		m_Preview.SetForceFlip(false);
		m_Preview.SetItem(m_PreviewItem);
		m_Preview.SetModelOrientation(vector.Zero);
		m_Preview.SetView(m_PreviewItem.GetViewIndex());
		m_Preview.Show(true);
	}

	protected bool SameAttachments(TStringArray attachments)
	{
		int count = 0;
		if (attachments)
			count = attachments.Count();

		if (m_PreviewAttachments.Count() != count)
			return false;

		for (int i = 0; i < count; i++)
		{
			if (m_PreviewAttachments[i] != attachments[i])
				return false;
		}

		return true;
	}

	protected void ClearPreview()
	{
		if (m_Preview)
			m_Preview.Show(false);
	}

	protected void DestroyPreviewItem()
	{
		ClearPreview();

		if (g_Game && m_PreviewItem)
			g_Game.ObjectDelete(m_PreviewItem);

		m_PreviewItem  = NULL;
		m_PreviewClass = "";
		if (m_PreviewAttachments)
			m_PreviewAttachments.Clear();
	}
}
