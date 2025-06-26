#define UNICODE
#define _UNICODE
#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

HWND hWnd;
ULONG_PTR gdiplusToken;
HWND napis_waga_pasazerow = nullptr;
HWND napis_liczba_pasazerow = nullptr;

const int bazowe_id_przycisku_pietra = 1000;
const int timer_ruch = 1;
const int timer_bezczynnosc = 2;
const int timer_okres = 1500;
const int czas_bezczynnosci = 5000;
const int liczba_pieter = 5;
const int waga_pasazera = 70;
const int wysokosc_pietra = 100;

int aktualne_pietro = 1;
int liczba_pasazerow = 0;

std::map<int, std::vector<int>> zadania_pasazerow;
std::vector<int> pasazerowie_w_windzie;
std::set<int> gorne_pietra;
std::set<int> dolne_pietra;

bool w_ruchu = false;
bool wroc_na_parter = false;
int kierunek_ruchu = 0;

int zaokraglij_do_y(int pietro) {
    return (liczba_pieter - pietro) * wysokosc_pietra + 10;
}

void aktualizuj_dane() {
    std::wstring w = L"Waga: " + std::to_wstring(liczba_pasazerow * waga_pasazera) + L" kg";
    SetWindowTextW(napis_waga_pasazerow, w.c_str());
    std::wstring o = L"Osoby: " + std::to_wstring(liczba_pasazerow);
    SetWindowTextW(napis_liczba_pasazerow, o.c_str());
}

void dodaj_zadanie(int od, int doo) {
    if (od == doo) return;
    auto& zadanie = zadania_pasazerow[od];
    zadanie.push_back(doo);
    if (doo > od) gorne_pietra.insert(od);
    else dolne_pietra.insert(od);
}

bool czy_zadanie_gora() {
    for (int pietro = aktualne_pietro + 1; pietro <= liczba_pieter; ++pietro) {
        if (gorne_pietra.count(pietro) || dolne_pietra.count(pietro)) return true;
        if (std::find(pasazerowie_w_windzie.begin(), pasazerowie_w_windzie.end(), pietro) != pasazerowie_w_windzie.end())
            return true;
    }
    return false;
}

bool czy_zadanie_dol() {
    for (int pietro = aktualne_pietro - 1; pietro >= 1; --pietro) {
        if (gorne_pietra.count(pietro) || dolne_pietra.count(pietro)) return true;
        if (std::find(pasazerowie_w_windzie.begin(), pasazerowie_w_windzie.end(), pietro) != pasazerowie_w_windzie.end())
            return true;
    }
    return false;
}

bool czy_zadanie_aktualne_pietro() {
    if (!zadania_pasazerow[aktualne_pietro].empty()) return true;
    if (std::find(pasazerowie_w_windzie.begin(), pasazerowie_w_windzie.end(), aktualne_pietro) != pasazerowie_w_windzie.end())
        return true;
    return false;
}

int okresl_kierunek() {
    if (kierunek_ruchu == 1) {
        if (czy_zadanie_gora()) return 1;
        if (czy_zadanie_dol()) return -1;
    }
    else if (kierunek_ruchu == -1) {
        if (czy_zadanie_dol()) return -1;
        if (czy_zadanie_gora()) return 1;
    }
    else {
        if (czy_zadanie_gora()) return 1;
        if (czy_zadanie_dol()) return -1;
    }
    return 0;
}

void rozpocznij_ruch() {
    if (w_ruchu) return;
    kierunek_ruchu = okresl_kierunek();
    if (kierunek_ruchu != 0) {
        w_ruchu = true;
        SetTimer(hWnd, timer_ruch, timer_okres, nullptr);
        KillTimer(hWnd, timer_bezczynnosc);
        SetTimer(hWnd, timer_bezczynnosc, czas_bezczynnosci, NULL);
    }
}

