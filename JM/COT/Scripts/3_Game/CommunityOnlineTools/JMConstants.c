// =============================================================================
//  CF type stubs.
//
//  DayZ's vanilla `scripts/3_Game/tools/jsonfileloader.c` provides
//  `JsonFileLoader<T>`, and vanilla `scripts/1_core/param.c` provides
//  `Param1` / `Param2`.  The @CF mod provides `CF_Modules`.  All three
//  resolve via standard load order - no stubs needed.  This block is
//  kept as a comment marker for future CF dependency audit.
// =============================================================================

class JMConstants
{
	static const string OFFLINE_GUID 	= "OFFLINE";
	static const string OFFLINE_STEAM 	= "OFFLINE";
	static const string OFFLINE_NAME 	= "OFFLINE";
	
	static const string PERM_ROOT = "ROOT";

	static const string DIR_COT = "$profile:CommunityOnlineTools\\";
	static const string DIR_PF = "$profile:PermissionsFramework\\";

	static const string DIR_PERMISSIONS = DIR_PF + "Permissions\\";
	static const string DIR_ROLES 		= DIR_PF + "Roles\\";
	static const string DIR_PLAYERS 	= DIR_PF + "Players\\";
	static const string DIR_BANS 		= DIR_PF + "Bans\\";
	static const string DIR_NOTES 		= DIR_PF + "Notes\\";

	static const string EXT_PERMISSION 	= ".txt";
	static const string EXT_ROLE 		= ".txt";
	static const string EXT_PLAYER 		= ".json";
	static const string EXT_BAN 		= ".json";
	static const string EXT_NOTE 		= ".json";
	static const string EXT_CSV 		= ".csv";
	static const string EXT_WINDOWS_DEFAULT = ".txt";

	static const string FILE_WEBHOOK 	= DIR_COT + "Webhooks.json";
	static const string FILE_WEATHER 	= DIR_COT + "Weather.json";
	static const string FILE_TELEPORT 	= DIR_COT + "Teleports_";
	static const string DIR_LOGS 		= DIR_COT + "Logs\\";
	static const string DIR_LOADOUTS 	= DIR_COT + "Loadouts\\";
	static const string DIR_DELETIONS 	= DIR_COT + "Deletions\\";
	static const string DIR_COMPENSATIONS = DIR_COT + "Compensations\\";
	static const string DIR_EXPORTS 	= DIR_COT + "Exports\\";

	static const string EXT_LOG 		= ".log";
	static const string EXT_TELEPORT 	= ".json";
	static const string EXT_LOADOUT 	= ".json";

	// RPC security limits
	static const int    RPC_MAX_GUIDS         = 64;   // max targets per multi-target RPC

	//! Cap on how many inventory entries one payload carries. A full loadout is
	//! 60-120 entities; 256 leaves headroom without the payload approaching the
	//! reliable-RPC size limit. If real inventories ever exceed this, add a page
	//! index to the request rather than raising the cap.
	static const int    INVENTORY_MAX_ITEMS  = 256;
	static const float  RPC_RATE_LIMIT_S      = 0.1;  // minimum seconds between RPCs per sender

	// Player stat clamp ranges  [min, max]
	static const float STAT_HEALTH_MIN        = 0.0;
	static const float STAT_HEALTH_MAX        = 100.0;
	static const float STAT_BLOOD_MIN         = 0.0;
	static const float STAT_BLOOD_MAX         = 5000.0;
	static const float STAT_SHOCK_MIN         = 0.0;
	static const float STAT_SHOCK_MAX         = 100.0;
	static const float STAT_ENERGY_MIN        = 0.0;
	static const float STAT_ENERGY_MAX        = 20000.0;
	static const float STAT_WATER_MIN         = 0.0;
	static const float STAT_WATER_MAX         = 5000.0;
	static const float STAT_STAMINA_MIN       = 0.0;
	static const float STAT_STAMINA_MAX       = 100.0;
	static const float STAT_HEATBUFFER_MIN    = 0.0;
	static const float STAT_HEATBUFFER_MAX    = 1.0;

