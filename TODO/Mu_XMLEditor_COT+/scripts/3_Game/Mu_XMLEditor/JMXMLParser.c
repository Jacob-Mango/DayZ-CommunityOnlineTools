/*
	JMXMLParser
	-----------
	Prosty, liniowy parser/edytor plikow types.xml (oraz podobnych plikow
	ekonomii DayZ), oparty na natywnych funkcjach silnika (OpenFile/FGets/
	FPrintln), bez zaleznosci od zewnetrznych bibliotek (CF.XML itd.).

	Logika 1:1 odtworzona z modulu XMLEditor moda VPPAdminTools (autorstwa
	zespolu Vanilla++ DayZ SA Project), przepisana pod COT i przemianowana
	(JMXMLParser / JMXMLLineElement), aby nie kolidowala z klasami innych
	modow (np. oryginalnym XMLParser z VPPAdminTools), gdyby ktos mial
	zainstalowane oba mody jednoczesnie.

	Dzialanie:
		1. Load()                        - wczytuje plik linia po linii
		2. GetElementString(tag, name)   - zwraca linie nalezace do <type name="..."> ... </type>
		3. GetElementStringValue(elem,…) - znajduje linie z konkretnym tagiem (<lifetime>123</lifetime>)
		4. ReplaceElementValue(...)      - podmienia wartosc w buforze linii
		5. SaveChanges()                 - zapisuje caly bufor z powrotem na dysk
*/

// Enforce Script's string.IndexOf() ma TYLKO jeden parametr (szukany tekst) -
// nie ma przeciazenia z indeksem startowym jak w C#/Java. Ten helper symuluje
// IndexOf(needle, fromIndex) przez Substring + IndexOf + doliczenie offsetu.
// Globalna funkcja (nie metoda klasy), zeby byla dostepna takze z 4_World.
int Mu_XML_IndexOfFrom(string haystack, string needle, int fromIndex)
{
	if (fromIndex < 0 || fromIndex >= haystack.Length())
		return -1;

	string sub = haystack.Substring(fromIndex, haystack.Length() - fromIndex);
	int idx = sub.IndexOf(needle);
	if (idx == -1)
		return -1;

	return idx + fromIndex;
}

class JMXMLLineElement
{
	string data;
	int    index;

	void JMXMLLineElement(string d, int i)
	{
		data  = d;
		index = i;
	}
};

class JMXMLParser
{
	private ref array<ref JMXMLLineElement> m_RawLines;
	private string                          FILE_PATH;
	private FileHandle                      m_FileHandle;

	void JMXMLParser(string filePath)
	{
		FILE_PATH = filePath;
	}

	bool Load()
	{
		m_FileHandle = OpenFile(FILE_PATH, FileMode.READ);
		if (m_FileHandle == 0)
			return false;

		m_RawLines = new array<ref JMXMLLineElement>;
		string line_content = "";
		int char_count = FGets(m_FileHandle, line_content);
		int line_index = 1;

		while (char_count != -1)
		{
			m_RawLines.Insert(new JMXMLLineElement(line_content, line_index));
			char_count = FGets(m_FileHandle, line_content);
			line_index++;
		}

		CloseFile(m_FileHandle);
		return true;
	}

	// Bardzo lekka walidacja - sprawdza czy plik faktycznie zawiera tagi <types> i przynajmniej jeden <type
	bool VerifyXml()
	{
		if (!m_RawLines || m_RawLines.Count() == 0)
			return false;

		bool foundRoot = false;
		bool foundType = false;

		foreach (JMXMLLineElement line : m_RawLines)
		{
			if (!foundRoot && line.data.IndexOf("<types>") != -1)
				foundRoot = true;

			if (!foundType && line.data.IndexOf("<type ") != -1)
				foundType = true;

			if (foundRoot && foundType)
				return true;
		}

		return false;
	}

