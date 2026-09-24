//! The Object Spawner's property card: the per-class rows (quantity, health, temperature, cooking state, liquid, ammo, disinfect) that are rebuilt on every selection, and the values the admin picked, which outlive those rows.
class JMObjectSpawnerProperties
{
	protected JMObjectSpawnerForm m_Form;

	//! Property rows are rebuilt per selection rather than shown and hidden: a
	//! hidden child still owns its cell in a GridSpacer, which would leave a
	//! gap where an inapplicable slider used to be. m_PropsHost stays put so
	//! the rows keep their place in the card; m_PropsGrid is what gets
	//! replaced.
	protected Widget m_PropsHost;
	protected Widget m_PropsGrid;
	protected UIActionText m_PropsNone;

	//! Ranges, not single values: the low and high handles are the bounds the
	//! server rolls between, per spawned entity. Dragging them together is how
	//! you ask for one exact value.
	//!
	//! Both start COLLAPSED to one handle - see m_RangeToggle.
	protected UIActionSliderRange m_QuantityItem;
	protected UIActionSliderRange m_HealthItem;

	//! Switches the two spannable rows above between one value and a range.
	//!
	//! Off by default: rolling a spawn between two bounds is the advanced case,
	//! and the pair of handles reads as a range whether or not the admin meant
	//! one - a full magazine asked for by dragging both ends together is easy
	//! to leave a round short by accident. With it off the rows keep exactly
	//! the same look, minus the second handle.
	//!
	//! It lives in the card's TITLE BAR rather than in a row of its own: it is
	//! a mode for the whole card, not a property of the item being spawned, and
	//! a row would have put it in the same list as quantity and health where it
	//! reads as one more thing that gets spawned. It also outlives the rows -
	//! the grid under it is destroyed on every selection, the header is not.
	protected UIActionImageButton m_RangeToggle;
	protected bool m_RangesEnabled;

	//! Kept because the header action is created once and the ROWS it governs
	//! come and go: a class with neither a quantity nor a health bar has
	//! nothing to switch, and the button is hidden for it.
	protected UIActionCard m_PropsCard;

	//! Still a single value. Temperature reads back as a NAMED state next to the
	//! number, and there is no sensible name for a range.
	protected UIActionSlider m_TemperatureItem;

	//! Cooking state for food.
	//!
	//! A dropdown built into a card that is torn down on every selection is only
	//! safe because UIActionDropdown now unlinks its floating list panel with
	//! itself - the panel is parented to the FORM root, not to the control, so
	//! before that it would have been left behind once per rebuild.
	protected UIActionDropdown m_FoodStage;

	//! FoodStageType for each entry of m_FoodStage, in the same order. Only the
	//! stages the class actually declares are offered, so the indices are not
	//! the enum's.
	protected ref TIntArray m_FoodStageTypes;
	protected int m_FoodStageValue;

	//! What a liquid container is filled with. The values are liquid-type bits,
	//! except on a blood container where the module wants a 1-based index into
	//! the blood types instead - see CollectLiquids.
	protected UIActionDropdown m_LiquidType;
	protected ref TIntArray m_LiquidValues;
	protected int m_LiquidValue;

	//! Which cartridge a magazine is loaded with, as one of its own ammoItems
	//! classnames. "" leaves it on whatever it is configured to hold.
	protected UIActionDropdown m_AmmoType;
	protected ref TStringArray m_AmmoValues;
	protected string m_AmmoValue;

	//! Rags, bandages and sewing kits spawn either soiled or disinfected.
	protected UIActionToggleSwitch m_Disinfect;
	protected bool m_DisinfectValue;

	//! The sliders are destroyed on every rebuild, so what the admin chose has
	//! to outlive them. Health and quantity are kept as FRACTIONS of their own
	//! span - the span changes with the class, and 30 rounds of one calibre
	//! should come back as 30 rounds of the next, not as the number 30 against
	//! a different maximum. Temperature has a fixed range, so it is absolute.
	protected float m_HealthLowPct;
	protected float m_HealthHighPct;
	protected float m_QuantityLowPct;
	protected float m_QuantityHighPct;
	protected float m_TemperatureValue;
	protected bool  m_TemperatureSet;

	//! Samples taken across the health range to build its gradient. 32 puts
	//! every band edge within about 3% of where vanilla draws it.
	static const int HEALTH_GRADIENT_STOPS = 32;

	void JMObjectSpawnerProperties( JMObjectSpawnerForm form )
	{
		m_Form = form;

		m_FoodStageTypes = new TIntArray;
		m_LiquidValues   = new TIntArray;
		m_AmmoValues     = new TStringArray;

		//! Both ranges open at their full span, so switching ranges ON asks for
		//! a spawn randomised across everything the class allows until the
		//! handles are pulled together. Until then only the high handle is
		//! shown, and it is the one value that spawns.
		m_HealthLowPct    = 0.0;
		m_HealthHighPct   = 1.0;
		m_QuantityLowPct  = 0.0;
		m_QuantityHighPct = 1.0;

		m_RangesEnabled = false;

		m_TemperatureValue = GameConstants.STATE_NEUTRAL_TEMP;
	}