	// Spawner config file
	static const string FILE_SPAWNER_CONFIG   = DIR_COT + "SpawnerConfig.json";

	// Discord embed color palette (decimal representation of hex colors)
	static const int WEBHOOK_COLOR_DEFAULT    = 16766720;  // 0xFFCC00 - amber/gold  (default, generic info)
	static const int WEBHOOK_COLOR_INFO       = 3447003;   // 0x3498DB - blue        (join, neutral info)
	static const int WEBHOOK_COLOR_SUCCESS    = 3066993;   // 0x2ECC71 - green       (server startup, positive)
	static const int WEBHOOK_COLOR_WARNING    = 16098851;  // 0xF5A623 - orange      (admin actions, modifications)
	static const int WEBHOOK_COLOR_DANGER     = 15158332;  // 0xE74C3C - red         (death, ban, damage)
	static const int WEBHOOK_COLOR_CRITICAL   = 10038562;  // 0x992D22 - dark red    (mass delete, critical admin ops)
	static const int WEBHOOK_COLOR_MODERATION = 10181046;  // 0x9B59B6 - purple      (kick, permissions, roles)
	static const int WEBHOOK_COLOR_NEUTRAL    = 9807270;   // 0x95A5A6 - grey        (leave, shutdown)
	static const int WEBHOOK_COLOR_SPAWN      = 1752220;   // 0x1ABC9C - teal        (object/loadout spawn)
	static const int WEBHOOK_COLOR_TELEPORT   = 3553599;   // 0x3643FF - indigo      (teleport actions)
	static const int WEBHOOK_COLOR_ESP        = 8311585;   // 0x7ED321 - lime green  (ESP actions on objects)
	static const int WEBHOOK_COLOR_CAMERA     = 8900331;   // 0x87CEEB - sky blue    (camera actions)

