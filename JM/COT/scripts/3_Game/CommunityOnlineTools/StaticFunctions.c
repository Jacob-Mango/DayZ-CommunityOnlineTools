static const int COT_ZERO_PAD_SIZE = 8;
static string m_COT_ZeroPad[COT_ZERO_PAD_SIZE] = {"", "0", "00", "000", "0000", "00000", "000000", "0000000"};
	
// ------------------------------------------------------------
// COT FormatFloat
// ------------------------------------------------------------
static string FormatFloat( float value, int decimals ) 
{
	if ( decimals == -1 )
		return "" + value;

	int power = Math.Pow( 10, decimals );
	float number = Math.Round( value * power ) / power;
	string result = "" + number;
	int idx = result.IndexOf(".") + 1;
	if ( idx > 0 )
	{
		int l = decimals - ( result.Length() - idx );
		if ( l > 0 && l < COT_ZERO_PAD_SIZE )
			return result + m_COT_ZeroPad[l];
		return result.Substring( 0, result.Length() - l );
	}
	return result + "." + m_COT_ZeroPad[decimals];
}

static int ToSingleDigit( string s )
{
	if ( s.Length() != 1 ) return -1;
	
	switch(s)
	{
		case "0":
			return 0;
		case "1":
			return 1;
		case "2":
			return 2;
		case "3":
			return 3;
		case "4":
			return 4;
		case "5":
			return 5;
		case "6":
			return 6;
		case "7":
			return 7;
		case "8":
			return 8;
		case "9":
			return 9;
	}
	return -1;
}

static float ToFloat( string text, bool onlyPositive = false )
{
	float f = 0;
	float d = 0;
	int atDec = 0;
	bool foundDec = false;

	bool isNegative = false;

	int start = 0;

	if ( text.Get(0) == "+" )
	{
		start = 1;
	} else if ( text.Get(0) == "-" )
	{
		start = 1;
		isNegative = true;
	}

	for ( int i = start; i < text.Length(); i++ )
	{
		int n = ToSingleDigit( text.Get(i) );

		if ( n > -1 )
		{
			f = f * 10;
			f = f + n;
			if ( foundDec )
			{
				atDec = atDec + 1;
			}
		} else if ( text.Get(i) == "." || text.Get(i) == "," ) 
		{
			if ( foundDec ) // we broke but we aren't gonna crash the game... just say what we have is fine.
				break;
			foundDec = true;
		}
	}
	
	f = f / Math.Pow( 10, atDec );

	if ( isNegative && !onlyPositive )
	{
		f = f * -1;
	}

	return f;
}

static string VectorToString( vector vec, int decimals = -1 ) 
{
	return "" + FormatFloat(vec[0], decimals) + ", " + FormatFloat(vec[1], decimals) + ", " + FormatFloat(vec[2], decimals);
}

static TStringArray GetChildrenFromBaseClass( string strConfigName, string strBaseClass )
{
	string child_name = "";
	int count = GetGame().ConfigGetChildrenCount ( strConfigName );
	TStringArray class_names = new TStringArray;

	for (int p = 0; p < count; p++)
	{
		GetGame().ConfigGetChildName ( strConfigName, p, child_name );

		if ( GetGame().IsKindOf(child_name, strBaseClass ) && ( child_name != strBaseClass ) )
		{
			class_names.Insert(child_name);
		}
	}

	return class_names;
}

