#include "icb_gui.h"
#include "ic_media.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <windows.h> // Balık hareketi ve kuş için threading

// Zıplama durumu ve ayarları
bool ziplamaAktif = false;
const int ziplamaYuksekligi = 15; // Zıplama yüksekliği
const int ziplamaHizi = 20;       // Her karede yukarı hareket mesafesi

// Merdiven koordinatları
const int merdivenSolX = 126, merdivenSagX = 160;
const int merdivenUstY = 200, merdivenAltY = 400;

// Karakterin başlangıç pozisyonu
int karakterX = 66, karakterY = 280;
const int hareketMesafesi = 10; // Hareket mesafesi (piksel)

int MouseLogBox; // Fare hareketlerini göstermek için metin kutusu

// Arka plan için değişkenler
int arkaplanPosX = 0; // Arka planın başlangıç pozisyonu
const int pencereGenislik = 800, pencereYukseklik = 600;
int arkaplanGenislik = 1400, arkaplanYukseklik = 600; // Arka plan boyutları
ICBYTES arkaplanilk, arkaplandevam; // İki arka plan resmi

// Karakter animasyonu için değişkenler
ICBYTES karakter; // Karakter sprite dosyası
int animasyonKare = 0; // Şu anki animasyon karesi
int karakterKoordinatlar[3][4] = { {8, 9, 40, 60}, {58, 10, 40, 60}, {110, 10, 40, 60} }; // X, Y, Genişlik, Yükseklik

// Kuş animasyonu için değişkenler
ICBYTES kus; // Kuş sprite dosyası
int kusX = 312, kusY = 165; // Kuşun başlangıç koordinatları
int kusYon = -1; // -1: sağdan sola, 1: soldan sağa
int kusAnimasyonKare = 0; // Kuşun animasyon karesi
int kusKoordinatlar[4][4] = {
    {0, 2, 14, 11},
    {12, 2, 17, 14},
    {32, 3, 15, 15},
    {49, 2, 23, 13}
};

// Balık animasyonu için değişkenler
ICBYTES fish; // Balık sprite dosyası
int balikX = 546, balikY = 432;
int balikYon = -1; // -1: Sağdan sola, 1: Soldan sağa
const int balikHareketMesafesi = 5; // Balık hareket mesafesi
int balikKoordinatlar[2][4] = { {35, 31, 31, 27}, {68, 26, 31, 33} }; // X, Y, Genişlik, Yükseklik

// Taşlar (küçük platformlar) için koordinatlar
struct Tas {
    int x, y, genislik, yukseklik;
};

// Taşların orijinal koordinatları (taşlar kaydırmaya göre güncellenmeli)
Tas taslar[] = {
    {134, 338, 28, 20}, {174, 352, 28, 20}, {230, 295, 28, 20},
    {264, 306, 28, 20}, {230, 349, 28, 20}, {265, 266, 28, 20},
    {302, 268, 28, 20}
};

const int tasSayisi = sizeof(taslar) / sizeof(taslar[0]);

int anaPencere;
std::atomic<bool> calisiyor(true); // Thread kontrolü için

const int suSeviyesi = 400; // Örnek su seviyesi

// Karakterin merdiven üzerinde olup olmadığını kontrol eden fonksiyon
bool merdivendeMi(int x, int y) {
    int karakterMerkezX = x + karakterKoordinatlar[animasyonKare][2] / 2;
    int karakterMerkezY = y + karakterKoordinatlar[animasyonKare][3] / 2;

    return (karakterMerkezX >= 126 && karakterMerkezX <= 143 &&
        karakterMerkezY >= 200 && karakterMerkezY <= 390);
}


bool sudaMi() {
    return karakterY > suSeviyesi;
}

bool tasUzerindeMi() {
    int karakterMerkezX = karakterX + karakterKoordinatlar[animasyonKare][2] / 2;
    int karakterAltY = karakterY + karakterKoordinatlar[animasyonKare][3];

    for (int i = 0; i < tasSayisi; i++) {
        int tasGuncelX = (taslar[i].x - arkaplanPosX) % arkaplanGenislik; // Taşları arka plan kaymasına göre düzenle
        int tasUstY = taslar[i].y; // Y konumu değişmeyeceği için sabit kalıyor

        bool xUygun = (karakterMerkezX >= tasGuncelX && karakterMerkezX <= tasGuncelX + taslar[i].genislik);
        bool yUygun = (karakterAltY >= tasUstY - 5 && karakterAltY <= tasUstY + 5);

        if (xUygun && yUygun) {
            return true;
        }
    }
    return false;
}