	//! Creates the card in the form's right column. The rows inside it are built per
	//! selection by Rebuild(); the card and its header action are created once.
	void Build( Widget rightContent )
	{
		m_PropsCard = UIActionManager.CreateCard( rightContent, "#STR_COT_OBJECT_MODULE_PROPERTIES" );
		m_PropsHost = UIActionManager.CreateGridSpacer( m_PropsCard.GetContent(), 1, 1 );

		//! An icon button, not a toggle switch: the header strip is 30px tall
		//! and gives its children no label room. The STATE is the tint - accent
		//! while ranges are on, the plain button fill while they are off - and
		//! the tooltip names both what it is doing and what a click would do.
		m_RangeToggle = m_PropsCard.AddCardHeaderAction( JMConstants.Lucide( "arrow-left-right" ), this, "" );
		if ( m_RangeToggle ) m_RangeToggle.SetOnClick( this, "Click_ToggleRanges" );

		PaintRangeToggle();
	}

	//! Everything a spawn takes from the card. -1 is the module's "not specified" for
	//! health, temperature and quantity, so a property the selected class does not
	//! have simply is not sent.
	//!
	//! Health and quantity travel as a PAIR: the low end as the return value, the
	//! high end as module state alongside the setup mode. The server rolls between
	//! them once per entity, so one call that fills five inventories produces five
	//! different results.
	void ReadSpawnValues( out float health, out float temp, out float quantity, out int itemState )
	{
		JMObjectSpawnerModule module = m_Form.GetModule();

		health   = RangeLowOr( m_HealthItem, -1 );
		temp     = SliderValueOr( m_TemperatureItem, -1 );
		quantity = RangeLowOr( m_QuantityItem, -1 );

		module.m_SpawnHealthMax   = RangeHighOr( m_HealthItem, -1 );
		module.m_SpawnQuantityMax = RangeHighOr( m_QuantityItem, -1 );

		//! 0 is the module's "leave the item state alone". The three things it
		//! can mean - liquid type, cooking stage, cleanness - are mutually
		//! exclusive per class, and only one control for it is ever built.
		itemState = 0;

		if ( m_FoodStage )
			itemState = m_FoodStageValue;
		else if ( m_LiquidType )
			itemState = m_LiquidValue;
		else if ( m_Disinfect && m_Disinfect.IsChecked() )
			itemState = 1;

		//! Module state rather than another parameter, the same way the setup
		//! mode travels. Cleared when the class has no ammo to choose, or a
		//! magazine picked earlier would follow every later spawn.
		if ( m_AmmoType )
			module.m_SpawnAmmoType = m_AmmoValue;
		else
			module.m_SpawnAmmoType = "";
	}

	//! Dragging a slider must NOT run the full preview refresh: that rebuilds
	//! the property rows, which destroys the very slider being dragged. Only
	//! the health the preview entity is wearing is re-applied.
	void Click_SetHealth( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
		UpdateHealthItemColor();
		RefreshPreviewHealth();
	}

	void Click_SetTemperature( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
		UpdateTemperatureItemColor();
	}

	//! Push the health slider onto the entity already being previewed.
	//!
	//! Same pass the spawn runs: the value lands on the item, on its damage
	//! zones, and on every attachment at the same PERCENTAGE of that
	//! attachment's own max health. Without the cascade a ruined rifle previews
	//! with a pristine optic and magazine hanging off it.
	void RefreshPreviewHealth()
	{
		if ( !m_Form.GetPreviewItem() || !m_HealthItem )
			return;

		if ( m_Form.GetPreviewItem().IsTransport() )
			return;

		m_Form.GetModule().SetupEntityHealth( m_Form.GetPreviewItem(), m_HealthItem.GetRangeHigh(), -1 );
	}

	void UpdateHealthItemColor()
	{
		//! The row is absent for a class with no health at all, rather than
		//! present and disabled - see Rebuild.
		if ( !m_HealthItem )
			return;

		//! The bar is SHADED across itself rather than given one colour: a range
		//! that runs from ruined to pristine is not any single condition, and
		//! painting it one colour could only ever describe one end of it.
		//!
		//! The ramp is sampled densely rather than handed over as the five
		//! condition colours, because the vanilla bands are not evenly spaced -
		//! they break at 30%, 50% and 70% - and evenly spaced stops would put
		//! the colour changes in the wrong places.
		TIntArray stops = new TIntArray;

		for ( int i = 0; i < HEALTH_GRADIENT_STOPS; i++ )
			stops.Insert( HealthColor( ( 1.0 * i ) / ( HEALTH_GRADIENT_STOPS - 1 ) ) );

		m_HealthItem.SetGradient( stops );
		m_HealthItem.SetAlpha( 1.0 );
	}