	// Icons
	static const string ICON_AERO_BIKE             = "JM/COT/GUI/textures/icons/aero-bike.edds";
	static const string ICON_AIRPLANE              = "JM/COT/GUI/textures/icons/lucide/plane.edds";
	static const string ICON_ANTICLOCKWISE         = "JM/COT/GUI/textures/icons/lucide/rotate-ccw.edds";
	static const string ICON_APC                   = "JM/COT/GUI/textures/icons/apc.edds";
	static const string ICON_ARROW_CURSOR          = "JM/COT/GUI/textures/icons/lucide/mouse-pointer-2.edds";
	static const string ICON_AUTO_REPAIR           = "JM/COT/GUI/textures/icons/lucide/wrench.edds";
	static const string ICON_BACKWARD_TIME         = "JM/COT/GUI/textures/icons/lucide/history.edds";
	static const string ICON_BALACLAVA             = "JM/COT/GUI/textures/icons/balaclava.edds";
	static const string ICON_BATON                 = "JM/COT/GUI/textures/icons/baton.edds";
	static const string ICON_BATTERY_0             = "JM/COT/GUI/textures/icons/lucide/battery.edds";
	static const string ICON_BATTERY_100           = "JM/COT/GUI/textures/icons/lucide/battery-full.edds";
	static const string ICON_BATTERY_25            = "JM/COT/GUI/textures/icons/lucide/battery-low.edds";
	static const string ICON_BATTERY_50            = "JM/COT/GUI/textures/icons/lucide/battery-medium.edds";
	static const string ICON_BATTERY_75            = "JM/COT/GUI/textures/icons/lucide/battery-medium.edds";
	static const string ICON_BATTERY_MINUS         = "JM/COT/GUI/textures/icons/lucide/battery-warning.edds";
	static const string ICON_BATTERY_PACK_ALT      = "JM/COT/GUI/textures/icons/lucide/battery-charging.edds";
	static const string ICON_BATTERY_PACK          = "JM/COT/GUI/textures/icons/lucide/battery-charging.edds";
	static const string ICON_BATTERY_PLUS          = "JM/COT/GUI/textures/icons/lucide/battery-plus.edds";
	static const string ICON_BEER_BOTTLE           = "JM/COT/GUI/textures/icons/beer-bottle.edds";
	static const string ICON_BELT                  = "JM/COT/GUI/textures/icons/belt.edds";
	static const string ICON_BLEEDING_WOUND        = "JM/COT/GUI/textures/icons/lucide/droplet.edds";
	static const string ICON_BUS                   = "JM/COT/GUI/textures/icons/bus.edds";
	static const string ICON_CAMPING_TENT          = "JM/COT/GUI/textures/icons/lucide/tent.edds";
	static const string ICON_CANCEL                = "JM/COT/GUI/textures/icons/lucide/ban.edds";
	static const string ICON_CANNED_FISH           = "JM/COT/GUI/textures/icons/canned-fish.edds";
	static const string ICON_CLOSE                 = "JM/COT/GUI/textures/icons/lucide/x.edds";
	static const string ICON_CAR_DOOR              = "JM/COT/GUI/textures/icons/car-door.edds";
	static const string ICON_CAR_KEY               = "JM/COT/GUI/textures/icons/car-key.edds";
	static const string ICON_CAR_WHEEL             = "JM/COT/GUI/textures/icons/car-wheel.edds";
	static const string ICON_CHECK_MARK            = "JM/COT/GUI/textures/icons/lucide/check.edds";
	static const string ICON_CHEVRON_DOWN          = "JM/COT/GUI/textures/icons/lucide/chevron-down.edds";
	static const string ICON_CITY_CAR              = "JM/COT/GUI/textures/icons/city-car.edds";
	static const string ICON_CLOCKWISE             = "JM/COT/GUI/textures/icons/lucide/refresh-cw.edds";
	static const string ICON_CLOTHES               = "JM/COT/GUI/textures/icons/clothes.edds";
	static const string ICON_COG                   = "JM/COT/GUI/textures/icons/lucide/settings.edds";
	//! Title-bar collapse pair. The arrows point at each other to collapse and
	//! away from each other to restore, so the button reads as its own state.
	static const string ICON_COLLAPSE_VERTICAL     = "JM/COT/GUI/textures/icons/lucide/chevrons-down-up.edds";
	static const string ICON_EXPAND_VERTICAL       = "JM/COT/GUI/textures/icons/lucide/chevrons-up-down.edds";
	static const string ICON_COINS                 = "JM/COT/GUI/textures/icons/lucide/coins.edds";
	static const string ICON_DRIPPING_TUBE         = "JM/COT/GUI/textures/icons/dripping-tube.edds";
	static const string ICON_ELECTRIC              = "JM/COT/GUI/textures/icons/lucide/zap.edds";
	static const string ICON_EXPAND                = "JM/COT/GUI/textures/icons/lucide/maximize-2.edds";
	static const string ICON_FAMAS                 = "JM/COT/GUI/textures/icons/famas.edds";
	static const string ICON_FAST_BACKWARD         = "JM/COT/GUI/textures/icons/lucide/rewind.edds";
	static const string ICON_FAST_FORWARD          = "JM/COT/GUI/textures/icons/lucide/fast-forward.edds";
	static const string ICON_FLAT_TIRE             = "JM/COT/GUI/textures/icons/flat-tire.edds";
	static const string ICON_FLATBED_COVERED       = "JM/COT/GUI/textures/icons/flatbed-covered.edds";
	static const string ICON_FLYING_FLAG           = "JM/COT/GUI/textures/icons/lucide/flag.edds";
	static const string ICON_FOG                   = "JM/COT/GUI/textures/icons/lucide/cloud-fog.edds";
	static const string ICON_FOLDED_PAPER          = "JM/COT/GUI/textures/icons/lucide/file-text.edds";
	static const string ICON_FORKLIFT              = "JM/COT/GUI/textures/icons/forklift.edds";
	static const string ICON_FOUNTAIN_PEN          = "JM/COT/GUI/textures/icons/lucide/pen-tool.edds";
	static const string ICON_FULL_FOLDER           = "JM/COT/GUI/textures/icons/lucide/folder.edds";
	static const string ICON_FULL_MOTO_HELMET      = "JM/COT/GUI/textures/icons/full-motorcycle-helmet.edds";
	static const string ICON_GAS_PUMP              = "JM/COT/GUI/textures/icons/lucide/fuel.edds";
	static const string ICON_GLADIUS               = "JM/COT/GUI/textures/icons/gladius.edds";
	static const string ICON_GLIDER                = "JM/COT/GUI/textures/icons/glider.edds";
	static const string ICON_GLOVES                = "JM/COT/GUI/textures/icons/gloves.edds";
	static const string ICON_GRADUATE_CAP          = "JM/COT/GUI/textures/icons/lucide/graduation-cap.edds";
	static const string ICON_HAMBURGER_MENU        = "JM/COT/GUI/textures/icons/lucide/menu.edds";
	static const string ICON_HAMMER_DROP           = "JM/COT/GUI/textures/icons/lucide/hammer.edds";
	static const string ICON_HEALTH_DECREASE       = "JM/COT/GUI/textures/icons/lucide/trending-down.edds";
	static const string ICON_HEALTH_INCREASE       = "JM/COT/GUI/textures/icons/lucide/trending-up.edds";
	static const string ICON_HEALTH_NORMAL         = "JM/COT/GUI/textures/icons/lucide/heart-pulse.edds";
	static const string ICON_HEART_BEATS           = "JM/COT/GUI/textures/icons/lucide/activity.edds";
	static const string ICON_HEART_MINUS           = "JM/COT/GUI/textures/icons/lucide/heart-minus.edds";
	static const string ICON_HEART_PLUS            = "JM/COT/GUI/textures/icons/lucide/heart-plus.edds";
	static const string ICON_HEARTS                = "JM/COT/GUI/textures/icons/lucide/heart.edds";
	static const string ICON_HELICOPTER            = "JM/COT/GUI/textures/icons/helicopter.edds";
	static const string ICON_HOME_GARAGE           = "JM/COT/GUI/textures/icons/lucide/warehouse.edds";
	static const string ICON_HORIZONTAL_FLIP       = "JM/COT/GUI/textures/icons/lucide/flip-horizontal.edds";
	static const string ICON_INFO                  = "JM/COT/GUI/textures/icons/lucide/info.edds";
	static const string ICON_JEEP                  = "JM/COT/GUI/textures/icons/jeep.edds";
	static const string ICON_KNAPSACK              = "JM/COT/GUI/textures/icons/knapsack.edds";
	static const string ICON_KNEE_BANDAGE          = "JM/COT/GUI/textures/icons/lucide/bandage.edds";
	static const string ICON_LEE_ENFIELD           = "JM/COT/GUI/textures/icons/lee-enfield.edds";
	static const string ICON_LIGHT_BACKPACK        = "JM/COT/GUI/textures/icons/light-backpack.edds";
	static const string ICON_LIGHTNING_STORM       = "JM/COT/GUI/textures/icons/lucide/cloud-lightning.edds";
	static const string ICON_MACHINE_GUN_MAG       = "JM/COT/GUI/textures/icons/machine-gun-magazine.edds";
	static const string ICON_MEAT                  = "JM/COT/GUI/textures/icons/meat.edds";
	static const string ICON_MEDICINES             = "JM/COT/GUI/textures/icons/lucide/pill.edds";
	static const string ICON_MOON                  = "JM/COT/GUI/textures/icons/lucide/moon.edds";
	static const string ICON_MOVE                  = "JM/COT/GUI/textures/icons/lucide/move.edds";
	static const string ICON_MP5K                  = "JM/COT/GUI/textures/icons/mp5k.edds";
	static const string ICON_MUSHROOM_GILLS        = "JM/COT/GUI/textures/icons/mushroom-gills.edds";
	static const string ICON_NOTEBOOK              = "JM/COT/GUI/textures/icons/lucide/notebook.edds";
	static const string ICON_OPEN_FOLDER           = "JM/COT/GUI/textures/icons/lucide/folder-open.edds";
	static const string ICON_PAUSE                 = "JM/COT/GUI/textures/icons/lucide/pause.edds";
	static const string ICON_PENCIL                = "JM/COT/GUI/textures/icons/lucide/pencil.edds";
	static const string ICON_PIG                   = "JM/COT/GUI/textures/icons/pig.edds";
	//! Pin pair - a plain pin while the window is pinned, a struck-through pin
	//! while it is not, so the icon carries the state and not just its colour.
	static const string ICON_PIN                   = "JM/COT/GUI/textures/icons/lucide/pin.edds";
	static const string ICON_PIN_OFF               = "JM/COT/GUI/textures/icons/lucide/pin-off.edds";
	static const string ICON_PISTOL                = "JM/COT/GUI/textures/icons/pistol-gun.edds";
	static const string ICON_PLAIN_CIRCLE          = "JM/COT/GUI/textures/icons/lucide/circle.edds";
	static const string ICON_PLAIN_SQUARE          = "JM/COT/GUI/textures/icons/lucide/square.edds";
	static const string ICON_PLAY                  = "JM/COT/GUI/textures/icons/lucide/play.edds";
	static const string ICON_POSITION_MARKER       = "JM/COT/GUI/textures/icons/lucide/map-pin.edds";
	static const string ICON_POWER_BUTTON          = "JM/COT/GUI/textures/icons/lucide/power.edds";
	static const string ICON_RADAR_SWEEP           = "JM/COT/GUI/textures/icons/lucide/radar.edds";
	static const string ICON_RADIOACTIVE           = "JM/COT/GUI/textures/icons/lucide/radiation.edds";
	static const string ICON_RAINING               = "JM/COT/GUI/textures/icons/lucide/cloud-rain.edds";
	static const string ICON_RESIZE                = "JM/COT/GUI/textures/icons/lucide/scaling.edds";
	static const string ICON_REVOLVER              = "JM/COT/GUI/textures/icons/revolver.edds";
	static const string ICON_SAILBOAT              = "JM/COT/GUI/textures/icons/sailboat.edds";
	static const string ICON_SALMON                = "JM/COT/GUI/textures/icons/salmon.edds";
	static const string ICON_SAVE_ARROW            = "JM/COT/GUI/textures/icons/lucide/save.edds";
	static const string ICON_SAWED_OFF_SHOTGUN     = "JM/COT/GUI/textures/icons/sawed-off-shotgun.edds";
	static const string ICON_SCOOTER               = "JM/COT/GUI/textures/icons/scooter.edds";
	static const string ICON_SETTINGS_KNOBS        = "JM/COT/GUI/textures/icons/lucide/sliders-horizontal.edds";
	static const string ICON_SHAMBLING_ZOMBIE      = "JM/COT/GUI/textures/icons/lucide/skull.edds";
	static const string ICON_SHARP_AXE             = "JM/COT/GUI/textures/icons/sharp-axe.edds";
	static const string ICON_SHOTGUN_ROUNDS        = "JM/COT/GUI/textures/icons/shotgun-rounds.edds";
	static const string ICON_SKIMMER_HAT           = "JM/COT/GUI/textures/icons/skimmer-hat.edds";
	static const string ICON_SNOWING               = "JM/COT/GUI/textures/icons/lucide/snowflake.edds";
	static const string ICON_SOLAR_TIME            = "JM/COT/GUI/textures/icons/lucide/clock.edds";
	static const string ICON_SPEAKER_OFF           = "JM/COT/GUI/textures/icons/lucide/volume-x.edds";
	static const string ICON_SPEAKER               = "JM/COT/GUI/textures/icons/lucide/volume-2.edds";
	static const string ICON_STACK                 = "JM/COT/GUI/textures/icons/lucide/copy.edds";
	static const string ICON_STAHLHELM             = "JM/COT/GUI/textures/icons/stahlhelm.edds";
	static const string ICON_STONE_CRAFTING        = "JM/COT/GUI/textures/icons/stone-crafting.edds";
	static const string ICON_STOPWATCH             = "JM/COT/GUI/textures/icons/lucide/timer.edds";
	static const string ICON_SUN_CLOUD             = "JM/COT/GUI/textures/icons/lucide/cloud-sun.edds";
	static const string ICON_SUN                   = "JM/COT/GUI/textures/icons/lucide/sun.edds";
	static const string ICON_SUNGLASSES            = "JM/COT/GUI/textures/icons/sunglasses.edds";
	static const string ICON_SUNRISE               = "JM/COT/GUI/textures/icons/lucide/sunrise.edds";
	static const string ICON_SUNSET                = "JM/COT/GUI/textures/icons/lucide/sunset.edds";
	static const string ICON_T_SHIRT               = "JM/COT/GUI/textures/icons/t-shirt.edds";
	static const string ICON_TANK                  = "JM/COT/GUI/textures/icons/tank.edds";
	static const string ICON_THERMOMETER_COLD      = "JM/COT/GUI/textures/icons/lucide/thermometer-snowflake.edds";
	static const string ICON_THERMOMETER_HOT       = "JM/COT/GUI/textures/icons/lucide/thermometer-sun.edds";
	static const string ICON_TINKER                = "JM/COT/GUI/textures/icons/lucide/wrench.edds";
	static const string ICON_TOMATO                = "JM/COT/GUI/textures/icons/tomato.edds";
	static const string ICON_TRASH_CAN             = "JM/COT/GUI/textures/icons/lucide/trash-2.edds";
	static const string ICON_TROUSERS              = "JM/COT/GUI/textures/icons/trousers.edds";
	static const string ICON_UNCERTAINTY           = "JM/COT/GUI/textures/icons/lucide/circle-help.edds";
	static const string ICON_UNLIT_BOMB            = "JM/COT/GUI/textures/icons/lucide/bomb.edds";
	static const string ICON_VERTICAL_FLIP         = "JM/COT/GUI/textures/icons/lucide/flip-vertical.edds";
	static const string ICON_WAX_TABLET            = "JM/COT/GUI/textures/icons/lucide/scroll-text.edds";
	static const string ICON_WINDSOCK              = "JM/COT/GUI/textures/icons/lucide/wind.edds";
	static const string ICON_WINTER_HAT            = "JM/COT/GUI/textures/icons/winter-hat.edds";
	static const string ICON_WORLD                 = "JM/COT/GUI/textures/icons/lucide/globe.edds";


