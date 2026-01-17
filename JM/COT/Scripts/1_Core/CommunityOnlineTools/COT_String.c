class COT_String: string
{
	/**
	 * @brief prepare keyword search
	 *
	 * To be called on the string to be searched for
	 *
	 * @param [out] requireAllKeywords  will be set to true if the search string does not contain `OR` operator or pipe symbol `|`, else false
	 * 
	 * @note alters search string in-place!
	 */
	TStringArray KeywordSearch_Prepare(out bool requireAllKeywords)
	{
		value.Replace(" OR ", "|");
		value.ToLower();

		TStringArray keywords = {};

		if (value.IndexOf("|") == -1)
		{
			requireAllKeywords = true;
			value.Split(" ", keywords);
		}
		else
		{
			requireAllKeywords = false;
			value.Replace(" | ", "|");
			value.Replace(" ", "|");
			value.Split("|", keywords);
		}

		return keywords;
	}

	/**
	 * @brief keyword search implementation
	 *
	 * To be called on the string to be searched in
	 *
	 * @param search  complete search string
	 * @param keywords  search string split into keywords (see KeywordSearch_Prepare)
	 * @param requireAllKeywords  whether to require matching all keywords or only one
	 */
	bool KeywordSearchImpl(string search, TStringArray keywords, bool requireAllKeywords)
	{
		if (value != search)
		{
			foreach (string keyword: keywords)
			{
				if (value.IndexOf(keyword) == -1)
				{
					if (requireAllKeywords)
						return false;
					else
						continue;
				}

				if (!requireAllKeywords)
					return true;
			}

			if (!requireAllKeywords)
				return false;
		}

		return true;
	}

	/**
	 * @brief keyword search implementation (extended)
	 *
	 * To be called on the string to be searched in
	 * 
	 * @param search  complete search string
	 * @param keywords  search string split into keywords (see KeywordSearch_Prepare)
	 * @param requireAllKeywords  whether to require matching all keywords or only one
	 * @param [inout] closestMatch  closest matching search result string for autocompletion
	 */
	bool KeywordSearchImplEx(string search, TStringArray keywords, bool requireAllKeywords, inout string closestMatch)
	{
		if (value != search)
		{
			foreach (string keyword: keywords)
			{
				int index = value.IndexOf(keyword);

				if (index == -1)
				{
					if (requireAllKeywords)
						return false;
					else
						continue;
				}

				if (index == 0 && search == keyword)
				{
					if (!closestMatch || value.Length() < closestMatch.Length())
						closestMatch = value;
				}

				if (!requireAllKeywords)
					return true;
			}

			if (!requireAllKeywords)
				return false;
		}
		else
		{
			closestMatch = value;
		}

		return true;
	}
}

typedef string COT_String;