void YerCekimi() {
    if (!merdivendeMi(karakterX, karakterY) && !ziplamaAktif) {
        if (!tasUzerindeMi()) { // Eğer taşın üstünde değilse düşmeye devam et
            karakterY += hareketMesafesi;
            if (karakterY > pencereYukseklik - karakterKoordinatlar[animasyonKare][3]) {
                karakterY = pencereYukseklik - karakterKoordinatlar[animasyonKare][3];
            }
        }
    }
}

void DrawRectangle(ICBYTES& ekran, int x, int y, int genislik, int yukseklik, int r, int g, int b) {
    for (int i = 0; i < yukseklik; i++) {
        for (int j = 0; j < genislik; j++) {
            if (x + j >= 0 && x + j < pencereGenislik && y + i >= 0 && y + i < pencereYukseklik) {
                ekran.C(x + j, y + i, 0) = r;
                ekran.C(x + j, y + i, 1) = g;
                ekran.C(x + j, y + i, 2) = b;
            }
        }
    }
}


// Karakteri ekrana çizen fonksiyon
void KarakterCiz(ICBYTES& ekran) {
    int kareX = karakterKoordinatlar[animasyonKare][0];
    int kareY = karakterKoordinatlar[animasyonKare][1];
    int kareGenislik = karakterKoordinatlar[animasyonKare][2];
    int kareYukseklik = karakterKoordinatlar[animasyonKare][3];

    for (int y = 0; y < kareYukseklik; y++) {
        for (int x = 0; x < kareGenislik; x++) {
            ekran.C(karakterX + x, karakterY + y, 0) = karakter.C(kareX + x, kareY + y, 0);
            ekran.C(karakterX + x, karakterY + y, 1) = karakter.C(kareX + x, kareY + y, 1);
            ekran.C(karakterX + x, karakterY + y, 2) = karakter.C(kareX + x, kareY + y, 2);
        }
    }
}

// Kuşu ekrana çizen fonksiyon
void KusCiz(ICBYTES& ekran) {
    int kareX = kusKoordinatlar[kusAnimasyonKare][0];
    int kareY = kusKoordinatlar[kusAnimasyonKare][1];
    int kareGenislik = kusKoordinatlar[kusAnimasyonKare][2];
    int kareYukseklik = kusKoordinatlar[kusAnimasyonKare][3];

    for (int y = 0; y < kareYukseklik; y++) {
        for (int x = 0; x < kareGenislik; x++) {
            unsigned char alpha = kus.C(kareX + x, kareY + y, 3); // Alfa kanalı
            if (alpha > 0) { // Sadece transparan olmayan pikselleri çiz
                ekran.C(kusX + x, kusY + y, 0) = kus.C(kareX + x, kareY + y, 0);
                ekran.C(kusX + x, kusY + y, 1) = kus.C(kareX + x, kusY + y, 1);
                ekran.C(kusX + x, kusY + y, 2) = kus.C(kareX + x, kusY + y, 2);
            }
        }
    }
}

// Balığı ekrana çizen fonksiyon
void BalikCiz(ICBYTES& ekran) {
    int kareX = balikKoordinatlar[balikYon == -1 ? 0 : 1][0];
    int kareY = balikKoordinatlar[balikYon == -1 ? 0 : 1][1];
    int kareGenislik = balikKoordinatlar[balikYon == -1 ? 0 : 1][2];
    int kareYukseklik = balikKoordinatlar[balikYon == -1 ? 0 : 1][3];

    for (int y = 0; y < kareYukseklik; y++) {
        for (int x = 0; x < kareGenislik; x++) {
            ekran.C(balikX + x, balikY + y, 0) = fish.C(kareX + x, kareY + y, 0);
            ekran.C(balikX + x, balikY + y, 1) = fish.C(kareX + x, kareY + y, 1);
            ekran.C(balikX + x, balikY + y, 2) = fish.C(kareX + x, kareY + y, 2);
        }
    }
}


