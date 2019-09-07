// TODO: MOVE MOST OF THIS INTO A SEPERATE MOD

static JMCameraBase CurrentActiveCamera; // active static camera "staticcamera"
static bool COTPlayerIsRemoved = false;

static bool COT_ESP_Toggled = false;

static bool DISABLE_ALL_INPUT = false;

static PlayerBase GetPlayerObjectByIdentity( PlayerIdentity identity )
{
	if ( !GetGame().IsMultiplayer() )
		return GetGame().GetPlayer();
	
	if ( identity == NULL )
		return NULL;

	int networkIdLowBits;
	int networkIdHighBits;
	GetGame().GetPlayerNetworkIDByIdentityID( identity.GetPlayerId(), networkIdLowBits, networkIdHighBits );

	return PlayerBase.Cast( GetGame().GetObjectByNetworkId( networkIdLowBits, networkIdHighBits ) );
}

static void CreateLocalAdminNotification( string message, string icon = "set:ccgui_enforce image:HudBuild" )
{
	NotificationSystem.AddNotificationExtended( 1.5, "Admin", message, icon );
}

static void SendAdminNotification( PlayerIdentity from, PlayerIdentity to, string message, string icon = "set:ccgui_enforce image:HudBuild" )
{
	string title = "Community Online Tools";

	if ( GetGame().IsClient() || !GetGame().IsMultiplayer() )
	{
		NotificationSystem.AddNotificationExtended( 1.5, title, message, icon );
	} else 
	{
		if ( to != NULL )
		{
			title = "Community Online Tools - " + from.GetName();

			NotificationSystem.SendNotificationToPlayerIdentityExtended( to, 1.5, title, message, icon );
		} else 
		{
			NotificationSystem.SendNotificationToPlayerIdentityExtended( from, 1.5, title, message, icon );
		}
	}
}

static vector GetCurrentPosition()
{
	if ( CurrentActiveCamera != NULL )
	{
		return CurrentActiveCamera.GetPosition();
	}

	if ( GetGame().GetPlayer() != NULL )
	{
		return GetGame().GetPlayer().GetPosition();
	}

	return "0 0 0";
}

static vector GetPointerPos( float distance = 100.0, Object ignore = NULL )
{
	if ( !ignore )
	{
		ignore = GetGame().GetPlayer();
	}

	vector dir = GetGame().GetPointerDirection();

	vector from = GetGame().GetCurrentCameraPosition();

	vector to = from + ( dir * distance );

	vector rayStart = from;
	vector rayEnd = to;
	vector hitPos;
	vector hitNormal;
	int hitComponentIndex;
	DayZPhysics.RaycastRV( rayStart, rayEnd, hitPos, hitNormal, hitComponentIndex, NULL, NULL, ignore );

	return hitPos;
}

static vector GetCursorPos()
{
	if ( !GetPlayer() )
	{
		return "0 0 0";
	}

	vector rayStart = GetGame().GetCurrentCameraPosition();
	vector rayEnd = rayStart + GetGame().GetCurrentCameraDirection() * 10000;
	vector hitPos;
	vector hitNormal;
	int hitComponentIndex;
	DayZPhysics.RaycastRV(rayStart, rayEnd, hitPos, hitNormal, hitComponentIndex, NULL, NULL, GetPlayer());

	return hitPos;
}

static void Message( PlayerBase player, string txt ) 
{
	GetLogger().Log( txt, "JM_COT_StaticFunctions" );

	if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
	{
		player.MessageImportant( txt);
		//GetGame().RPCSingleParam( man, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>(txt), false, man.GetIdentity() );
	} else 
	{
    	GetGame().GetMission().OnEvent( ChatMessageEventTypeID, new ChatMessageEventParams( CCDirect, "", txt, "" ) );
	}
}

static Weapon GetWeaponInHands()
{
	Weapon weapon_in_hands;
	if ( GetPlayer() && GetPlayer().GetItemInHands() ) Class.CastTo(weapon_in_hands,  GetPlayer().GetItemInHands());

	return weapon_in_hands;
}

static ref PlayerBase GetPlayer()
{
	return PlayerBase.Cast( GetGame().GetPlayer() );
}

static ZombieBase SpawnInfected(vector pos)
{
	return ZombieBase.Cast(GetGame().CreateObject( WorkingZombieClasses().GetRandomElement(), pos, false, true ));
}

