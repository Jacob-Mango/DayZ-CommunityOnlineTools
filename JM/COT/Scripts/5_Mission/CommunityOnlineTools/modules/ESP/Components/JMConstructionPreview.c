//! Client-side, read-only drawing of construction parts for the ESP action menu: a preview of
//! the hovered construction row, and per-building nodes (one box per right-click target).
//! Shapes only: showing a part is a server-synced animation source and hiding rubble spawns
//! rubble, which the client may not fake. Boxes are model-space, drawn as ONCE shapes.
class JMConstructionPreview: Managed
{
	static const int COLOR_BUILD     = JMTheme.SUCCESS;
	static const int COLOR_REQUIRED  = JMTheme.WARNING;
	static const int COLOR_DISMANTLE = JMTheme.DANGER;
	static const int COLOR_PART      = JMTheme.INFO;
	static const int COLOR_NODE      = JMTheme.BLUE_400;

	//! Opacity of a box's faces. Its edges are drawn opaque and through walls,
	//! the faces are not: a part inside a wall still shows where it is.
	static const float FILL_OPACITY = 0.15;

	//! Radius of the marker at the centre of a node, in metres.
	static const float NODE_MARKER_RADIUS = 0.1;

	protected EntityAI m_PreviewTarget;

	//! Min and max of each box in turn, model space.
	protected ref array<vector> m_PreviewBoxes;

	//! One per box.
	protected ref array<int> m_PreviewColors;

	//! The buildings showing their nodes, and their boxes as min/max pairs.
	protected ref array<EntityAI> m_NodeTargets;
	protected ref array<ref array<vector>> m_NodeBoxes;

	void JMConstructionPreview()
	{
		m_PreviewBoxes = new array<vector>;
		m_PreviewColors = new array<int>;
		m_NodeTargets = new array<EntityAI>;
		m_NodeBoxes = new array<ref array<vector>>;
	}

	//! What building `partNames` puts up: each named part, and in a second colour
	//! every missing required part COT builds along with it.
	void PreviewBuild( EntityAI target, notnull TStringArray partNames )
	{
		ClearPreview();

		ConstructionBase construction = JMESPModule.GetConstructionOf( target );
		if ( !construction )
			return;

		TStringArray toBuild = new TStringArray;

		foreach ( string partName : partNames )
			construction.COT_GetPartsToBuild( partName, toBuild );

		foreach ( string buildName : toBuild )
		{
			int color = COLOR_REQUIRED;

			if ( partNames.Find( buildName ) > -1 )
				color = COLOR_BUILD;

			AddPreviewBox( construction, buildName, color );
		}

		m_PreviewTarget = target;
	}

	//! What dismantling `partNames` takes down: each named part and every built
	//! part that depends on it.
	void PreviewDismantle( EntityAI target, notnull TStringArray partNames )
	{
		ClearPreview();

		ConstructionBase construction = JMESPModule.GetConstructionOf( target );
		if ( !construction )
			return;

		TStringArray toDismantle = new TStringArray;

		foreach ( string partName : partNames )
			construction.COT_GetPartsToDismantle( partName, toDismantle );

		foreach ( string dismantleName : toDismantle )
			AddPreviewBox( construction, dismantleName, COLOR_DISMANTLE );

		m_PreviewTarget = target;
	}

	//! Just where the named parts are, for a row that neither builds nor
	//! dismantles - a built part's own page, a repair.
	void PreviewParts( EntityAI target, notnull TStringArray partNames )
	{
		ClearPreview();

		ConstructionBase construction = JMESPModule.GetConstructionOf( target );
		if ( !construction )
			return;

		foreach ( string partName : partNames )
			AddPreviewBox( construction, partName, COLOR_PART );

		m_PreviewTarget = target;
	}

	void ClearPreview()
	{
		m_PreviewTarget = NULL;
		m_PreviewBoxes.Clear();
		m_PreviewColors.Clear();
	}

	protected void AddPreviewBox( ConstructionBase construction, string partName, int color )
	{
		vector min;
		vector max;

		if ( !construction.COT_GetPartBounds( partName, min, max ) )
			return;

		m_PreviewBoxes.Insert( min );
		m_PreviewBoxes.Insert( max );
		m_PreviewColors.Insert( color );
	}

	bool IsShowingNodes( EntityAI target )
	{
		return m_NodeTargets.Find( target ) > -1;
	}

	//! A node is one view geometry selection a construction part answers to - the
	//! thing JMESPModule.OpenWorldContextMenu narrows the menu by. A part with no
	//! selection of its own answers to its main part's, so parts sharing one are
	//! one node.
	void SetShowNodes( EntityAI target, bool show )
	{
		int index = m_NodeTargets.Find( target );

		if ( !show )
		{
			if ( index > -1 )
			{
				m_NodeTargets.Remove( index );
				m_NodeBoxes.Remove( index );
			}

			return;
		}

		if ( index > -1 )
			return;

		ConstructionBase construction = JMESPModule.GetConstructionOf( target );
		if ( !construction )
			return;

		map< string, ref JMConstructionPartData > parts = new map< string, ref JMConstructionPartData >;
		construction.COT_GetParts( parts, false );

		TStringArray selections = new TStringArray;
		array<vector> boxes = new array<vector>;

		foreach ( string partName, JMConstructionPartData part : parts )
		{
			vector min;
			vector max;
			string selection = partName;

			if ( !construction.COT_GetSelectionBounds( selection, min, max ) )
			{
				selection = part.m_MainPartName;

				if ( !construction.COT_GetSelectionBounds( selection, min, max ) )
					continue;
			}

			if ( selections.Find( selection ) > -1 )
				continue;

			selections.Insert( selection );
			boxes.Insert( min );
			boxes.Insert( max );
		}

		m_NodeTargets.Insert( target );
		m_NodeBoxes.Insert( boxes );
	}

	//! Called every frame on the client.
	void Draw()
	{
		vector transform[4];

		if ( m_PreviewTarget )
		{
			m_PreviewTarget.GetTransform( transform );

			for ( int i = 0; i < m_PreviewColors.Count(); ++i )
				DrawBox( transform, m_PreviewBoxes[i * 2], m_PreviewBoxes[i * 2 + 1], m_PreviewColors[i] );
		}

		//! Backwards, because a building that was streamed out or deleted is
		//! dropped on the way.
		for ( int t = m_NodeTargets.Count() - 1; t >= 0; --t )
		{
			EntityAI target = m_NodeTargets[t];

			if ( !target )
			{
				m_NodeTargets.Remove( t );
				m_NodeBoxes.Remove( t );
				continue;
			}

			target.GetTransform( transform );

			array<vector> boxes = m_NodeBoxes[t];

			for ( int b = 0; b < boxes.Count(); b += 2 )
			{
				vector min = boxes[b];
				vector max = boxes[b + 1];

				DrawBox( transform, min, max, COLOR_NODE );

				vector center = target.ModelToWorld( ( min + max ) * 0.5 );
				Shape.CreateSphere( COLOR_NODE, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.NOOUTLINE, center, NODE_MARKER_RADIUS );
			}
		}
	}

	protected void DrawBox( vector transform[4], vector min, vector max, int color )
	{
		Shape faces = Shape.Create( ShapeType.BBOX, JMTheme.Fade( color, FILL_OPACITY ), ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOZWRITE | ShapeFlags.NOOUTLINE, min, max );
		faces.SetMatrix( transform );

		Shape edges = Shape.Create( ShapeType.BBOX, color, ShapeFlags.ONCE | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER, min, max );
		edges.SetMatrix( transform );
	}
}