	// -------------------------------------------------------------------------
	//  Lucide icon set (2034 icons, ISC) under GUI/textures/icons/lucide/.
	//
	//  Addressed by name rather than by one constant per icon: 2034 static
	//  strings would eat a pointless chunk of the 3_Game module's static memory
	//  pool, and that pool failing is a compile error, not a runtime one.
	//
	//    image.LoadImageFile( 0, JMConstants.Lucide( "shield-check" ) );
	//
	//  Names are the upstream Lucide names verbatim - see lucide.dev/icons.
	// -------------------------------------------------------------------------
	static string Lucide( string name )
	{
		return "JM/COT/GUI/textures/icons/lucide/" + name + ".edds";
	}

	// -------------------------------------------------------------------------
	//  Rounded surface textures
	//
	//  Pure white with a shaped alpha channel, so an ImageWidget using them can
	//  be tinted with SetColor() exactly like the flat PanelWidgets they replace.
	//  DayZ has no border-radius; a rounded control is an ImageWidget wearing one
	//  of these instead of a PanelWidget.
	//
	//  Each texture's aspect is picked close to its real use so the corner radius
	//  barely distorts when the widget stretches. Fixed-size controls (toggle
	//  thumb, radio dot, swatch) get an exact-size texture and do not distort.
	// -------------------------------------------------------------------------
	static const string SURFACE_CHECK              = "JM/COT/GUI/textures/ui/surface_check.paa";
}
