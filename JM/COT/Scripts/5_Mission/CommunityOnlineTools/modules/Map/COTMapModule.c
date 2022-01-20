[CF_RegisterModule(COTMapModule)]
class COTMapModule : COTModule
{
	MapWidget cot_mapmodule_mapwidget;

	override void OnInit()
	{
		super.OnInit();

		GetPermissionsManager().RegisterPermission("Admin.Map");

		GetPermissionsManager().RegisterPermission("Admin.Map.View");
		GetPermissionsManager().RegisterPermission("Admin.Map.Players");

		GetPermissionsManager().RegisterPermission("Admin.Player.Teleport.Position");
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission("Admin.Map.View");
	}

	override string GetInputToggle()
	{
		return "UACOTToggleMap";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/mapview_form.layout";
	}

	override string GetTitle()
	{
		return "#STR_COT_MAP_MODULE_NAME";
	}

	override string GetIconName()
	{
		return "M";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnShow(Class sender, CF_EventArgs args)
	{
		if (!GetPermissionsManager().HasPermission("Admin.Map.View"))
			return;

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateMapMarkers, 1000, true);
	}

	override void OnHide(Class sender, CF_EventArgs args)
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateMapMarkers);
	}

	void UpdateMapMarkers()
	{
		cot_mapmodule_mapwidget.ClearUserMarks();

		if (!GetPermissionsManager().HasPermission("Admin.Map.Players"))
			return;

		foreach (auto player : GetPermissionsManager().GetPlayers())
		{
			cot_mapmodule_mapwidget.AddUserMark(player.GetPosition(), player.GetName(), ARGB(255, 230, 20, 20), "\\JM\\COT\\GUI\\textures\\dot.paa");
		}

		GetCommunityOnlineTools().RefreshClientPositions();
	}

	void OnDoubleClick(CF_ModelBase sender, CF_MouseEventArgs args)
	{
		JMTeleportModule mod;
		if (!CF_Modules<JMTeleportModule>.Get(mod))
		{
			return;
		}

		mod.Position(SnapToGround(cot_mapmodule_mapwidget.ScreenToMap(Vector(args.X, args.Y, 0))));
	}
};
