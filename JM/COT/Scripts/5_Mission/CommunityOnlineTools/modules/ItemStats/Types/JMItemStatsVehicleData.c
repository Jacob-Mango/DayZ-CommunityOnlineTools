class JMItemStatsVehicleData: JMItemStatsBaseData
{
    int fuelCapacity;
    int fuelConsumption;
}

/*
class JMItemStatsCargoData
{
    ref TIntArray m_ItemsCargoSize;

	void JMItemStatsCargoData(string path)
	{
		m_ItemsCargoSize = g_Game.ConfigGetInt(path + " itemsCargoSize");
	}

	string GetExportHeaderData(string name)
	{
		string result;

		result += name +" itemsCargoSize";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_Burst.ToString();

		return result;
	}
}


class JMItemStatsFireModeData
{
    float m_ReloadTime;
    float m_Dispersion;
    int m_Burst;

	void JMItemStatsFireModeData(string path)
	{
		m_ReloadTime = g_Game.ConfigGetFloat(path + " reloadTime");
		m_Dispersion = g_Game.ConfigGetFloat(path + " dispersion");
		m_Burst = g_Game.ConfigGetInt(path + " burst");
	}

	string GetExportHeaderData(string name)
	{
		string result;

		result += name +" ReloadTime";
        result += ", ";
		result += name +" Dispersion";
        result += ", ";
		result += name +" Burst";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_ReloadTime.ToString();
        result += ", ";
		result += m_Dispersion.ToString();
        result += ", ";
		result += m_Burst.ToString();

		return result;
	}
}

class JMItemStatsOpticsInfoData
{
    int m_DistanceZoomMin;
    int m_DistanceZoomMax;
    ref TIntArray m_DiscreteDistance;

	void JMItemStatsOpticsInfoData(string path)
	{
		m_DistanceZoomMin = g_Game.ConfigGetInt(path + " distanceZoomMin");
		m_DistanceZoomMax = g_Game.ConfigGetInt(path + " distanceZoomMax");
        
		m_DiscreteDistance = new TIntArray;
		g_Game.ConfigGetIntArray(path + " discreteDistance", m_DiscreteDistance);
	}

	string GetExportHeaderData()
	{
		string result;

		result += "OpticsInfo DistanceZoomMin";
        result += ", ";
		result += "OpticsInfo DistanceZoomMax";
        result += ", ";
		result += "OpticsInfo DiscreteDistance";

		return result;
	}

	string GetExportData()
	{
		string result;

		result += m_DistanceZoomMin.ToString();
        result += ", ";
		result += m_DistanceZoomMax.ToString();
        result += ", ";
		string mode;
		foreach(int dist: m_DiscreteDistance)
		{
			mode += dist.ToString() + " ";
		}
		result += mode;

		return result;
	}
}


		class SimulationModule: SimulationModule
		{
			class Steering
			{
				maxSteeringAngle=30;
				increaseSpeed[]={0,40,30,20,60,10,100,5}
				decreaseSpeed[]={0,90,60,70,100,50}
				centeringSpeed[]={0,0,15,35,60,65,100,90}
			}
			class Throttle
			{
				reactionTime=0.80000001;
				defaultThrust=0.75;
				gentleThrust=0.5;
				turboCoef=3.8;
				gentleCoef=0.5;
			}
			class Brake
			{
				pressureBySpeed[]={0,0.85000002,10,0.75,20,0.44999999,40,0.34999999,80,0.40000001,100,0.44999999,120,0.64999998}
				gentleCoef=0.69999999;
				minPressure=0.30000001;
				reactionTime=0.34999999;
				driverless=0.1;
			}
			class Aerodynamics
			{
				frontalArea=2;
				dragCoefficient=0.34999999;
			}
			drive="DRIVE_RWD";
			class Engine
			{
				torqueCurve[]={600,0,990,50,1800,75,3000,82,5000,71,7000,0}
				inertia=0.2;
				frictionTorque=80;
				rollingFriction=0.5;
				viscousFriction=0;
				rpmIdle=900;
				rpmMin=1000;
				rpmClutch=1500;
				rpmRedline=5750;
			}
			class Clutch
			{
				maxTorqueTransfer=165;
				uncoupleTime=0.1;
				coupleTime=0.30000001;
			}
			class Gearbox
			{
				type="GEARBOX_MANUAL";
				reverse=3.27;
				ratios[]={3.8,2.1199999,1.41,0.95999998}
			}
			class Axles: Axles
			{
				class Front: Front
				{
					maxBrakeTorque=1220;
					wheelHubMass=5;
					wheelHubRadius=0.17;
					class Suspension
					{
						stiffness=25000;
						compression=1600;
						damping=5200;
						travelMaxUp=0.07;
						travelMaxDown=0.079999998;
					}
					class Wheels: Wheels
					{
						class Left: Left
						{
							animDamper="damper_1_1";
							inventorySlot="Sedan_02_Wheel_1_1";
						}
						class Right: Right
						{
							animDamper="damper_2_1";
							inventorySlot="Sedan_02_Wheel_2_1";
						}
					}
				}
				class Rear: Rear
				{
					maxBrakeTorque=1040;
					maxHandbrakeTorque=2400;
					wheelHubMass=5;
					wheelHubRadius=0.17;
					class Differential
					{
						ratio=4.2199998;
						type="DIFFERENTIAL_OPEN";
					}
					class Suspension
					{
						stiffness=21500;
						compression=1800;
						damping=5600;
						travelMaxUp=0.1;
						travelMaxDown=0.1;
					}
					class Wheels: Wheels
					{
						class Left: Left
						{
							animDamper="damper_1_2";
							inventorySlot="Sedan_02_Wheel_1_2";
						}
						class Right: Right
						{
							animDamper="damper_2_2";
							inventorySlot="Sedan_02_Wheel_2_2";
						}
					}
				}
			}
		}
		*/