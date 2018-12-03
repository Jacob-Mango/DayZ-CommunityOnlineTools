class GameMeta
{
    autoptr ref array< string > Vehicles = new ref array< string >;
    autoptr ref array< string > BaseBuilding = new ref array< string >;

    static ref GameMeta DeriveFromSettings( ref GameSettings settings )
    {
        ref GameMeta meta = new ref GameMeta;

        for ( int i = 0; i < GameSettings_Vehicles.Count(); i++ )
        {
            meta.Vehicles.Insert( GameSettings_Vehicles.GetKey( i ) );
        }

        for ( int j = 0; j < GameSettings_BaseBuilding.Count(); j++ )
        {
            meta.BaseBuilding.Insert( GameSettings_BaseBuilding.GetKey( j ) );
        }

        return meta;
    }
}