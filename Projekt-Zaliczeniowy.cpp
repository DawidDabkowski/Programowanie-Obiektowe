#include <iostream>
#include <queue>       // priority_queue
#include <cassert>     // Dodatkowe sprawdzania (assert)
#include <cstdlib>     // Generacja liczb losowych (srand, rand)
#include <ctime>       // time, użyty tylko w srand
#include <cmath>       // floor
#include <vector>      // vector
#include <fstream>
// Zaślepka
#include <sstream>     // ostringstream - zaślepka dla to_string
#include <cstring>

using namespace std;

// deklaracje klas i struktur wystepujacych w programie 

struct ElementBiletu;
struct ElementLinii;

class Czas;
class Zdarzenie;
class Compare;
class KolejkaZdarzen;
class Bilet;
class BiletTeatr;
class Statystyki;
class Obiekt;
    class Przystanek;
    class Linia;
        class LiniaTramwajowa;
    class Pojazd;
        class Tramwaj;
    class Miejsce;
        class Uniwersytet;
        class Teatr;
        class CentrumHandlowe;
    class Osoba;
        class Pracujacy;
        class Artysta;
        class Student;
        class Uczen;
        
//--- Zaślepka: potrzebna tylko dla MinGW pod Windows zwn na błąd w tamtej implementacji ------------------

template <typename T> string to_string(const T& n) {
    ostringstream stm;
    stm << n;
    return stm.str();
};

//--- Koniec zaślepki ----------------------------------

class Czas {
private:
    int miesiac, dzien, godzina, minuta, dzienTygodnia;
public:
    // API
    int DajMiesiac() const {return miesiac;};
    int DajDzien() const {return dzien;};
    int DajGodzina() const {return godzina;};
    int DajMinuta() const {return minuta;};
    int DajDzienTygodnia() const {return dzienTygodnia;};
    Czas Dzisiaj();
    Czas Srodek();
    Czas Jutro();
    void operator+=(const Czas&);
    Czas operator+(const Czas&);
    void operator+=(const int&);
    Czas operator+(const int&);
    void operator-=(const Czas&);
    Czas operator-(const Czas&);
    void operator-=(const int&);
    Czas operator-(const int&);
    Czas& operator=(const Czas&);
    bool operator>(Czas);
    bool operator<(Czas);
    bool operator==(Czas);
    bool operator!=(Czas);
    bool operator>=(Czas);
    bool operator<=(Czas);
    Czas& operator++();
    Czas operator++(int);
    Czas& operator--();
    Czas operator--(int);
    // operator << zdefiniowany jako funkcja
    // Techniczna
    string ToString() const;
    void Skroc();
    Czas(int mies, int dz, int godz, int min);
    Czas(const Czas&) = default;
    ~Czas() = default;
};

class Zdarzenie {
private:
    static int idNo;    // Zmienna klasowa, numer kolejnego zdarzenia
    Czas czas;          // Kiedy ma nastąpić zdarzenie?
    Obiekt& kto;        // Kto zgłosił to zdarzenie?
    string czynnosc;    // Jaka czynnosc zainicjalizowala zdarzenie?
    int id;             // Identyfikator zdarzenia
public:
    // API
    virtual void Zajdz() const; // Niech to zdarzenie zajdzie
    virtual Czas DajCzas() const {return czas;}; // Daje czas zdarzenia
    virtual int DajId() const {return id;}; // Daje identyfikator zdarzenia
    virtual Obiekt& DajKto() const {return kto;}; // Daje ref do osoby, ktora wywolala zdarzenie
    virtual string DajCzynnosc() const {return czynnosc;}; // Daje nazwe czynnosci, ktora osoba ma wykonac
    virtual string to_stringImpl() const; // Postać tekstowa obiektu, przydatna szczególnie przy debugowaniu. Zwykle używa się nazwy to_string().
    //techniczne
    Zdarzenie(Czas, Obiekt&, string); // Konstruktor główny
    Zdarzenie() = delete; // Zdarzenia wymagają danych, konstr. bezarg. to nie dla nich!
    Zdarzenie(const Zdarzenie&) = delete; // Nie chcemy kopiować zdarzeń
    Zdarzenie& operator=(const Zdarzenie&) = delete; // P. wyżej/
    virtual ~Zdarzenie() = default; // Ta deklaracja jest konieczna zwn virtual!
};
int Zdarzenie::idNo = 0; // Zmienna klasowa z Zdarzenie

class Compare { // Klasa obiektów porównujących zdarzenia, potrzebna zwn priority_queue
public:

    bool operator()(const Zdarzenie* z1, const Zdarzenie* z2) {
        return ((z1->DajCzas() > z2->DajCzas()) || 
                ((z1->DajCzas() == z2->DajCzas()) && (z1->DajId() > z2->DajId())) );
    }
};

class KolejkaZdarzen {
    priority_queue<const Zdarzenie*, std::vector<const Zdarzenie*>, Compare> zdarzenia; // Mam kolejkę zdarzeń i nie zawaham się jej użyć
    Czas czas; // Aktualny czas symulacji
public:
    // API
    virtual void Wstaw(const Zdarzenie*); // Wstawia zdarzenie do wykonania w przyszłości
    virtual Czas& KtoraGodzina() {return czas;}; // Która jest właściwie teraz godzina?
    virtual int DzienTygodnia() {return czas.DajDzienTygodnia();};
    virtual void NiechSieDzieje(Czas koniecSwiata); // Wykonuj zdarzenia do wskazanego czasu
    // techniczne
    KolejkaZdarzen(); // Zaczynamy o godzinie 0 z pustą kolejką zdarzeń
    KolejkaZdarzen(const KolejkaZdarzen&) = delete; // Nie kopiuj mnie, proszę. A nawet nalegam.
    KolejkaZdarzen& operator=(const KolejkaZdarzen&) = delete; // Patrz konstr. kop.
    virtual ~KolejkaZdarzen() = default; // Nie mam wskaźników, więc default mi pasuje!
};

// --- Definicja globalnej kolejki ---

KolejkaZdarzen kol;

// --- Koniec definicji ---

struct ElementBiletu {
    int liniaId;
    int przystanekId;
    bool czyNaprzod;
    ElementBiletu* nast;
    
    ElementBiletu(int linId, int przystId, bool czyNap, ElementBiletu* nas) :
        liniaId(linId), przystanekId(przystId), czyNaprzod(czyNap), nast(nas) {};
};

class Bilet {
protected:
    ElementBiletu* trasa;
    ElementBiletu* aktualny;
public:
    // API
    void DodajPrzesiadke(int, int, bool);
//    void DodajOdwrotny();
    ElementBiletu* DajKoniec();
    ElementBiletu* DajAktualny() {return aktualny;};
    void Przesun() {aktualny = aktualny->nast;};
    void Uprosc();
    // techniczne
    void Wypisz();
    Bilet();
    Bilet(int, int, bool);
    //Bilet(int, int, bool, int, int, bool);
    Bilet(const Bilet&) = default;
    ~Bilet();
};

class BiletTeatr {
protected:
    int teatrId;
    Czas czasSpektaklu;
public:
    // API
    int DajTeatrId() {return teatrId;};
    Czas DajCzasSpektaklu() {return czasSpektaklu;};
    // techniczne
    BiletTeatr() = delete;
    BiletTeatr(int tId, Czas czasSpek) : teatrId(tId), czasSpektaklu(czasSpek) {};
    BiletTeatr(const BiletTeatr&) = delete;
    ~BiletTeatr() = default;
};

class Statystyki {
private:
    int liczbaPrzejazdow[7];    // +1 gdy osoba wchodzi do tramwaju
    int czasCzekania[7];        // +X gdy os. wchodzi; -Y gdy schodzi/taxi
    int liczbaCzekania[7];      // +1 gdy osoba wchodzi na przystanek
    int osoboSpektakle[7];      // +1 gdy osoba wychodzi do teatru
    int miejscoSpektakle[7];    // +X gdy teatr odgrywa spektakl
    int liczbaZakupow[7];       // +1 gdy osoba wychodzi na zakupy
public:
    // API
    void zmienLiczbaPrzejazdow(int l) {liczbaPrzejazdow[kol.KtoraGodzina().DajDzienTygodnia()] += l;};
    void zmienCzasCzekania(int l) {czasCzekania[kol.KtoraGodzina().DajDzienTygodnia()] += l;};
    void zmienLiczbaCzekania(int l) {liczbaCzekania[kol.KtoraGodzina().DajDzienTygodnia()] += l;};
    void zmienOsoboSpektakle (int l) {osoboSpektakle[kol.KtoraGodzina().DajDzienTygodnia()] += l;};
    void zmienMiejscoSpektakle(int l) {miejscoSpektakle[kol.KtoraGodzina().DajDzienTygodnia()] += l;};
    void zmienLiczbaZakupow(int l) {liczbaZakupow[kol.KtoraGodzina().DajDzienTygodnia()] += l;};
    string ToString() const; 
    // techniczne
    Statystyki();
    Statystyki(const Statystyki&) = delete;
    ~Statystyki() = default;
};