/*
static Weapon_Base CreateWeapon( PlayerBase oPlayer )
{
	Weapon_Base oWpn = Weapon_Base.Cast(oPlayer.GetInventory().CreateInInventory( "M4A1_Black" ));
	oWpn.GetInventory().CreateAttachment( "M4_Suppressor" );
	oWpn.GetInventory().CreateAttachment( "M4_RISHndgrd_Black" );
	oWpn.GetInventory().CreateAttachment( "M4_MPBttstck_Black" );
	oWpn.GetInventory().CreateAttachment( "ACOGOptic" );

	return oWpn;
}
*/

static Weapon_Base CreateWeapon( PlayerBase oPlayer, string sWeapon )
{
	Weapon_Base oWpn = Weapon_Base.Cast(oPlayer.GetInventory().CreateInInventory( sWeapon ));
	oWpn.GetInventory().CreateAttachment( "PistolSuppressor" );
	EntityAI optic = oWpn.GetInventory().CreateAttachment( "ReflexOptic" );
	optic.GetInventory().CreateAttachment("Battery9V");

	return oWpn;
}

static Magazine LoadMag( PlayerBase oPlayer, Weapon_Base oWpn )
{	
	Magazine oMag = Magazine.Cast(oPlayer.GetInventory().CreateInInventory( "Mag_UMP_25Rnd" ));
	oPlayer.GetWeaponManager().AttachMagazine( oMag );

	return oMag;
}

static PlayerBase CreateCustomDefaultCharacter()
{
	PlayerBase oPlayer = PlayerBase.Cast( GetGame().CreatePlayer( NULL, GetGame().CreateRandomPlayer(), GetSpawnPoints().GetRandomElement(), 0, "NONE") );

	EntityAI item = NULL;

	item = oPlayer.GetInventory().CreateInInventory( "AviatorGlasses" );
	item = oPlayer.GetInventory().CreateInInventory( "MilitaryBeret_UN" );
	item = oPlayer.GetInventory().CreateInInventory( "M65Jacket_Black" );
	item = oPlayer.GetInventory().CreateInInventory( "PlateCarrierHolster" );
	item = oPlayer.GetInventory().CreateInInventory( "TacticalGloves_Black" );
	item = oPlayer.GetInventory().CreateInInventory( "HunterPants_Autumn" );
	item = oPlayer.GetInventory().CreateInInventory( "MilitaryBoots_Black" );
	item = oPlayer.GetInventory().CreateInInventory( "AliceBag_Camo" );

	item = oPlayer.GetInventory().CreateInInventory( "Mag_UMP_25Rnd" );

	Weapon_Base oWpn = CreateWeapon(oPlayer, "UMP45");
	LoadMag(oPlayer, oWpn);

	oPlayer.LocalTakeEntityToHands( oWpn );
	oPlayer.SetQuickBarEntityShortcut( oWpn, 0, true );

	return oPlayer;
}

static float CAMERA_FOV = 1.0;
static float CAMERA_TARGETFOV = 1.0;
static float CAMERA_FOV_SPEED_MODIFIER = 5.0;
static float CAMERA_SPEED = 5.0;
static float CAMERA_BOOST_MULT = 10.0;
static float CAMERA_VELDRAG = 0.9;
static float CAMERA_MSENS = 0.8;
static float CAMERA_SMOOTH = 0.8;
static bool  CAMERA_DOF = false;
static bool  CAMERA_AFOCUS = true;
static float CAMERA_BLUR = 0.0;
static float CAMERA_FLENGTH = 50.0;
static float CAMERA_FNEAR = 50.0;
static float CAMERA_FDIST = 0.0;
static float CAMERA_DOFFSET = 0.0;
static float CAMERA_SMOOTH_BLUR = 0.0;

static float EXPOSURE = 0.0;
static float CHROMABERX = 0.0;
static float CHROMABERY = 0.0;

static float HUESHIFT = 0.0;
static float ROTBLUR = 0.0;
static float MINDEPTH = 2.5;
static float MAXDEPTH = 4.5;
static float RADBLURX = 0.0;
static float RADBLURY = 0.0;
static float RADBLUROFFX = 0.0;
static float RADBLUROFFY = 0.0;
static float VIGNETTE = 0.0;

static float VARGB[4] = { 0, 0, 0, 0 };
static float CARGB[4] = { 0, 0, 0, 1 };

static int VIEWDISTANCE = 1600;