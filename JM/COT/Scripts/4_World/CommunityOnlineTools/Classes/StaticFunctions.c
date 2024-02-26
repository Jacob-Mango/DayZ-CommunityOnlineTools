// TODO: MOVE MOST OF THIS INTO A SEPERATE MOD

static bool COTPlayerIsRemoved = false;

static bool COT_ESP_Toggled = false;

static PlayerBase GetPlayerObjectByIdentity( PlayerIdentity identityGetPlayerObjectByIdentity )
{
	if ( !GetGame().IsMultiplayer() )
		return PlayerBase.Cast( GetGame().GetPlayer() );

	return PlayerBase.Cast( identityGetPlayerObjectByIdentity.GetPlayer() );
}

static void COTCreateNotification( PlayerIdentity ident, StringLocaliser message, string icon = "set:ccgui_enforce image:HudBuild", float time = 1.5 )
{
	NotificationSystem.Create( new StringLocaliser( "STR_COT_NOTIFICATION_TITLE_ADMIN" ), message, icon, ARGB( 255, 221, 38, 38 ), time, ident );
}

static void COTCreateLocalAdminNotification( StringLocaliser message, string icon = "set:ccgui_enforce image:HudBuild", float time = 1.5 )
{
	NotificationSystem.Create( new StringLocaliser( "STR_COT_NOTIFICATION_TITLE_ADMIN" ), message, icon, ARGB( 255, 221, 38, 38 ), time, NULL );
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

static vector COT_PerformRayCast(vector rayStart, vector rayEnd, Object ignore, out bool hit = false)
{
	Object hitObject;
	vector hitPos;
	vector hitNormal;
	int hitComponentIndex;
	float hitFraction;

	vector p0;
	vector p1;

	bool h0 = DayZPhysics.RayCastBullet(rayStart, rayEnd, 0xFFFFFFFFFF, ignore, hitObject, p0, hitNormal, hitFraction);
	bool h1 = DayZPhysics.RaycastRV(rayStart, rayEnd, p1, hitNormal, hitComponentIndex, NULL, NULL, ignore, false, false, ObjIntersectGeom);

	hit = h0 | h1;

	if (h0 && h1)
	{
		float d0 = vector.Distance(rayStart, p0);
		float d1 = vector.Distance(rayStart, p1);

		if (d0 < d1)
		{
			return p0;
		}

		return p1;
	}

	if (h0)
	{
		return p0;
	}

	return p1;
}

static bool COT_SurfaceIsWater( vector position )
{
	string type;
	GetGame().SurfaceGetType3D(position[0], position[1] + 0.1, position[2], type);
	if (type.Contains("water"))
		return true;

	return GetGame().SurfaceIsSea(position[0], position[2]) || GetGame().SurfaceIsPond(position[0], position[2]);
}

static vector GetPointerPos( float distance = 100.0, Object ignore = NULL )
{
	if (!ignore)
	{
		ignore = GetPlayer();
	}

	vector dir = GetGame().GetPointerDirection();

	vector from = GetGame().GetCurrentCameraPosition();
	vector to = from + (dir * distance);

	return COT_PerformRayCast(from, to, ignore);
}

static vector GetCursorPos( Object ignore = NULL )
{
	vector rayStart = GetGame().GetCurrentCameraPosition();
	vector rayDirection = GetGame().GetCurrentCameraDirection();

	if (!ignore)
	{
		ignore = GetPlayer();
	}

	vector rayEnd = rayStart + ( rayDirection * 10000.0 );

	return COT_PerformRayCast(rayStart, rayEnd, ignore);
}

static void Message(PlayerBase player, string txt, string style = "colorImportant") 
{
	if (GetGame().IsServer() && GetGame().IsMultiplayer())
		player.Message(txt, style);
	else
		GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCSystem, "", txt, style));
};


static Weapon GetWeaponInHands()
{
	Weapon weapon_in_hands;
	if ( GetPlayer() && GetPlayer().GetItemInHands() ) Class.CastTo(weapon_in_hands,  GetPlayer().GetItemInHands());

	return weapon_in_hands;
}

static PlayerBase GetPlayer()
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