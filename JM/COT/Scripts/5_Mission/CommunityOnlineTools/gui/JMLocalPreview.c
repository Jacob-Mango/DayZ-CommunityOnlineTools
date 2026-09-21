//! Client-local copies of items, built to feed an ItemPreviewWidget.
//!
//! DayZ cannot draw an item that is not a real entity, and the items COT shows (another player's
//! inventory, world items found by a scan) are usually not replicated to this client. So a preview is a
//! local copy spawned from the classname, with the state the server reported applied on top - health,
//! quantity, food stage - so a ruined, half-eaten or rotten item is drawn as it really is instead of
//! pristine. It is never the real item; nothing done to it goes anywhere near the server.
//!
//! Shared by the Player Manager's inventory and the Loot Analysis hover preview:
//!
//!     EntityAI ent = JMLocalPreview.Spawn( "Apple", health, quantity, stage );
//!     JMLocalPreview.Attach( ent, "PistolSuppressor", health, quantity, stage );
//!     JMLocalPreview.Freeze( ent );      // LAST - see Freeze
//!     ...
//!     JMLocalPreview.Destroy( ent );
class JMLocalPreview
{
	//! Spawn `type` with the reported state. Pass a negative value for anything that does not
	//! apply (no health / no quantity / not food): it is left at the class default.
	static EntityAI Spawn( string type, float health = -1, float quantity = -1, int foodStage = -1 )
	{
		EntityAI ent = EntityAI.Cast( g_Game.CreateObject( type, vector.Zero, true, false, false ) );
		if ( !ent )
			return null;

		dBodyActive( ent, ActiveState.INACTIVE );
		dBodyDynamic( ent, false );

		Apply( ent, health, quantity, foodStage );

		return ent;
	}

	//! Apply the reported state to an existing local copy.
	static void Apply( EntityAI ent, float health, float quantity, int foodStage )
	{
		if ( !ent )
			return;

		if ( health >= 0 )
			ApplyHealth( ent, health );

		ItemBase asItem;
		if ( !Class.CastTo( asItem, ent ) )
			return;

		if ( quantity >= 0 )
			asItem.SetQuantity( quantity, false );

		Edible_Base asFood;
		if ( foodStage >= 0 && Class.CastTo( asFood, asItem ) && asFood.GetFoodStage() )
			asFood.GetFoodStage().ChangeFoodStage( foodStage );
	}

	protected static void ApplyHealth( EntityAI ent, float health )
	{
		ent.SetHealth( "", "", health );

		float maxHealth = MiscGameplayFunctions.GetTypeMaxGlobalHealth( ent.GetType() );
		if ( maxHealth <= 0 )
			maxHealth = ent.GetMaxHealth( "", "" );

		if ( maxHealth <= 0 )
			return;

		float health01 = Math.Clamp( health / maxHealth, 0, 1 );

		TStringArray zones = new TStringArray;
		ent.GetDamageZones( zones );

		foreach ( string zone : zones )
			ent.SetHealth01( zone, "Health", health01 );
	}

	//! Give `parent` a child copy, with the reported state. A slot id (or -1) is tried first so a
	//! thing that fits several slots lands in the one it was in; then any attachment slot; then the
	//! LOCAL inventory operation, which is the client-side path vanilla itself uses and the one that
	//! seats a magazine the Create* calls refuse. `attachmentsOnly` skips the cargo fallback (a
	//! container's contents never show through its model). Returns null when nothing took it.
	static EntityAI Attach( EntityAI parent, string type, float health = -1, float quantity = -1, int foodStage = -1, int slotId = -1, bool attachmentsOnly = true )
	{
		if ( !parent )
			return null;

		EntityAI child;

		if ( slotId >= 0 )
			child = parent.GetInventory().CreateAttachmentEx( type, slotId );

		if ( !child )
			child = parent.GetInventory().CreateAttachment( type );

		if ( !child )
			child = AttachLocal( parent, type, slotId );

		if ( !child && !attachmentsOnly )
			child = parent.GetInventory().CreateInInventory( type );

		if ( !child )
			return null;

		Apply( child, health, quantity, foodStage );

		return child;
	}

	//! Spawn a child loose and hand it to the parent with a LOCAL inventory operation - the path
	//! EntityAI itself exposes for client-side work, and the one that takes a magazine when
	//! CreateAttachment will not. Returns null when the parent refused it.
	static EntityAI AttachLocal( EntityAI parent, string type, int slotId )
	{
		EntityAI ent = EntityAI.Cast( g_Game.CreateObject( type, vector.Zero, true, false, false ) );
		if ( !ent )
			return null;

		bool taken = false;

		if ( slotId >= 0 )
			taken = parent.LocalTakeEntityAsAttachmentEx( ent, slotId );

		if ( !taken )
			taken = parent.LocalTakeEntityAsAttachment( ent );

		if ( !taken )
		{
			g_Game.ObjectDelete( ent );
			return null;
		}

		return ent;
	}

	//! Freeze a preview tree once it is fully built.
	//!
	//! Simulation is switched off LAST, not at spawn: a weapon whose simulation is already off does
	//! not run the state change that seats a magazine, so a rifle built the other way round came up
	//! with its optic and no mag.
	static void Freeze( EntityAI ent )
	{
		if ( !ent )
			return;

		ent.DisableSimulation( true );
	}

	static void Destroy( EntityAI ent )
	{
		if ( ent && g_Game )
			g_Game.ObjectDelete( ent );
	}

	//! Point an ItemPreviewWidget at `ent` with exactly the call sequence vanilla's own inventory
	//! icon uses, in the same order. The engine frames the model itself from the config's bounding
	//! box and invView - overriding the position is what used to push previews back into a fraction
	//! of their cell.
	static void Show( ItemPreviewWidget preview, EntityAI ent )
	{
		if ( !preview || !ent )
			return;

		preview.SetForceFlipEnable( true );
		preview.SetForceFlip( false );
		preview.SetItem( ent );
		preview.SetModelOrientation( vector.Zero );
		preview.SetView( ent.GetViewIndex() );
		preview.Show( true );
	}
}
