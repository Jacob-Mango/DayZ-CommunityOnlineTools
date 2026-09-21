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

	// Permissions
	static const string PERM_PLAYER                               = "Admin.Player";
	static const string PERM_PLAYER_VIEW                          = "Admin.Player.View";
	static const string PERM_PLAYER_HEAL                          = "Admin.Player.Heal";
	static const string PERM_PLAYER_GODMODE                       = "Admin.Player.GodMode";
	static const string PERM_PLAYER_FREEZE                        = "Admin.Player.Freeze";
	static const string PERM_PLAYER_INVISIBILITY                  = "Admin.Player.Invisibility";
	static const string PERM_PLAYER_RAGDOLL                        = "Admin.Player.Ragdoll";
	static const string PERM_PLAYER_TELEPORT_POSITION             = "Admin.Player.Teleport.Position";
	static const string PERM_PLAYER_TELEPORT_SENDERTO             = "Admin.Player.Teleport.SenderTo";
	static const string PERM_PLAYER_TELEPORT_LOCATION             = "Admin.Player.Teleport.Location";
	static const string PERM_PLAYER_TELEPORT_LOCATION_CREATE      = "Admin.Player.Teleport.Location.Create";
	static const string PERM_PLAYER_TELEPORT_LOCATION_EDIT        = "Admin.Player.Teleport.Location.Edit";
	static const string PERM_PLAYER_TELEPORT_LOCATION_REMOVE      = "Admin.Player.Teleport.Location.Remove";
	static const string PERM_PLAYER_KICK                          = "Admin.Player.Kick";
	static const string PERM_PLAYER_BAN                           = "Admin.Player.Ban";
	static const string PERM_PLAYER_STRIP                         = "Admin.Player.Strip";
	static const string PERM_PLAYER_CLEARCARGO                    = "Admin.Player.ClearCargo";
	static const string PERM_PLAYER_SPECTATE                      = "Admin.Player.Spectate";
	static const string PERM_PLAYER_MESSAGE                       = "Admin.Player.Message";
	static const string PERM_PLAYER_STATS                         = "Admin.Player.Stats";
	static const string PERM_PLAYER_STATISTICS_VIEW                = "Admin.Player.Statistics.View";
	static const string PERM_PLAYER_INVENTORY                     = "Admin.Player.Inventory";
	static const string PERM_PLAYER_INJECTED_PANEL                = "Admin.Player.InjectedPanel";
	static const string PERM_MAP_PLAYERS                          = "Admin.Map.Players";
	static const string PERM_MAP_VIEW                             = "Admin.Map.View";
	static const string PERM_WEATHER_VIEW                         = "Weather.View";
	static const string PERM_WEATHER_QUICKACTION                  = "Weather.QuickAction";
	static const string PERM_WEATHER_QUICKACTION_CLEAR            = "Weather.QuickAction.Clear";
	static const string PERM_WEATHER_QUICKACTION_CLOUDY           = "Weather.QuickAction.Cloudy";
	static const string PERM_WEATHER_QUICKACTION_STORM            = "Weather.QuickAction.Storm";
	static const string PERM_WEATHER_QUICKACTION_DATE             = "Weather.QuickAction.Date";
	static const string PERM_WEATHER_FREEZETIME                   = "Weather.FreezeTime";
	static const string PERM_WEATHER_DATE                         = "Weather.Date";
	static const string PERM_WEATHER_WIND                         = "Weather.Wind";
	static const string PERM_WEATHER_WIND_FUNCPARAMS            = "Weather.Wind.FunctionParams";
	static const string PERM_WEATHER_STORM                        = "Weather.Storm";
	static const string PERM_WEATHER_SANDSTORM                    = "Weather.Sandstorm";
	static const string PERM_WEATHER_OVERCAST                     = "Weather.Overcast";
	static const string PERM_WEATHER_FOG                          = "Weather.Fog";
	static const string PERM_WEATHER_FOG_DYNAMIC                  = "Weather.Fog.Dynamic";
	static const string PERM_WEATHER_RAIN                         = "Weather.Rain";
	static const string PERM_WEATHER_RAIN_THRESHOLDS              = "Weather.Rain.Thresholds";
	static const string PERM_WEATHER_SNOW                         = "Weather.Snow";
	static const string PERM_WEATHER_SNOW_THRESHOLDS              = "Weather.Snow.Thresholds";
	static const string PERM_WEATHER_PRESET                       = "Weather.Preset";
	static const string PERM_WEATHER_PRESET_USE                   = "Weather.Preset.Use";
	static const string PERM_WEATHER_PRESET_CREATE                = "Weather.Preset.Create";
	static const string PERM_WEATHER_PRESET_UPDATE                = "Weather.Preset.Update";
	static const string PERM_WEATHER_PRESET_REMOVE                = "Weather.Preset.Remove";
	static const string PERM_WEATHER_PRESET_DYNAMIC               = "Weather.Preset.Dynamic";
	static const string PERM_VEHICLES_VIEW                        = "Vehicles.View";
	static const string PERM_VEHICLES_DELETE                      = "Vehicles.Delete";
	static const string PERM_VEHICLES_DELETE_ALL                  = "Vehicles.Delete.All";
	static const string PERM_VEHICLES_DELETE_DESTROYED            = "Vehicles.Delete.Destroyed";
	static const string PERM_VEHICLES_DELETE_UNCLAIMED            = "Vehicles.Delete.Unclaimed";
	static const string PERM_VEHICLES_TELEPORT                    = "Vehicles.Teleport";
	static const string PERM_VEHICLES_REPAIR                      = "Vehicles.Repair";
	static const string PERM_VEHICLES_REFUEL                      = "Vehicles.Refuel";
	static const string PERM_VEHICLES_UNSTUCK                     = "Vehicles.Unstuck";
	static const string PERM_VEHICLES_COVER                       = "Vehicles.Cover";
	static const string PERM_VEHICLES_LOCK                        = "Vehicles.Lock";
	static const string PERM_VEHICLES_UNPAIR                      = "Vehicles.UnPair";
	static const string PERM_VEHICLES_CLEARCARGO                  = "Vehicles.ClearCargo";
	static const string PERM_VEHICLES_SPAWNKEY                    = "Vehicles.SpawnKey";
	static const string PERM_ESP_VIEW                             = "ESP.View";
	static const string PERM_ESP_OBJECT_MOVETOCURSOR              = "ESP.Object.MoveToCursor";
	static const string PERM_ESP_OBJECT_DELETEALL                 = "ESP.Object.DeleteAll";
	static const string PERM_ESP_OBJECT_DUPLICATEALL              = "ESP.Object.DuplicateAll";
	static const string PERM_WEBHOOK_VIEW                         = "Webhook.View";
	static const string PERM_WEBHOOK_MANAGE                       = "Webhook.Manage";
	static const string PERM_WEBHOOK_MANAGE_URL                   = "Webhook.Manage.URL";
	static const string PERM_WEBHOOK_MANAGE_URL_ADD               = "Webhook.Manage.URL.Add";
	static const string PERM_WEBHOOK_MANAGE_URL_REMOVE            = "Webhook.Manage.URL.Remove";
	static const string PERM_WEBHOOK_MANAGE_URL_EDIT              = "Webhook.Manage.URL.Edit";
	static const string PERM_WEBHOOK_MANAGE_TYPE                  = "Webhook.Manage.Type";
	static const string PERM_WEBHOOK_MANAGE_TYPE_ADD              = "Webhook.Manage.Type.Add";
	static const string PERM_WEBHOOK_MANAGE_TYPE_REMOVE           = "Webhook.Manage.Type.Remove";
	static const string PERM_WEBHOOK_MANAGE_TYPE_STATE            = "Webhook.Manage.Type.State";
	static const string PERM_ITEMSPAWNER_VIEW                     = "ItemSpawner.View";
	static const string PERM_ITEMSPAWNER_SPAWN                    = "ItemSpawner.Spawn";
	static const string PERM_OBJECTSPAWNER_VIEW                   = "ObjectSpawner.View";
	static const string PERM_OBJECTSPAWNER_SPAWN                  = "ObjectSpawner.Spawn";
	static const string PERM_ROLEMANAGER_VIEW                     = "RoleManager.View";
	static const string PERM_BANS_VIEW                            = "Bans.View";
	static const string PERM_CAMERA_VIEW                          = "Camera.View";
	static const string PERM_BAN_VIEW                             = "Admin.Ban.View";
	static const string PERM_BAN_UNBAN                            = "Admin.Ban.Unban";

	//! Per-event Start/Cancel permissions are built at runtime as
	//! PERM_NAMALSK + "." + eventName + ".Start"/".Cancel" - there is no fixed
	//! constant per event since the event list itself is data, not code.
	static const string PERM_NAMALSK                              = "Namalsk";
	static const string PERM_NAMALSK_VIEW                         = "Namalsk.View";
	static const string PERM_ANTICHEAT_VIEW                       = "Admin.AntiCheat.View";
	static const string PERM_ANTICHEAT_CLEAR                      = "Admin.AntiCheat.Clear";
	static const string PERM_LOOTANALYSIS_VIEW                    = "Admin.LootAnalysis.View";
	static const string PERM_LOOTANALYSIS_ITEMSCAN                = "Admin.LootAnalysis.ItemScan";
	static const string PERM_LOOTANALYSIS_DISTRIBUTION            = "Admin.LootAnalysis.Distribution";
	static const string PERM_LOOTANALYSIS_DELETE                  = "Admin.LootAnalysis.Delete";
	static const string PERM_LOOTANALYSIS_EDIT                    = "Admin.LootAnalysis.Edit";
	static const string PERM_ROLES_VIEW                           = "Admin.Roles.View";
	static const string PERM_ROLES_CREATE                         = "Admin.Roles.Create";
	static const string PERM_ROLES_DELETE                         = "Admin.Roles.Delete";
	static const string PERM_ROLES_PERMISSIONS                    = "Admin.Roles.Permissions";

	//! Every permission node the mod registers or checks has a constant here, so a
	//! literal typo cannot silently create (or miss) a node. Values are the on-disk
	//! role-file keys: never change one, add a new constant instead.
	static const string PERM_ACTIONS_QUICKACTIONS                           = "Actions.QuickActions";
	static const string PERM_COMPENSATIONS_DELETE                           = "Compensations.Delete";
	static const string PERM_COMPENSATIONS_SPAWN                            = "Compensations.Spawn";
	static const string PERM_COMPENSATIONS_VIEW                             = "Compensations.View";
	static const string PERM_COT_VIEW                                       = "COT.View";
	static const string PERM_ENTITY_DELETE                                  = "Entity.Delete";
	static const string PERM_ENTITY_SPAWN_INVENTORY                         = "Entity.Spawn.Inventory";
	static const string PERM_ENTITY_SPAWN_POSITION                          = "Entity.Spawn.Position";
	static const string PERM_ENTITY_VIEW                                    = "Entity.View";
	static const string PERM_ESP_OBJECT_BASEBUILDING_BUILD                  = "ESP.Object.BaseBuilding.Build";
	static const string PERM_ESP_OBJECT_BASEBUILDING_BUILD_MATERIALSNOTREQUIRED= "ESP.Object.BaseBuilding.Build.MaterialsNotRequired";
	static const string PERM_ESP_OBJECT_BASEBUILDING_DISMANTLE              = "ESP.Object.BaseBuilding.Dismantle";
	static const string PERM_ESP_OBJECT_BASEBUILDING_REPAIR                 = "ESP.Object.BaseBuilding.Repair";
	static const string PERM_ESP_OBJECT_BASEBUILDING_SETHEALTH              = "ESP.Object.BaseBuilding.SetHealth";
	static const string PERM_ESP_OBJECT_CAR_LOCKWHEELS                      = "ESP.Object.Car.LockWheels";
	static const string PERM_ESP_OBJECT_CAR_REFUEL                          = "ESP.Object.Car.Refuel";
	static const string PERM_ESP_OBJECT_CAR_UNSTUCK                         = "ESP.Object.Car.Unstuck";
	static const string PERM_ESP_OBJECT_CHANGECOLOR                         = "ESP.Object.ChangeColor";
	static const string PERM_ESP_OBJECT_CLEARCARGO                          = "ESP.Object.ClearCargo";
	static const string PERM_ESP_OBJECT_DELETE                              = "ESP.Object.Delete";
	static const string PERM_ESP_OBJECT_FIREPLACE                           = "ESP.Object.Fireplace";
	static const string PERM_ESP_OBJECT_FLAG                                = "ESP.Object.Flag";
	static const string PERM_ESP_OBJECT_GETCODE                             = "ESP.Object.GetCode";
	static const string PERM_ESP_OBJECT_HEAL                                = "ESP.Object.Heal";
	static const string PERM_ESP_OBJECT_IMMOBILIZE                          = "ESP.Object.Immobilize";
	static const string PERM_ESP_OBJECT_LOCK                                = "ESP.Object.Lock";
	static const string PERM_ESP_OBJECT_OPENCLOSE                           = "ESP.Object.OpenClose";
	static const string PERM_ESP_OBJECT_SETATTACHMENT                       = "ESP.Object.SetAttachment";
	static const string PERM_ESP_OBJECT_SETCLEANNESS                        = "ESP.Object.SetCleanness";
	static const string PERM_ESP_OBJECT_SETCODE                             = "ESP.Object.SetCode";
	static const string PERM_ESP_OBJECT_SETFOODSTAGE                        = "ESP.Object.SetFoodStage";
	static const string PERM_ESP_OBJECT_SETHEALTH                           = "ESP.Object.SetHealth";
	static const string PERM_ESP_OBJECT_SETLIQUID                           = "ESP.Object.SetLiquid";
	static const string PERM_ESP_OBJECT_SETORIENTATION                      = "ESP.Object.SetOrientation";
	static const string PERM_ESP_OBJECT_SETPOSITION                         = "ESP.Object.SetPosition";
	static const string PERM_ESP_OBJECT_SETQUANTITY                         = "ESP.Object.SetQuantity";
	static const string PERM_ESP_OBJECT_TRAP                                = "ESP.Object.Trap";
	static const string PERM_ESP_OBJECT_WEAPON                              = "ESP.Object.Weapon";

	//! Undo/redo an action another admin performed. Without it an admin can only
	//! step back their own actions on the shared JMActionHistory stack.
	static const string PERM_ACTIONHISTORY_UNDOOTHERS                       = "ActionHistory.UndoOthers";
	static const string PERM_EXAMPLE_BUTTON                                 = "Admin.Example.Button";
	static const string PERM_EXAMPLE_DROPDOWN                               = "Admin.Example.Dropdown";
	static const string PERM_EXAMPLE_VIEW                                   = "Admin.Example.View";
	static const string PERM_EXPANSION_TERRITORY                            = "Expansion.Territory";
	static const string PERM_EXPANSION_TERRITORY_SETLEVEL                   = "Expansion.Territory.SetLevel";
	static const string PERM_EXPANSION_TERRITORY_VIEW                       = "Expansion.Territory.View";
	static const string PERM_ITEMS_CREATESET                                = "Items.CreateSet";
	static const string PERM_ITEMSTATS_VIEW                                 = "ItemStats.View";
	static const string PERM_LOADOUTS_BACKUP                                = "Loadouts.Backup";
	static const string PERM_LOADOUTS_CREATE                                = "Loadouts.Create";
	static const string PERM_LOADOUTS_DELETE                                = "Loadouts.Delete";
	static const string PERM_LOADOUTS_LOAD                                  = "Loadouts.Load";
	static const string PERM_LOADOUTS_SPAWN_CURSOR                          = "Loadouts.Spawn.Cursor";
	static const string PERM_LOADOUTS_SPAWN_SELECTEDPLAYERS                 = "Loadouts.Spawn.SelectedPlayers";
	static const string PERM_LOADOUTS_SPAWN_TARGET                          = "Loadouts.Spawn.Target";
	static const string PERM_LOADOUTS_VIEW                                  = "Loadouts.View";
	static const string PERM_MAP                                            = "Admin.Map";
	static const string PERM_MAPEDITOR_DELETE                               = "Admin.MapEditor.Delete";
	static const string PERM_MAPEDITOR_SPAWN                                = "Admin.MapEditor.Spawn";
	static const string PERM_MAPEDITOR_TRANSFORM                            = "Admin.MapEditor.Transform";
	static const string PERM_MAPEDITOR_VIEW                                 = "Admin.MapEditor.View";
	static const string PERM_PLAYER_ACCESSINVENTORY                         = "Admin.Player.AccessInventory";
	static const string PERM_PLAYER_ADMINNVG                                = "Admin.Player.AdminNVG";
	static const string PERM_PLAYER_BLEED_ADD                               = "Admin.Player.Bleed.Add";
	static const string PERM_PLAYER_BLEED_STOP                              = "Admin.Player.Bleed.Stop";
	static const string PERM_PLAYER_BROKENLEGS                              = "Admin.Player.BrokenLegs";
	static const string PERM_PLAYER_CANNOTBETARGETEDBYAI                    = "Admin.Player.CannotBeTargetedByAI";
	static const string PERM_PLAYER_DISEASE_ADD                             = "Admin.Player.Disease.Add";
	static const string PERM_PLAYER_DISEASE_REMOVE                          = "Admin.Player.Disease.Remove";
	static const string PERM_PLAYER_DRY                                     = "Admin.Player.Dry";
	static const string PERM_PLAYER_HEAL_ATTACHMENTS                        = "Admin.Player.Heal.Attachments";
	static const string PERM_PLAYER_HEAL_CARGO                              = "Admin.Player.Heal.Cargo";
	static const string PERM_PLAYER_INVENTORY_DELETE                        = "Admin.Player.Inventory.Delete";
	static const string PERM_PLAYER_INVENTORY_MODIFY                        = "Admin.Player.Inventory.Modify";
	static const string PERM_PLAYER_INVENTORY_REPAIR                        = "Admin.Player.Inventory.Repair";
	static const string PERM_PLAYER_INVENTORY_TAKE                          = "Admin.Player.Inventory.Take";
	static const string PERM_PLAYER_NOTIF                                   = "Admin.Player.Notif";
	static const string PERM_PLAYER_PERMISSIONS                             = "Admin.Player.Permissions";
	static const string PERM_PLAYER_READ                                    = "Admin.Player.Read";
	static const string PERM_PLAYER_RECEIVEDAMAGEDEALT                      = "Admin.Player.ReceiveDamageDealt";
	static const string PERM_PLAYER_REMOVECOLLISION                         = "Admin.Player.RemoveCollision";
	static const string PERM_PLAYER_ROLES                                   = "Admin.Player.Roles";
	static const string PERM_PLAYER_SCALE                                   = "Admin.Player.Scale";
	static const string PERM_PLAYER_SET_BLOOD                               = "Admin.Player.Set.Blood";
	static const string PERM_PLAYER_SET_BLOODYHANDS                         = "Admin.Player.Set.BloodyHands";
	static const string PERM_PLAYER_SET_ENERGY                              = "Admin.Player.Set.Energy";
	static const string PERM_PLAYER_SET_HEALTH                              = "Admin.Player.Set.Health";
	static const string PERM_PLAYER_SET_HEATBUFFER                          = "Admin.Player.Set.HeatBuffer";
	static const string PERM_PLAYER_SET_SHOCK                               = "Admin.Player.Set.Shock";
	static const string PERM_PLAYER_SET_STAMINA                             = "Admin.Player.Set.Stamina";
	static const string PERM_PLAYER_SET_WATER                               = "Admin.Player.Set.Water";
	static const string PERM_PLAYER_STOPBLEEDING                            = "Admin.Player.StopBleeding";
	static const string PERM_PLAYER_TELEPORT_POSITION_CURSOR                = "Admin.Player.Teleport.Position.Cursor";
	static const string PERM_PLAYER_TELEPORT_POSITION_CURSOR_NOLOG          = "Admin.Player.Teleport.Position.Cursor.NoLog";
	static const string PERM_PLAYER_TELEPORT_POSITION_VIEW                  = "Admin.Player.Teleport.Position.View";
	static const string PERM_PLAYER_TELEPORT_PREVIOUS                       = "Admin.Player.Teleport.Previous";
	static const string PERM_PLAYER_UNLIMITEDAMMO                           = "Admin.Player.UnlimitedAmmo";
	static const string PERM_PLAYER_UNLIMITEDSTAMINA                        = "Admin.Player.UnlimitedStamina";
	static const string PERM_PLAYER_VOMIT                                   = "Admin.Player.Vomit";
	static const string PERM_ROLES_UPDATE                                   = "Admin.Roles.Update";
	static const string PERM_TRANSPORT_REPAIR                               = "Admin.Transport.Repair";
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
	static const string FILE_WEATHER_STATE = DIR_COT + "WeatherState.json";
	static const string FILE_TELEPORT 	= DIR_COT + "Teleports_";
	static const string FILE_ESP 		= DIR_COT + "ESP.json";
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
	//! The stat itself runs -30..30 (vanilla). The Player tab's slider is 0-3 and is sent x10, so this
	//! has to reach 30: at 1.0 every value above a tenth of the slider was clamped away.
	static const float STAT_HEATBUFFER_MAX    = 30.0;

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
	static const string ICON_CHEVRON_LEFT          = "JM/COT/GUI/textures/icons/lucide/chevron-left.edds";
	static const string ICON_CHEVRON_RIGHT         = "JM/COT/GUI/textures/icons/lucide/chevron-right.edds";
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
	//! Select-all / deselect-all pair. One source of truth: every list in COT
	//! that can be bulk-selected uses these two, so the gesture looks the same
	//! whether it is the player roster, the ESP categories or the tracked
	//! objects. Both are list glyphs on purpose - a bare tick reads as "this
	//! one item is on" rather than "act on the whole list".
	static const string ICON_SELECT_ALL            = "JM/COT/GUI/textures/icons/lucide/list-checks.edds";
	static const string ICON_DESELECT_ALL          = "JM/COT/GUI/textures/icons/lucide/list-x.edds";
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
	static const string ICON_WARNING               = "JM/COT/GUI/textures/icons/lucide/triangle-alert.edds";
	static const string ICON_WAX_TABLET            = "JM/COT/GUI/textures/icons/lucide/scroll-text.edds";
	static const string ICON_WINDSOCK              = "JM/COT/GUI/textures/icons/lucide/wind.edds";
	static const string ICON_WINTER_HAT            = "JM/COT/GUI/textures/icons/winter-hat.edds";
	static const string ICON_WORLD                 = "JM/COT/GUI/textures/icons/lucide/globe.edds";

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
}