	// Zwraca wszystkie linie znajdujace sie pomiedzy <type name="elementName"> a odpowiadajacym </type>
	array<ref JMXMLLineElement> GetElementString(string tag, string elementName)
	{
		array<ref JMXMLLineElement> result = new array<ref JMXMLLineElement>;

		string openTag  = "<" + tag + " name=\"" + elementName + "\"";
		string closeTag = "</" + tag + ">";

		bool capturing = false;
		foreach (JMXMLLineElement line : m_RawLines)
		{
			if (!capturing && line.data.IndexOf(openTag) != -1)
			{
				capturing = true;
			}

			if (capturing)
			{
				result.Insert(line);

				if (line.data.IndexOf(closeTag) != -1)
					break;
			}
		}

		return result;
	}

	// W obrebie podzbioru linii (np. zwroconego przez GetElementString) znajduje
	// linie z konkretnym elementem, np. <lifetime>123456</lifetime>
	JMXMLLineElement GetElementStringValue(string elementType, array<ref JMXMLLineElement> scope)
	{
		if (!scope)
			return null;

		string openTag = "<" + elementType;

		foreach (JMXMLLineElement line : scope)
		{
			int tagPos = line.data.IndexOf(openTag);
			if (tagPos == -1)
				continue;

			// upewnij sie, ze to nie jest np. <lifetimeXYZ ...> tylko dokladnie nasz tag
			int afterTag = tagPos + openTag.Length();
			if (afterTag >= line.data.Length())
				continue;

			string nextChar = line.data.Substring(afterTag, 1);
			if (nextChar != ">" && nextChar != " ")
				continue;

			int valueStart = Mu_XML_IndexOfFrom( line.data, ">", tagPos ) + 1;
			int valueEnd   = Mu_XML_IndexOfFrom( line.data, "</" + elementType + ">", valueStart );
			if (valueStart == -1 || valueEnd == -1 || valueEnd < valueStart)
				continue;

			string value = line.data.Substring(valueStart, valueEnd - valueStart);
			return new JMXMLLineElement(value, line.index);
		}

		return null;
	}

	// Podmienia wartosc liczbowa/tekstowa wewnatrz tagu na danej linii (1-indexed line.index)
	bool ReplaceElementValue(string elementType, int lineIndex, string newValue)
	{
		if (!m_RawLines || lineIndex <= 0 || lineIndex > m_RawLines.Count())
			return false;

		JMXMLLineElement target = m_RawLines.Get( lineIndex - 1 );

		string openTag  = "<" + elementType;
		string closeTag = "</" + elementType + ">";

		int tagPos     = target.data.IndexOf(openTag);
		int valueStart = Mu_XML_IndexOfFrom( target.data, ">", tagPos ) + 1;
		int valueEnd   = Mu_XML_IndexOfFrom( target.data, closeTag, valueStart );

		if (tagPos == -1 || valueStart == -1 || valueEnd == -1)
			return false;

		string before = target.data.Substring(0, valueStart);
		string after  = target.data.Substring(valueEnd, target.data.Length() - valueEnd);

		target.data = before + newValue + after;
		return true;
	}

	// Nadpisuje oryginalny plik aktualna zawartoscia bufora linii
	bool SaveChanges(string path = "")
	{
		if (path == "")
			path = FILE_PATH;

		if (!m_RawLines)
			return false;

		FileHandle handle = OpenFile(path, FileMode.WRITE);
		if (handle == 0)
			return false;

		foreach (JMXMLLineElement line : m_RawLines)
		{
			FPrintln(handle, line.data);
		}

		CloseFile(handle);
		return true;
	}

	// Zwraca liste nazw "type name=" zdefiniowanych w pliku (do listy rozwijanej w GUI)
	array<string> GetAllTypeNames()
	{
		array<string> names = new array<string>;
		if (!m_RawLines)
			return names;

		string marker = "<type name=\"";
		foreach (JMXMLLineElement line : m_RawLines)
		{
			int start = line.data.IndexOf(marker);
			if (start == -1)
				continue;

			start += marker.Length();
			int end = Mu_XML_IndexOfFrom( line.data, "\"", start );
			if (end == -1)
				continue;

			names.Insert(line.data.Substring(start, end - start));
		}

		return names;
	}
};
