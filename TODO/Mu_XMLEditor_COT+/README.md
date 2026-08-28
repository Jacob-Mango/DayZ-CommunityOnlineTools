# Mu_XMLEditor — moduł XML Editor dla COT (Community Online Tools)

Rozszerzenie moda Community Online Tools dodające w pełni zintegrowany edytor
ekonomii (`types.xml`) bezpośrednio w panelu admina, bez konieczności
zatrzymywania serwera ani edycji plików ręcznie.

---

## Funkcje

### Edytor types.xml
- Automatyczne wykrywanie wszystkich plików `types.xml` zarejestrowanych
  w `cfgeconomycore.xml` (mody, niestandardowe mapy) + plik root
- Zakładka **„--- Wszystkie ---"** scalająca typy ze wszystkich plików w jedną listę
- Pole wyszukiwania z filtrowaniem w czasie rzeczywistym po fragmencie nazwy
- Podgląd i edycja wartości: `nominal`, `lifetime`, `restock`, `min`,
  `quantmin`, `quantmax`, `cost`
- **Automatyczny backup** przed każdym zapisem →
  `$profile:Mu_XMLEditor/Backups/<nazwa>_RRRR-MM-DD_HH-MM-SS.xml`
- Logi akcji w admin-logu COT i webhooku Discord (kategorie: Edit / Scan)

### Skanowanie lootu na mapie
- Wyszukiwanie wszystkich egzemplarzy wybranego typu przedmiotu w promieniu
  wokół pozycji admina (domyślnie 800 m, regulowane w GUI)
- Wynik: liczba znalezionych egzemplarzy + współrzędne najbliższego
- **Automatyczne otwieranie mapy COT** z pomarańczowymi znacznikami na każdej
  znalezionej pozycji — bez budowania własnej mapy, dopięte przez
  `modded class JMMapForm`
- Przycisk teleportacji bezpośrednio do najbliższego egzemplarza

---

## Wymagane uprawnienia

Nadawane przez istniejący system ról COT (moduł Zarządzanie Graczami):

| Uprawnienie | Zakres |
|---|---|
| `Admin.XMLEditor.View` | dostęp do modułu i podgląd wartości |
| `Admin.XMLEditor.Edit` | zapis zmian w pliku types.xml |
| `Admin.XMLEditor.ScanLoot` | skanowanie pozycji lootu na mapie |
| `Admin.Map.View` | wymagane przez moduł Mapa COT do wyświetlenia znaczników (uprawnienie natywne COT, nie nasze) |

---

## Instalacja

### Wymagania
- **Community Online Tools** — musi być załadowane przed `Mu_XMLEditor`
- DayZ 1.29+

### Kroki
1. Umieść `Mu_XMLEditor.pbo` w folderze `@Mu_XMLEditor\Addons\` na serwerze
   **i** po stronie klienta
2. Dodaj `@Mu_XMLEditor` do parametru startowego `-mod=`, **po**
   `@Community-Online-Tools`:
   ```
   -mod=@CF;@Community-Online-Tools;@Mu_XMLEditor
   ```
3. Zrestartuj serwer i klienta
4. W panelu COT przejdź do **Zarządzanie Graczami** i nadaj sobie uprawnienia
   `Admin.XMLEditor.*` oraz `Admin.Map.View`
5. Moduł pojawi się w sidebarze COT jako ikona **X** / „XML Editor"

---

## Struktura plików źródłowych

```
Mu_XMLEditor/
├── $PREFIX$                           # "Mu_XMLEditor"
├── config.cpp                         # CfgPatches + CfgMods
├── stringtable.csv                    # etykieta klawisza skrótu
├── Data/Inputs.xml                    # skrót RightAlt+X (zmienialny w opcjach gry)
├── GUI/layouts/xmleditor_form.layout  # kontener formularza
└── scripts/
    ├── 3_Game/Mu_XMLEditor/
    │   ├── JMXMLParser.c              # liniowy parser/edytor XML, niezależny od CF.XML
    │   └── JMXMLEditorRPC.c           # enum RPC (pula 10480–10490)
    └── 5_Mission/Mu_XMLEditor/
        ├── JMXMLEditorModule.c        # logika serwerowa, RPC, backup, webhook
        ├── JMXMLEditorForm.c          # GUI (UIActionManager, wzorzec COT)
        ├── JMModuleConstructor.c      # rejestracja modułu w COT
        ├── JMXMLEditorMapMarkers.c    # magazyn pozycji dla znaczników mapy
        └── JMMapFormPatch.c           # modded class JMMapForm — dopięcie znaczników
```

---

## Uwagi techniczne

- Parser XML (`JMXMLParser`) działa na natywnych funkcjach silnika
  (`OpenFile`/`FGets`/`FPrintln`) bez zależności od `CF.XML` — działa
  poprawnie dla standardowej struktury pliku types.xml; przy bardzo
  niestandardowym formatowaniu (wieloliniowe atrybuty) może wymagać korekty
- Skanowanie lootu przeszukuje obszar wokół **aktualnej pozycji admina**
  (jeden call `DayZPlayerUtils.SceneGetEntitiesInBox`), nie globalnie po
  całej mapie — świadomy kompromis wydajnościowy
- Znaczniki lootu na mapie są **tymczasowe** — znikają po zamknięciu okna
  mapy lub uruchomieniu nowego skanu; nie są zapisywane między sesjami

---

## Możliwe rozszerzenia

- Przycisk „Przywróć z backupu" — lista plików z
  `$profile:Mu_XMLEditor/Backups/` z możliwością przywrócenia jednym kliknięciem
- Skan globalny całej mapy (kafelkowanie `SceneGetEntitiesInBox` jak w ESP COT)
- Warunkowy plik types dla konkretnej mapy (wzorzec `IsModLoaded()` z
  `JMModuleConstructor` w COT)