class Obiekt {
protected:  
    string typ;         // typ odroznia obiekty roznych podklas
    int id;             // identyfikator odroznia obiekty w obrebie podklasy
    int przystanekId;   // id aktualnego polozenia obiektu; -1 oznacza obiekt niematerialny lub "poza" miastem
    Czas zajetyDo;      // w niektorych przypadkach obiekt jest zajety i nie powinien reagowac na zdarzenia do jakiejs godziny
    Bilet* bilet;       // obiekt bedacy w podrozy rowniez nie powinien reagowac stad potrzeba rozrozniana czy obiekt ma bilet
    // zmienne stat. daja obiektom dostep do glob. inform. uzywanych w metodach
    static int liczbaPrzystankow;           
    static int liczbaLiniiTramwajowych;                  
    static int liczbaUniwersytetow;         
    static int liczbaTeatrow;               
    static int liczbaCentrowHandlowych;
    static Przystanek* przystanki;
    static LiniaTramwajowa* linieTramwajowe;
    static Uniwersytet* uniwersytety;
    static Teatr* teatry;
    static CentrumHandlowe* centraHandlowe;
    static Statystyki* statystyki;
public:
    // API
    virtual bool Reaguj(string) = 0;
    virtual string DajTyp() const {return typ;};
    virtual int DajId() const {return id;};
    virtual int DajPrzystanekId() const {return przystanekId;};
    virtual Czas DajZajetyDo() const {return zajetyDo;};
    virtual Bilet* DajBilet() {return bilet;};
    virtual void PrzesunBilet() {bilet->Przesun();};
    virtual void KasujBilet();
    // operator << zdefiniowany jako funkcja
    // techniczne
    virtual string ToString() const;
    virtual string ToStringSimple() const;
    static void UstawLiczbaPrzystankow(int l) {liczbaPrzystankow = l;};
    static void UstawLiczbaLiniiTramwajowych(int l) {liczbaLiniiTramwajowych = l;};
    static void UstawLiczbaUniwersytetow(int l) {liczbaUniwersytetow = l;};
    static void UstawLiczbaTeatrow(int l) {liczbaTeatrow = l;};
    static void UstawLiczbaCentrowHandlowych(int l) {liczbaCentrowHandlowych = l;};
    static void UstawPrzystanki(Przystanek* przyst) {przystanki = przyst;};
    static void UstawLinieTramwajowe(LiniaTramwajowa* lin) {linieTramwajowe = lin;};
    static void UstawUniwersytety(Uniwersytet* uniw) {uniwersytety = uniw;};
    static void UstawTeatry(Teatr* te) {teatry = te;};
    static void UstawCentraHandlowe(CentrumHandlowe* centra) {centraHandlowe = centra;};
    static void UstawStatystyki(Statystyki* stat) {statystyki = stat;};
    explicit Obiekt(string, int, int); // explicit, bo chcę konstruktora, ale nie konwersję
    Obiekt();
    Obiekt(const Obiekt&) = default;
    Obiekt& operator=(const Obiekt&) = default;
    virtual ~Obiekt() = default; // To jest bardzo ważne zw. virtual
};

class Przystanek : public Obiekt {
protected:
    string polozenie;
    deque<Osoba*> kolejka;      // nowe osoby wchodza na poczatek
public:
    // API
    bool Reaguj(string) override;   
    void PlanujDzien();
    void KonczDzialanie();
    void Ustaw(Osoba* o) {kolejka.push_front(o);};
    void Oddal(int i) {kolejka.erase(kolejka.begin()+i);};
    string DajPolozenie() const {return polozenie;};
    Osoba* DajOsoba(int i) {return kolejka[i];};
    int DajDlugosc() {return kolejka.size();};
    // techniczne
    string ToString() const;
    Przystanek();
    Przystanek(int, string);
    Przystanek(const Przystanek&) = delete;
    ~Przystanek() = default;
};

struct ElementLinii {
    int przystanekId;
    int czas;
    ElementLinii* poprz;
    ElementLinii* nast;
    
    ElementLinii(int przystId, int cz, ElementLinii* pop, ElementLinii* nas) : przystanekId(przystId),
    czas(cz), poprz(pop), nast(nas){};
};

class Linia : public Obiekt {       // Linia impl. jako lista dwukierunkowa
protected:
    int liczbaPojazdow;
    ElementLinii* trasa;
public:
    // API
    virtual void DodajPrzystanek(int, int);
    ElementLinii* DajPoczatekTrasy();
    ElementLinii* DajKoniecTrasy();
    int CzasPostoju() {return DajKoniecTrasy()->czas;};
    // techniczne
    virtual string ToString() const;
    Linia();
    Linia(int, int);
    Linia(const Linia&) = delete;
    virtual ~Linia();
};

class LiniaTramwajowa : public Linia {
protected:
    static double procentZmniejszenia;
    double odstep;
    Tramwaj* tramwaje;
    int wypuszczone;
public:
    // API
    bool Reaguj(string);
    void PlanujDzien();
    void WyslijTramwaj(bool);
    // techniczne
    double LiczOdstep();
    static void UstawProcentZmniejszenia(double p) {procentZmniejszenia = p;};
    string ToString() const;
    LiniaTramwajowa();
    LiniaTramwajowa(int, int);
    LiniaTramwajowa(const Linia&) = delete;
//    ~LiniaTramwajowa();
};

class Pojazd : public Obiekt {
protected:
    int liniaId;
    ElementLinii* polozenieTrasa;
public:
    // API
    // techniczne
    string ToString() const;
    Pojazd();
    Pojazd(string, int, int);
    Pojazd(const Pojazd&) = delete;
    ~Pojazd() = default;    
};

class Tramwaj : public Pojazd {
protected:
    static int pojemnosc;
    vector<Osoba*> pasazerowie;
    bool czyNaprzod;
public:
    // API
    bool Reaguj(string);
    void RuszajNaprzod();
    void RuszajWstecz();
    void AktualizujPasazerow();
    void Wysiadac();
    void Wsiadac();
    void KonczKursowanie();
    void UstawPrzystanekId(int przystId) {przystanekId = przystId;};
    void UstawPolozenieTrasa(ElementLinii* polTrasa) {polozenieTrasa = polTrasa;};
    // techniczne
    string ToString() const;
    static void UstawPojemnosc(int poj) {pojemnosc = poj;};
    Tramwaj();
    Tramwaj(int, int);
    Tramwaj(const Tramwaj&) = delete;
    ~Tramwaj() = default;
};

class Miejsce : public Obiekt {
protected:
    string nazwa;
public:
    // API
    virtual string DajNazwa() const {return nazwa;};
    // techniczne
    virtual string ToString() const;
    Miejsce();
    Miejsce(string, int, string, int);
    Miejsce(const Miejsce&) = delete;
    ~Miejsce() = default;
};

class Uniwersytet : public Miejsce {
protected:
    // ...
public:
    // API
    bool Reaguj(string) {return true;};
    // techniczne
    string ToString() const;
    Uniwersytet();
    Uniwersytet(int, string, int);
    Uniwersytet(const Uniwersytet&) = default;
    ~Uniwersytet() = default;
};

class Teatr : public Miejsce {
protected:
    int pojemnosc;
    bool czyOtwarty[5];
    Czas pierwszyWolny;
    Czas czasSpektaklu;
    int zapelnieniePierwszyWolny;
public:
    // API
    Czas DajPierwszyWolny() {return pierwszyWolny;};
    Czas DajCzasSpektaklu() {return czasSpektaklu;};
    bool Reaguj(string);
    void SprzedajBilet();
    void OdgrywajSpektakl();
    void KonczSpektakl();
    // techniczne
    void AktualizujPierwszyWolny();
    string ToString() const;
    Teatr();
    Teatr(int, string, int, int);
    Teatr(const Teatr&) = delete;
    ~Teatr() = default;
};

class CentrumHandlowe : public Miejsce {
protected:
    // ...
public:
    // API
    bool Reaguj(string) {return true;};
    // techniczne
    string ToString() const;
    CentrumHandlowe();
    CentrumHandlowe(int, string, int);
    CentrumHandlowe(const CentrumHandlowe&) = delete;
    ~CentrumHandlowe() = default;
};

class Osoba : public Obiekt {
protected:
    int mieszkanieId;
    static double pZakupy;
    double pTeatr;
    BiletTeatr* biletTeatr;
public:
    // API
    virtual bool Reaguj(string);
    virtual void PlanujDzien() = 0;
    virtual void Podroz(int);
    //virtual void Podroz2(int);
    //virtual Bilet SzukajBiletu(Bilet, bool*, int);
    virtual void UstawNaPrzystanku();
    virtual void ZejdzZPrzystanku();
    virtual void WsiadzDoTramwaju();
    virtual void WysiadzZTramwaju();
    virtual int DajMieszkanieId() {return mieszkanieId;};
    virtual void NaZakupy();
    virtual void ZZakupow();
    virtual Czas LosujCzasNaZakupy();
    virtual void RezerwujTeatr();
    virtual void DoTeatru();
    virtual void ZTeatru();
    virtual void KasujBiletTeatr();
    virtual void ZamowTaxi();
    virtual void UstawPrzystanekId(int przystId) {przystanekId = przystId;};
    // techniczne
    virtual string ToString() const;
    static void UstawPZakupy(double p) {pZakupy = p;};
    Osoba();
    Osoba(string, int, double);
    Osoba(const Osoba&) = delete;
    ~Osoba() = default;
};

class Pracujacy : public Osoba {
protected:
    int pracaId;
    Czas czasDoPracy;
    static double pWyjazd;
    static double pTeatr;
public:
    // API
    bool Reaguj(string);
    void PlanujDzien();
    void DoPracy();
    void WyjazdSluzbowy();
    void ZPracy();
    // techniczne
    string ToString() const;
    static void UstawPWyjazd(double p) {pWyjazd = p;};
    static void UstawPTeatr(double p) {pTeatr = p;};
    Pracujacy();
    Pracujacy(int);
    Pracujacy(const Pracujacy&) = delete;
    ~Pracujacy() = default;
};

class Artysta : public Osoba {
protected:
    int pracaId[7];
    static double pTeatr;
public:
    // API
    bool Reaguj(string);
    Czas LosujCzasDoPracy();
    void PlanujDzien();
    void DoPracy();
    void ZPracy();
    // techniczne
    string ToString() const;
    static void UstawPTeatr(double p) {pTeatr = p;};
    Artysta();
    Artysta(int);
    Artysta(const Artysta&) = delete;
    ~Artysta() = default;
};

class Student : public Osoba {
protected:
    int uniwersytet1Id;
    int uniwersytet2Id;
    Czas czasNaUniwersytet[5];
    Czas czasZajec[5];
    static double pTeatr;
public:
    // API
    bool Reaguj(string);
    void PlanujDzien();
    void NaUniwersytet();
    void ZUniwersytetu();
    // techniczne
    string ToString() const;
    static void UstawPTeatr(double p) {pTeatr = p;};
    Student();
    Student(int, bool);
    Student(const Student&) = delete;
    ~Student() = default;
};

class Uczen : public Osoba {
protected:
    int szkolaId;
    Czas czasLekcji[5];
    static double pTeatr;
public:
    // API
    bool Reaguj(string);
    void PlanujDzien();
    void DoSzkoly();
    void ZeSzkoly();
    // techniczne
    string ToString() const;
    static void UstawPTeatr(double);
    Uczen();
    Uczen(int);
    Uczen(const Uczen&) = delete;
    ~Uczen() = default;
};

// --- Implementacja Czas ---

Czas Czas::Dzisiaj() {
    Czas a(miesiac, dzien, 0, 0);
    return a;
};

Czas Czas::Srodek() {
    Czas a(0, 0, 0, 0);
    a.minuta = (((miesiac * 30 + dzien)*24 + godzina)*60 + minuta) / 2;
    a.Skroc();
    return a;
};

Czas Czas::Jutro() {
    Czas a(miesiac, dzien + 1, 0, 0);
    return a;
};

