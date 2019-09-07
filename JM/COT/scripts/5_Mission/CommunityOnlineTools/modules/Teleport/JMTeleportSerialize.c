const string TELEPORT_SETTINGS_FILE = JMConstants.DIR_COT + "TeleportModule.json";

class JMTeleportSerialize
{
	ref array< ref JMTeleportLocation > Locations;

	void JMTeleportSerialize()
	{
		Locations = new array< ref JMTeleportLocation >;
	}

	static ref JMTeleportSerialize Load()
	{
		ref JMTeleportSerialize settings = new JMTeleportSerialize();

		if ( FileExist( TELEPORT_SETTINGS_FILE ) )
		{
			JsonFileLoader<JMTeleportSerialize>.JsonLoadFile( TELEPORT_SETTINGS_FILE, settings );
		} else 
		{
			settings.Defaults();
			settings.Save();
		}

		return settings;
	}

	void Save()
	{
		JsonFileLoader<JMTeleportSerialize>.JsonSaveFile( TELEPORT_SETTINGS_FILE, this );
	}

	void AddLocation( string name, vector position, float radius = 4.0 )
	{
		Locations.Insert( new JMTeleportLocation( name, position, radius ) );
	}

	void Defaults()
	{
		AddLocation( "Altar", "8090.49 0 9326.95" );
		AddLocation( "Balota", "4393.72 0 2412.05" );
		AddLocation( "Balota Airstrip", "5025.65 0 2363.44" );
		AddLocation( "Bashnya", "4156.51 0 11747.29" );
		AddLocation( "Bay Mutnaya", "5634.24 0 1947.33" );
		AddLocation( "Bay Tikhaya", "1190.83 0 2194.76" );
		AddLocation( "Bay Zelenaya", "11140.75 0 3090.17" );
		AddLocation( "Belaya Polana", "14148.24 0 15004.23" );
		AddLocation( "Berezhki", "15152.52 0 13815.69" );
		AddLocation( "Berezino", "11856.29 0 8991.54" );
		AddLocation( "Berezino Lumberyard", "12661.27 0 9590.91" );
		AddLocation( "Berezino Shipyard", "13178.96 0 10213.62" );
		AddLocation( "Black Forest", "9083.59 0 7555.09" );
		AddLocation( "Black Lake", "13360.76 0 11902.36" );
		AddLocation( "Black Mtn", "10246.34 0 11984.03" );
		AddLocation( "Bogatyrka", "1504.85 0 8940.5" );
		AddLocation( "Bor", "3317.02 0 4014.26" );
		AddLocation( "Cap Golova", "8350.07 0 2443.64" );
		AddLocation( "Cernaya Polana", "12199.96 0 13666.45" );
		AddLocation( "Chernogorsk", "6572.9 0 2447.53" );
		AddLocation( "Chernogorsk Factory", "6406.17 0 2710.03" );
		AddLocation( "Chyornaya Bay", "7706.96 0 3087.25" );
		AddLocation( "Crown Castle", "1430 0 9218.07" );
		AddLocation( "Deep Lake", "1853.38 0 14860.82" );
		AddLocation( "Devil's Castle", "6909.76 0 11371.53" );
		AddLocation( "Dichina Military", "4622.19 0 8283.76" );
		AddLocation( "Dobroe", "13096.32 0 14953.18" );
		AddLocation( "Dolina", "11327.42 0 6645.57" );
		AddLocation( "Drakon Island", "11185.96 0 2491.77" );
		AddLocation( "Drozhino", "3446.32 0 4929.12" );
		AddLocation( "Dubky", "6653.59 0 3674.47" );
		AddLocation( "Dubrovka", "10397.6 0 9795.56" );
		AddLocation( "Dubrovka Vyshnaya", "9952.95 0 10392.8" );
		AddLocation( "Electrozavodsk", "10502.8 0 2336.88" );
		AddLocation( "Electrozavodsk Power", "10358.14 0 2593.85" );
		AddLocation( "Electrozavodsk Shipyard", "9961.48 0 1788.85" );
		AddLocation( "Gnome Castle", "7410.93 0 9114.04" );
		AddLocation( "Gorka", "9646.16 0 8810.25" );
		AddLocation( "Green Mtn", "3744.3 0 6013.15" );
		AddLocation( "Grishino", "6018.26 0 10256.4" );
		AddLocation( "Grozovoy Pass", "3237.3 0 15293.46" );
		AddLocation( "Guba", "14609.06 0 13283.4" );
		AddLocation( "Guglovo", "8411.8 0 6687.87" );
		AddLocation( "Gvozdno", "8706.38 0 11895.07" );
		AddLocation( "Kabanino", "5298.35 0 8563.76" );
		AddLocation( "Kamenka", "1892.27 0 2155.87" );
		AddLocation( "Kamensk", "6633.17 0 14466.59" );
		AddLocation( "Kamyshovo", "12170.79 0 3447.95" );
		AddLocation( "Karmanovka", "12519.8 0 14679.5" );
		AddLocation( "Khelm", "12286.8 0 10794.4" );
		AddLocation( "Klen Mtn", "11491.72 0 11331.18" );
		AddLocation( "Komarovo", "3619.86 0 2342.53" );
		AddLocation( "Kozlova Mtn", "8826.96 0 2858.41" );
		AddLocation( "Kozlovka", "4451.57 0 4577.18" );
		AddLocation( "Krasnoe", "6486.37 0 14981.87" );
		AddLocation( "Krasnostav", "11228.8 0 12223.2" );
		AddLocation( "Krasnostav Airstrip", "12016.22 0 12490.07" );
		AddLocation( "Krutoy Cap", "13624.71 0 3892.25" );
		AddLocation( "Kumyrna", "8403.05 0 6030.65" );
		AddLocation( "Lopatino", "2749.87 0 9991.8" );
		AddLocation( "Mamino", "7981.69 0 13057.3" );
		AddLocation( "Map Edge NE", "15321.19 0 15327.49" );
		AddLocation( "Map Edge NW", "44.12 0 15330.89" );
		AddLocation( "Map Edge SW", "19.82 0 1584.2" );
		AddLocation( "Mogilevka", "7502.8 0 5237.32" );
		AddLocation( "Msta", "11258.3 0 5511.28" );
		AddLocation( "Myshkino", "2056.57 0 7425.29" );
		AddLocation( "Myshkino Military", "1266.66 0 7225.99" );
		AddLocation( "Myshkino Trainyard", "885.72 0 7696.21" );
		AddLocation( "Nadezhdino", "5821.39 0 4764.33" );
		AddLocation( "Nagornoe", "9333.93 0 14666.87" );
		AddLocation( "Nizhnoye", "12895.4 0 8040.79" );
		AddLocation( "Novaya Petrovka", "3471.91 0 12988.1" );
		AddLocation( "Novaya Trainyard", "3526.6 0 12579.7" );
		AddLocation( "Novodmitrovsk", "11595 0 14696.5" );
		AddLocation( "Novoselky", "6052.29 0 3285.59" );
		AddLocation( "Novy Sobor", "7159.26 0 7688.51" );
		AddLocation( "NW Airfield ATC", "4618.3 0 10439.17" );
		AddLocation( "NW Airfield Range", "5373.21 0 9979.8" );
		AddLocation( "NW Airfield South", "4520.11 0 9421.26" );
		AddLocation( "NW Airfield Tents", "4163.31 0 11014.24" );
		AddLocation( "Olha", "13320.9 0 12893.05" );
		AddLocation( "Oreshka Pass", "9877.39 0 6009.26" );
		AddLocation( "Orlovets", "12136.28 0 7281.89" );
		AddLocation( "Orlovets Factory", "11489.7 0 7592.98" );
		AddLocation( "Otmel Island", "11642.89 0 3095.52" );
		AddLocation( "Pavlovo", "1626.37 0 3846.56" );
		AddLocation( "Pavlovo Military", "2101.77 0 3290.93" );
		AddLocation( "Pogorevka", "4376.71 0 6413.21" );
		AddLocation( "Polana", "10678.7 0 8090.18" );
		AddLocation( "Polesovo", "5817 0 13490" );
		AddLocation( "Prigorodki", "7984.89 0 3302.17" );
		AddLocation( "Prud Lake", "6627.34 0 9288.07" );
		AddLocation( "Pulkovo", "4902.36 0 5650.65" );
		AddLocation( "Pusta", "9176.44 0 3858.71" );
		AddLocation( "Pustoshka", "2992.21 0 7805.54" );
		AddLocation( "Ratnoe", "6305.55 0 12708.4" );
		AddLocation( "Rify", "13783.18 0 11233" );
		AddLocation( "Rog Castle", "11255 0 4293.29" );
		AddLocation( "Rogovo", "4731 0 6775.61" );
		AddLocation( "Romashka", "8148.5 0 11015.2" );
		AddLocation( "Schadenfreude Island", "24000 0 0" );
		AddLocation( "Severograd", "7893.4 0 12576.3" );
		AddLocation( "Shakhovka", "9670.8 0 6495.85" );
		AddLocation( "Silence Lake", "1203.95 0 6507.42" );
		AddLocation( "Sinystok", "1447.49 0 11924.24" );
		AddLocation( "Skalisty Castle", "13493.95 0 3254.96" );
		AddLocation( "Skalisty Island", "13661.17 0 2960.38" );
		AddLocation( "Skvsch Biathlon Arena", "488.57 0 11118.7" );
		AddLocation( "Smirnovo", "11629.3 0 15023.1" );
		AddLocation( "Solnichniy", "13469.1 0 6239" );
		AddLocation( "Solnichniy Factory", "13069.59 0 7161.34" );
		AddLocation( "Sosnovka", "2512.53 0 6371.41" );
		AddLocation( "Sosnovy Pass", "2736.13 0 6757.38" );
		AddLocation( "Staroye", "10164.18 0 5460.93" );
		AddLocation( "Stary Sobor", "6072.64 0 7748.56" );
		AddLocation( "Stary Yar", "4935.61 0 15068" );
		AddLocation( "Storozh Island (Prison)", "2611.2 0 1306.15" );
		AddLocation( "Svergino", "9533.2 0 13728.4" );
		AddLocation( "Svetlojarsk", "13931.93 0 13231.39" );
		AddLocation( "Three Crosses Gas", "312.94 0 9362.44" );
		AddLocation( "Three Valleys", "12842.1 0 5500.33" );
		AddLocation( "Tisy", "3413.75 0 14810.27" );
		AddLocation( "Tisy Military", "1681.3 0 14116.6" );
		AddLocation( "Topolka Dam", "10280.5 0 3507.5" );
		AddLocation( "Topolniki", "2821.19 0 12366.5" );
		AddLocation( "Troitskoe", "7559.44 0 13515.9" );
		AddLocation( "Troitskoe Military", "7866.4 0 14676.1" );
		AddLocation( "Tulga", "12808.7 0 4457.13" );
		AddLocation( "Turovo", "13576.5 0 14128.6" );
		AddLocation( "Vavilovo", "2211.83 0 11016.4" );
		AddLocation( "Veresnik Mtn", "4439.42 0 8078.14" );
		AddLocation( "Vybor", "3800 0 8844.25" );
		AddLocation( "Vyshnoye", "6532.61 0 6090.53" );
		AddLocation( "Vysotovo", "5616.26 0 2561.77" );
		AddLocation( "Willow Lake", "13254.2 0 11607.8" );
		AddLocation( "Zabolotye", "1156.26 0 10006.2" );
		AddLocation( "Zaprudnoe", "4927.47 0 13027.2" );
		AddLocation( "Zelenogorsk", "2713.4 0 5391.75" );
		AddLocation( "Zelenogorsk Trainyard", "2490.65 0 5151.76" );
		AddLocation( "Zub Castle", "6541.79 0 5591.69" );
		AddLocation( "Zvir", "477.35 0 5202.98" );
	}
}