static TVectorArray GetSpawnPoints()
{
	return { "15135.1 0 13901.1", "15017.8 0 13892.4", "14887.1 0 14547.9", "14749.7 0 13248.7",
			 "14697.6 0 13418.4", "14537.3 0 14755.7", "14415.3 0 14025.2", "14338.0 0 12859.5",
			 "14263.8 0 12748.7", "14172.2 0 12304.9", "14071.4 0 12033.3", "14054.9 0 11341.3",
			 "14017.8 0 2959.1", "13905.5 0 12489.7", "13852.4 0 11686.0", "13846.6 0 12050.0",
			 "13676.0 0 12262.1", "13617.4 0 12759.8", "13610.1 0 11223.6", "13594.3 0 4064.0",
			 "13587.8 0 6026.5", "13571.1 0 3056.8", "13552.6 0 4653.7", "13529.9 0 3968.3",
			 "13520.8 0 4223.7", "13504.0 0 5004.5", "13476.7 0 6136.3", "13441.6 0 5262.2",
			 "13426.6 0 5747.3", "13416.8 0 11840.4", "13400.8 0 4120.7", "13395.8 0 5902.8",
			 "13385.0 0 3946.6", "13374.4 0 6454.3", "13367.1 0 10837.1", "13366.3 0 4906.0",
			 "13337.1 0 5120.8", "13326.7 0 5489.1", "13312.7 0 6771.1", "13288.7 0 11415.1",
			 "13261.6 0 11785.2", "13171.6 0 6534.8", "13159.8 0 5401.7", "13155.2 0 5475.2",
			 "13084.9 0 7938.6", "13056.8 0 4848.5", "13048.1 0 8357.6", "13048.1 0 3867.7",
			 "12991.7 0 7287.1", "12983.0 0 5539.1", "12978.9 0 9727.8", "12950.2 0 5226.7",
			 "12942.1 0 8393.1", "12891.5 0 3673.9", "12628.7 0 10495.2", "12574.3 0 3592.8",
			 "12566.3 0 6682.6", "12465.2 0 8009.0", "12354.5 0 3480.0", "13262.8 0 7225.8" };
}

