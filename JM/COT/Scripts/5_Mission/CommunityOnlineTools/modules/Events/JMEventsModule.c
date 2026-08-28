// =============================================================================
//  JMEventsModule
//
//  Admin UI over dynamic + static world events: heli crashes, contaminated
//  zones, airdrops, and a few static map objects. Delegates everything to
//  JMEventsAdapter.
// =============================================================================
class JMEventsModule: JMEntityManagerModule
{
	override JMEntityManagerAdapter CreateAdapter()
	{
		return new JMEventsAdapter();
	}

	override int GetRPCRequest()    { return JMEventsModuleRPC.Request;     }
	override int GetRPCSend()       { return JMEventsModuleRPC.Send;        }
	override int GetRPCAction()     { return JMEventsModuleRPC.Action;      }
	override int GetRPCSendUpsert() { return JMEventsModuleRPC.SendUpsert;  }
	override int GetRPCSendRemove() { return JMEventsModuleRPC.SendRemove;  }

	override void GetWebhookTypes( out array<string> types )
	{
		types.Insert( "Teleport" );
		types.Insert( "Delete"   );
		types.Insert( "Spawn"    );
	}
}