	//! Force a vanilla colour constant opaque.
	//!
	//! The condition and temperature constants in Colors are written with an
	//! alpha byte of ZERO - COLOR_PRISTINE is 0x0040FF00. They are meant for
	//! text colouring, where the alpha is supplied separately. Handed to
	//! Widget.SetColor, which reads plain ARGB, every one of them paints a
	//! fully transparent widget: the colour is applied exactly as asked and
	//! nothing appears.
	//!
	//! The single-value slider hid this by calling SetAlpha(1.0) straight after
	//! SetColor - but that only reaches the one widget SetAlpha touches, so
	//! anything else painted from these constants stays invisible.
	static int Opaque( int color )
	{
		return color | 0xFF000000;
	}

	//! The vanilla condition bands, as a colour.
	protected int HealthColor( float health01 )
	{
		if ( health01 >= 0.7 )
			return Opaque( Colors.COLOR_PRISTINE );

		if ( health01 >= 0.5 )
			return Opaque( Colors.COLOR_WORN );

		if ( health01 >= 0.3 )
			return Opaque( Colors.COLOR_DAMAGED );

		if ( health01 > 0 )
			return Opaque( Colors.COLOR_BADLY_DAMAGED );

		return Opaque( Colors.COLOR_RUINED );
	}

	void UpdateTemperatureItemColor()
	{
		if ( !m_TemperatureItem )
			return;

		int value = m_TemperatureItem.GetCurrent();

		//! Same zero-alpha constants as the condition colours - see Opaque.
		m_TemperatureItem.SetColor( Opaque( ObjectTemperatureState.GetStateData(value).m_Color ) );
		if (ObjectTemperatureState.GetStateData(value).m_State != GameConstants.STATE_NEUTRAL_TEMP)
			m_TemperatureItem.SetFormat( "#STR_COT_FORMAT_DEGREE " + ObjectTemperatureState.GetStateData(value).m_LocalizedName );
		else
			m_TemperatureItem.SetFormat("#STR_COT_FORMAT_DEGREE");

		m_TemperatureItem.SetAlpha( 1.0 );
	}

	// -------------------------------------------------------------------------
	//  Properties
	// -------------------------------------------------------------------------

