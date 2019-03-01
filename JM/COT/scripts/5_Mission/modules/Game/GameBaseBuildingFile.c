const string GAME_BASEBUILDING_FOLDER = ROOT_COT_DIR + "GameModule_BaseBuilding";

class GameBaseBuildingFile
{
    string Name;

    ref array< ref GameBaseBuildingSpawn > BaseBuildingAttachments = new ref array< ref GameBaseBuildingSpawn >;

    static ref GameBaseBuildingFile Load( string file )
    {
        ref GameBaseBuildingFile settings = new ref GameBaseBuildingFile;

        JsonFileLoader< GameBaseBuildingFile >.JsonLoadFile( GAME_BASEBUILDING_FOLDER + "/" + file, settings );

        return settings;
    }

    void Save()
    {
        JsonFileLoader< GameBaseBuildingFile >.JsonSaveFile( GAME_BASEBUILDING_FOLDER + "/" + Name, this );
    }

    void Defaults()
    {
    }

}