// Arka planı ve karakteri çizen fonksiyon
void ekraniCiz() {
    ICBYTES ekran;
    CreateMatrix(ekran, pencereGenislik, pencereYukseklik, 3, ICB_UCHAR);

    // Ekranı siyah (0, 0, 0) ile doldur
    for (int y = 0; y < pencereYukseklik; y++) {
        for (int x = 0; x < pencereGenislik; x++) {
            ekran.C(x, y, 0) = -1; // Kırmızı kanal
            ekran.C(x, y, 1) = 0; // Yeşil kanal
            ekran.C(x, y, 2) = 0; // Mavi kanal
            ekran.C(x, y, 3) = 0x00;
        }
    }

    // Arka planı çiz
    for (int y = 0; y < pencereYukseklik; y++) {
        for (int x = 0; x < pencereGenislik; x++) {
            int globalX = (arkaplanPosX + x) % arkaplanGenislik;
            if (globalX < arkaplanGenislik / 2) {
                ekran.C(x, y, 0) = arkaplanilk.C(globalX, y % arkaplanYukseklik, 0);
                ekran.C(x, y, 1) = arkaplanilk.C(globalX, y % arkaplanYukseklik, 1);
                ekran.C(x, y, 2) = arkaplanilk.C(globalX, y % arkaplanYukseklik, 2);
            }
            else {
                ekran.C(x, y, 0) = arkaplandevam.C(globalX - arkaplanGenislik / 2, y % arkaplanYukseklik, 0);
                ekran.C(x, y, 1) = arkaplandevam.C(globalX - arkaplanGenislik / 2, y % arkaplanYukseklik, 1);
                ekran.C(x, y, 2) = arkaplandevam.C(globalX - arkaplanGenislik / 2, y % arkaplanYukseklik, 2);
            }
        }
    }

    // Taşları çiz (arka plan kaymasını hesaba kat)
    for (int i = 0; i < tasSayisi; i++) {
        int tasX = (taslar[i].x - arkaplanPosX) % arkaplanGenislik;  // Global konum hesaplaması
        int tasY = taslar[i].y;
        DrawRectangle(ekran, tasX, tasY, taslar[i].genislik, taslar[i].yukseklik, 255, 255, 255);
    }

    KarakterCiz(ekran);
    DisplayImage(anaPencere, ekran);
}



// Kuş hareketini güncelleyen fonksiyon
/*/DWORD WINAPI KusHareket(LPVOID lpParam) {
    while (calisiyor) {
        kusX += kusYon * 5; // Kuşun hareket mesafesi
        if (kusYon == -1 && kusX <= 111) { // Sağdan sola giderken sınıra ulaştıysa
            kusYon = 1; // Yön değiştir
            kusAnimasyonKare = 2; // Soldan sağa animasyon başlasın
        }
        if (kusYon == 1 && kusX >= 312) { // Soldan sağa giderken sınıra ulaştıysa
            kusYon = -1; // Yön değiştir
            kusAnimasyonKare = 0; // Sağdan sola animasyon başlasın
        }

        ekraniCiz(); // Ekranı güncelle

        Sleep(100); // Kuşun hareket hızı
    }
    return 0;
}*/

/*DWORD WINAPI BalikHareket(LPVOID lpParam) {
    int hareketMesafesiSayaci = 0; // Balığın kaç piksel hareket ettiğini saymak için bir sayaç

    while (true) {
        // Balığın mevcut yönününe göre X pozisyonunu güncelle
        balikX += balikYon * balikHareketMesafesi;
        hareketMesafesiSayaci += balikHareketMesafesi;

        // Belirli bir mesafeden sonra yön değiştir
        if (hareketMesafesiSayaci >= 350) { // Örneğin 200 piksel gittikten sonra
            balikYon *= -1; // Yön değişimi
            hareketMesafesiSayaci = 0; // Sayaç sıfırlanır
        }

        // Sınır kontrolü
        if (balikX < 0 || balikX + balikKoordinatlar[0][2] > pencereGenislik) {
            balikYon *= -1; // Yön değiştir

        }

        // Ekran sınırlarına geldiğinde yön değiştir
        if (balikX < 0 || balikX + balikKoordinatlar[0][2] > pencereGenislik) {
            balikYon *= -1; // Yön değişimi
            hareketMesafesiSayaci = 0; // Sayaç sıfırlanır
        }

        ekraniCiz(); // Ekranı sürekli yeniden çiz

        // Balığın hareket hızını ayarlamak için gecikme süresi
        Sleep(50);
    }
    return 0;
}*/

