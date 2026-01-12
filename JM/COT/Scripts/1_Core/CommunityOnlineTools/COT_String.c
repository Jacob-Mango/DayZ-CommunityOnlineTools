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
			//! @note `string search = "test|"; search.Split("|", keywords);` will create a keywords array with *one* entry {"test"}
			//! but we only want to set closestMatch if search string matches the keyword
			if (search == keywords[0])
			{
				int index = value.IndexOf(search);

				if (index == -1)
					return false;

				if (index == 0)
				{
					if (!closestMatch || value.Length() < closestMatch.Length())
						closestMatch = value;
				}
			}
			else
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

				return false;
			}
		}
		else
		{
			closestMatch = value;
		}

		return true;
	}
}

typedef string COT_String;
