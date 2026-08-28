/*
	Doczepia znaczniki znalezionego lootu (z JMXMLEditorMapMarkers) do
	istniejacego cyklu odswiezania mapy COT (JMMapForm::UpdateMapMarkers,
	odpalane co 1s gdy mapa jest widoczna). Nie tworzymy wlasnego MapWidget -
	korzystamy z tego, ktory juz ma "Mapa" w COT.
*/
modded class JMMapForm
{
	override void UpdateMapMarkers()
	{
		super.UpdateMapMarkers();

		UpdateXMLEditorLootMarkers();
	}

	private void UpdateXMLEditorLootMarkers()
	{
		if ( !JMXMLEditorMapMarkers.Positions || JMXMLEditorMapMarkers.Positions.Count() == 0 )
			return;

		int color = ARGB( 255, 255, 140, 0 ); // pomaranczowy - odroznia sie od graczy (zolty/czerwony/zielony)

		foreach ( vector pos : JMXMLEditorMapMarkers.Positions )
		{
			m_MapWidget.AddUserMark( pos, JMXMLEditorMapMarkers.Label, color, JM_COT_ICON_DOT + ".paa" );
		}
	}
};