static TStringArray WorkingZombieClasses()
{
	return { "ZmbM_HermitSkinny_Base","ZmbM_HermitSkinny_Beige","ZmbM_HermitSkinny_Black","ZmbM_HermitSkinny_Green",
			 "ZmbM_HermitSkinny_Red","ZmbM_FarmerFat_Base","ZmbM_FarmerFat_Beige","ZmbM_FarmerFat_Blue","ZmbM_FarmerFat_Brown",
			 "ZmbM_FarmerFat_Green","ZmbF_CitizenANormal_Base","ZmbF_CitizenANormal_Beige","ZmbF_CitizenANormal_Brown",
			 "ZmbF_CitizenANormal_Blue","ZmbM_CitizenASkinny_Base","ZmbM_CitizenASkinny_Blue","ZmbM_CitizenASkinny_Brown",
			 "ZmbM_CitizenASkinny_Grey","ZmbM_CitizenASkinny_Red","ZmbM_CitizenBFat_Base","ZmbM_CitizenBFat_Blue","ZmbM_CitizenBFat_Red",
			 "ZmbM_CitizenBFat_Green","ZmbF_CitizenBSkinny_Base","ZmbF_CitizenBSkinny","ZmbM_PrisonerSkinny_Base","ZmbM_PrisonerSkinny",
			 "ZmbM_FirefighterNormal_Base","ZmbM_FirefighterNormal","ZmbM_FishermanOld_Base","ZmbM_FishermanOld_Blue","ZmbM_FishermanOld_Green",
			 "ZmbM_FishermanOld_Grey","ZmbM_FishermanOld_Red","ZmbM_JournalistSkinny_Base","ZmbM_JournalistSkinny","ZmbF_JournalistNormal_Base",
			 "ZmbF_JournalistNormal_Blue","ZmbF_JournalistNormal_Green","ZmbF_JournalistNormal_Red","ZmbF_JournalistNormal_White",
			 "ZmbM_ParamedicNormal_Base","ZmbM_ParamedicNormal_Blue","ZmbM_ParamedicNormal_Green","ZmbM_ParamedicNormal_Red",
			 "ZmbM_ParamedicNormal_Black","ZmbF_ParamedicNormal_Base","ZmbF_ParamedicNormal_Blue","ZmbF_ParamedicNormal_Green",
			 "ZmbF_ParamedicNormal_Red","ZmbM_HikerSkinny_Base","ZmbM_HikerSkinny_Blue","ZmbM_HikerSkinny_Green","ZmbM_HikerSkinny_Yellow",
			 "ZmbF_HikerSkinny_Base","ZmbF_HikerSkinny_Blue","ZmbF_HikerSkinny_Grey","ZmbF_HikerSkinny_Green","ZmbF_HikerSkinny_Red",
			 "ZmbM_HunterOld_Base","ZmbM_HunterOld_Autumn","ZmbM_HunterOld_Spring","ZmbM_HunterOld_Summer","ZmbM_HunterOld_Winter",
			 "ZmbF_SurvivorNormal_Base","ZmbF_SurvivorNormal_Blue","ZmbF_SurvivorNormal_Orange","ZmbF_SurvivorNormal_Red",
			 "ZmbF_SurvivorNormal_White","ZmbM_SurvivorDean_Base","ZmbM_SurvivorDean_Black","ZmbM_SurvivorDean_Blue","ZmbM_SurvivorDean_Grey",
			 "ZmbM_PolicemanFat_Base","ZmbM_PolicemanFat","ZmbF_PoliceWomanNormal_Base","ZmbF_PoliceWomanNormal","ZmbM_PolicemanSpecForce_Base",
			 "ZmbM_PolicemanSpecForce","ZmbM_SoldierNormal_Base","ZmbM_SoldierNormal","ZmbM_usSoldier_normal_Base",
			 "ZmbM_usSoldier_normal_Woodland","ZmbM_usSoldier_normal_Desert","ZmbM_CommercialPilotOld_Base","ZmbM_CommercialPilotOld_Blue",
			 "ZmbM_CommercialPilotOld_Olive","ZmbM_CommercialPilotOld_Brown","ZmbM_CommercialPilotOld_Grey","ZmbM_PatrolNormal_Base",
			 "ZmbM_PatrolNormal_PautRev","ZmbM_PatrolNormal_Autumn","ZmbM_PatrolNormal_Flat","ZmbM_PatrolNormal_Summer","ZmbM_JoggerSkinny_Base",
			 "ZmbM_JoggerSkinny_Blue","ZmbM_JoggerSkinny_Green","ZmbM_JoggerSkinny_Red","ZmbF_JoggerSkinny_Base","ZmbF_JoggerSkinny_Blue",
			 "ZmbF_JoggerSkinny_Brown","ZmbF_JoggerSkinny_Green","ZmbF_JoggerSkinny_Red","ZmbM_MotobikerFat_Base","ZmbM_MotobikerFat_Beige",
			 "ZmbM_MotobikerFat_Black","ZmbM_MotobikerFat_Blue","ZmbM_VillagerOld_Base","ZmbM_VillagerOld_Blue","ZmbM_VillagerOld_Green",
			 "ZmbM_VillagerOld_White","ZmbM_SkaterYoung_Base","ZmbM_SkaterYoung_Blue","ZmbM_SkaterYoung_Brown","ZmbM_SkaterYoung_Green",
			 "ZmbM_SkaterYoung_Grey","ZmbF_SkaterYoung_Base","ZmbF_SkaterYoung_Brown","ZmbF_SkaterYoung_Striped","ZmbF_SkaterYoung_Violet",
			 "ZmbF_DoctorSkinny_Base","ZmbF_DoctorSkinny","ZmbF_BlueCollarFat_Base","ZmbF_BlueCollarFat_Blue","ZmbF_BlueCollarFat_Green",
			 "ZmbF_BlueCollarFat_Red","ZmbF_BlueCollarFat_White","ZmbF_MechanicNormal_Base","ZmbF_MechanicNormal_Beige","ZmbF_MechanicNormal_Green",
			 "ZmbF_MechanicNormal_Grey","ZmbF_MechanicNormal_Orange","ZmbM_MechanicSkinny_Base","ZmbM_MechanicSkinny_Blue","ZmbM_MechanicSkinny_Grey",
			 "ZmbM_MechanicSkinny_Green","ZmbM_MechanicSkinny_Red","ZmbM_ConstrWorkerNormal_Base","ZmbM_ConstrWorkerNormal_Beige",
			 "ZmbM_ConstrWorkerNormal_Black","ZmbM_ConstrWorkerNormal_Green","ZmbM_ConstrWorkerNormal_Grey","ZmbM_HeavyIndustryWorker_Base",
			 "ZmbM_HeavyIndustryWorker","ZmbM_OffshoreWorker_Base","ZmbM_OffshoreWorker_Green","ZmbM_OffshoreWorker_Orange","ZmbM_OffshoreWorker_Red",
			 "ZmbM_OffshoreWorker_Yellow","ZmbF_NurseFat_Base","ZmbF_NurseFat","ZmbM_HandymanNormal_Base","ZmbM_HandymanNormal_Beige",
			 "ZmbM_HandymanNormal_Blue","ZmbM_HandymanNormal_Green","ZmbM_HandymanNormal_Grey","ZmbM_HandymanNormal_White","ZmbM_DoctorFat_Base",
			 "ZmbM_DoctorFat","ZmbM_Jacket_Base","ZmbM_Jacket_beige","ZmbM_Jacket_black","ZmbM_Jacket_blue","ZmbM_Jacket_bluechecks",
			 "ZmbM_Jacket_brown","ZmbM_Jacket_greenchecks","ZmbM_Jacket_grey","ZmbM_Jacket_khaki","ZmbM_Jacket_magenta","ZmbM_Jacket_stripes",
			 "ZmbF_PatientOld_Base","ZmbF_PatientOld","ZmbM_PatientSkinny_Base","ZmbM_PatientSkinny","ZmbF_ShortSkirt_Base","ZmbF_ShortSkirt_beige",
			 "ZmbF_ShortSkirt_black","ZmbF_ShortSkirt_brown","ZmbF_ShortSkirt_green","ZmbF_ShortSkirt_grey","ZmbF_ShortSkirt_checks",
			 "ZmbF_ShortSkirt_red","ZmbF_ShortSkirt_stripes","ZmbF_ShortSkirt_white","ZmbF_ShortSkirt_yellow","ZmbF_VillagerOld_Base",
			 "ZmbF_VillagerOld_Blue","ZmbF_VillagerOld_Green","ZmbF_VillagerOld_Red","ZmbF_VillagerOld_White","ZmbM_Soldier","ZmbM_SoldierAlice",
			 "ZmbM_SoldierHelmet","ZmbM_SoldierVest","ZmbM_SoldierAliceHelmet","ZmbM_SoldierVestHelmet","ZmbF_MilkMaidOld_Base",
			 "ZmbF_MilkMaidOld_Beige","ZmbF_MilkMaidOld_Black","ZmbF_MilkMaidOld_Green","ZmbF_MilkMaidOld_Grey","ZmbM_priestPopSkinny_Base",
			 "ZmbM_priestPopSkinny","ZmbM_ClerkFat_Base","ZmbM_ClerkFat_Brown","ZmbM_ClerkFat_Grey","ZmbM_ClerkFat_Khaki","ZmbM_ClerkFat_White",
			 "ZmbF_Clerk_Normal_Base","ZmbF_Clerk_Normal_Blue","ZmbF_Clerk_Normal_White","ZmbF_Clerk_Normal_Green","ZmbF_Clerk_Normal_Red" };
}