void rusz_winde() {
    if (!w_ruchu) return;
    if (wroc_na_parter) {
        if (aktualne_pietro > 1) {
            --aktualne_pietro;
        }
        else if (aktualne_pietro < 1) {
            ++aktualne_pietro;
        }
        if (aktualne_pietro == 1) {
            wroc_na_parter = false;
            w_ruchu = false;
            kierunek_ruchu = 0;
            KillTimer(hWnd, timer_ruch);
            SetTimer(hWnd, timer_bezczynnosc, czas_bezczynnosci, NULL);
        }
        aktualizuj_dane();
        InvalidateRect(hWnd, nullptr, TRUE);
        return;
    }
    auto aktualny_pasazer = pasazerowie_w_windzie.begin();
    bool winda_pusta = false;
    while (aktualny_pasazer != pasazerowie_w_windzie.end()) {
        if (*aktualny_pasazer == aktualne_pietro) {
            aktualny_pasazer = pasazerowie_w_windzie.erase(aktualny_pasazer);
            liczba_pasazerow--;
            winda_pusta = true;
        }
        else {
            ++aktualny_pasazer;
        }
    }

    auto& oczekujacy = zadania_pasazerow[aktualne_pietro];
    auto aktualny_oczekujacy = oczekujacy.begin();
    bool zabrany = false;
    int dmc_windy = 600 / waga_pasazera;
    while (aktualny_oczekujacy != oczekujacy.end() && liczba_pasazerow < dmc_windy) {
        pasazerowie_w_windzie.push_back(*aktualny_oczekujacy);
        liczba_pasazerow++;
        if (*aktualny_oczekujacy > aktualne_pietro) gorne_pietra.insert(aktualne_pietro);
        else dolne_pietra.insert(aktualne_pietro);
        aktualny_oczekujacy = oczekujacy.erase(aktualny_oczekujacy);
        zabrany = true;
    }
    if (zadania_pasazerow[aktualne_pietro].empty()) {
        gorne_pietra.erase(aktualne_pietro);
        dolne_pietra.erase(aktualne_pietro);
    }

    aktualizuj_dane();
    InvalidateRect(hWnd, nullptr, TRUE);

    if (winda_pusta || zabrany) {
        if (!pasazerowie_w_windzie.empty()) {
            int cel = pasazerowie_w_windzie.front();
            kierunek_ruchu = (cel > aktualne_pietro) ? 1 : -1;
        }
        else {
            int nastepny_kierunek = okresl_kierunek();
            if (nastepny_kierunek == 0) {
                kierunek_ruchu = 0;
                w_ruchu = false;
                KillTimer(hWnd, timer_ruch);
                SetTimer(hWnd, timer_bezczynnosc, czas_bezczynnosci, NULL);
                return;
            }
            kierunek_ruchu = nastepny_kierunek;
        }
    }

    if (!pasazerowie_w_windzie.empty()) {
        int natepny_cel = pasazerowie_w_windzie.front();
        if (aktualne_pietro < natepny_cel) ++aktualne_pietro;
        else if (aktualne_pietro > natepny_cel) --aktualne_pietro;
    }
    else {
        int nastepny_kierunek = okresl_kierunek();
        if (nastepny_kierunek == 0) {
            kierunek_ruchu = 0;
            w_ruchu = false;
            KillTimer(hWnd, timer_ruch);
            SetTimer(hWnd, timer_bezczynnosc, czas_bezczynnosci, NULL);
            return;
        }
        kierunek_ruchu = nastepny_kierunek;
        aktualne_pietro += kierunek_ruchu;
    }
}