	//! Build only the sliders that mean something for this class.
	//!
	//! The old form kept all three on screen and called Disable() on whichever
	//! did not apply - but a disabled UIAction is painted under a 90%-opaque
	//! panel, so "not applicable" and "unreadable" looked the same. The rows
	//! are built instead, and a row that does not apply is simply absent.
	//!
	//! They are REBUILT rather than shown and hidden because a hidden child
	//! still owns its cell in a GridSpacer, which would leave a gap exactly
	//! where the missing slider used to be.
	void Rebuild( string classname )
	{
		if ( !m_PropsHost )
			return;

		CaptureProperties();

		//! The food dropdown floats a list panel over the form, and the form is
		//! what dismisses those. Take it off the overlay register before the
		//! widget it belongs to stops existing.
		if ( m_FoodStage )
		{
			m_FoodStage.Close();
			m_Form.RemoveOverlay( m_FoodStage );
		}

		if ( m_LiquidType )
		{
			m_LiquidType.Close();
			m_Form.RemoveOverlay( m_LiquidType );
		}

		if ( m_AmmoType )
		{
			m_AmmoType.Close();
			m_Form.RemoveOverlay( m_AmmoType );
		}

		if ( m_PropsGrid )
		{
			m_PropsGrid.Unlink();
			m_PropsGrid = NULL;
		}

		m_QuantityItem    = NULL;
		m_TemperatureItem = NULL;
		m_HealthItem      = NULL;
		m_FoodStage       = NULL;
		m_LiquidType      = NULL;
		m_AmmoType        = NULL;
		m_Disinfect       = NULL;
		m_PropsNone       = NULL;

		float maxHealth = 0;

		if ( classname != "" )
			maxHealth = MiscGameplayFunctions.GetTypeMaxGlobalHealth( classname );

		bool wantsHealth = ( maxHealth > 0 );
		bool wantsQuantity = false;
		bool wantsTemperature = false;

		float quantityMin = 0;
		float quantityMax = 0;
		float quantityStep = 1;

		array<string> foodStages = new array<string>;
		array<string> liquids    = new array<string>;
		array<string> ammoTypes  = new array<string>;

		bool wantsFoodStage = false;
		bool wantsDisinfect = false;
		bool wantsLiquid    = false;
		bool wantsAmmo      = false;

		ItemBase item = ItemBase.Cast( m_Form.GetPreviewItem() );

		if ( item )
		{
			if ( item.IsLiquidContainer() || ( item.HasFoodStage() && item.CanBeCooked() ) )
				wantsTemperature = true;

			//! Only the stages the class actually declares - offering "Boiled"
			//! for something with no boiled stage would spawn it unchanged and
			//! look like the control did nothing.
			if ( item.HasFoodStage() && item.IsInherited( Edible_Base ) )
			{
				CollectFoodStages( classname, foodStages );
				wantsFoodStage = ( foodStages.Count() > 0 );
			}

			//! Rags, bandages and sewing kits. The engine already answers which
			//! ones, so there is no class list to keep in step.
			wantsDisinfect = item.CanBeDisinfected();

			if ( item.IsLiquidContainer() )
			{
				CollectLiquids( classname, item, liquids );

				//! One entry is the "leave it as configured" row on its own,
				//! which is not worth a control.
				wantsLiquid = ( liquids.Count() > 1 );
			}

			//! What a magazine calls quantity is its ammo count, and it has its
			//! own floor: an ammo pile cannot hold zero rounds and still be a
			//! pile.
			Magazine mag;

			if ( Class.CastTo( mag, item ) )
			{
				if ( mag.GetAmmoMax() > 0 )
				{
					wantsQuantity = true;
					quantityMax   = mag.GetAmmoMax();
					quantityStep  = 1;

					if ( mag.IsAmmoPile() && mag.GetAmmoMax() > 1 )
						quantityMin = 1.0;
				}
			}
			else if ( item.GetQuantityMax() - item.GetQuantityMin() > 0 )
			{
				wantsQuantity = true;
				quantityMin   = item.GetQuantityMin();
				quantityMax   = item.GetQuantityMax();

				if ( item.IsSplitable() )
					quantityStep = 1;
				else
					quantityStep = 0.1;
			}
		}

		//! Which cartridge, for a magazine that lists more than the one.
		//!
		//! Read from the class rather than from the preview entity: ammoItems is
		//! declared on each magazine directly, so the config answers even when
		//! there is no entity to ask, and a class that is not a magazine simply
		//! has none.
		CollectAmmoTypes( classname, ammoTypes );
		wantsAmmo = ( ammoTypes.Count() > 1 );

		//! The switch is only shown where there is something for it to switch.
		//! A class with neither a quantity nor a health bar has no spannable
		//! property at all.
		bool wantsRanges = ( wantsQuantity || wantsHealth );

		ShowRangeToggle( wantsRanges );

		int rows = 0;

		if ( wantsQuantity )    rows++;
		if ( wantsHealth )      rows++;
		if ( wantsTemperature ) rows++;
		if ( wantsFoodStage )   rows++;
		if ( wantsLiquid )      rows++;
		if ( wantsAmmo )        rows++;
		if ( wantsDisinfect )   rows++;

		if ( rows == 0 )
		{
			m_PropsGrid = UIActionManager.CreateGridSpacer( m_PropsHost, 1, 1 );
			m_PropsNone = UIActionManager.CreateText( m_PropsGrid, "#STR_COT_OBJECT_MODULE_NO_PROPERTIES", "" );
			return;
		}

		m_PropsGrid = UIActionManager.CreateGridSpacer( m_PropsHost, rows, 1 );

		if ( wantsQuantity )
		{
			m_QuantityItem = UIActionManager.CreateSliderRange( m_PropsGrid, "#STR_COT_OBJECT_MODULE_QUANTITY", quantityMin, quantityMax, this, "Click_SetQuantity" );
			m_QuantityItem.SetFormat( "#STR_COT_FORMAT_NUMBER" );
			m_QuantityItem.SetStep( quantityStep );
			m_QuantityItem.SetRange( quantityMin + m_QuantityLowPct * ( quantityMax - quantityMin ), quantityMin + m_QuantityHighPct * ( quantityMax - quantityMin ) );

			//! After SetRange, never before: collapsing pins the low bound onto
			//! whatever the high one is at the time.
			m_QuantityItem.SetSingle( !m_RangesEnabled );
		}

		if ( wantsHealth )
		{
			m_HealthItem = UIActionManager.CreateSliderRange( m_PropsGrid, "#STR_COT_OBJECT_MODULE_HEALTH", 0, maxHealth, this, "Click_SetHealth" );
			m_HealthItem.SetFormat( "#STR_COT_FORMAT_NUMBER" );
			m_HealthItem.SetStep( 1 );
			m_HealthItem.SetRange( m_HealthLowPct * maxHealth, m_HealthHighPct * maxHealth );
			m_HealthItem.SetSingle( !m_RangesEnabled );

			UpdateHealthItemColor();
		}

		if ( wantsTemperature )
		{
			m_TemperatureItem = UIActionManager.CreateSlider( m_PropsGrid, "#STR_COT_OBJECT_MODULE_TEMPERATURE", GameConstants.STATE_COLD_LVL_FOUR, GameConstants.STATE_HOT_LVL_FOUR, this, "Click_SetTemperature" );
			m_TemperatureItem.SetSliderWidth( 0.6 );
			m_TemperatureItem.SetStepValue( 1 );
			m_TemperatureItem.SetFormat( "#STR_COT_FORMAT_DEGREE" );
			m_TemperatureItem.SetCurrent( m_TemperatureValue );

			UpdateTemperatureItemColor();
		}

		if ( wantsFoodStage )
		{
			m_FoodStage = UIActionManager.CreateDropdown( m_PropsGrid, "#STR_COT_OBJECT_MODULE_FOODSTAGE", m_Form.GetLayoutRoot(), this, "Click_SetFoodStage", foodStages );
			m_FoodStage.SetWidth( 1.0 );
			m_FoodStage.SetTooltip( "#STR_COT_OBJECT_MODULE_FOODSTAGE_DESC" );

			m_Form.AddOverlay( m_FoodStage );

			//! Carry the chosen stage across the rebuild where the next class
			//! also offers it; fall back to its first stage where it does not.
			int stageIndex = m_FoodStageTypes.Find( m_FoodStageValue );

			if ( stageIndex < 0 )
				stageIndex = 0;

			m_FoodStage.SetSelection( stageIndex, false );
			m_FoodStageValue = m_FoodStageTypes[stageIndex];
		}

		if ( wantsLiquid )
		{
			m_LiquidType = UIActionManager.CreateDropdown( m_PropsGrid, "#STR_COT_OBJECT_MODULE_LIQUID", m_Form.GetLayoutRoot(), this, "Click_SetLiquid", liquids );
			m_LiquidType.SetWidth( 1.0 );
			m_LiquidType.SetTooltip( "#STR_COT_OBJECT_MODULE_LIQUID_DESC" );

			m_Form.AddOverlay( m_LiquidType );

			int liquidIndex = m_LiquidValues.Find( m_LiquidValue );

			if ( liquidIndex < 0 )
				liquidIndex = 0;

			m_LiquidType.SetSelection( liquidIndex, false );
			m_LiquidValue = m_LiquidValues[liquidIndex];
		}

		if ( wantsAmmo )
		{
			m_AmmoType = UIActionManager.CreateDropdown( m_PropsGrid, "#STR_COT_OBJECT_MODULE_AMMOTYPE", m_Form.GetLayoutRoot(), this, "Click_SetAmmoType", ammoTypes );
			m_AmmoType.SetWidth( 1.0 );
			m_AmmoType.SetTooltip( "#STR_COT_OBJECT_MODULE_AMMOTYPE_DESC" );

			m_Form.AddOverlay( m_AmmoType );

			int ammoIndex = m_AmmoValues.Find( m_AmmoValue );

			if ( ammoIndex < 0 )
				ammoIndex = 0;

			m_AmmoType.SetSelection( ammoIndex, false );
			m_AmmoValue = m_AmmoValues[ammoIndex];
		}

		if ( wantsDisinfect )
		{
			m_Disinfect = UIActionManager.CreateToggleSwitch( m_PropsGrid, "#STR_COT_OBJECT_MODULE_DISINFECTED", this, "Click_SetDisinfect", m_DisinfectValue );
			m_Disinfect.SetTooltip( "#STR_COT_OBJECT_MODULE_DISINFECTED_DESC" );
		}

		if ( m_Form.GetRightScroller() )
			m_Form.GetRightScroller().UpdateScroller();
	}