// W moim programie wszystkie skladowe czasu licze od 0, dla prostoty i
// spojnosci zapisu czasu jako daty i jako odstepu czasu.

void Czas::operator+=(const Czas& a) {
    miesiac += a.miesiac;
    dzien += a.dzien;
    godzina += a.godzina;
    minuta += a.minuta;
    dzienTygodnia = (dzienTygodnia + a.dzienTygodnia) % 7;
    Skroc();
};

Czas Czas::operator+(const Czas& a) {
    Czas wyn(*this);
    wyn += a;
    return wyn;
};

void Czas::operator+=(const int& a) {
    Czas b(0, 0, 0, a);
    (*this) += b;
};

Czas Czas::operator+(const int& a) {
    Czas wyn(*this);
    wyn += a;
    return wyn;
};

void Czas::operator-=(const Czas& a) {
    assert(*this>a);
    minuta = ((miesiac * 30 + dzien)*24 + godzina)*60 + minuta;
    godzina = 0;
    dzien = 0;
    miesiac = 0;
    dzienTygodnia = 0;
    minuta -= ((a.miesiac * 30 + a.dzien)*24 + a.godzina)*60 + a.minuta;
    Skroc();
};

Czas Czas::operator-(const Czas& a) {
    Czas wyn(*this);
    wyn -= a;
    return wyn;
};

void Czas::operator-=(const int& a) {
    Czas b(0, 0, 0, a);
    assert(*this>b);
    (*this) -= b;
};

Czas Czas::operator-(const int& a) {
    Czas wyn(*this);
    wyn -= a;
    return wyn;
};

Czas& Czas::operator=(const Czas& a) {
    if(&a != this) {
        miesiac = a.miesiac;
        dzien = a.dzien;
        godzina = a.godzina;
        minuta = a.minuta;
        dzienTygodnia = a.dzienTygodnia;
    }
    return *this;
};

bool Czas::operator>(Czas a) {
    if (miesiac > a.miesiac)
        return true;
    else if (miesiac < a.miesiac)
        return false;
    else if (dzien > a.dzien)
        return true;
    else if (dzien < a.dzien)
        return false;
    else if (godzina > a.godzina)
        return true;
    else if (godzina < a.godzina)
        return false;
    else if (minuta > a.minuta)
        return true;
    else return false;
};

bool Czas::operator<(Czas a) {
    if (a> *this)
        return true;
    else return false;
};

bool Czas::operator==(Czas a) {
    if (*this>a || *this<a)
        return false;
    else return true;
};

bool Czas::operator!=(Czas a) {
    if (*this == a)
        return false;
    else return true;
};

bool Czas::operator>=(Czas a) {
    if (*this<a)
        return false;
    else return true;
};

bool Czas::operator<=(Czas a) {
    if (*this>a)
        return false;
    else return true;
};

Czas& Czas::operator++(){
    (*this) += 1;
    return *this;
};

Czas Czas::operator++(int){
    Czas wyn(*this);
    ++(*this);
    return wyn;
};

Czas& Czas::operator--(){
    (*this) -= 1;
    return *this;
};

Czas Czas::operator--(int){
    Czas wyn(*this);
    --(*this);
    return wyn;
};

ostream& operator<< (ostream &wyjscie, const Czas& c) {
   wyjscie << c.ToString();
   return wyjscie;
};

string Czas::ToString() const {
    string a = "Czas[";
    a += (miesiac < 10 ? "0" : "") + to_string(miesiac) + "." +
    (dzien < 10 ? "0" : "") + to_string(dzien);
    switch (dzienTygodnia) {
        case 0: a += "-pn-"; break;
        case 1: a += "-wt-"; break;
        case 2: a += "-sr-"; break;
        case 3: a += "-cw-"; break;
        case 4: a += "-pt-"; break;
        case 5: a += "-sb-"; break;
        case 6: a += "-nd-"; break;
        default: a += "-BLAD-"; break;
    };
    a += (godzina < 10 ? "0" : "") + to_string(godzina) + ":" +
            (minuta < 10 ? "0" : "") + to_string(minuta) + "]; ";
    return a;
};   

void Czas::Skroc() {
    int pom;
    if (minuta > 59) {
        pom = minuta;
        minuta %= 60;
        pom = (pom - minuta) / 60;
        godzina += pom;
    }
    if (godzina > 23) {
        pom = godzina;
        godzina %= 24;
        pom = (pom - godzina) / 24;
        dzien += pom;
        dzienTygodnia = (dzienTygodnia + pom) % 7;
    }
    if (dzien > 29) {
        pom = dzien;
        dzien %= 30;
        pom = (pom - dzien) / 30;
        miesiac += pom;
    }
};

Czas::Czas(int mies = 0, int dz = 0, int godz = 0, int min = 0) {
    if (mies < 0 || dz < 0 || godz < 0 || min < 0) {
        cout << "BLAD: Czas::Zly parametr konstruktora!";
        exit(13);
    }
    miesiac = mies;
    dzien = dz;
    godzina = godz;
    minuta = min;
    dzienTygodnia = ((miesiac * 30) + dzien) % 7;
    Skroc();
};

// --- Implementacja Bilet ---

void Bilet::DodajPrzesiadke(int linId, int przystId, bool czyNap) {
    if (trasa == nullptr)
        trasa = new ElementBiletu(linId, przystId, czyNap, nullptr);
    else {
        ElementBiletu* p = trasa;
        while (p->nast != nullptr)
            p = p->nast;
        // ok jestesmy w ostatnim elemencie
        p->nast = new ElementBiletu(linId, przystId, czyNap, nullptr);
    }
    aktualny = trasa;
};

ElementBiletu* Bilet::DajKoniec() {
    if (trasa == nullptr)
        return nullptr;
    else {
        ElementBiletu* p = trasa;
        while (p->nast != nullptr)
            p = p->nast;
        // ok jestesmy w ostatnim elemencie
        return p;
    }    
};

void Bilet::Uprosc() {
    if (trasa != nullptr) {
        ElementBiletu* p = trasa;
        while (p->nast != nullptr) {
            if (p->liniaId == p->nast->liniaId &&
                    p->czyNaprzod == p->nast->czyNaprzod) {
                ElementBiletu* temp = p->nast;
                p->nast = p->nast->nast;
                delete temp;                
            } else 
                p = p->nast;
        }
    }
};

void Bilet::Wypisz() {
    cout << "Bilet(";
    if (trasa == nullptr)
        cout << "pusty) ";
    else {
        ElementBiletu* p = trasa;
        while (p->nast != nullptr) {
            cout << "linId" << to_string(p->liniaId) << ", ";
            cout << "przystId" << to_string(p->przystanekId);
            p = p->nast;
        }
        // ok jestesmy w ostatnim elemencie
        cout << "linId" << to_string(p->liniaId) << ", ";
        cout << "przystId" << to_string(p->przystanekId) << ") ";
    }  
};

Bilet::Bilet() {
    aktualny = trasa = nullptr;
}

Bilet::Bilet(int lin, int cId, bool czyNap) {
    aktualny = trasa = nullptr;
    DodajPrzesiadke(lin, cId, czyNap);
};

Bilet::~Bilet() {
    ElementBiletu* p = trasa;
    if (p != nullptr) {
        while (p->nast != nullptr){
            ElementBiletu* temp = p->nast;
            delete p;
            p = temp;
        }
        delete p;
    }
};

// --- Implementacja Statystyki ---

Statystyki::Statystyki() {
    for (int i=0; i<7; i++) {
        liczbaPrzejazdow[i] = 0;
        czasCzekania[i] = 0;
        liczbaCzekania[i] = 0;
        osoboSpektakle[i] = 0;
        miejscoSpektakle[i] = 0;
        liczbaZakupow[i] = 0;        
    }
}

string Statystyki::ToString() const {
    string stat = "LACZNE:\n";
    int a = 0;
    for (int i=0; i<7; i++)
        a += liczbaPrzejazdow[i];
    stat += "Laczna liczba przejazdow = " + to_string(a) + "\n";
    double b = 0;
    double c = 0;
    for (int i=0; i<7; i++) {
        b += czasCzekania[i];
        c += liczbaCzekania[i];        
    }
    stat += "Sredni czas czekania na przystanku = " + (c != 0? to_string(b/c) : "?") + "min\n";
    double d = 0;
    double e = 0;
    for (int i=0; i<7; i++) {
        d += osoboSpektakle[i];
        e += miejscoSpektakle[i];        
    }
    stat += "Sredni procent wypelnienia teatru = " + (e != 0? to_string(d/e * 100) : "?") + "%\n";
    double f = 0;
    for (int i=0; i<5; i++)
        f += liczbaZakupow[i];
    stat += "Srednia liczba zakupow w tygodniu roboczym = " + to_string(f/5) + "\n";
    for (int i=0; i<7; i++) {
        stat += "DZIEN TYGODNIA " + to_string(i+1) + "\n";
        stat += "Laczna liczba przejazdow = " + to_string(liczbaPrzejazdow[i]) + "\n";
        stat += "Laczny czas czekania na przystankach = " + to_string(czasCzekania[i]) + "min\n";
        stat += "Liczba osobo-spektakli = " + to_string(osoboSpektakle[i]) + "\n";
        stat += "Liczba zakupow = " + to_string(liczbaZakupow[i]) + "\n";
    }
    return stat;
}

// --- Implementacja Obiekt ---

void Obiekt::KasujBilet() {
    delete bilet;
    bilet = nullptr;
}

string Obiekt::ToString() const {
    string a = "Obiekt[typ=" + to_string(typ) + ", id=" + to_string(id);
    if (przystanekId != -1)
        a += ", polozenie=" + przystanki[przystanekId].DajPolozenie();
    return a += "]; ";
};

string Obiekt::ToStringSimple() const {
    string a = "Obiekt[typ=" + to_string(typ) + ", id=" + to_string(id);
    if (przystanekId != -1)
        a += ", polozenie=" + przystanki[przystanekId].DajPolozenie();
    return a += "]; ";
};

Obiekt::Obiekt() : typ("niezdefiniowano"), id(-1), przystanekId(-1), bilet(nullptr) {
    Czas a;
    zajetyDo = a;
};

Obiekt::Obiekt(string t, int d, int przystId) : typ(t), id(d), bilet(nullptr),
        przystanekId(przystId) {
    Czas a;
    zajetyDo = a;
};

ostream& operator<< (ostream &wyjscie, const Obiekt& o) {
   wyjscie << o.ToString();
   return wyjscie;
};

// --- Implementacja Przystanek ---

