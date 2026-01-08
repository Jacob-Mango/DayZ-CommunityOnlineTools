modded class Math
{
	/**
	@brief Returns smallest possible angle difference
		@param a \p float Angle in interval [0,360]
		@param b \p float Target angle in interval [0,360]
		@return \p float - Angle difference in interval [-180,180]
		@code
			Print( ExpansionMath.AngleDiff2(270.0, 0.0) );
			Print( ExpansionMath.AngleDiff2(350.0, 1.0) );
			Print( ExpansionMath.AngleDiff2(60.0, 275.0) );

			>> 90.0
			>> 11.0
			>> -145.0
		@endcode
	*/
	static float COT_AngleDiff2(float a, float b)
	{
		float d = b - a;
		if (d > 180)
			return d - 360;
		else if (d < -180)
			return d + 360;
		return d;
	}

	/**
	 * @brief Converts direction vector to orientation vector
	 *
	 * @note dir.VectorToAngles() produces a different result that can not be used as orientation vector
	 */
	static vector COT_DirToOri(vector dir)
	{
		vector ori;
		ori[0] = Math.Atan2(dir[0], dir[2]) * Math.RAD2DEG;
		ori[1] = Math.Atan2(dir[1], Math.Sqrt(dir[0] * dir[0] + dir[2] * dir[2])) * Math.RAD2DEG;
		return ori;
	}

	static float COT_LookUp(float value, int num, float inVals[], float outVals[])
	{
		value = Math.Clamp(value, inVals[0], inVals[num - 1]);

		for (int i = 0; i < num - 1; i++)
		{
			if (inVals[i] <= value && value <= inVals[i + 1])
			{
				float delta = (value - inVals[i]) / (inVals[i + 1] - inVals[i]);
				return Math.Lerp(outVals[i], outVals[i + 1], delta);
			}
		}

		//! Should never reach this point, just here to make code compile as we need a return value
		return 0;
	}
}