	//! Fill m_FoodStageTypes and `labels` with the cooking stages `classname`
	//! declares, in enum order.
	//!
	//! A food class lists its stages under Food FoodStages, one subclass per
	//! stage, and there is no script-side query for them - so the config is the
	//! only place that knows which of the six a given item has.
	protected void CollectFoodStages( string classname, out array<string> labels )
	{
		m_FoodStageTypes.Clear();
		labels.Clear();

		if ( classname == "" )
			return;

		TStringArray configNames = { "Raw", "Baked", "Boiled", "Dried", "Burned", "Rotten" };
		TIntArray    stageTypes  = { FoodStageType.RAW, FoodStageType.BAKED, FoodStageType.BOILED, FoodStageType.DRIED, FoodStageType.BURNED, FoodStageType.ROTTEN };
		TStringArray stageLabels = { "#STR_COT_OBJECT_FOODSTAGE_RAW", "#STR_COT_OBJECT_FOODSTAGE_BAKED", "#STR_COT_OBJECT_FOODSTAGE_BOILED", "#STR_COT_OBJECT_FOODSTAGE_DRIED", "#STR_COT_OBJECT_FOODSTAGE_BURNED", "#STR_COT_OBJECT_FOODSTAGE_ROTTEN" };

		for ( int i = 0; i < configNames.Count(); i++ )
		{
			if ( !g_Game.ConfigIsExisting( "CfgVehicles " + classname + " Food FoodStages " + configNames[i] ) )
				continue;

			m_FoodStageTypes.Insert( stageTypes[i] );

			//! The selector prints entry text verbatim, so the key is resolved
			//! here rather than handed over as a key.
			labels.Insert( stageLabels[i] );
		}
	}

