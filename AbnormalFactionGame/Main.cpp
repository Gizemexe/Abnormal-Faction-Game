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
const int merdivenX1 = 91, merdivenX2 = 282, merdivenY1 = 25, merdivenY2 = 74;

// Karakterin merdiven üzerinde olup olmadığını kontrol eden fonksiyon
bool merdivendeMi(int x, int y) {
    return x >= merdivenX1 && x <= merdivenX2 && y >= merdivenY1 && y <= merdivenY2;
}


// Karakterin başlangıç pozisyonu
int karakterX = 66, karakterY = 280;
const int hareketMesafesi = 10; // Hareket mesafesi (piksel)

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

int anaPencere;
std::atomic<bool> calisiyor(true); // Thread kontrolü için

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
        }
    }

    // Arkaplanı çiz
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

    KarakterCiz(ekran);
    KusCiz(ekran);
    BalikCiz(ekran);
    DisplayImage(anaPencere, ekran);
}

// Kuş hareketini güncelleyen fonksiyon
DWORD WINAPI KusHareket(LPVOID lpParam) {
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
}

DWORD WINAPI BalikHareket(LPVOID lpParam) {
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
}

// Klavye girdisini işleyen fonksiyon
// Klavye girdisini işleyen fonksiyon
void klavyeGirdisi(int tus) {
    if (ziplamaAktif) return; // Zıplama sırasında başka hareket engellenir

    bool hareketEtti = false;

    switch (tus) {
    case 37: // Sol ok
        if (karakterX > 0) {
            karakterX -= hareketMesafesi;
            hareketEtti = true;
        }
        break;
    case 39: // Sağ ok
        if (karakterX < pencereGenislik - karakterKoordinatlar[animasyonKare][2]) {
            karakterX += hareketMesafesi;
            hareketEtti = true;
        }
        break;
    case 38: // Yukarı ok
        if (merdivendeMi(karakterX, karakterY)) { // Merdivende yukarı çıkma kontrolü
            karakterY -= hareketMesafesi;
            hareketEtti = true;
        }
        break;
    case 40: // Aşağı ok
        if (merdivendeMi(karakterX, karakterY)) { // Merdivende aşağı inme kontrolü
            karakterY += hareketMesafesi;
            hareketEtti = true;
        }
        break;
    case 32: // Space (zıplama)
        ziplamaAktif = true;
        std::thread([]() {
            int baslangicY = karakterY;
            while (karakterY > baslangicY - ziplamaYuksekligi) { // Yukarı zıplama
                karakterY -= ziplamaHizi;
                ekraniCiz();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            while (karakterY < baslangicY) { // Aşağı düşme
                karakterY += ziplamaHizi;
                ekraniCiz();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            ziplamaAktif = false; // Zıplama biter
            }).detach();
            break;
    }

    // Hareket gerçekleştiyse animasyon karesini ilerlet
    if (hareketEtti) {
        animasyonKare = (animasyonKare + 1) % 3;
    }

    // Ekranı güncelle
    ekraniCiz();
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
    HANDLE threadKus = CreateThread(NULL, 0, KusHareket, NULL, 0, NULL);
    if (!threadKus) {
        MessageBox(NULL, "Kuş hareket thread'i başlatılamadı.", "Hata", MB_OK);
        return;
    }

    HANDLE threadBalik = CreateThread(NULL, 0, BalikHareket, NULL, 0, NULL);
    if (!threadBalik) {
        MessageBox(NULL, "Balık hareket thread'i başlatılamadı.", "Hata", MB_OK);
        return;
    }

    // Klavye girdisini bağla
    ICG_SetOnKeyPressed(klavyeGirdisi);

}