// Klavye girdisini işleyen fonksiyon
void klavyeGirdisi(int tus) {
    if (ziplamaAktif) return;

    bool hareketEtti = false;
    int ekranMerkezi = pencereGenislik / 2;

    if (merdivendeMi(karakterX, karakterY)) {
        switch (tus) {
        case 38: // Yukarı ok (Merdivende yukarı çıkma)
            if (karakterY > merdivenUstY) {
                karakterY -= hareketMesafesi;
                hareketEtti = true;
            }
            break;
        case 40: // Aşağı ok (Merdivende aşağı inme)
            if (karakterY < merdivenAltY) {
                karakterY += hareketMesafesi;
                hareketEtti = true;
            }
            break;
        }
    }
    else {
        switch (tus) {
        case 37: // Sol ok (Sol hareket)
            if (karakterX > 100) {
                if (arkaplanPosX > 0) {
                    arkaplanPosX -= hareketMesafesi;
                }
                else {
                    karakterX -= hareketMesafesi;
                }
                hareketEtti = true;
            }
            break;
        case 39: // Sağ ok (Sağ hareket)
            if (karakterX < ekranMerkezi) {
                karakterX += hareketMesafesi;
            }
            else if (arkaplanPosX + pencereGenislik < arkaplanGenislik) {
                arkaplanPosX += hareketMesafesi;
            }
            else {
                karakterX += hareketMesafesi;
            }
            hareketEtti = true;
            break;
        case 32: // Space (Zıplama)
            if (!merdivendeMi(karakterX, karakterY)) {
                ziplamaAktif = true;
                std::thread([]() {
                    int baslangicY = karakterY;

                    // Yukarı çıkış
                    while (karakterY > baslangicY - ziplamaYuksekligi) {
                        karakterY -= ziplamaHizi;
                        ekraniCiz();
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));

                        // Eğer karakter taşın üstüne çıkarsa zıplamayı durdur
                        if (tasUzerindeMi()) {
                            ziplamaAktif = false;
                            return;
                        }
                    }

                    // Aşağı iniş (Yerçekimi etkisi)
                    while (!tasUzerindeMi() && karakterY < baslangicY) {
                        karakterY += ziplamaHizi;
                        ekraniCiz();
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    ziplamaAktif = false;
                    }).detach();
            }
            break;

        }
    }

    if (hareketEtti) {
        animasyonKare = (animasyonKare + 1) % 3;
        ICG_printf(MouseLogBox, "Karakter Konum: X=%d, Y=%d, ArkaplanX=%d\n", karakterX, karakterY, arkaplanPosX);
        ekraniCiz();
    }
}
void ICGUI_Create() {
    ICG_MWSize(pencereGenislik, pencereYukseklik);
    ICG_MWTitle("Animasyonlu Karakter");
    ICG_MWColor(0, 0, 0);
}

// ICGUI ana fonksiyonu
void ICGUI_main() {
    anaPencere = ICG_FramePanel(0, 0, pencereGenislik, pencereYukseklik);

    // Görselleri yükle
    if (!ReadImage("arkaplanilk.bmp", arkaplanilk)) {
        MessageBox(NULL, "Arkaplanilk resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }
    if (!ReadImage("arkaplandevamdene.bmp", arkaplandevam)) {
        MessageBox(NULL, "Arkaplandevam resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }
    if (!ReadImage("karakter.bmp", karakter)) {
        MessageBox(NULL, "Karakter resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }
    if (!ReadImage("kus.bmp", kus)) {
        MessageBox(NULL, "Kuş resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }
    if (!ReadImage("fish.bmp", fish)) {
        MessageBox(NULL, "Balık resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }

    ekraniCiz(); // İlk ekran çizimi

    // Threadleri başlat
   /* HANDLE threadKus = CreateThread(NULL, 0, KusHareket, NULL, 0, NULL);
    if (!threadKus) {
        MessageBox(NULL, "Kuş hareket thread'i başlatılamadı.", "Hata", MB_OK);
        return;
    }

    HANDLE threadBalik = CreateThread(NULL, 0, BalikHareket, NULL, 0, NULL);
    if (!threadBalik) {
        MessageBox(NULL, "Balık hareket thread'i başlatılamadı.", "Hata", MB_OK);
        return;
    }*/

    // Klavye girdisini bağla
    ICG_SetOnKeyPressed(klavyeGirdisi);

    MouseLogBox = ICG_MLEditSunken(10, 700, 600, 80, "", SCROLLBAR_V);
}