	//! Every liquid the game defines, as bit and name. One table, so the list
	//! of what a container accepts and the name of what it already holds can
	//! never disagree.
	static void LiquidTable( out TIntArray bits, out TStringArray keys )
	{
		bits = { LIQUID_WATER, LIQUID_CLEANWATER, LIQUID_FRESHWATER, LIQUID_RIVERWATER, LIQUID_STILLWATER, LIQUID_HOTWATER, LIQUID_SALTWATER, LIQUID_SNOW, LIQUID_VODKA, LIQUID_BEER, LIQUID_GASOLINE, LIQUID_DIESEL, LIQUID_DISINFECTANT, LIQUID_SOLUTION, LIQUID_SALINE, LIQUID_BLOOD_0_P, LIQUID_BLOOD_0_N, LIQUID_BLOOD_A_P, LIQUID_BLOOD_A_N, LIQUID_BLOOD_B_P, LIQUID_BLOOD_B_N, LIQUID_BLOOD_AB_P, LIQUID_BLOOD_AB_N };
		keys = { "#STR_COT_OBJECT_LIQUID_WATER", "#STR_COT_OBJECT_LIQUID_CLEANWATER", "#STR_COT_OBJECT_LIQUID_FRESHWATER", "#STR_COT_OBJECT_LIQUID_RIVERWATER", "#STR_COT_OBJECT_LIQUID_STILLWATER", "#STR_COT_OBJECT_LIQUID_HOTWATER", "#STR_COT_OBJECT_LIQUID_SALTWATER", "#STR_COT_OBJECT_LIQUID_SNOW", "#STR_COT_OBJECT_LIQUID_VODKA", "#STR_COT_OBJECT_LIQUID_BEER", "#STR_COT_OBJECT_LIQUID_GASOLINE", "#STR_COT_OBJECT_LIQUID_DIESEL", "#STR_COT_OBJECT_LIQUID_DISINFECTANT", "#STR_COT_OBJECT_LIQUID_SOLUTION", "#STR_COT_OBJECT_LIQUID_SALINE", "#STR_COT_OBJECT_LIQUID_BLOOD_0_P", "#STR_COT_OBJECT_LIQUID_BLOOD_0_N", "#STR_COT_OBJECT_LIQUID_BLOOD_A_P", "#STR_COT_OBJECT_LIQUID_BLOOD_A_N", "#STR_COT_OBJECT_LIQUID_BLOOD_B_P", "#STR_COT_OBJECT_LIQUID_BLOOD_B_N", "#STR_COT_OBJECT_LIQUID_BLOOD_AB_P", "#STR_COT_OBJECT_LIQUID_BLOOD_AB_N" };
	}

	//! Name for one liquid bit; "Empty" for a container that starts with none.
	protected string LiquidName( int bit )
	{
		TIntArray bits;
		TStringArray keys;
		LiquidTable( bits, keys );

		for ( int i = 0; i < bits.Count(); i++ )
		{
			if ( bits[i] == bit )
				return keys[i];
		}

		return "#STR_COT_OBJECT_LIQUID_NONE";
	}

	//! "<name> (default)" - the row that changes nothing, named after what the
	//! class actually spawns with. "As configured" told the admin nothing they
	//! could act on; the answer to "what do I get if I leave this alone" is the
	//! whole reason the row is there.
	protected string DefaultLabel( string name )
	{
		return COT_String.TranslateEx( "#STR_COT_OBJECT_DEFAULT_FORMAT" , name );
	}

	//! Fill m_LiquidValues and `labels` with what this container can hold.
	//!
	//! A container declares its liquids as a BITMASK in liquidContainerType, so
	//! a canteen is not offered petrol and a jerrycan is not offered water.
	protected void CollectLiquids( string classname, ItemBase item, out array<string> labels )
	{
		m_LiquidValues.Clear();
		labels.Clear();

		//! 0 is the module's "leave the item state alone", so the first row is
		//! whatever the class is configured to hold, named. Every list below
		//! then SKIPS that liquid: picking it explicitly and leaving it alone
		//! are the same spawn, and one of them is already row zero.
		int init = item.GetLiquidTypeInit();

		m_LiquidValues.Insert( 0 );
		labels.Insert( DefaultLabel( LiquidName( init ) ) );

		if ( classname == "" )
			return;

		//! A blood container is the one case where what travels is NOT the
		//! liquid bit: the module raises LIQUID_BLOOD_0_P to the power of the
		//! value it is given, so a blood bag wants a 1-based index instead.
		if ( item.IsBloodContainer() )
		{
			TStringArray bloodKeys = { "#STR_COT_OBJECT_LIQUID_BLOOD_0_P", "#STR_COT_OBJECT_LIQUID_BLOOD_0_N", "#STR_COT_OBJECT_LIQUID_BLOOD_A_P", "#STR_COT_OBJECT_LIQUID_BLOOD_A_N", "#STR_COT_OBJECT_LIQUID_BLOOD_B_P", "#STR_COT_OBJECT_LIQUID_BLOOD_B_N", "#STR_COT_OBJECT_LIQUID_BLOOD_AB_P", "#STR_COT_OBJECT_LIQUID_BLOOD_AB_N", "#STR_COT_OBJECT_LIQUID_SALINE" };

			for ( int b = 0; b < bloodKeys.Count(); b++ )
			{
				//! The module reads a blood value as an exponent, so index b
				//! stands for the bit LIQUID_BLOOD_0_P << b.
				if ( Math.Pow( 2, b ) == init )
					continue;

				m_LiquidValues.Insert( b + 1 );
				labels.Insert( bloodKeys[b] );
			}

			return;
		}

		//! From the ITEM, never from "CfgVehicles <class> liquidContainerType".
		//!
		//! That absolute path answers 0 for every container in the game -
		//! measured, not assumed - because liquidContainerType is declared on a
		//! base class and g_Game.ConfigGetInt does not walk the inheritance
		//! chain. The entity's own relative lookup does, and GetLiquidContainerMask
		//! is the result of it.
		int mask = item.GetLiquidContainerMask();

		if ( mask == 0 )
			return;

		TIntArray bits;
		TStringArray keys;
		LiquidTable( bits, keys );

		for ( int i = 0; i < bits.Count(); i++ )
		{
			if ( ( mask & bits[i] ) == 0 )
				continue;

			if ( bits[i] == init )
				continue;

			m_LiquidValues.Insert( bits[i] );
			labels.Insert( keys[i] );
		}
	}