bool Przystanek::Reaguj(string czynnosc) {
    if (czynnosc == "planuje dzien")
        PlanujDzien();
    else if (czynnosc == "konczy dzialanie")
        KonczDzialanie();
    else
        cout << "BLAD: Przystanek.Reaguj::Podano zla nazwe czynnosci! " << czynnosc << endl;
};

void Przystanek::PlanujDzien() {
    Czas b(0, 0, 23, 0);
    kol.Wstaw(new Zdarzenie(kol.KtoraGodzina()+b, *this, "konczy dzialanie"));  
    kol.Wstaw(new Zdarzenie(kol.KtoraGodzina().Jutro(), *this, "planuje dzien"));  
};

void Przystanek::KonczDzialanie() {
    for (int i=0; i<kolejka.size(); i++) {
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *kolejka[i], "zamawia taxi"));
        Czas a = kol.KtoraGodzina();
        statystyki->zmienCzasCzekania(60*a.DajGodzina()+a.DajMinuta());
        kolejka[i]->KasujBilet();
    }
    kolejka.clear();
};

string Przystanek::ToString() const {
    string a = Obiekt::ToString() + "Przystanek[oczekujacy=(";
    for (int i=0; i<kolejka.size(); i++)
        a += kolejka[i]->DajTyp() + " " + to_string(kolejka[i]->DajId()) + ", ";
    return a += ")]; ";
};

Przystanek::Przystanek() : Obiekt(), polozenie("niezdefiniowano") {};

Przystanek::Przystanek(int d, string pol) : Obiekt("przystanek", d, d), 
        polozenie(pol) {};

// --- Implementacja Linia ---
        
string Linia::ToString() const {        
    string a = Obiekt::ToString() + "Linia[ilosc pojazdow=" + to_string(liczbaPojazdow) 
            + ", trasa=(";
    ElementLinii* p = trasa;
    if (p == nullptr)
        a += ")]; ";
    else {
        while (p->nast != nullptr){         
            a += przystanki[p->przystanekId].DajPolozenie() + ", " + 
                    to_string(p->czas) + ", ";
            p = p->nast;
        }
        a += przystanki[p->przystanekId].DajPolozenie() + ", " + 
                to_string(p->czas) + ")]; ";
    } return a;
};    
        
void Linia::DodajPrzystanek(int przystID, int czas) {
    if (trasa == nullptr)
        trasa = new ElementLinii(przystID, czas, nullptr, nullptr);        
    else {
        ElementLinii* p = trasa;
        while (p->nast != nullptr)
            p = p->nast;
        // dobra jestesmy w ostatnim elemencie
        p->nast = new ElementLinii(przystID, czas, p, nullptr);
    }
};

ElementLinii* Linia::DajPoczatekTrasy() {
    return trasa;
};

ElementLinii* Linia::DajKoniecTrasy() {
    if (trasa == nullptr)
        return trasa;
    else{
        ElementLinii* p = trasa;
        while (p->nast != nullptr)
            p = p->nast;
        return p;
    }    
};

Linia::Linia() : Obiekt(), liczbaPojazdow(-1), trasa(nullptr) {};

Linia::Linia(int d, int liczbaPoj) : Obiekt("linia", d, -1), liczbaPojazdow(liczbaPoj), 
        trasa(nullptr) {};

Linia::~Linia(){
    ElementLinii* p = trasa;
    if (p != nullptr) {
        while (p->nast != nullptr){
            ElementLinii* temp = p->nast;
            delete p;
            p = temp;
        }
        delete p;
    }
};

// --- Implementacja LiniaTramwajowa ---

bool LiniaTramwajowa::Reaguj(string czynnosc) {
    if (czynnosc == "planuje dzien")
        PlanujDzien();
    else if (czynnosc == "wysyla tramwaj z poczatku")
        WyslijTramwaj(false);
    else if (czynnosc == "wysyla tramwaj z konca")
        WyslijTramwaj(true);
    else
        cout << "BLAD:LiniaTramwajowa.Reaguj::Podano zla nazwe czynnosci!" << czynnosc << endl;   
};

void LiniaTramwajowa::PlanujDzien() {
    odstep = LiczOdstep();
    wypuszczone = 0;
    int i = 0;
    Czas a(0, 0, 6, 0);
    a += kol.KtoraGodzina();
    Czas b(0, 0, 23, 0);
    b += kol.KtoraGodzina();
    if (kol.DzienTygodnia()<5){
        while (i < liczbaPojazdow && a <= b) {
            kol.Wstaw(new Zdarzenie(a, *this, "wysyla tramwaj z poczatku"));
            i++;
            if (i < liczbaPojazdow){
                kol.Wstaw(new Zdarzenie(a, *this, "wysyla tramwaj z konca")); 
                i++;
                a += floor(odstep);
            }
        }
    } else { 
        int ilosc = floor(liczbaPojazdow*(100-procentZmniejszenia)/100);
        int ods = floor(odstep*100/(100-procentZmniejszenia));
        while (i < ilosc && a <= b) {
            kol.Wstaw(new Zdarzenie(a, *this, "wysyla tramwaj z poczatku"));

            i++;
            if (i < ilosc) {
                kol.Wstaw(new Zdarzenie(a, *this, "wysyla tramwaj z konca"));
                i++;
                a += ods;
            }
        }
    }
    kol.Wstaw(new Zdarzenie(kol.KtoraGodzina().Jutro(), *this, "planuje dzien"));
};

void LiniaTramwajowa::WyslijTramwaj(bool odwrotnie) {
    ElementLinii* start = (odwrotnie == true? DajKoniecTrasy() : DajPoczatekTrasy());
    tramwaje[wypuszczone].UstawPrzystanekId(start->przystanekId);
    tramwaje[wypuszczone].UstawPolozenieTrasa(start);
    if (odwrotnie == false)
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), tramwaje[wypuszczone], "zatrzymuje sie (kierunek naprzod)"));
    else
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), tramwaje[wypuszczone], "zatrzymuje sie (kierunek wstecz)"));        
    wypuszczone++;
};

double LiniaTramwajowa::LiczOdstep(){
    ElementLinii* p = trasa;
    double o = 0;
    if (p != nullptr) {
        while (p->nast != nullptr){         
            o += p->czas;
            p = p->nast;
        }
        o += p->czas;
    }
    o *= 2;
    o /= liczbaPojazdow;
    o = floor(o);
    return o;
};

string LiniaTramwajowa::ToString() const {
    return Linia::ToString() + "LiniaTramwajowa[...]; ";
};

LiniaTramwajowa::LiniaTramwajowa() : Linia(), odstep(-1), wypuszczone(0),
tramwaje(nullptr) {};

LiniaTramwajowa::LiniaTramwajowa(int d, int liczbaPoj) : Linia(d, liczbaPoj), 
        odstep(-1), wypuszczone(0) {
    tramwaje = new Tramwaj[liczbaPoj];
    for (int i=0; i<liczbaPoj; i++) {
//        Tramwaj* p = new Tramwaj(i, id);
//        tramwaje[i] = *p;
        tramwaje[i] = Tramwaj(i, id);
        cout << tramwaje[i] << endl;
    }
};
 
// LiniaTramwajowa::~LiniaTramwajowa() {
//    delete[] tramwaje;
// }

 // --- Impl. Pojazd ---

string Pojazd::ToString() const {
    return Obiekt::ToString() + "Pojazd[liniaId=" + to_string(liniaId) + "]; ";
};

Pojazd::Pojazd() : Obiekt(), liniaId(-1), polozenieTrasa(nullptr) {};

Pojazd::Pojazd(string t, int d, int lin) : Obiekt(t, d, -1), liniaId(lin), polozenieTrasa(nullptr) {};

// --- Implementacja Tramwaj ---

bool Tramwaj::Reaguj(string czynnosc) {
    if (czynnosc == "zatrzymuje sie (kierunek naprzod)")
        RuszajNaprzod();
    else if (czynnosc == "zatrzymuje sie (kierunek wstecz)")
        RuszajWstecz();
    else if (czynnosc == "konczy kursowanie")
        KonczKursowanie();
    else
        cout << "BLAD: Tramwaj.Reaguj::Podano zla nazwe czynnosci!" << czynnosc << endl;   
};

void Tramwaj::RuszajNaprzod() {
    czyNaprzod = true;
    AktualizujPasazerow();
    Wysiadac();
    Wsiadac();
    Czas b(0, 0, 23, 0);
    b += kol.KtoraGodzina().Dzisiaj();
    int c = polozenieTrasa->czas;
    if (kol.KtoraGodzina()+c >= b)
        kol.Wstaw(new Zdarzenie(b, *this, "konczy kursowanie"));
    else if (polozenieTrasa->nast == nullptr)
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina()+c, *this, "zatrzymuje sie (kierunek wstecz)"));        
    else {
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina()+c, *this, "zatrzymuje sie (kierunek naprzod)"));
        polozenieTrasa = polozenieTrasa->nast;
        przystanekId = polozenieTrasa->przystanekId;
    }
};

void Tramwaj::RuszajWstecz() {
    czyNaprzod = false;
    AktualizujPasazerow();
    Wysiadac();
    Wsiadac();
    Czas b(0, 0, 23, 0);
    b += kol.KtoraGodzina().Dzisiaj();
    if (polozenieTrasa->poprz == nullptr){
        int c = linieTramwajowe[liniaId].CzasPostoju();
        if (kol.KtoraGodzina()+c >= b)
            kol.Wstaw(new Zdarzenie(b, *this, "konczy kursowanie"));
        else
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina()+c, *this, "zatrzymuje sie (kierunek naprzod)"));
    } else {
        int c = polozenieTrasa->poprz->czas;
        if (kol.KtoraGodzina()+c >= b)
            kol.Wstaw(new Zdarzenie(b, *this, "konczy kursowanie"));
        else {
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina()+c, *this, "zatrzymuje sie (kierunek wstecz)"));  
            polozenieTrasa = polozenieTrasa->poprz;
            przystanekId = polozenieTrasa->przystanekId;
        }          
    }
};

void Tramwaj::AktualizujPasazerow() {
    for (int i=0; i<pasazerowie.size(); i++)
        pasazerowie[i]->UstawPrzystanekId(przystanekId);
};

