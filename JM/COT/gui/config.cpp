class CfgPatches
{
    class JM_COT_GUI
    {
        units[]={};
        weapons[]={};
        requiredVersion=0.1;
        requiredAddons[]={
			"DZ_Data",
			"DZ_Gear_Navigation"
		};
    };
};

class RscMapControl;
class JM_COT_MapMenu: RscMapControl
{
	scaleMin=0.02;
	scaleMax=3.000;
	scaleDefault=1;
	ptsPerSquareSea=8;
	ptsPerSquareTxt=10;
	ptsPerSquareCLn=10;
	ptsPerSquareExp=10;
	ptsPerSquareCost=10;
	ptsPerSquareFor=9;
	ptsPerSquareForEdge=15;
	ptsPerSquareRoad=4;
	ptsPerSquareObj=15;
	maxSatelliteAlpha=1;
	alphaFadeStartScale=0.08;
	alphaFadeEndScale=0.16;
	userMapPath="dz\gear\navigation\data\usermap";
	maxUserMapAlpha=0;
	alphaUserMapFadeStartScale=0.34;
	alphaUserMapFadeEndScale=0.34;
	showCountourInterval=0;
	colorLevels[]={0.64999998,0.60000002,0.44999999,0.94999999};
	colorSea[]={0.63999999,0.75999999,0.88999999,1};
	colorForest[]={0.36000001,0.77999997,0.079999998,0.5};
	colorRocks[]={0.5,0.5,0.5,0.5};
	colorCountlines[]={0.85000002,0.80000001,0.64999998,1};
	colorMainCountlines[]={0.44999999,0.40000001,0.25,1};
	colorCountlinesWater[]={0.25,0.40000001,0.5,0.30000001};
	colorMainCountlinesWater[]={0.25,0.40000001,0.5,0.89999998};
	colorPowerLines[]={0.1,0.1,0.1,1};
	colorRailWay[]={0.80000001,0.2,0,1};
	colorForestBorder[]={0.40000001,0.80000001,0,1};
	colorRocksBorder[]={0.5,0.5,0.5,1};
	colorOutside[]={1,1,1,1};
	colorTracks[]={0.77999997,0.66000003,0.34,1};
	colorRoads[]={0.69,0.43000001,0.23,1};
	colorMainRoads[]={0.52999997,0.34999999,0,1};
	colorTracksFill[]={0.95999998,0.91000003,0.60000002,1};
	colorRoadsFill[]={0.92000002,0.73000002,0.41,1};
	colorMainRoadsFill[]={0.83999997,0.61000001,0.20999999,1};
	colorGrid[]={0.15000001,0.15000001,0.050000001,0.89999998};
	colorGridMap[]={0.25,0.25,0.1,0.75};
	fontNames="gui/fonts/sdf_MetronBook24";
	sizeExNames=0.039999999;
	colorNames[]={0,0,0,1};
	fontGrid="gui/fonts/sdf_MetronBook24";
	sizeExGrid=0.0099999998;
	fontLevel="gui/fonts/sdf_MetronBook24";
	sizeExLevel=0.0099999998;
	colorMountPoint[]={0.44999999,0.40000001,0.25,1};
	mapPointDensity=0.12;
	text="";
	fontLabel="gui/fonts/sdf_MetronBook24";
	fontInfo="gui/fonts/sdf_MetronBook24";
	class Legend
	{
		x=0.050000001;
		y=0.85000002;
		w=0.40000001;
		h=0.1;
		font="gui/fonts/sdf_MetronBook24";
		sizeEx=0.02;
		colorBackground[]={1,1,1,0.5};
		color[]={0,0,0,1};
	};
	class Bush
	{
		icon="\dz\gear\navigation\data\map_bush_ca.paa";
		color[]={0.40000001,0.80000001,0,0.80000001};
		size=14;
		importance="0.2 * 14 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class SmallTree
	{
		icon="\dz\gear\navigation\data\map_smalltree_ca.paa";
		color[]={0.40000001,0.80000001,0,0.80000001};
		size=12;
		importance="0.6 * 12 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Tree
	{
		icon="\dz\gear\navigation\data\map_tree_ca.paa";
		color[]={0.40000001,0.80000001,0,0.80000001};
		size=12;
		importance="0.9 * 16 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Rock
	{
		icon="\dz\gear\navigation\data\map_rock_ca.paa";
		color[]={0.1,0.1,0.1,0.80000001};
		size=12;
		importance="0.5 * 12 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Fuelstation
	{
		icon="\dz\gear\navigation\data\map_fuelstation_ca.paa";
		size=16;
		color[]={0,0,0,1};
		importance="2 * 16 * 0.05";
		coefMin=0.75;
		coefMax=4;
	};
	class Lighthouse
	{
		icon="\dz\gear\navigation\data\map_lighthouse_ca.paa";
		size=14;
		color[]={0,0,0,1};
		importance="3 * 16 * 0.05";
		coefMin=0.89999998;
		coefMax=4;
	};
	class Stack
	{
		icon="\dz\gear\navigation\data\map_stack_ca.paa";
		size=16;
		color[]={0,0,0,1};
		importance="2 * 16 * 0.05";
		coefMin=0.89999998;
		coefMax=4;
	};
	class Transmitter
	{
		icon="\dz\gear\navigation\data\map_transmitter_ca.paa";
		color[]={0,0,0,1};
		size=16;
		importance="2 * 16 * 0.05";
		coefMin=0.89999998;
		coefMax=4;
	};
	class Watertower
	{
		icon="\dz\gear\navigation\data\map_watertower_ca.paa";
		color[]={0.2,0.44999999,0.69999999,1};
		size=16;
		importance="1.2 * 16 * 0.05";
		coefMin=0.89999998;
		coefMax=4;
	};
	class Shipwreck
	{
		icon="\dz\gear\navigation\data\map_shipwreck_ca.paa";
		color[]={0,0,0,1};
		size=16;
		importance="1.2 * 16 * 0.05";
		coefMin=0.89999998;
		coefMax=4;
	};
	class Monument
	{
		icon="\dz\gear\navigation\data\map_monument_ca.paa";
		color[]={0,0,0,1};
		size=20;
		importance="1.2 * 16 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class BusStop
	{
		icon="\dz\gear\navigation\data\map_busstop_ca.paa";
		color[]={0.15000001,0.25999999,0.87,1};
		size=12;
		importance="1 * 10 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Fountain
	{
		icon="\dz\gear\navigation\data\map_fountain_ca.paa";
		color[]={0.2,0.44999999,0.69999999,1};
		size=15;
		importance="1 * 12 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Waterpump
	{
		icon="\dz\gear\navigation\data\map_waterpump_ca.paa";
		color[]={0.2,0.44999999,0.69999999,1};
		size=16;
		importance="1 * 12 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Hospital
	{
		icon="\dz\gear\navigation\data\map_hospital_ca.paa";
		color[]={0.15000001,0.25999999,0.87,1};
		size=17;
		importance="2 * 16 * 0.05";
		coefMin=0.5;
		coefMax=4;
	};
	class Store
	{
		icon="\dz\gear\navigation\data\map_store_ca.paa";
		color[]={0,0,0,1};
		size=20;
		importance="2 * 16 * 0.05";
		coefMin=0.5;
		coefMax=4;
	};
	class Police
	{
		icon="\dz\gear\navigation\data\map_police_ca.paa";
		color[]={0.11,0.1,0.25,1};
		size=20;
		importance="2 * 16 * 0.05";
		coefMin=0.5;
		coefMax=4;
	};
	class FireDep
	{
		icon="\dz\gear\navigation\data\map_firedep_ca.paa";
		color[]={0.764,0,0,1};
		size=20;
		importance="2 * 16 * 0.05";
		coefMin=0.5;
		coefMax=4;
	};
	class TouristSign
	{
		icon="\dz\gear\navigation\data\map_tsign_ca.paa";
		color[]={0,0,0,1};
		size=18;
		importance="2 * 16 * 0.05";
		coefMin=0.5;
		coefMax=4;
	};
	class Tourism
	{
		icon="\dz\gear\navigation\data\map_tourism_ca.paa";
		color[]={0,0,0,1};
		size=14;
		importance="2 * 16 * 0.05";
		coefMin=0.5;
		coefMax=4;
	};
	class Cross
	{
		icon="\dz\gear\navigation\data\map_cross_ca.paa";
		size=16;
		color[]={0,0,0,1};
		importance="0.7 * 16 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Chapel
	{
		icon="\dz\gear\navigation\data\map_chapel_ca.paa";
		color[]={0,0,0,1};
		size=16;
		importance="1 * 16 * 0.05";
		coefMin=0.89999998;
		coefMax=4;
	};
	class Church
	{
		icon="\dz\gear\navigation\data\map_church_ca.paa";
		color[]={0,0,0,1};
		size=10;
		importance="2 * 16 * 0.05";
		coefMin=0.89999998;
		coefMax=4;
	};
	class ViewTower
	{
		icon="\dz\gear\navigation\data\map_viewtower_ca.paa";
		color[]={0,0,0,1};
		size=16;
		importance="2.5 * 16 * 0.05";
		coefMin=0.5;
		coefMax=4;
	};
	class TouristShelter
	{
		icon="\dz\gear\navigation\data\map_tshelter_ca.paa";
		color[]={0,0,0,1};
		size=16;
		importance="2 * 16 * 0.05";
		coefMin=0.5;
		coefMax=4;
	};
	class Quay
	{
		icon="\dz\gear\navigation\data\map_quay_ca.paa";
		size=16;
		color[]={0,0,0,1};
		importance="2 * 16 * 0.05";
		coefMin=0.5;
		coefMax=4;
	};
	class Bunker
	{
		icon="\dz\gear\navigation\data\map_bunker_ca.paa";
		color[]={0,0,0,1};
		size=14;
		importance="1.5 * 14 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Fortress
	{
		icon="\dz\gear\navigation\data\map_fortress_ca.paa";
		size=16;
		color[]={0,0,0,1};
		importance="2 * 16 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Rocks
	{
		icon="\dz\gear\navigation\data\map_rock_ca.paa";
		color[]={0.1,0.1,0.1,0.80000001};
		size=12;
		importance="0.5 * 12 * 0.05";
		coefMin=0.25;
		coefMax=4;
	};
	class Ruin
	{
		icon="\dz\gear\navigation\data\map_ruin_ca.paa";
		size=16;
		color[]={0,0,0,1};
		importance="1.2 * 16 * 0.05";
		coefMin=1;
		coefMax=4;
	};
};