void wizualizuj_winde(HDC hdc, RECT prt) {
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);

    Rect prostokat(prt.left, prt.top, prt.right - prt.left, prt.bottom - prt.top);
    SolidBrush bialy(Color(255, 255, 255));
    g.FillRectangle(&bialy, prostokat);

    Pen kolor_szybu(Color(255, 100, 100, 100), 2);
    g.DrawRectangle(&kolor_szybu, 150, 10, 100, wysokosc_pietra * liczba_pieter);

    for (int pietro = 1; pietro <= liczba_pieter; ++pietro) {
        int y = zaokraglij_do_y(pietro) + wysokosc_pietra - 1;
        if (pietro % 2 == 1)
            g.DrawLine(&kolor_szybu, 25, y, 250, y);
        else
            g.DrawLine(&kolor_szybu, 150, y, 375, y);
    }

    FontFamily cz(L"Segoe UI");
    Font czcionka(&cz, 10.0f);
    SolidBrush kolor_czcionki(Color(255, 0, 0, 0));
    for (int pietro = 1; pietro <= liczba_pieter; ++pietro) {
        std::wstring napis = std::to_wstring(pietro);
        int y = zaokraglij_do_y(pietro) + wysokosc_pietra - 20;
        int x = (pietro % 2 == 1) ? 120 : 360;
        g.DrawString(napis.c_str(), -1, &czcionka, PointF((REAL)x, (REAL)y), &kolor_czcionki);
    }

    SolidBrush kolor_kabiny(Color(255, 90, 120, 255));
    Rect kabina(152, zaokraglij_do_y(aktualne_pietro), 96, wysokosc_pietra - 4);
    g.FillRectangle(&kolor_kabiny, kabina);

    SolidBrush kolor_pasazera_winda(Color(255, 255, 200, 0));
    for (size_t i = 0; i < pasazerowie_w_windzie.size(); ++i) {
        int px = kabina.X + 10 + static_cast<int>(i * 24);
        int py = kabina.Y + (kabina.Height - 20) / 2;
        g.FillEllipse(&kolor_pasazera_winda, px, py, 20, 20);

        std::wstring tekst = std::to_wstring(pasazerowie_w_windzie[i]);
        g.DrawString(tekst.c_str(), -1, &czcionka, PointF((REAL)(px + 2), (REAL)(py + 2)), &kolor_czcionki);
    }

    SolidBrush kolor_pasazera(Color(255, 0, 128, 0));
    for (const auto& para : zadania_pasazerow) {
        int pietro = para.first;
        const auto& cele = para.second;
        int baza_y = zaokraglij_do_y(pietro) + wysokosc_pietra / 2;
        int baza_x = (pietro % 2 == 1) ? 140 : 270;

        for (size_t i = 0; i < cele.size(); ++i) {
            int x = baza_x + static_cast<int>((pietro % 2 == 1 ? -1 : 1) * static_cast<int>(i) * 24);
            if (pietro % 2 == 1) x -= 20;
            int y = baza_y;

            g.FillEllipse(&kolor_pasazera, x, y, 20, 20);
            std::wstring tekst = std::to_wstring(cele[i]);
            g.DrawString(tekst.c_str(), -1, &czcionka, PointF((REAL)(x + 2), (REAL)(y + 2)), &kolor_czcionki);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uint, WPARAM wParam, LPARAM lParam) {
    switch (uint) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        wizualizuj_winde(hdc, ps.rcPaint);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_TIMER:
        if (wParam == timer_ruch) {
            rusz_winde();
        }
        else if (wParam == timer_bezczynnosc) {
            if (!w_ruchu && pasazerowie_w_windzie.empty() && gorne_pietra.empty() && dolne_pietra.empty()) {
                if (aktualne_pietro != 1) {
                    kierunek_ruchu = (1 > aktualne_pietro) ? 1 : -1;
                    w_ruchu = true;
                    wroc_na_parter = true;
                    KillTimer(hwnd, timer_bezczynnosc);
                    SetTimer(hwnd, timer_ruch, timer_okres, nullptr);
                }
            }
        }
        return 0;

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id >= bazowe_id_przycisku_pietra && id < bazowe_id_przycisku_pietra + 1000) {
            int id_przycisku = id - bazowe_id_przycisku_pietra;
            int od = id_przycisku / 10;
            int doo = id_przycisku % 10;
            od++;
            doo++;
            if (od == doo) return 0;
            if (od < 1 || od > liczba_pieter || doo < 1 || doo > liczba_pieter) return 0;
            dodaj_zadanie(od, doo);
            KillTimer(hwnd, timer_bezczynnosc);
            SetTimer(hwnd, timer_bezczynnosc, czas_bezczynnosci, NULL);
            rozpocznij_ruch();
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uint, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    const wchar_t NAZWA_KLASY[] = L"Winda";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = NAZWA_KLASY;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);

    hWnd = CreateWindowEx(0, NAZWA_KLASY, L"Symulator windy", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 700, nullptr, nullptr, hInst, nullptr);

    ShowWindow(hWnd, nCmdShow);

    napis_waga_pasazerow = CreateWindowW(L"STATIC", L"Waga: 0 kg", WS_CHILD | WS_VISIBLE, 340, 10, 140, 20, hWnd, nullptr, hInst, nullptr);
    napis_liczba_pasazerow = CreateWindowW(L"STATIC", L"Osoby: 0", WS_CHILD | WS_VISIBLE, 340, 35, 140, 20, hWnd, nullptr, hInst, nullptr);

    aktualizuj_dane();

    for (int pietro = 1; pietro <= liczba_pieter; ++pietro) {
        int y = zaokraglij_do_y(pietro) + wysokosc_pietra / 2 - 12;
        int x = (pietro % 2 == 1) ? 10 : 270;
        for (int cel = 1; cel <= liczba_pieter; ++cel) {
            if (cel == pietro) continue;
            int id_przycisku = bazowe_id_przycisku_pietra + ((pietro - 1) * 10 + (cel - 1));
            CreateWindowW(L"BUTTON", std::to_wstring(cel).c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, 24, 24, hWnd, (HMENU)(id_przycisku), hInst, nullptr);
            x += 28;
        }
    }

    SetTimer(hWnd, timer_bezczynnosc, czas_bezczynnosci, NULL);

    MSG wiadomosc;
    while (GetMessage(&wiadomosc, nullptr, 0, 0)) {
        TranslateMessage(&wiadomosc);
        DispatchMessage(&wiadomosc);
    }
    GdiplusShutdown(gdiplusToken);
    return 0;
}