void Tramwaj::Wysiadac() {
    int a = pasazerowie.size();
    for (int i=a-1; i>=0; i--){
        if (pasazerowie[i]->DajBilet()->DajAktualny()->przystanekId == przystanekId) {
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *pasazerowie[i], "wysiada z tramwaju"));
            pasazerowie[i]->PrzesunBilet();
            przystanki[przystanekId].Ustaw(pasazerowie[i]);
            if (pasazerowie[i]->DajBilet()->DajAktualny() == nullptr)
                kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *pasazerowie[i], "schodzi z przystanku"));
            pasazerowie.erase(pasazerowie.begin()+i); 
        }
    }
};

void Tramwaj::Wsiadac() {
    int a = przystanki[przystanekId].DajDlugosc();
    for (int i=a-1; i>=0; i--) {
        if (przystanki[przystanekId].DajOsoba(i)->DajBilet() != nullptr &&
                przystanki[przystanekId].DajOsoba(i)->DajBilet()->DajAktualny() != nullptr &&
                przystanki[przystanekId].DajOsoba(i)->DajBilet()->DajAktualny()->liniaId == liniaId && 
                przystanki[przystanekId].DajOsoba(i)->DajBilet()->DajAktualny()->czyNaprzod == czyNaprzod &&
                pasazerowie.size()<pojemnosc) {
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *przystanki[przystanekId].DajOsoba(i), "wsiada do tramwaju"));
            pasazerowie.push_back(przystanki[przystanekId].DajOsoba(i));
            przystanki[przystanekId].Oddal(i);
        }        
    }
};

void Tramwaj::KonczKursowanie() {
    for (int i=0; i<pasazerowie.size(); i++) {
        pasazerowie[i]->KasujBilet();
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *pasazerowie[i], "zamawia taxi"));        
    }
    pasazerowie.clear();
    przystanekId = -1;
};

string Tramwaj::ToString() const {
    string a = Pojazd::ToString() + "Tramwaj[zapelnienie=" + to_string(pasazerowie.size()) + ", pasazerowie=(";
    for (int i=0; i<pasazerowie.size(); i++)
        a += to_string(pasazerowie[i]->DajTyp()) + " " + to_string(pasazerowie[i]->DajId()) + ", ";
    return a+= ")]; ";
};

Tramwaj::Tramwaj() : Pojazd(), czyNaprzod(true) {};

Tramwaj::Tramwaj(int i, int lin) : Pojazd("tramwaj", i, lin), czyNaprzod(true) {};
 
// --- Implementacja Miejsce ---

string Miejsce::ToString() const {
    return Obiekt::ToString() + "Miejsce[nazwa=" + nazwa + "]; ";
};

Miejsce::Miejsce() : Obiekt() {};

Miejsce::Miejsce(string t, int i, string naz, int przyst) : Obiekt(t, i, przyst), nazwa(naz){};

// ----------------- Impl. Uniwersytet --------------------

string Uniwersytet::ToString() const {
    return Miejsce::ToString() + "Uniwersytet[...]; ";
}

Uniwersytet::Uniwersytet() : Miejsce() {};

Uniwersytet::Uniwersytet(int i, string naz, int przyst) : 
        Miejsce("uniwersytet", i, naz, przyst) {};

// ---------------- Impl. Teatr ---------------------------

bool Teatr::Reaguj(string czynnosc){
    if (czynnosc == "sprzedaje bilet")
        SprzedajBilet();
    else if (czynnosc == "odgrywa spektakl")
        OdgrywajSpektakl();
    else if (czynnosc == "konczy spektakl")
        KonczSpektakl();
    else 
        cout << "BLAD: Teatr.Reaguj::Podano zla nazwe czynnosci!" << czynnosc << endl;  
    return true;
};

void Teatr::SprzedajBilet(){
    // bilety na seans wyprzedane lub seans juz sie skonczyl
    if (zapelnieniePierwszyWolny == pojemnosc || kol.KtoraGodzina() >= pierwszyWolny){
        zapelnieniePierwszyWolny = 0;       
        AktualizujPierwszyWolny();          
    }
    zapelnieniePierwszyWolny++;
    if (zapelnieniePierwszyWolny == 1){     // mamy widza, ruszamy z organizacja spektaklu!
        Czas a(0, 0, 1, rand() % 121);
        czasSpektaklu = a;
        kol.Wstaw(new Zdarzenie(pierwszyWolny, *this, "odgrywa spektakl"));  
        kol.Wstaw(new Zdarzenie(pierwszyWolny + czasSpektaklu, *this, "konczy spektakl"));
    }
};

void Teatr::OdgrywajSpektakl() {
    statystyki->zmienMiejscoSpektakle(pojemnosc);
};

void Teatr::KonczSpektakl() {};

void Teatr::AktualizujPierwszyWolny(){
    // termin jest niedobry wiec szukamy nastepnego
    bool dobry = false;
    Czas a(0, 0, 17, 0);
    Czas b(0, 0, 19, 0);
    while (dobry == false || kol.KtoraGodzina() >= pierwszyWolny) {
        switch (pierwszyWolny.DajDzienTygodnia()){
            case 0: pierwszyWolny = pierwszyWolny.Jutro() + b; dobry = czyOtwarty[1]; break;
            case 1: pierwszyWolny = pierwszyWolny.Jutro() + b; dobry = czyOtwarty[2]; break;
            case 2: pierwszyWolny = pierwszyWolny.Jutro() + b; dobry = czyOtwarty[3]; break;
            case 3: pierwszyWolny = pierwszyWolny.Jutro() + b; dobry = czyOtwarty[4]; break;
            case 4: pierwszyWolny = pierwszyWolny.Jutro() + a; dobry = true; break;
            case 5: 
                if (pierwszyWolny.DajGodzina() == 17){
                    pierwszyWolny = pierwszyWolny.Dzisiaj() + b;        
                    dobry = true;
                } else {
                    pierwszyWolny = pierwszyWolny.Jutro() + a;
                    dobry = true;
                } break;
            case 6:
                if (pierwszyWolny.DajGodzina() == 17){
                    pierwszyWolny = pierwszyWolny.Dzisiaj() + b;
                    dobry = true;
                } else {
                    pierwszyWolny = pierwszyWolny.Jutro() + b;
                    dobry = czyOtwarty[0]; 
                } break;
            default: cout << "BLAD: Teatr.AktualizujPierwszyWolny::Zly dzien tygodnia!" << endl; break;
        }
    }
};

string Teatr::ToString() const {
    return Miejsce::ToString() + "Teatr[pojemnosc=" + 
            to_string(pojemnosc) + ", pierwszyWolny=" + 
            pierwszyWolny.ToString() + "zapelnieniePierwszyWolny=" +
            to_string(zapelnieniePierwszyWolny) + "]; ";
};

Teatr::Teatr() : Miejsce(), pojemnosc(-1), zapelnieniePierwszyWolny(-1) {
    for (int i=0; i<5; i++)
        czyOtwarty[i] = false;
    Czas a;
    pierwszyWolny = a;
    czasSpektaklu = a;
};

Teatr::Teatr(int d, string naz, int przyst, int poj) : Miejsce("teatr", d, 
        naz, przyst), pojemnosc(poj), zapelnieniePierwszyWolny(0) {
    for (int i=0; i<5; i++)
        czyOtwarty[i] = rand() % 2;
    Czas a(0, 0, 19, 0);
    pierwszyWolny = a;
    if (czyOtwarty[0] == false)
        AktualizujPierwszyWolny();
    Czas b;
    czasSpektaklu = b;
};


// --- Implementacja CentrumHandlowe ---

string CentrumHandlowe::ToString() const {
    return Miejsce::ToString() + "CentrumHandlowe[...]; ";
};

CentrumHandlowe::CentrumHandlowe() : Miejsce() {};

CentrumHandlowe::CentrumHandlowe(int d, string naz, int przyst) : 
                Miejsce("centrum handlowe", d, naz, przyst) {};

// --- Implementacja Osoba ---

bool Osoba::Reaguj(string czynnosc) {
    if (czynnosc == "planuje dzien")
        PlanujDzien();
    else if (czynnosc == "ustawia sie na przystanku")
        UstawNaPrzystanku();
    else if (czynnosc == "schodzi z przystanku")
        ZejdzZPrzystanku();
    else if (czynnosc == "wsiada do tramwaju")
        WsiadzDoTramwaju();
    else if (czynnosc == "wysiada z tramwaju")
        WysiadzZTramwaju();
    else if (czynnosc == "wychodzi na zakupy")
        NaZakupy();
    else if (czynnosc == "wychodzi z zakupow")
        ZZakupow();
    else if (czynnosc == "rezerwuje teatr")
        RezerwujTeatr();
    else if (czynnosc == "wychodzi do teatru")
        DoTeatru();
    else if (czynnosc == "wychodzi z teatru")
        ZTeatru();
    else if (czynnosc == "zamawia taxi")
        ZamowTaxi();
    else 
        return false;
    return true;
};                

void Osoba::Podroz(int cId) {       // szuka biletu pod warunkiem ze da sie dojechac bezposrednio
    if (cId != przystanekId) {
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "ustawia sie na przystanku"));
        for (int i=0; i < liczbaLiniiTramwajowych; i++) {
            ElementLinii* p = linieTramwajowe[i].DajPoczatekTrasy();
            if (p != nullptr) {
                while (p != nullptr) {  // linie jednoprzystankowe i tak nas nie interesuja
                    if (p->przystanekId == przystanekId) {
                        // dobra, linia przebiega przez nasz przystanek! teraz szukamy celu
                        ElementLinii* nasz = p;
                        while (p != nullptr) {
                             if (p->przystanekId == cId) {
                                bilet = new Bilet(i, cId, true);
                                cout << "A\n";
                                return;
                             }
                             p = p->nast;
                        }
                        p = nasz;
                        while (p != nullptr) {
                            if (p->przystanekId == cId) {
                                bilet = new Bilet(i, cId, false);
                                cout << "B\n";
                                return;
                            }
                            p = p->poprz;
                        }
                        p = nasz;
                    }
                    p = p->nast;
                } 
            }
        }    
    }    
};

//void Osoba::Podroz2(int cId) {        // nieukonczona wersja szukania trasy do dowolnego punktu
//    if (cId != przystanekId) {
//        Bilet bil();
//        bool odwiedzony[liczbaPrzystankow];
//        for (int i=0; i<liczbaPrzystankow; i++)
//            odwiedzony[i] = false;
//        odwiedzony[przystanekId] = true;       
//        *bilet = SzukajBiletu(bil, odwiedzony, cId);
//        //bilet.Uprosc();
//        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "ustawia sie na przystanku"));          
//    }  
//;

