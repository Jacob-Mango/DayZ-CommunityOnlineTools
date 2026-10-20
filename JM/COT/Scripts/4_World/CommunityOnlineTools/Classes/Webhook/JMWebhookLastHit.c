//! The last hit a player took, kept so their death webhook can say how it happened (who, with what,
//! from how far, which part of the body) - PlayerKilled only hands over the final damage source.
class JMWebhookLastHit : Managed
{
	//! Formatted attacker (Steam link / AI / zombie name); "" when the hit had no attacker.
	string AttackerText;

	string WeaponName;
	string HitAmmo;
	string HitZone;
	string HitKind;

	//! Metres between attacker and victim, -1 when there was no player attacker.
	float Metres = -1;

	float HealthDamage;
	float HitTime;
}