static set< Object > GetObjectsAt( vector from, vector to, Object ignore = NULL, float radius = 0.5, Object with = NULL )
{
	vector contact_pos;
	vector contact_dir;
	int contact_component;

	set< Object > geom = new set< Object >;
	set< Object > view = new set< Object >;

	DayZPhysics.RaycastRV( from, to, contact_pos, contact_dir, contact_component, geom, with, ignore, false, false, ObjIntersectGeom, radius );
	DayZPhysics.RaycastRV( from, to, contact_pos, contact_dir, contact_component, view, with, ignore, false, false, ObjIntersectView, radius );

	for ( int i = 0; i < geom.Count(); i++ )
	{
		view.Insert( geom[i] );
	}

	return view;
}

static Object GetPointerObject( float distance = 100.0, Object ignore = NULL, float radius = 0.5, Object with = NULL )
{
	vector dir = GetGame().GetPointerDirection();

	vector from = GetGame().GetCurrentCameraPosition();

	vector to = from + ( dir * distance );

	auto objs = GetObjectsAt( from, to, ignore, radius, with );

	if ( objs && objs.Count() > 0 )
	{
		return objs[ 0 ];
	}

	return NULL;
}

static Object GetCursorObject( float distance = 100.0, Object ignore = NULL, float radius = 0.5, Object with = NULL )
{
	vector rayStart = GetGame().GetCurrentCameraPosition();
	vector rayEnd = rayStart + GetGame().GetCurrentCameraDirection() * distance;

	auto objs = GetObjectsAt( rayStart, rayEnd, ignore, radius, with );

	if ( objs && objs.Count() > 0 )
	{
		return objs[ 0 ];
	}

	return NULL;
}

