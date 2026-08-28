// Formatage d'une duree (secondes) en texte court ("3j 5h", "5h 0min", "12min").
class KF_Time
{
	static string HumanDuration(int secs)
	{
		if (secs <= 0)
			return "expire";

		int days = secs / 86400;
		int rem  = secs - days * 86400;
		int hours = rem / 3600;
		rem = rem - hours * 3600;
		int mins = rem / 60;

		if (days > 0)
			return days.ToString() + "j " + hours.ToString() + "h";
		if (hours > 0)
			return hours.ToString() + "h " + mins.ToString() + "min";
		return mins.ToString() + "min";
	}
}
