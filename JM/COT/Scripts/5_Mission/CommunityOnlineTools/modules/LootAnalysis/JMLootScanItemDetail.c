// =============================================================================
//  JMLootScanItemDetail
//
//  Per-marker detail for one Item Scan result, built client-side from the
//  parallel arrays RPC_SendItemScanResults sends (health/quantity/lifetime/
//  attachments-per-entity) - the same "several primitive arrays, same
//  index" shape AddDistributionMarkers already uses for usage types, chosen
//  over a ref array<ref DTO> Param because this codebase has no precedent
//  for serializing an array of custom classes over RPC and parallel
//  primitive arrays are a proven pattern here.
//
//  Kept client-side only; never sent over RPC itself.
// =============================================================================
class JMLootScanItemDetail
{
	string m_ClassName;

	//! 0..1, or -1 when the class reports no health/quantity of its own.
	float m_HealthPct;
	float m_QuantityPct;

	//! Remaining seconds until the entity would decay, or -1 when it has no
	//! lifetime (does not despawn).
	int m_LifetimeSeconds;

	//! Magazine + chambered rounds, or -1 for anything that is not a firearm.
	//! Firearms report -1 for m_QuantityPct (GetQuantityNormalized() means
	//! nothing for them) - the hover panel shows this instead of Quantity
	//! whenever it is >= 0.
	int m_AmmoCount;
	ref TStringArray m_Attachments;

	//! What the model preview needs to draw the entity as it really is - health, quantity, food
	//! stage - for the item itself and, index-parallel to m_Attachments, for each attachment.
	ref JMLootPreviewState m_State = new JMLootPreviewState();
	ref array<ref JMLootPreviewState> m_AttachmentStates = new array<ref JMLootPreviewState>;

	void JMLootScanItemDetail(string className, float healthPct, float quantityPct, int lifetimeSeconds, TStringArray attachments, int ammoCount = -1)
	{
		m_ClassName       = className;
		m_HealthPct       = healthPct;
		m_QuantityPct     = quantityPct;
		m_LifetimeSeconds = lifetimeSeconds;
		m_AmmoCount       = ammoCount;

		m_Attachments = new TStringArray;
		if (attachments)
			m_Attachments.Copy(attachments);
	}
}
