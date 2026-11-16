//! How one spawn request wants its entity set up after it is created.
//!
//! Travels with the request. The server must never keep these on
//! JMObjectSpawnerModule: the module is one object for every admin, so a value
//! left there by one request is picked up by the next one, whoever sent it.
//! The defaults are what a spawn with no options of its own gets (the chat
//! command).
class JMObjectSpawnOptions : Managed
{
	int m_SetupMode = COT_ObjectSetupMode.DEBUGSPAWN;

	//! Ammo pile class to fill a spawned magazine with; empty when the magazine
	//! keeps its configured ammo.
	string m_AmmoType;

	//! Upper bound of the quantity / health range the spawn rolls from, -1 when
	//! the value is fixed. The lower bound is the request's quantity / health.
	float m_QuantityMax = -1;
	float m_HealthMax = -1;
}