//Bilet Osoba::SzukajBiletu(Bilet bil, bool odwiedzony[], int cId) {
//    if (bil.DajKoniec()->przystanekId == cId)
//        return bil;
//    if (cId != przystanekId) {
//        Bilet bil1 = bil;
//        Bilet bil2 = bil;
//        for (int i=0; i<liczbaLiniiTramwajowych; i++) {
//            ElementLinii* p = linieTramwajowe[i].DajPoczatekTrasy();
//            if (p != nullptr) {
//                while (p->nast != nullptr) {        // linie jednoprzystankowe i tak nas nie interesuja
//                    if (p->przystanekId == przystanekId) {
//                        // dobra znalezlismy nasz przystanek na linii! jedziemy dalej
//                        if (p->nast != nullptr && odwiedzony[p->nast->przystanekId] == false) {
//                            int nastepny = p->nast->przystanekId;
//                            bil1.DodajPrzesiadke(i, nastepny, true);
//                            odwiedzony[p->nast->przystanekId] = true;
//                            return SzukajBiletu(bil1, odwiedzony[], cId);
//                        }
//                        if (p->poprz != nullptr && odwiedzony[p->poprz->przystanekId] == false) {
//                            int poprzedni = p->poprz->przystanekId;
//                            bil2.DodajPrzesiadke(i, poprzedni, false);
//                            odwiedzony[poprzedni] = true;
//                            return SzukajBiletu(bil2, odwiedzony[], cId);
//                        }    
//                    }
//                    p = p->nast;
//                }
//            }
//        }
//    }
//    return Bilet();
//};     
                
void Osoba::UstawNaPrzystanku() {
    przystanki[przystanekId].Ustaw(this);
    statystyki->zmienLiczbaCzekania(1);
    Czas a = kol.KtoraGodzina();
    statystyki->zmienCzasCzekania(-60*a.DajGodzina()-a.DajMinuta());
    
    if (bilet == nullptr)
        cout << "JEST NUL\n\n";
};

void Osoba::ZejdzZPrzystanku() {
    for (int i=0; i<przystanki[przystanekId].DajDlugosc(); i++) {
        if (przystanki[przystanekId].DajOsoba(i) == this)
            przystanki[przystanekId].Oddal(i);
    }
    Czas a = kol.KtoraGodzina();
    statystyki->zmienCzasCzekania(60*a.DajGodzina()+a.DajMinuta()); 
    KasujBilet();
};

void Osoba::WsiadzDoTramwaju() {
    statystyki->zmienLiczbaPrzejazdow(1);     
    Czas a = kol.KtoraGodzina();
    statystyki->zmienCzasCzekania(60*a.DajGodzina()+a.DajMinuta());    
}

void Osoba::WysiadzZTramwaju() {
    Czas a = kol.KtoraGodzina();
    statystyki->zmienCzasCzekania(-60*a.DajGodzina()-a.DajMinuta());
}
                
void Osoba::NaZakupy() {
    // osoba idzie na zakupy
    Podroz(centraHandlowe[rand() % liczbaCentrowHandlowych].DajPrzystanekId());
    statystyki->zmienLiczbaZakupow(1);
    // oraz planuje co dalej
    Czas a(0, 0, 1, rand() % 121);
    a += kol.KtoraGodzina();
    kol.Wstaw(new Zdarzenie(a, *this, "wychodzi z zakupow"));
    zajetyDo = a;
};

void Osoba::ZZakupow() {
    // osoba wraca z zakupow
    Podroz(mieszkanieId);
};

Czas Osoba::LosujCzasNaZakupy() {
    Czas a(0, 0, 10, rand() % 481);
    return a;
};

void Osoba::RezerwujTeatr(){
    int tId = rand() % liczbaTeatrow;
    teatry[tId].Reaguj("sprzedaje bilet");
    Czas a = teatry[tId].DajPierwszyWolny();
    kol.Wstaw(new Zdarzenie((a-60 < kol.KtoraGodzina() ? kol.KtoraGodzina() : a-60), *this, 
        "wychodzi do teatru"));
    biletTeatr = new BiletTeatr(tId, teatry[tId].DajCzasSpektaklu());
};

void Osoba::DoTeatru(){
    // osoba idzie do teatru
    Podroz(teatry[biletTeatr->DajTeatrId()].DajPrzystanekId());
    statystyki->zmienOsoboSpektakle(1);
    Czas a = kol.KtoraGodzina() + 60 + biletTeatr->DajCzasSpektaklu();
    kol.Wstaw(new Zdarzenie(a, *this, "wychodzi z teatru"));
    zajetyDo = a;
};

void Osoba::ZTeatru(){
    KasujBiletTeatr();
    double p = rand() % 100;
    p /= 100;
    if (pTeatr > p)
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "rezerwuje teatr"));         
    // osoba wraca z teatru
    Podroz(mieszkanieId);
};

void Osoba::KasujBiletTeatr() {
    delete biletTeatr;
    biletTeatr = nullptr;
}

void Osoba::ZamowTaxi() {
    Czas a = kol.KtoraGodzina();
    statystyki->zmienCzasCzekania(60*a.DajGodzina()+a.DajMinuta());  
    przystanekId = mieszkanieId;
};


string Osoba::ToString() const {
    return Obiekt::ToString() + "Osoba[mieszkanie=" + przystanki[mieszkanieId].DajPolozenie() + "]; ";
};

Osoba::Osoba() : Obiekt(), mieszkanieId(-1), pTeatr(-1), biletTeatr(nullptr) {};

Osoba::Osoba(string t, int i, double praw) : Obiekt(t, i, rand() % 3),
        mieszkanieId(przystanekId), pTeatr(praw), biletTeatr(nullptr) {};

// --- Implementacja Pracujacy ---

bool Pracujacy::Reaguj(string czynnosc) {
    bool a = Osoba::Reaguj(czynnosc);
    if (a == false){
        if (czynnosc == "wychodzi do pracy")
            DoPracy();
        else if (czynnosc == "wyjezdza sluzbowo")
            WyjazdSluzbowy();
        else if (czynnosc == "wychodzi z pracy")
            ZPracy();
        else cout << "BLAD: Pracujacy.Reaguj::Podano zla nazwe czynnosci!" << czynnosc << endl;        
    }
    return true;
};

void Pracujacy::PlanujDzien() {
    if (kol.DzienTygodnia() < 5)
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina() + czasDoPracy, *this, "wychodzi do pracy"));
    else kol.Wstaw(new Zdarzenie(kol.KtoraGodzina() + LosujCzasNaZakupy(), *this, "wychodzi na zakupy"));
    kol.Wstaw(new Zdarzenie(kol.KtoraGodzina().Jutro(), *this, "planuje dzien"));
    if (kol.KtoraGodzina().DajDzien() == 0)
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "rezerwuje teatr"));
};

void Pracujacy::DoPracy() {
    // Osoba idzie do pracy:
    Podroz(pracaId);
    // Nastepnie planuje:
    double p = rand() % 100;
    p /= 100;
    Czas b(0, 0, 9, 0);
    b += kol.KtoraGodzina();
    if (pWyjazd > p) {
        Czas a(0, 0, 0, rand() % 541);
        a += kol.KtoraGodzina();
        kol.Wstaw(new Zdarzenie(a, *this, "wyjezdza sluzbowo"));
        zajetyDo = a;
    } else
        zajetyDo = b;
    kol.Wstaw(new Zdarzenie(b, *this, "wychodzi z pracy"));
};

void Pracujacy::WyjazdSluzbowy() {
    // Osoba jedzie w losowe miejsce i wraca:
    int celId = rand() % liczbaPrzystankow;
    Podroz(celId);
    if (bilet != nullptr)
        bilet->DodajPrzesiadke(bilet->DajAktualny()->liniaId, pracaId, !(bilet->DajAktualny()->czyNaprzod));
        //bilet.DodajOdwrotny();
    Czas b(0, 0, 9, 0);
    zajetyDo = kol.KtoraGodzina().Dzisiaj() + czasDoPracy + b;
};

void Pracujacy::ZPracy() {
    double p = rand() % 100;
    p /= 100;
    if (pZakupy > p) {
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "wychodzi na zakupy"));
    } else
        Podroz(mieszkanieId);
};

string Pracujacy::ToString() const {
    return Osoba::ToString() + "Pracujacy[praca=" + 
            przystanki[pracaId].DajPolozenie() + "]; ";
};

Pracujacy::Pracujacy() : Osoba(), pracaId(-1) {
    Czas a;
    czasDoPracy = a;
};

Pracujacy::Pracujacy(int d) : Osoba("pracujacy", d, pTeatr),
pracaId(rand() % liczbaPrzystankow) {
    Czas a(0, 0, 6, rand() % 241);
    czasDoPracy = a;
};

// --- Implementacja Artysta ---

bool Artysta::Reaguj(string czynnosc){
    bool a = Osoba::Reaguj(czynnosc);
    if (a == false){
        if (czynnosc == "wychodzi do pracy")
            DoPracy();
        else if (czynnosc == "wychodzi z pracy")
            ZPracy();
        else cout << "BLAD: Artysta.Reaguj::Podano zla nazwe czynnosci!" << czynnosc << endl;           
    }
    return true;    
};

Czas Artysta::LosujCzasDoPracy() {
    Czas b(0, 0, 8, rand() % 241);
    return b;
};

void Artysta::PlanujDzien() {
    kol.Wstaw(new Zdarzenie(kol.KtoraGodzina() + LosujCzasDoPracy(), *this, "wychodzi do pracy"));
    kol.Wstaw(new Zdarzenie(kol.KtoraGodzina().Jutro(), *this, "planuje dzien"));
    if (kol.KtoraGodzina().DajDzien() == 0)
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "rezerwuje teatr"));
};

void Artysta::DoPracy() {
    // Osoba idzie do pracy:
    Podroz(pracaId[kol.DzienTygodnia()]);
    // Nastepnie planuje:
    Czas a(0, 0, 5, rand() % 121);
    a += kol.KtoraGodzina();
    kol.Wstaw(new Zdarzenie(a, *this, "wychodzi z pracy"));
    zajetyDo = a;
};

void Artysta::ZPracy() {
    // Osoba wraca z pracy:
    double p = rand() % 100;
    p /= 100;
    if (kol.KtoraGodzina().DajDzienTygodnia() >= 5 || pZakupy > p)
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "wychodzi na zakupy"));
    else
        Podroz(mieszkanieId);
};

string Artysta::ToString() const {
    return Osoba::ToString() + "Artysta[praca=(...)]; ";
};

Artysta::Artysta() : Osoba() {
    for (int i = 0; i < 7; i++)
        pracaId[i] = -1;
};

