#ifdef JM_CommunityOnlineTools
// Example: tracking custom teleport undo/redo steps in JMTeleportHistory.
modded class JMTeleportHistory
{
	static void RecordSubModMove( Object target )
	{
		PushObject( target );
	}
}
#endif
