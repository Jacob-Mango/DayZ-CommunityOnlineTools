class JMVehicleMetaData
{
	int m_ID;
	string m_ClassName;
	vector m_Position;
	vector m_Orientation;

	int m_VehicleType;
	int m_DestructionType;

	bool m_HasKeys;
	
	void JMVehicleMetaData( CarScript car )
	{
		m_ID = car.GetID();
		m_ClassName = car.ClassName();
		m_Position = car.GetPosition();
		m_Orientation = car.GetOrientation();

		m_VehicleType = EXVT_NONE;
		if ( car.IsCar() )
			m_VehicleType |= EXVT_CAR;
		if ( car.IsBoat() )
			m_VehicleType |= EXVT_BOAT;
		if ( car.IsHelicopter() )
			m_VehicleType |= EXVT_HELICOPTER;
		if ( car.IsPlane() )
			m_VehicleType |= EXVT_PLANE;
		
		m_DestructionType = EXDT_NONE;
		if ( car.IsExploded() )
			m_DestructionType |= EXDT_EXPLODED;
		if ( car.IsExploded() )
			m_DestructionType |= EXDT_DESTROYED;

		m_HasKeys = car.HasKey();
	}

	string GetVehicleType()
	{
		string type = "";

		if ( m_VehicleType & EXVT_CAR )
			type += "Car ";
		if ( m_VehicleType & EXVT_BOAT )
			type += "Boat ";
		if ( m_VehicleType & EXVT_HELICOPTER )
			type += "Helicopter ";
		if ( m_VehicleType & EXVT_PLANE )
			type += "Plane ";
		if ( m_VehicleType & EXVT_BIKE )
			type += "Bike ";

		return type;
	}

	string GetVehicleDestructionState()
	{
		string type = "";

		if ( m_DestructionType != EXDT_NONE )
		{
			if ( m_DestructionType & EXDT_DESTROYED )
			{
				type += "Destroyed ";
			}
			if ( m_DestructionType & EXDT_EXPLODED )
			{
				type += "Exploded ";
			}
		} else
		{
			type = "None";
		}

		return type;
	}

	bool IsDestroyed()
	{
		return m_DestructionType & EXDT_DESTROYED;
	}

	bool IsExploded()
	{
		return m_DestructionType & EXDT_EXPLODED;
	}
}