Artysta::Artysta(int d) : Osoba("artysta", d, pTeatr) {
    int miejscaPracy[] ={rand() % liczbaPrzystankow, rand() % liczbaPrzystankow, rand() % liczbaPrzystankow};
    for (int i = 0; i < 7; i++)
        pracaId[i] = miejscaPracy[rand() % 3];
};

// --- Implementacja Student ---

bool Student::Reaguj(string czynnosc) {
    bool a = Osoba::Reaguj(czynnosc);
    if (a == false){
        if (czynnosc == "wychodzi na uniwersytet")
            NaUniwersytet();
        else if (czynnosc == "wychodzi z uniwersytetu")
            ZUniwersytetu();
        else cout << "BLAD: Student.Reaguj::Podano zla nazwe czynnosci!" << czynnosc << endl;          
    return true;
    }    
};

void Student::PlanujDzien() {
    if (kol.DzienTygodnia() < 5) {
        if (uniwersytet2Id != -1) {
            Czas a(0, 0, 0, 1);
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina() + czasNaUniwersytet[kol.DzienTygodnia()] +
                    czasZajec[kol.DzienTygodnia()].Srodek() + a, *this, "wychodzi na uniwersytet"));
        }
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina() + czasNaUniwersytet[kol.DzienTygodnia()],
                *this, "wychodzi na uniwersytet"));
    } else
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina() + LosujCzasNaZakupy(), *this, "wychodzi na zakupy"));
    kol.Wstaw(new Zdarzenie(kol.KtoraGodzina().Jutro(), *this, "planuje dzien"));
    if (kol.KtoraGodzina().DajDzien() == 0)
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "rezerwuje teatr"));
};

void Student::NaUniwersytet() {
    if (uniwersytet2Id == -1) {
        // student jedzie na uniwersytet1
        Podroz(uniwersytety[uniwersytet1Id].DajPrzystanekId());
        // i planuje co dalej
        Czas a = kol.KtoraGodzina() + czasZajec[kol.DzienTygodnia()];
        kol.Wstaw(new Zdarzenie(a, *this, "wychodzi z uniwersytetu"));
        zajetyDo = a;
    } else {
        // student jedzie na uniwersytet2
        Podroz(uniwersytety[uniwersytet2Id].DajPrzystanekId());
        // i planuje co dalej
        Czas a = kol.KtoraGodzina() + czasZajec[kol.DzienTygodnia()].Srodek();
        kol.Wstaw(new Zdarzenie(a, *this, "wychodzi z uniwersytetu"));
        zajetyDo = a;
    }
};

void Student::ZUniwersytetu() {
    double p = rand() % 100;
    p /= 100;
    if (pZakupy > p && kol.KtoraGodzina() == kol.KtoraGodzina().Dzisiaj() +
            czasNaUniwersytet[kol.DzienTygodnia()] + czasZajec[kol.DzienTygodnia()]) {
        // student jedzie na zakupy
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "wychodzi na zakupy"));
    } else
        // student wraca do domu
        Podroz(mieszkanieId);
};

string Student::ToString() const {
    return Osoba::ToString() + "Student[uniwersytet1=" + 
            uniwersytety[uniwersytet1Id].DajNazwa() + ", uniwersytet2=" +
            (uniwersytet2Id == -1? "brak" : uniwersytety[uniwersytet2Id].DajNazwa()) + "]; ";
};

Student::Student() : Osoba(), uniwersytet1Id(-1), uniwersytet2Id(-1) {
    Czas a;
    for (int i = 0; i < 5; i++) {
        czasNaUniwersytet[i] = a;
        czasZajec[i] = a;
    }
};

Student::Student(int d, bool drugi) : Osoba("student", d, pTeatr) {
    uniwersytet1Id = rand() % liczbaUniwersytetow;
    uniwersytet2Id = -1;
    if (drugi == true && liczbaUniwersytetow > 1) {
        do // napisac lepsze losowanie?
            uniwersytet2Id = rand() % liczbaUniwersytetow; 
        while
            (uniwersytet2Id == uniwersytet1Id);
    }
    for (int i = 0; i < 5; i++) {
        Czas a(0, 0, 8, rand() % 241);
        Czas b(0, 0, 2, rand() % 481);
        Czas c(0, 0, 20, 0);
        Czas e;
        e = (a + b >= c ? c - a : b);
        czasNaUniwersytet[i] = a;
        czasZajec[i] = e;
    }
};

// --- Implementacja Uczen ---

bool Uczen::Reaguj(string czynnosc) {
    bool a = Osoba::Reaguj(czynnosc);
    if (a == false){
        if (czynnosc == "wychodzi do szkoly")
            DoSzkoly();
        else if (czynnosc == "wychodzi ze szkoly")
            ZeSzkoly();
        else cout << "BLAD: Uczen.Reaguj::Podano zla nazwe czynnosci!" << czynnosc << endl;          
    return true;
    }
};

void Uczen::PlanujDzien() {
    if (kol.DzienTygodnia() < 5) {
        Czas a(0, 0, 8, 0);
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina() + a, *this, "wychodzi do szkoly"));
    }
    kol.Wstaw(new Zdarzenie(kol.KtoraGodzina().Jutro(), *this, "planuje dzien"));
    if (kol.KtoraGodzina().DajDzien() == 0)
        kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), *this, "rezerwuje teatr"));
};

void Uczen::DoSzkoly() {
    // uczen jedzie na uniwersytet1
    Podroz(szkolaId);
    // i planuje co dalej
    Czas a = kol.KtoraGodzina() + czasLekcji[kol.DzienTygodnia()];
    kol.Wstaw(new Zdarzenie(a, *this, "wychodzi ze szkoly"));
    zajetyDo = a;
};

void Uczen::ZeSzkoly() {
    // uczen wraca do domu
    Podroz(mieszkanieId);
};

string Uczen::ToString() const {
    return Osoba::ToString() + "Uczen[szkola=" + przystanki[szkolaId].DajPolozenie() + "]; ";
};

void Uczen::UstawPTeatr(double p) {
    pTeatr = p;
};

Uczen::Uczen() : Osoba() {
    Czas a;
    for (int i = 0; i < 5; i++)
        czasLekcji[i] = a;
};

Uczen::Uczen(int d) : Osoba("uczen", d, pTeatr) {
    szkolaId = rand() % liczbaPrzystankow;
    for (int j = 0; j < 5; j++) {
        Czas a(0, 0, 4, rand() % 301);
        czasLekcji[j] = a;
    }
};

// --- Implementacja Zdarzenie ---

Zdarzenie::Zdarzenie(Czas cz, Obiekt& kt, string czyn)
: czas(cz), kto(kt), czynnosc(czyn), id(idNo) {
    idNo++;
};

void Zdarzenie::Zajdz() const {
        kto.Reaguj(czynnosc);
};

string Zdarzenie::to_stringImpl() const {
    return "Zdarzenie[this=" + ::to_string(this) + ": id=" + ::to_string(id) + ", czas=" + ::to_string(&czas) + ", kto=" + ::to_string(&kto) + ", kol=" + ::to_string(&kol) + "]";
};

ostream& operator<< (ostream& wyjscie, const Zdarzenie& zdarz) {
   wyjscie << "Zdarzenie[id=" << zdarz.DajId() << "]; ";
   return wyjscie;
};

//ostream& operator<<(ostream& o, const Zdarzenie& z) {
//    o << z.to_stringImpl();
//    return o;
//}

// --- Implementacja KolejkaZdarzen ---

KolejkaZdarzen::KolejkaZdarzen() : zdarzenia() {
    Czas a;
    czas = a;
};

void KolejkaZdarzen::Wstaw(const Zdarzenie* z) {
    assert(czas <= z->DajCzas());
    zdarzenia.push(z);
};

void KolejkaZdarzen::NiechSieDzieje(Czas koniecSwiata) {
    while (!zdarzenia.empty() and czas <= koniecSwiata) {
        const Zdarzenie* z = zdarzenia.top();
        czas = z->DajCzas();
        if (((czas >= z->DajKto().DajZajetyDo()) && (z->DajKto().DajBilet() == nullptr))
                || z->DajCzynnosc() == "ustawia sie na przystanku" || z->DajCzynnosc() == "wsiada do tramwaju"
                || z->DajCzynnosc() == "wysiada z tramwaju" || z->DajCzynnosc() == "schodzi z przystanku") {
            cout << czas << *z << z->DajKto().ToStringSimple() << z->DajCzynnosc() << endl;            
            z->Zajdz();
        }
//                cout << *z << endl; // Ta linijka może się przydać przy debugowaniu
        zdarzenia.pop(); // Dla obiektów (tu są wskaźniki, więc ich to nie dotyczy) woła destruktor elementu usuwanego z kolejki
        delete z;
    }
    // czyszczenie kolejki - żeby nie zostawiać w kolejce śmieci
    while (!zdarzenia.empty()) {
        const Zdarzenie* z = zdarzenia.top();
        zdarzenia.pop();
        delete z;
    }
};

// --- Funkcje pomocnicze ---

int SzukajPrzystanku(string naz, Przystanek przyst[], int liczbaPrzyst) {
    // wyszukuje Id przystanku o podanej nazwie
    for (int i = 0; i < liczbaPrzyst; i++) {
        if (przyst[i].DajPolozenie() == naz)
            return i;
    };
    cout << "BLAD: Funkcja SzukajPrzystanku nie znalazla przystanku o nazwie " << naz << "!";
    return -1;
};

// ---------------- Program główny -----------------------

int Obiekt::liczbaPrzystankow;
int Obiekt::liczbaLiniiTramwajowych;
int Obiekt::liczbaUniwersytetow;
int Obiekt::liczbaTeatrow;
int Obiekt::liczbaCentrowHandlowych;
Przystanek* Obiekt::przystanki;
LiniaTramwajowa* Obiekt::linieTramwajowe;
Uniwersytet* Obiekt::uniwersytety;
Teatr* Obiekt::teatry;
CentrumHandlowe* Obiekt::centraHandlowe;

double LiniaTramwajowa::procentZmniejszenia;
double Pracujacy::pWyjazd;
double Osoba::pZakupy;
double Pracujacy::pTeatr;
double Artysta::pTeatr;
double Student::pTeatr;
double Uczen::pTeatr;
int Tramwaj::pojemnosc;

Statystyki* Obiekt::statystyki;