static bool SHIFT()
{
    return( ( KeyState( KeyCode.KC_LSHIFT ) > 0 ) || ( KeyState( KeyCode.KC_RSHIFT ) > 0 ) );
}

static bool CTRL()
{
    return( ( KeyState( KeyCode.KC_LCONTROL ) > 0 ) || ( KeyState( KeyCode.KC_RCONTROL ) > 0 ) );
}

static bool ALT()
{
    return( ( KeyState( KeyCode.KC_LMENU ) > 0 ) || ( KeyState( KeyCode.KC_RMENU ) > 0 ) );
}

static bool WINKEY()
{
    return( ( KeyState( KeyCode.KC_LWIN ) > 0 ) || ( KeyState( KeyCode.KC_RWIN ) > 0 ) );
}

static string FileAttributeToString( FileAttr attr )
{
	string fileType = "";
	if ( attr & FileAttr.DIRECTORY )
	{
		fileType = fileType + "DIRECTORY ";
	}
	if ( attr & FileAttr.HIDDEN )
	{
		fileType = fileType + "HIDDEN ";
	}
	if ( attr & FileAttr.READONLY )
	{
		fileType = fileType + "READONLY ";
	}
	if ( attr & FileAttr.INVALID )
	{
		fileType = fileType + "INVALID ";
	}
	return fileType;
}

static vector SnapToGround(vector pos)
{
	float pos_x = pos[0];
	float pos_z = pos[2];
	float pos_y = GetGame().SurfaceY( pos_x, pos_z );
	vector tmp_pos = Vector( pos_x, pos_y, pos_z );
	tmp_pos[1] = tmp_pos[1] + pos[1];

	return tmp_pos;
}

static void SnapToGroundNew( Object object ) 
{
	vector pos = object.GetPosition();
	pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
	
	vector clippingInfo[2];
	vector objectBBOX[2];
	
	object.GetCollisionBox( objectBBOX );
	object.ClippingInfo( clippingInfo );

	//float clipY = objectBBOX[1][1] / 2.0//- clippingInfo[0][1];
	//pos[1] = pos[1] + objectBBOX[1][1] - clipY;
	pos[1] = pos[1] + clippingInfo[1][1] / 2.0;//objectBBOX[0][1] - clipY

	object.SetPosition(pos);

	ForceTargetCollisionUpdate( object );
}

static void ForceTargetCollisionUpdate( Object oObj )
{
	if ( !oObj ) return;

	vector roll = oObj.GetOrientation();
	roll [ 2 ] = roll [ 2 ] - 1;
	oObj.SetOrientation( roll );
	roll [ 2 ] = roll [ 2 ] + 1;
	oObj.SetOrientation( roll );
}

static void ToggleCursor()
{
	if ( GetGame().GetInput().HasGameFocus( INPUT_DEVICE_MOUSE ) )
	{
		GetGame().GetInput().ChangeGameFocus( 1, INPUT_DEVICE_MOUSE );
		GetGame().GetUIManager().ShowUICursor( true );
	}
	else
	{
		GetGame().GetUIManager().ShowUICursor( false );
		GetGame().GetInput().ResetGameFocus( INPUT_DEVICE_MOUSE );
	}
}

