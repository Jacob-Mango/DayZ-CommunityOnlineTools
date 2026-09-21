//! The state of one scanned entity that the model preview needs to draw it as it really is: absolute
//! health, quantity and food stage (-1 = does not apply). Read off the live server entity, sent as a
//! compact "health,quantity,stage" string (see the parallel `states` array of RPC_SendItemScanResults)
//! and applied to the client-local copy by JMLocalPreview.
class JMLootPreviewState
{
	float Health = -1;
	float Quantity = -1;
	int Stage = -1;

	//! Server: read the state off a live entity.
	static JMLootPreviewState FromEntity( EntityAI entity )
	{
		JMLootPreviewState state = new JMLootPreviewState();
		if ( !entity )
			return state;

		if ( entity.GetMaxHealth( "", "" ) > 0 )
			state.Health = entity.GetHealth( "", "" );

		//! A magazine's quantity is its AMMO COUNT - GetQuantity on one answers 1 of 1.
		Magazine asMag;
		ItemBase asItem;

		if ( Class.CastTo( asMag, entity ) )
		{
			state.Quantity = asMag.GetAmmoCount();
		}
		else if ( Class.CastTo( asItem, entity ) && asItem.GetQuantityMax() > 0 )
		{
			state.Quantity = asItem.GetQuantity();
		}

		Edible_Base asFood;
		if ( Class.CastTo( asFood, entity ) && asFood.GetFoodStage() )
			state.Stage = asFood.GetFoodStage().GetFoodStageType();

		return state;
	}

	string Encode()
	{
		return Health.ToString() + "," + Quantity.ToString() + "," + Stage.ToString();
	}

	//! Client: the inverse of Encode(); a malformed string decodes to "nothing applies".
	static JMLootPreviewState Decode( string encoded )
	{
		JMLootPreviewState state = new JMLootPreviewState();

		TStringArray parts = new TStringArray;
		encoded.Split( ",", parts );

		if ( parts.Count() >= 3 )
		{
			state.Health = parts[0].ToFloat();
			state.Quantity = parts[1].ToFloat();
			state.Stage = parts[2].ToInt();
		}

		return state;
	}

	//! Part of the preview cache key: two states of the same item must not share one model.
	string Key()
	{
		return Encode();
	}
}
