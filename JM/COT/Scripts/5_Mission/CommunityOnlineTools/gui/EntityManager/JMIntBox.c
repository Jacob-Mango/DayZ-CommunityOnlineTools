// =============================================================================
//  JMIntBox - tiny class wrapper to stash an int in UserData slots that only
//  accept Class references.
// =============================================================================
class JMIntBox: Managed
{
	int Value;

	void JMIntBox( int v = 0 ) { Value = v; }
}