	//! Display name for one ammo pile class, falling back to the classname.
	protected string AmmoPileName( string pile )
	{
		string display;

		//! @note ConfigGetText returns localized string
		if ( g_Game.ConfigGetText( CFG_MAGAZINESPATH + " " + pile + " displayName", display ) && display != "" )
			return display;

		return pile;
	}

	//! Fill m_AmmoValues and `labels` with the ammo piles this magazine accepts.
	//!
	//! ammoItems is a flat config array, so it has to be read with
	//! ConfigGetTextArray - ConfigGetChildrenCount answers 0 for one of those.
	protected void CollectAmmoTypes( string classname, out array<string> labels )
	{
		m_AmmoValues.Clear();
		labels.Clear();

		TStringArray ammoItems = new TStringArray;

		if ( classname != "" )
			g_Game.ConfigGetTextArray( CFG_MAGAZINESPATH + " " + classname + " ammoItems", ammoItems );

		//! What the magazine loads itself with is a CfgAmmo cartridge, while
		//! everything the admin can pick is an ammo PILE. Match the two up by
		//! the cartridge each pile names, so the first row can say which of the
		//! offered rounds is already the default rather than "as configured".
		string defaultCartridge;

		if ( classname != "" )
			g_Game.ConfigGetText( CFG_MAGAZINESPATH + " " + classname + " ammo", defaultCartridge );

		string defaultName = "";
		string defaultPile = "";

		for ( int d = 0; d < ammoItems.Count(); d++ )
		{
			string pileCartridge;

			if ( !g_Game.ConfigGetText( CFG_MAGAZINESPATH + " " + ammoItems[d] + " ammo", pileCartridge ) )
				continue;

			if ( pileCartridge != defaultCartridge || pileCartridge == "" )
				continue;

			defaultPile = ammoItems[d];
			defaultName = AmmoPileName( defaultPile );
			break;
		}

		//! No pile claims the configured cartridge - name the first one it
		//! accepts, which is what it will be loaded with anyway.
		if ( defaultName == "" && ammoItems.Count() > 0 )
		{
			defaultPile = ammoItems[0];
			defaultName = AmmoPileName( defaultPile );
		}

		if ( defaultName == "" )
			defaultName = "#STR_COT_OBJECT_AMMO_NONE";

		m_AmmoValues.Insert( "" );
		labels.Insert( DefaultLabel( defaultName ) );

		for ( int i = 0; i < ammoItems.Count(); i++ )
		{
			string pile = ammoItems[i];

			if ( pile == "" )
				continue;

			//! Already row zero, under its own name.
			if ( pile == defaultPile )
				continue;

			m_AmmoValues.Insert( pile );
			labels.Insert( AmmoPileName( pile ) );
		}
	}

	void Click_SetLiquid( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
	}

