#ifndef CF_XML_READER_SINGLEQUOTE_FIX
modded class CF_XML_Reader
{
	override string GetQuotedWord(array<string> allowed, out bool isQuoted)
	{
		string word = "";

		string c = SkipWhitespace();

		string quoteChar;

		if (c == "\"" || c == "'")
		{
			quoteChar = c;
			c = "";
			isQuoted = true;
		}

		while ((IsLetterOrDigit(c, isQuoted) || c == "" || IsCharacterAllowed(c, allowed)) && !EOF())
		{
			word = word + c;
			c = GetCharacter();
			if (isQuoted && c == quoteChar)
			{
				break;
			}
		}

		if (!isQuoted)
			BackChar();

		return word;
	}
}
#endif
