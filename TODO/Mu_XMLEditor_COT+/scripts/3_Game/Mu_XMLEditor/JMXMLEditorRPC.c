/*
	Wlasna pula ID dla RPC tego modulu.
	COT rezerwuje bloki po ~20 numerow na modul (patrz 3_Game/CommunityOnlineTools/RPC.c,
	ostatni zajety blok to JMLoadoutModuleRPC od 10460). Bierzemy nastepny wolny blok: 10480.
	Jest to zwykly, niezalezny enum we wlasnym pliku - nie modyfikujemy pliku RPC.c z COT.
*/
enum JMXMLEditorModuleRPC
{
	INVALID = 10480,
	GetTypesFiles,
	HandleTypesFiles,
	GetTypesFromFile,
	HandleTypesList,
	GetDetails,
	HandleDetails,
	SaveChanges,
	ScanLoot,
	HandleScanResults,
	COUNT
};
