//  JMWeatherNextState - one weighted edge of the dynamic weather state machine (target + chance).
//  Used by JMWeatherPhase.Edges (To = phase of the SAME state, or "" to leave it),
//  JMWeatherState.NextStates (To = another state) and JMWeatherState.Entry (phase drawn on entry).
//  Edges live on the node they leave, so there is no "from"; renames only touch naming nodes.
class JMWeatherNextState
{
	string To;

	//! Whole percent. Lists are kept summing to 100 by Normalize(), so the numbers
	//! on screen are the real odds - but Roll() does not depend on it.
	int Chance;

	void JMWeatherNextState()
	{
		To     = "";
		Chance = 0;
	}

	//! Sum of the positive chances. A zero or negative chance is the absence of a
	//! candidate, however it got into the list.
	static int Total( array< ref JMWeatherNextState > edges )
	{
		if ( !edges )
			return 0;

		int total = 0;

		for ( int i = 0; i < edges.Count(); i++ )
		{
			if ( edges[i].Chance > 0 )
				total += edges[i].Chance;
		}

		return total;
	}

	//! Index of the edge drawn, or -1 when there is nothing to draw from. Chances are WEIGHTS
	//! against their own total, not a fixed 100 (30 and 10 pick the first 3 times in 4). -1
	//! means no candidates; an empty To on the drawn edge is a valid "leave the state".
	static int Roll( array< ref JMWeatherNextState > edges )
	{
		int total = Total( edges );

		if ( total <= 0 )
			return -1;

		int roll = Math.RandomInt( 0, total );
		int cumulative = 0;

		for ( int i = 0; i < edges.Count(); i++ )
		{
			if ( edges[i].Chance <= 0 )
				continue;

			cumulative += edges[i].Chance;

			if ( roll < cumulative )
				return i;
		}

		return -1;
	}

	//! Index of the edge naming `to`, or -1.
	static int Find( array< ref JMWeatherNextState > edges, string to )
	{
		if ( !edges )
			return -1;

		for ( int i = 0; i < edges.Count(); i++ )
		{
			if ( edges[i].To == to )
				return i;
		}

		return -1;
	}

	//! Drop every edge naming `to`, then rescale what is left to 100.
	static void Remove( array< ref JMWeatherNextState > edges, string to )
	{
		if ( !edges )
			return;

		for ( int i = edges.Count() - 1; i >= 0; i-- )
		{
			if ( edges[i].To == to )
				edges.Remove( i );
		}

		Normalize( edges );
	}

	//! Point every edge naming `from` at `to` instead.
	static void Rename( array< ref JMWeatherNextState > edges, string from, string to )
	{
		if ( !edges )
			return;

		for ( int i = 0; i < edges.Count(); i++ )
		{
			if ( edges[i].To == from )
				edges[i].To = to;
		}
	}

	//! Drop non-positive and duplicate-target edges and rescale the rest to add
	//! up to exactly 100. Rounding drift is put on the last edge so the column
	//! reads 100 however the shares divide. An empty result stays empty.
	static void Normalize( array< ref JMWeatherNextState > edges )
	{
		if ( !edges )
			return;

		int i;

		for ( i = edges.Count() - 1; i >= 0; i-- )
		{
			if ( edges[i].Chance <= 0 || Find( edges, edges[i].To ) != i )
				edges.Remove( i );
		}

		int total = Total( edges );

		if ( total <= 0 || edges.Count() == 0 )
			return;

		int running = 0;

		for ( i = 0; i < edges.Count(); i++ )
		{
			edges[i].Chance = Math.Round( ( edges[i].Chance * 100.0 ) / total );
			running += edges[i].Chance;
		}

		int last = edges.Count() - 1;
		edges[last].Chance = Math.Clamp( edges[last].Chance + ( 100 - running ), 1, 100 );
	}

	//! Deep copy, so a payload never shares edges with the stored config.
	static array< ref JMWeatherNextState > Copy( array< ref JMWeatherNextState > edges )
	{
		array< ref JMWeatherNextState > result = new array< ref JMWeatherNextState >;

		if ( !edges )
			return result;

		JMWeatherNextState copy;

		for ( int i = 0; i < edges.Count(); i++ )
		{
			copy = new JMWeatherNextState;
			copy.To     = edges[i].To;
			copy.Chance = edges[i].Chance;

			result.Insert( copy );
		}

		return result;
	}
}