	void Click_SetAmmoType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
	}

	void Click_SetFoodStage( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
		RefreshPreviewFoodStage();
	}

	void Click_SetDisinfect( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		//! Nothing to preview - cleanness has no model of its own.
		m_DisinfectValue = action.IsChecked();
	}

	//! Cook the preview entity to the chosen stage so the model in the card is
	//! the model that will be spawned - a burned steak does not look like a raw
	//! one, and that is most of the reason to pick a stage at all.
	void RefreshPreviewFoodStage()
	{
		if ( !m_Form.GetPreviewItem() || m_FoodStageValue == 0 )
			return;

		Edible_Base food;

		if ( !Class.CastTo( food, m_Form.GetPreviewItem() ) )
			return;

		if ( !food.GetFoodStage() )
			return;

		food.ChangeFoodStage( m_FoodStageValue );
		food.GetFoodStage().UpdateVisualsEx( true );
	}

	//! Remember what the sliders were set to before they are destroyed.
	//!
	//! Health and quantity are kept as a fraction of their own range: the range
	//! is a property of the class, so a full magazine stays full when the next
	//! class holds a different number of rounds.
	protected void CaptureProperties()
	{
		//! While a slider is collapsed its low bound is PINNED to the value, so
		//! capturing it would overwrite where the range's low handle was last
		//! left - and switching ranges back on could then only ever reopen the
		//! span onto the value itself.
		if ( m_HealthItem && m_HealthItem.GetMax() > 0 )
		{
			if ( !m_HealthItem.IsSingle() )
				m_HealthLowPct = m_HealthItem.GetRangeLow() / m_HealthItem.GetMax();

			m_HealthHighPct = m_HealthItem.GetRangeHigh() / m_HealthItem.GetMax();
		}

		if ( m_QuantityItem )
		{
			float range = m_QuantityItem.GetMax() - m_QuantityItem.GetMin();

			if ( range > 0 )
			{
				if ( !m_QuantityItem.IsSingle() )
					m_QuantityLowPct = ( m_QuantityItem.GetRangeLow() - m_QuantityItem.GetMin() ) / range;

				m_QuantityHighPct = ( m_QuantityItem.GetRangeHigh() - m_QuantityItem.GetMin() ) / range;
			}
		}

		if ( m_TemperatureItem )
		{
			m_TemperatureValue = m_TemperatureItem.GetCurrent();
			m_TemperatureSet   = true;
		}

		if ( m_FoodStage )
		{
			int stageIndex = m_FoodStage.GetSelection();

			if ( stageIndex >= 0 && stageIndex < m_FoodStageTypes.Count() )
				m_FoodStageValue = m_FoodStageTypes[stageIndex];
		}

		if ( m_LiquidType )
		{
			int liquidIndex = m_LiquidType.GetSelection();

			if ( liquidIndex >= 0 && liquidIndex < m_LiquidValues.Count() )
				m_LiquidValue = m_LiquidValues[liquidIndex];
		}

		if ( m_AmmoType )
		{
			int ammoIndex = m_AmmoType.GetSelection();

			if ( ammoIndex >= 0 && ammoIndex < m_AmmoValues.Count() )
				m_AmmoValue = m_AmmoValues[ammoIndex];
		}

		if ( m_Disinfect )
			m_DisinfectValue = m_Disinfect.IsChecked();
	}

	void Click_SetQuantity( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
	}

	//! Switch the two spannable rows between one value and a range.
	//!
	//! An icon button carries no checked state of its own, so the mode is held
	//! here and the button is repainted from it.
	void Click_ToggleRanges( UIActionBase action )
	{
		m_RangesEnabled = !m_RangesEnabled;

		PaintRangeToggle();
		ApplyRangeMode();
	}

	//! Tint and label the header switch for the mode it is in.
	void PaintRangeToggle()
	{
		if ( !m_RangeToggle )
			return;

		if ( m_RangesEnabled )
		{
			m_RangeToggle.SetColor( JMTheme.ACCENT );
			m_RangeToggle.SetTooltip( "#STR_COT_OBJECT_MODULE_USE_RANGE_ON" );
		}
		else
		{
			m_RangeToggle.SetColor( JMTheme.BUTTON_FILL );
			m_RangeToggle.SetTooltip( "#STR_COT_OBJECT_MODULE_USE_RANGE_OFF" );
		}
	}

	//! Hide the header switch for a class with nothing to switch.
	//!
	//! Hidden rather than disabled: a disabled UIAction is painted under a
	//! 90%-opaque panel, which in a 30px title bar is a grey smudge that reads
	//! as a broken icon rather than as an inapplicable one.
	protected void ShowRangeToggle( bool show )
	{
		if ( !m_RangeToggle )
			return;

		m_RangeToggle.GetLayoutRoot().Show( show );
	}

	//! Put the quantity and health rows into whichever mode the switch is in.
	//!
	//! The rows are changed IN PLACE rather than rebuilt: Rebuild
	//! destroys every row in the card, and one of them is the switch that was
	//! just clicked - the engine answers a vanished press target by recentring
	//! the cursor, which is the same reason dragging a slider does not refresh
	//! the preview.
	//!
	//! Reopening a span has to put the low handle BACK: collapsing pinned it
	//! onto the value, and the fraction it was left at is the only record of
	//! where the admin had it.
	protected void ApplyRangeMode()
	{
		if ( m_QuantityItem )
		{
			m_QuantityItem.SetSingle( !m_RangesEnabled );

			if ( m_RangesEnabled )
			{
				float quantitySpan = m_QuantityItem.GetMax() - m_QuantityItem.GetMin();
				float quantityLow  = m_QuantityItem.GetMin() + m_QuantityLowPct * quantitySpan;

				m_QuantityItem.SetRange( quantityLow, m_QuantityItem.GetRangeHigh() );
			}
		}

		if ( m_HealthItem )
		{
			m_HealthItem.SetSingle( !m_RangesEnabled );

			if ( m_RangesEnabled )
			{
				float healthLow = m_HealthLowPct * m_HealthItem.GetMax();

				m_HealthItem.SetRange( healthLow, m_HealthItem.GetRangeHigh() );
			}
		}

		CaptureProperties();
	}

	protected float SliderValueOr( UIActionSlider slider, float fallback )
	{
		if ( !slider )
			return fallback;

		return slider.GetCurrent();
	}

	protected float RangeLowOr( UIActionSliderRange slider, float fallback )
	{
		if ( !slider )
			return fallback;

		return slider.GetRangeLow();
	}

	protected float RangeHighOr( UIActionSliderRange slider, float fallback )
	{
		if ( !slider )
			return fallback;

		return slider.GetRangeHigh();
	}
}
