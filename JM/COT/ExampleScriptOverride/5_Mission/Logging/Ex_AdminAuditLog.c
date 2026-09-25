#ifdef JM_CommunityOnlineTools
// Example: intercepting COT permissions and logging admin activity (JMPermissions).
modded class JMPermissions
{
	override static bool Has( string permission )
	{
		bool result = super.Has( permission );
		return result;
	}
}
#endif
