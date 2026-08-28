// =============================================================================
//  JMPlayerInventoryItem
//
//  One entry of a player's inventory, as sent to an admin client on demand.
//
//  The server walks the target's inventory with a PREORDER traversal and emits
//  one of these per entity. PREORDER is parent-before-child, so ParentIndex can
//  be resolved against the entries already emitted and the whole tree arrives as
//  a flat array with no recursion on either side.
//
//  Only the classname crosses the wire. Display name, icon and category all come
//  from CfgVehicles, which the client already has - sending them would roughly
//  double the payload for information the receiver can look up for free.
//
//  The network ID is what the delete / repair / take operations refer back to.
//  The server never trusts it as a world lookup: it is resolved ONLY against the
//  named target player's own inventory, so a client cannot reach an arbitrary
//  object by guessing an ID.
// =============================================================================

class JMPlayerInventoryItem
{
	//! Flags. Attachment and cargo are mutually exclusive in practice but are
	//! kept as separate bits so "neither" (hands) stays representable.
	static const int FLAG_ATTACHMENT = 1;
	static const int FLAG_CARGO      = 2;
	static const int FLAG_RUINED     = 4;
	static const int FLAG_WEAPON     = 8;
	static const int FLAG_MAGAZINE   = 16;

	//! Per-item capabilities, so the context menu can offer only the entries
	//! that mean something for the row that was hit. All three are answered by
	//! the SERVER against the live entity rather than guessed from the config:
	//! a jam is runtime state, and both of the others depend on what the item
	//! is actually carrying, not only on what its class allows.
	static const int FLAG_JAMMED     = 32;
	static const int FLAG_LIQUID     = 64;   //!< liquid container
	static const int FLAG_FOOD       = 128;  //!< has cookable food stages

	int    NetIdLow;
	int    NetIdHigh;

	string Type;         //!< classname, resolved to a display name client-side
	string SlotName;     //!< attachment slot name, or "" for cargo and hands

	int    Depth;        //!< 0 = directly on the player
	int    ParentIndex;  //!< index into the flat array; -1 at the top level

	float  Health;       //!< 0..100

	//! GameConstants.STATE_*: pristine / worn / damaged / badly damaged /
	//! ruined, as the ITEM's own config defines the bands. Sent rather than
	//! derived, because healthLevels is per-config and an assumed 70/50/30 is
	//! wrong for anything that sets its own.
	int    HealthLevel;
	float  Quantity;
	float  QuantityMax;
	float  QuantityMin;

	//! Live values the item-edit prompts open on, so a slider starts where the
	//! item already is instead of at a default the admin then has to undo.
	float  Temperature;
	int    LiquidType;   //!< LIQUID_*, 0 when empty or not a container
	int    Stage;        //!< FoodStageType, 0 (NONE) when not food. Named around the FoodStage CLASS, which the name would shadow.

	int    Flags;

	void JMPlayerInventoryItem()
	{
		NetIdLow    = 0;
		NetIdHigh   = 0;
		Depth       = 0;
		ParentIndex = -1;
		Health      = 0;
		HealthLevel = -1;
		Quantity    = 0;
		QuantityMax = 0;
		QuantityMin = 0;
		Temperature = 0;
		LiquidType  = 0;
		Stage       = 0;
		Flags       = 0;
	}

	void OnSend( ParamsWriteContext ctx )
	{
		ctx.Write( NetIdLow );
		ctx.Write( NetIdHigh );
		ctx.Write( Type );
		ctx.Write( SlotName );
		ctx.Write( Depth );
		ctx.Write( ParentIndex );
		ctx.Write( Health );
		ctx.Write( HealthLevel );
		ctx.Write( Quantity );
		ctx.Write( QuantityMax );
		ctx.Write( QuantityMin );
		ctx.Write( Temperature );
		ctx.Write( LiquidType );
		ctx.Write( Stage );
		ctx.Write( Flags );
	}

	//! Returns false on a short read so the caller can abandon the whole payload
	//! rather than render a half-decoded list.
	bool OnReceive( ParamsReadContext ctx )
	{
		if ( !ctx.Read( NetIdLow ) )    return false;
		if ( !ctx.Read( NetIdHigh ) )   return false;
		if ( !ctx.Read( Type ) )        return false;
		if ( !ctx.Read( SlotName ) )    return false;
		if ( !ctx.Read( Depth ) )       return false;
		if ( !ctx.Read( ParentIndex ) ) return false;
		if ( !ctx.Read( Health ) )      return false;
		if ( !ctx.Read( HealthLevel ) ) return false;
		if ( !ctx.Read( Quantity ) )    return false;
		if ( !ctx.Read( QuantityMax ) ) return false;
		if ( !ctx.Read( QuantityMin ) ) return false;
		if ( !ctx.Read( Temperature ) ) return false;
		if ( !ctx.Read( LiquidType ) )  return false;
		if ( !ctx.Read( Stage ) )       return false;
		if ( !ctx.Read( Flags ) )       return false;

		return true;
	}

	bool IsRuined()
	{
		return ( Flags & FLAG_RUINED ) != 0;
	}

	bool IsAttachment()
	{
		return ( Flags & FLAG_ATTACHMENT ) != 0;
	}

	bool IsMagazine()
	{
		return ( Flags & FLAG_MAGAZINE ) != 0;
	}

	bool IsWeapon()
	{
		return ( Flags & FLAG_WEAPON ) != 0;
	}

	bool IsJammed()
	{
		return ( Flags & FLAG_JAMMED ) != 0;
	}

	bool IsLiquidContainer()
	{
		return ( Flags & FLAG_LIQUID ) != 0;
	}

	bool HasFoodStage()
	{
		return ( Flags & FLAG_FOOD ) != 0;
	}

	//! A stack, a magazine or anything else with a quantity bar. Ammo piles and
	//! magazines report their AMMO COUNT here, not the count of the item itself.
	bool HasQuantity()
	{
		return QuantityMax > QuantityMin;
	}

	//! Which liquids this container's config will accept, as a LIQUID_* bit
	//! mask. Read from the config rather than sent: the mask is a per-class
	//! constant the client already has, so putting it on the wire would cost a
	//! field per item for something no item can disagree with.
	int GetLiquidContainerMask()
	{
		return GetGame().ConfigGetInt( "CfgVehicles " + Type + " liquidContainerType" );
	}

	//! Display name from CfgVehicles, falling back to the classname when the
	//! config carries no displayName (common for base classes and debug items).
	string GetDisplayName()
	{
		string display = "";
		GetGame().ConfigGetText( "CfgVehicles " + Type + " displayName", display );

		if ( display == "" )
			return Type;

		return display;
	}

	//! Where the item sits, for the table's second column.
	string GetLocationLabel()
	{
		if ( SlotName != "" )
			return SlotName;

		if ( ( Flags & FLAG_CARGO ) != 0 )
			return "#STR_COT_PLAYER_MODULE_INV_LOC_CARGO";

		return "#STR_COT_PLAYER_MODULE_INV_LOC_HANDS";
	}

	//! "12/30" for a stack or magazine, "-" for anything without a quantity.
	string GetQuantityLabel()
	{
		if ( QuantityMax <= 0 )
			return "-";

		int cur = Math.Round( Quantity );
		int max = Math.Round( QuantityMax );

		return cur.ToString() + "/" + max.ToString();
	}
}
