/*
	JMXMLEditorMapMarkers
	-----------------------
	Prosty, statyczny "schowek" pozycji znalezionych podczas skanu lootu.
	Wypelniany przez JMXMLEditorForm po otrzymaniu wynikow skanu, odczytywany
	cyklicznie przez zmodowany JMMapForm (patrz JMMapFormPatch.c), zeby
	wyswietlic znaczniki na istniejacej mapie COT - bez budowania wlasnej
	mapy od zera.
*/
class JMXMLEditorMapMarkers
{
	static ref array<vector> Positions = new array<vector>;
	static string Label = "";

	static void SetMarkers( string label, array<vector> positions )
	{
		Label = label;

		Positions = new array<vector>;
		foreach ( vector pos : positions )
			Positions.Insert( pos );
	}

	static void Clear()
	{
		Positions = new array<vector>;
		Label = "";
	}
};
