Dominik Gurczyński 203854 ACIR 2A
Norbert Harasim 203151 ACIR 2B

# Sprawozdanie: Symulator windy w C++ z wykorzystaniem Windows API i GDI+

---

## Zastosowane technologie

- **Język programowania:** C++  
- **Biblioteki systemowe:** Windows API, GDI+  
- **Środowisko:** Visual Studio, system operacyjny Windows  

---

## Architektura programu

### Główne komponenty:
1. **Główne okno aplikacji:** Tworzone przy użyciu `CreateWindowEx`.
2. **Kontrolki interfejsu:** 
   - Dwa `STATIC` do wyświetlania wagi i liczby pasażerów,
   - Przycisk dla każdego możliwego połączenia `początek -> cel` (bez samych siebie).
3. **Zegar (Timer):**
   - `timer_ruch` (ID=1) — kontroluje przemieszczenie windy,
   - `timer_bezczynnosc` (ID=2) — steruje powrotem windy na parter po czasie bezczynności.

### Struktury danych:
- `std::vector<int> pasazerowie_w_windzie` — aktualnie przewożeni pasażerowie,
- `std::map<int, std::vector<int>> zadania_pasazerow` — mapa pięter, na których oczekują pasażerowie (wartość: docelowe piętra),
- `std::set<int> gorne_pietra`, `dolne_pietra` — piętra z aktywnymi żądaniami w górę/w dół.

---

## Logika działania windy

### Obsługa przycisków

Każdy przycisk na interfejsie odpowiada za żądanie pasażera z piętra `od` na `doo`. Kliknięcie przycisku:
- Dodaje nowe zadanie,
- Aktywuje licznik bezczynności,
- Uruchamia windę, jeśli nie jest w ruchu.

### Ruch windy

Windą zarządza funkcja `rusz_winde()`:
1. Sprawdza, czy aktualnie na piętrze są pasażerowie do zabrania lub do wysadzenia,
2. Obsługuje wejście i wyjście pasażerów (z kontrolą maksymalnego DMC — 600 kg),
3. Decyduje o dalszym kierunku jazdy:
   - Preferuje kontynuowanie obecnego kierunku,
   - Jeśli brak żądań — winda się zatrzymuje.

### Powrót na parter

Jeśli winda jest pusta i nie ma żądań z pięter, to po czasie `czas_bezczynnosci` 5s, automatycznie zjeżdża na parter.

---

## Wizualizacja GDI+

Rysowanie realizowane w funkcji `wizualizuj_winde(HDC, RECT)`:
- Kabina windy — niebieski prostokąt,
- Szyb i podziały pięter — szare linie,
- Pasażerowie w windzie — żółte kółka z napisem celu,
- Pasażerowie oczekujący na piętrach — zielone kółka z napisem celu,
- Numery pięter i układ graficzny w pionie

---

## Najważniejsze zmienne
### `liczba_pieter` - Całkowita liczba pięter (5)
### `waga_pasazera` - Domyślna masa jednej osoby (70 kg)
### `dmc_windy` - Maksymalna liczba osób = 600 / 70
### `kierunek_ruchu` - Kierunek jazdy windy: `1` - w górę, `-1` - w dół, `0` - postój
### `w_ruchu` - Czy winda aktualnie jedzie
### `wroc_na_parter` - Czy winda powraca na parter

---

## Opis funkcji programu

### `int zaokraglij_do_y(int pietro)`
Oblicza współrzędną Y dla danego piętra w układzie graficznym (z góry na dół).

### `void aktualizuj_dane()`
Aktualizuje etykiety pokazujące wagę i liczbę pasażerów.

### `void dodaj_zadanie(int od, int doo)`
Dodaje nowe zadanie do realizacji przez windę (od → do).

### `bool czy_zadanie_gora()` / `czy_zadanie_dol()`
Sprawdzają, czy znajdują się zadania powyżej lub poniżej bieżącego piętra.

### `bool czy_zadanie_aktualne_pietro()`
Sprawdza, czy ktoś czeka lub ma wysiąść na aktualnym piętrze.

### `int okresl_kierunek()`
Na podstawie dostępnych zadań określa kierunek dalszego ruchu windy.

### `void rozpocznij_ruch()`
Jeśli winda stoi, to ustawia jej kierunek i rozpoczyna jazdę.

### `void rusz_winde()`
Realizuje przemieszczenie windy, obsługę pasażerów oraz zmianę pięter.

### `void wizualizuj_winde(HDC, RECT)`
Rysuje wszystkie elementy graficzne windy: szyb, kabinę, pasażerów itd.

### `LRESULT CALLBACK WndProc(...)`
Procedura obsługi komunikatów systemowych: malowanie, przyciski, zegary itd.

### `int WINAPI wWinMain(...)`
Główna funkcja programu – inicjalizacja, tworzenie GUI i pętla komunikatów.

