// =============================================================================
//  JMTransformHistoryEntry
//
//  Undo/redo for one ESP move/rotate gesture, on the shared global stack.
//  Pushed once per gesture (JMESPWidgetHandler.EndDrag, via JMESPModule.
//  RecordTransformHistory) rather than per throttled position/orientation
//  RPC sent during the drag - the drag itself has no natural single point
//  to record an undo step at, but its end does.
// =============================================================================
class JMTransformHistoryEntry: JMActionHistoryEntry
{
	protected Object m_Target;
	protected vector m_Position;
	protected vector m_Orientation;

	void JMTransformHistoryEntry( Object target, vector previousPosition, vector previousOrientation )
	{
		RequiredPermission = JMConstants.PERM_ESP_OBJECT_SETPOSITION;

		m_Target      = target;
		m_Position    = previousPosition;
		m_Orientation = previousOrientation;
	}

	override bool CanUndo()
	{
		return m_Target != NULL;
	}

	override bool CanRedo()
	{
		return m_Target != NULL;
	}

	//! Undo and Redo are the same swap in either direction: restore the
	//! stored transform and remember what was just replaced, so the next
	//! press (whichever button it is) goes right back.
	override bool Undo()
	{
		if ( !m_Target )
			return false;

		vector currentPos = m_Target.GetPosition();
		vector currentOri = m_Target.GetOrientation();

		Transport transport;
		if ( Class.CastTo( transport, m_Target ) )
		{
			COT.ForceTransportPositionAndOrientation( transport, m_Position, m_Orientation );
		}
		else
		{
			m_Target.SetPosition( m_Position );
			m_Target.SetOrientation( m_Orientation );
		}

		m_Position    = currentPos;
		m_Orientation = currentOri;

		return true;
	}

	override bool Redo()
	{
		return Undo();
	}

	override Object GetTarget()
	{
		return m_Target;
	}

	override void SetTarget( Object target )
	{
		m_Target = target;
	}

	override string GetDescription()
	{
		return "Move " + DescribeObject( m_Target );
	}
}