int main(int, char**) {
    srand(time(nullptr)); // Inicjuje gen. liczb losowych - przy szukaniu błędów lepiej tę instrukcję wykomentować (za każdym razem będzie wtedy ten sam ciąg liczb losowych)
    cout << "--- START PROGRAMU ---" << endl;
    
    // --- Wczytanie danych ---

    cout << "--- DANE SYMULACJI ---" << endl;

    fstream plik;
    plik.open ("in.txt", ios::in);
    if (plik.good() == false) {
        cout << "BLAD: Nie udalo sie otworzyc pliku 'in.txt'!";
    } else {
        int dniSymulacji;
            plik >> dniSymulacji;
            cout << "dni symulacji = " << dniSymulacji << endl;
            Czas koniecSymulacji(0, dniSymulacji, 0, 0);
        int liczbaPracujacych;
            plik >> liczbaPracujacych;
            Pracujacy pracujacy[liczbaPracujacych];
        int liczbaStudentow;
            plik >> liczbaStudentow;
            Student student[liczbaStudentow];
        int liczbaArtystow;
            plik >> liczbaArtystow;
            Artysta artysta[liczbaArtystow];
        int liczbaUczniow;
            plik >> liczbaUczniow;
            Uczen uczen[liczbaUczniow];
        double pracPWyjazd;
            plik >> pracPWyjazd;
            cout << "prawdop. wyjazdu sluzbowego dla pracujacych = " << pracPWyjazd << endl;
            Pracujacy::UstawPWyjazd(pracPWyjazd);
        double osobaPZakupy;
            plik >> osobaPZakupy;
            cout << "prawdop. zakupow po pracy/uczelni dla osoby = " << osobaPZakupy << endl;
            Osoba::UstawPZakupy(osobaPZakupy);
        int procentDwukierunkowych;
            plik >> procentDwukierunkowych;
            cout << "procent studentow z dwoma miejscami studiow = " << procentDwukierunkowych << endl;
            int liczbaDwukierunkowych = floor(procentDwukierunkowych/100*liczbaStudentow);
        int procentZmniejszenia;
            plik >> procentZmniejszenia;
            cout << "procent zmniejszenia liczby tramwajow w weekendy = " << procentZmniejszenia << endl;
            LiniaTramwajowa::UstawProcentZmniejszenia(procentZmniejszenia);
        double pracPTeatr;
            plik >> pracPTeatr;
            cout << "prawdop. pojscia do teatru dla pracujacych = " << pracPTeatr << endl;
            Pracujacy::UstawPTeatr(pracPTeatr);
        double studPTeatr;
            plik >> studPTeatr;
            cout << "prawdop. pojscia do teatru dla studentow = " << studPTeatr << endl;
            Student::UstawPTeatr(studPTeatr);
        double artPTeatr;
            plik >> artPTeatr;
            cout << "prawdop. pojscia do teatru dla artystow = " << artPTeatr << endl;
            Artysta::UstawPTeatr(artPTeatr);
        double uczPTeatr;
            plik >> uczPTeatr;
            cout << "prawdop. pojscia do teatru dla uczniow = " << uczPTeatr << endl;
            Uczen::UstawPTeatr(uczPTeatr);
        int pojemnosc;
            plik >> pojemnosc;
            cout << "pojemnosc tramwajow = " << pojemnosc << endl;
            Tramwaj::UstawPojemnosc(45);

        // --- Stworzenie miejsc ---

        int liczbaPrzystankow;
            plik >> liczbaPrzystankow;
            cout << "liczba przystankow = " << liczbaPrzystankow << endl;
            Obiekt::UstawLiczbaPrzystankow(liczbaPrzystankow);
        //Przystanek* przystanki = new Przystanek[liczbaPrzystankow];
        Przystanek przystanki[liczbaPrzystankow];
            Obiekt::UstawPrzystanki(przystanki);
            for (int i=0; i<liczbaPrzystankow; i++) {
                string naz;
                plik >> naz;
                przystanki[i] = Przystanek(i, naz);
                cout << przystanki[i] << endl;
            }
        int liczbaLiniiTramwajowych;
            plik >> liczbaLiniiTramwajowych;
            cout << "liczba linii tramwajowych = " << liczbaLiniiTramwajowych << endl;
            Obiekt::UstawLiczbaLiniiTramwajowych(liczbaLiniiTramwajowych);
            string wiersz;
            getline(plik, wiersz); // przejscie do nastepnego wiersza
        //LiniaTramwajowa* linieTramwajowe = new LiniaTramwajowa[liczbaLiniiTramwajowych];
        LiniaTramwajowa linieTramwajowe[liczbaLiniiTramwajowych];
            Obiekt::UstawLinieTramwajowe(linieTramwajowe);
            for (int i=0; i<liczbaLiniiTramwajowych; i++) {
                getline(plik, wiersz);
                stringstream lineStream(wiersz);
                int liczbaTramwajow;
                    lineStream >> liczbaTramwajow;
                    linieTramwajowe[i] = LiniaTramwajowa(i, liczbaTramwajow);
                string naz;
                int czas;
                while (lineStream >> naz >> czas)
                    linieTramwajowe[i].DodajPrzystanek(SzukajPrzystanku(naz, przystanki, liczbaPrzystankow), czas);
                cout << linieTramwajowe[i] << endl;
            }
        int liczbaUniwersytetow;
            plik >> liczbaUniwersytetow;
            cout << "liczba uniwersytetow = " << liczbaUniwersytetow << endl;
            Obiekt::UstawLiczbaUniwersytetow(liczbaUniwersytetow);
        Uniwersytet uniwersytety[liczbaUniwersytetow];
            Obiekt::UstawUniwersytety(uniwersytety);
            for (int i=0; i<liczbaUniwersytetow; i++) {
                string naz, przyst;
                plik >> naz >> przyst;
                uniwersytety[i] = Uniwersytet(i, naz, 
                        SzukajPrzystanku(przyst, przystanki, liczbaPrzystankow));
                cout << uniwersytety[i] << endl;
            }
        int liczbaTeatrow;
            plik >> liczbaTeatrow;
            cout << "liczba teatrow = " << liczbaTeatrow << endl;
            Obiekt::UstawLiczbaTeatrow(liczbaTeatrow);
        Teatr teatry[liczbaTeatrow];
            Obiekt::UstawTeatry(teatry); 
            for (int i=0; i<liczbaTeatrow; i++) {
                string naz, przyst;
                int poj;
                plik >> naz >> przyst >> poj;
                teatry[i] = Teatr(i, naz, 
                        SzukajPrzystanku(przyst, przystanki, liczbaPrzystankow), poj);
                cout << teatry[i] << endl;
            }
        int liczbaCentrowHandlowych;
            plik >> liczbaCentrowHandlowych;
            cout << "liczba centrow handlowych = " << liczbaCentrowHandlowych << endl;
            Obiekt::UstawLiczbaCentrowHandlowych(liczbaCentrowHandlowych);
        CentrumHandlowe centraHandlowe[liczbaCentrowHandlowych];
            Obiekt::UstawCentraHandlowe(centraHandlowe);
            for (int i=0; i<liczbaCentrowHandlowych; i++) {
                string naz, przyst;
                plik >> naz >> przyst;
                centraHandlowe[i] = CentrumHandlowe(i, naz,
                        SzukajPrzystanku(przyst, przystanki, liczbaPrzystankow));
                cout << centraHandlowe[i] << endl;
            }

        // ----------------- Stworzenie osob ---------------

        cout << "liczba pracujacych = " << liczbaPracujacych << endl;
        for (int i = 0; i < liczbaPracujacych; i++){
            pracujacy[i] = Pracujacy(i);
            cout << pracujacy[i] << endl;
        }
        cout << "liczba artystow = " << liczbaArtystow << endl;
        for (int i = 0; i < liczbaArtystow; i++){
            artysta[i] = Artysta(i);
            cout << artysta[i] << endl;
        }
        cout << "liczba studentow = " << liczbaStudentow << endl;
        for(int i = 0; i < liczbaDwukierunkowych; i++){
            student[i] = Student(i, true);
            cout << student[i] << endl;
        }
        for(int i = liczbaDwukierunkowych; i < liczbaStudentow; i++){            
            student[i] = Student(i, false);
            cout << student[i] << endl;
        }
        cout << "liczba uczniow = " << liczbaUczniow << endl;
        for (int i = 0; i < liczbaUczniow; i++){
            uczen[i] = Uczen(i);
            cout << uczen[i] << endl;
        }
        
        Statystyki stat;
        Obiekt::UstawStatystyki(&stat);
        
        plik.close(); 
        
        // ---------- Inicjalizacja zdarzen ------------

        for (int i = 0; i < liczbaPrzystankow; i++)
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), przystanki[i], "planuje dzien"));    
        for (int i = 0; i < liczbaLiniiTramwajowych; i++)
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), linieTramwajowe[i], "planuje dzien"));
        for (int i = 0; i < liczbaPracujacych; i++)
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), pracujacy[i], "planuje dzien"));
        for (int i = 0; i < liczbaArtystow; i++)
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), artysta[i], "planuje dzien"));
        for (int i = 0; i < liczbaStudentow; i++)
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), student[i], "planuje dzien"));
        for (int i = 0; i < liczbaUczniow; i++)
            kol.Wstaw(new Zdarzenie(kol.KtoraGodzina(), uczen[i], "planuje dzien"));

        // ---------------- Symulacja ------------------

        cout << "--- POCZATEK SYMULACJI ---" << endl;
        kol.NiechSieDzieje(koniecSymulacji);
        cout << "--- STATYSTYKI ---" << endl;
        cout << stat.ToString();
        cout << "--- KONIEC PROGRAMU ---" << endl;        
    }
}

// Dodatkowe zalozenia:
// - zawsze istnieje linia tramwajowa, ktora osoba dotrze bezposrednio do celu
//   (niestety, nie udalo mi sie wprowadzic ogolnej funkcji szukajacej trasy)
// - artysci zaczynaja prace o 8-12 i koncza po 5-7 h
// - zakupy "wracajac z pracy" tycza sie pracujacych, artystow i studentow
// - artysci robia zawsze weekendowe zakupy tuż po pracy
// - jesli jest tylko jedna uczelnia, to wszyscy studenci maja jeden uniwersytet
// - tramwaje o 23 koncza prace i juz sie nie zatrzymuja
// - funkcja szukajaca trasy zwraca nry linii, przystanki wejscia i kier jazdy
// - pracujacy nie wyjezdzaja o losowej porze lecz zaczynaja prace o losowej
// - osoba wybierajac sie do teatru wychodzi godzine wczesniej
// - jesli osoba nie wyjedzie do teatru to nie jest liczona jako osobospektakl
