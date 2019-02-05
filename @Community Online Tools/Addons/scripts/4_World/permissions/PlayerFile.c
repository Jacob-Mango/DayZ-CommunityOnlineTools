modded class PlayerFile
{
    override bool Load( PlayerIdentity identity )
    {
        bool foundFile = false;
        foundFile = super.Load( identity );

        if ( !foundFile )
        {
            Roles.Insert( "everyone" );
        }

        return foundFile;
    }
}