/*
	Token types:
	 0 - error, no token
	 1 - defined token (special characters etc. . / * )
	 2 - quoted string. Quotes are removed -> TODO
	 3 - alphabetic string
	 4 - number
	 5 - end of line -> TODO
*/
static bool CheckStringType( string str, int type ) 
{
	for(int i = 0; i<str.Length(); i++ ) 
	{
		string character = str.Get(i);
		string token;
		int result = character.ParseStringEx(token);
		if ( result == type ) return true;
	}
	return false;
}

string GetRandomChildFromBaseClass( string strConfigName, string strBaseClass, int minScope = -1, string strIgnoreClass = "" )
{
    string child_name = "";
    int count = GetGame().ConfigGetChildrenCount ( strConfigName );

	if ( count == 0 )
		return strBaseClass;

    array< string > class_names = new array<string>;

    for ( int p = 0; p < count; p++ )
    {
        GetGame().ConfigGetChildName ( strConfigName, p, child_name );

		if ( child_name.Contains( strIgnoreClass ) )
			continue;

        if ( ( minScope != -1 ) && ( GetGame().ConfigGetInt( strConfigName + " " + child_name + " scope" ) < minScope ) ) 
			continue;

        if ( GetGame().IsKindOf( child_name, strBaseClass ) && ( child_name != strBaseClass ) )
        {
            class_names.Insert( child_name );
        }
    }

    return class_names.GetRandomElement(); // GetRandomChildFromBaseClass( strConfigName, class_names.GetRandomElement(), minScope, strIgnoreClass );
}

static array< string > FindFilesInLocation( string folder )
{
	GetLogger().Log( "FindFilesInLocation( " + folder + " ) Start", "JM_COT_StaticFunctions" );
	array< string > files = new array< string >;
	string fileName;
	FileAttr fileAttr;
	FindFileHandle findFileHandle = FindFile( folder + "*", fileName, fileAttr, 0 );
	if ( findFileHandle )
	{
		GetLogger().Log( "  File: " + fileName, "JM_COT_StaticFunctions" );
		if ( fileName.Length() > 0 && !( fileAttr & FileAttr.DIRECTORY) )
		{
			files.Insert( fileName );
		}
		
		while ( FindNextFile( findFileHandle, fileName, fileAttr ) )
		{
			GetLogger().Log( "  File: " + fileName, "JM_COT_StaticFunctions" );
			if ( fileName.Length() > 0 && !( fileAttr & FileAttr.DIRECTORY) )
			{
				files.Insert( fileName );
			}
		}
	}
	CloseFindFile( findFileHandle );
	GetLogger().Log( "FindFilesInLocation( " + folder + " ) Finished", "JM_COT_StaticFunctions" );
	return files;
}

static void DeleteFiles( string folder, array< string > files )
{
	GetLogger().Log( "DeleteFiles( " + folder + " ) Start", "JM_COT_StaticFunctions" );
	for ( int i = 0; i < files.Count(); i++ )
	{
		GetLogger().Log( "  File: " + folder + files[i], "JM_COT_StaticFunctions" );
		DeleteFile( folder + files[i] );
	}
	GetLogger().Log( "DeleteFiles( " + folder + " ) Finished", "JM_COT_StaticFunctions" );
}

static string COT_FILE_EXIST = "do-not-delete";

static void CreateFilesExist( string folder )
{
	FileHandle file = OpenFile( folder + COT_FILE_EXIST, FileMode.WRITE );

	if ( file == 0 )
	{
		GetLogger().Err( "[COT::StaticFunctions] Can't write to the default file in " + folder, "JM_COT_StaticFunctions" );
		return;
	}

	FPrint( file, "Do not delete this file, this file allows the other files to be read. If it can't find this file the folder is cleared!" );
	CloseFile( file );
}

static bool ArrayContains( array< string > arr, string match )
{
	if ( arr.Count() == 0 ) return false;

	for ( int i = 0; i < arr.Count(); i++ )
	{
		if ( arr[i] == match )
			return true;
	}

	return false;
}