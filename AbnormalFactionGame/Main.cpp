#include "icb_gui.h"
#include "ic_media.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <windows.h> // Balık hareketi ve kuş için threading

// Karakterin başlangıç pozisyonu
int karakterX = 66, karakterY = 280;
const int hareketMesafesi = 10; // Hareket mesafesi (piksel)
ICBYTES gold;
const int goldX = 529, goldY = 473;

// Arka plan için değişkenler
int arkaplanPosX = 0; // Arka planın başlangıç pozisyonu
const int pencereGenislik = 800, pencereYukseklik = 600;
int arkaplanGenislik = 1400, arkaplanYukseklik = 600; // Arka plan boyutları
ICBYTES arkaplanilk, arkaplandevam; // İki arka plan resmi

// Karakter animasyonu için değişkenler
ICBYTES karakter; // Karakter sprite dosyası
int animasyonKare = 0; // Şu anki animasyon karesi
int karakterKoordinatlar[3][4] = { {8, 9, 40, 60}, {58, 10, 40, 60}, {110, 10, 40, 60} }; // X, Y, Genişlik, Yükseklik

// Zıplama durumu ve ayarları
bool ziplamaAktif = false;
const int ziplamaYuksekligi = 15; // Zıplama yüksekliği
const int ziplamaHizi = 20;       // Her karede yukarı hareket mesafesi

// Merdiven koordinatları
const int merdivenSolX = 126, merdivenSagX = 160;
const int merdivenUstY = 200, merdivenAltY = 400;

int MouseLogBox; // Fare hareketlerini göstermek için metin kutusu

ICBYTES yüzme;
int yüzmeKoordinatlar[4][4] = { {8,85,52,30},{66,85,56,33},{130,85,53,31},{187,85,57,32} };
const int yüzmeX = 175, yüzmeY = 414;
int yüzmeAnimasyonKare = 0;
bool yuzmede = false; // Karakter suya girdi mi?
// Yüzme alanı koordinatları {x, y, genişlik, yükseklik}
int yuzmeAlaniX = 175;
int yuzmeAlaniY = 415;
int yuzmeAlaniGenislik = 424;
int yuzmeAlaniYukseklik = 84;

// Kuş animasyonu için değişkenler
ICBYTES kus; // Kuş sprite dosyası
int kusX = 440, kusY = 247; // Kuşun başlangıç koordinatları
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

ICBYTES yarasa; // Yarasa sprite dosyası
int yarasaX = 309, yarasaY = 160; // Yarasanın başlangıç koordinatları
int yarasaYon = -1; // -1: Sağdan sola, 1: Soldan sağa
const int yarasaHareketMesafesi = 5;
int yarasaKoordinatlar[2][4] = { {0, 10, 19, 14}, {22, 9, 16, 9} }; // X, Y, Genişlik, Yükseklik


int anaPencere;
std::atomic<bool> calisiyor(true); // Thread kontrolü için

const int suSeviyesi = 400; // Örnek su seviyesi


// Karakteri ekrana çizen fonksiyon
void KarakterCiz(ICBYTES& ekran) {
    if (yuzmede) return; // Eğer karakter yüzüyorsa, normal çizim yapılmamalı

    int kareX = karakterKoordinatlar[animasyonKare][0];
    int kareY = karakterKoordinatlar[animasyonKare][1];
    int kareGenislik = karakterKoordinatlar[animasyonKare][2];
    int kareYukseklik = karakterKoordinatlar[animasyonKare][3];

    // Önce eski karakteri temizle
    for (int y = 0; y < kareYukseklik; y++) {
        for (int x = 0; x < kareGenislik; x++) {
            ekran.C(karakterX + x, karakterY + y, 0) = arkaplanilk.C(karakterX + x, karakterY + y, 0);
            ekran.C(karakterX + x, karakterY + y, 1) = arkaplanilk.C(karakterX + x, karakterY + y, 1);
            ekran.C(karakterX + x, karakterY + y, 2) = arkaplanilk.C(karakterX + x, karakterY + y, 2);
        }
    }

    // Yeni karakteri çiz
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

// Yarasa ekrana çizen fonksiyon
void YarasaCiz(ICBYTES& ekran) {
    int kareX = yarasaKoordinatlar[yarasaYon == -1 ? 0 : 1][0];
    int kareY = yarasaKoordinatlar[yarasaYon == -1 ? 0 : 1][1];
    int kareGenislik = yarasaKoordinatlar[yarasaYon == -1 ? 0 : 1][2];
    int kareYukseklik = yarasaKoordinatlar[yarasaYon == -1 ? 0 : 1][3];

    for (int y = 0; y < kareYukseklik; y++) {
        for (int x = 0; x < kareGenislik; x++) {
            unsigned char alpha = yarasa.C(kareX + x, kareY + y, 3); // Alfa kanalı kontrolü
            if (alpha > 0) { // Transparan olmayan pikselleri çiz
                ekran.C(yarasaX + x, yarasaY + y, 0) = yarasa.C(kareX + x, kareY + y, 0);
                ekran.C(yarasaX + x, yarasaY + y, 1) = yarasa.C(kareX + x, kareY + y, 1);
                ekran.C(yarasaX + x, yarasaY + y, 2) = yarasa.C(kareX + x, kareY + y, 2);
            }
        }
    }
}

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

bool yuzmeAlaniIcindeMi() {
    return (karakterX >= yuzmeAlaniX && karakterX <= yuzmeAlaniX + yuzmeAlaniGenislik &&
        karakterY >= yuzmeAlaniY && karakterY <= yuzmeAlaniY + yuzmeAlaniYukseklik);
}

void yuzmeModuGuncelle() {
    if (!merdivendeMi(karakterX, karakterY) && karakterY >= 370 && karakterX >= 176) {
        if (!yuzmede) {
            // Karakterin alt kısmı su yüzeyine hizalanmalı
            karakterY = yuzmeAlaniY + 5;
        }
        yuzmede = true;
    }
    else if (karakterY < 370 || karakterX < 176) {
        yuzmede = false;
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

void YuzmeCiz(ICBYTES& ekran) {
    if (!yuzmede) return; // Eğer yüzme modunda değilse, çizme

    // 🎯 **Animasyon karesini sürekli değiştir**
    yüzmeAnimasyonKare = (yüzmeAnimasyonKare + 1) % 4;

    int kareX = yüzmeKoordinatlar[yüzmeAnimasyonKare][0];
    int kareY = yüzmeKoordinatlar[yüzmeAnimasyonKare][1];
    int kareGenislik = yüzmeKoordinatlar[yüzmeAnimasyonKare][2];
    int kareYukseklik = yüzmeKoordinatlar[yüzmeAnimasyonKare][3];

    // 🎯 **Karakterin alt kısmından başlaması için ayarla**
    int cizimY = karakterY - (kareYukseklik / 2);

    for (int y = 0; y < kareYukseklik; y++) {
        for (int x = 0; x < kareGenislik; x++) {
            ekran.C(karakterX + x, cizimY + y, 0) = yüzme.C(kareX + x, kareY + y, 0);
            ekran.C(karakterX + x, cizimY + y, 1) = yüzme.C(kareX + x, kareY + y, 1);
            ekran.C(karakterX + x, cizimY + y, 2) = yüzme.C(kareX + x, kareY + y, 2);
        }
    }
}



void YerCekimi() {
    if (yuzmede) return; // Eğer yüzme modundaysa, yerçekimi uygulanmasın

    if (!merdivendeMi(karakterX, karakterY) && !ziplamaAktif) {
        karakterY += hareketMesafesi;
        if (karakterY > pencereYukseklik - karakterKoordinatlar[animasyonKare][3]) {
            karakterY = pencereYukseklik - karakterKoordinatlar[animasyonKare][3];
        }
    }
}


int GoldEkranX() {
    return goldX - arkaplanPosX;
}

bool goldGorunur = true; // Gold başlangıçta görünüyor

void GoldCiz(ICBYTES& ekran) {
    if (!goldGorunur) return;

    int goldGenislik = 18, goldYukseklik = 20;
    int ekranGoldX = goldX - arkaplanPosX;  // Gold'un dünya koordinatındaki yerini ekranla ilişkilendir

    for (int y = 0; y < goldYukseklik; y++) {
        for (int x = 0; x < goldGenislik; x++) {
            unsigned char alpha = gold.C(x, y, 3);
            if (alpha > 0) {
                ekran.C(ekranGoldX + x, goldY + y, 0) = gold.C(x, y, 0);
                ekran.C(ekranGoldX + x, goldY + y, 1) = gold.C(x, y, 1);
                ekran.C(ekranGoldX + x, goldY + y, 2) = gold.C(x, y, 2);
            }
        }
    }
}

bool GoldCarpistiMi() {
    if (!goldGorunur) return false;

    int goldGenislik = 9, goldYukseklik = 15;
    int karakterGenislik = karakterKoordinatlar[animasyonKare][2];
    int karakterYukseklik = karakterKoordinatlar[animasyonKare][3];

    int ekranGoldX = goldX - arkaplanPosX;  // Gold’un ekrandaki yerini al

    bool carpisti = (karakterX < ekranGoldX + goldGenislik &&
        karakterX + karakterGenislik > ekranGoldX &&
        karakterY < goldY + goldYukseklik &&
        karakterY + karakterYukseklik > goldY);

    if (carpisti) {
        goldGorunur = false;
    }

    return carpisti;
}

void ekraniCiz() {
    ICBYTES ekran;
    CreateMatrix(ekran, pencereGenislik, pencereYukseklik, 3, ICB_UCHAR);

    for (int y = 0; y < pencereYukseklik; y++) {
        for (int x = 0; x < pencereGenislik; x++) {
            int globalX = (arkaplanPosX + x) % arkaplanGenislik;
            ekran.C(x, y, 0) = arkaplanilk.C(globalX, y % arkaplanYukseklik, 0);
            ekran.C(x, y, 1) = arkaplanilk.C(globalX, y % arkaplanYukseklik, 1);
            ekran.C(x, y, 2) = arkaplanilk.C(globalX, y % arkaplanYukseklik, 2);
            ekran.C(x, y, 3) = 0x00;

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


    if (yuzmede) {
        YuzmeCiz(ekran);
    }
    else {
        KarakterCiz(ekran);
    }

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
DWORD WINAPI YarasaHareket(LPVOID lpParam) {
    int hareketMesafesiSayaci = 0;

    while (calisiyor) {
        yarasaX += yarasaYon * yarasaHareketMesafesi;
        hareketMesafesiSayaci += yarasaHareketMesafesi;

        // Yön değiştir
        if (hareketMesafesiSayaci >= 350) {
            yarasaYon *= -1;
            hareketMesafesiSayaci = 0;
        }

        // Sınır kontrolü
        if (yarasaX < 0 || yarasaX + yarasaKoordinatlar[0][2] > pencereGenislik) {
            yarasaYon *= -1;
            hareketMesafesiSayaci = 0;
        }

        ekraniCiz();
        Sleep(50); // Hareket hızı
    }
    return 0;
}

DWORD WINAPI BalikHareket(LPVOID lpParam) {
    int hareketMesafesiSayaci = 0; // Balığın kaç piksel hareket ettiğini saymak için bir sayaç

    while (calisiyor) {
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
void klavyeGirdisi(int tus) {
    bool hareketEtti = false;

    yuzmeModuGuncelle(); // Hareketten önce yüzme modunu güncelle

    if (merdivendeMi(karakterX, karakterY)) {
        switch (tus) {
        case 38: // Yukarı
            if (karakterY > merdivenUstY) {
                karakterY -= hareketMesafesi;
                hareketEtti = true;
            }
            break;
        case 40: // Aşağı
            if (karakterY < merdivenAltY) {
                karakterY += hareketMesafesi;
                hareketEtti = true;
            }
            break;
        }
    }
    else if (yuzmede) {
        // 📌 Yüzme alanı içinde serbestçe hareket edebilmesi için düzeltildi
        switch (tus) {
        case 37: // Sol
            if (karakterX > yuzmeAlaniX) { // Sol sınıra çarpmaması için kontrol eklendi
                karakterX -= hareketMesafesi;
                hareketEtti = true;
            }
            break;
        case 39: // Sağ
            if (karakterX < yuzmeAlaniX + yuzmeAlaniGenislik - karakterKoordinatlar[animasyonKare][2]) {
                karakterX += hareketMesafesi;
                hareketEtti = true;
            }
            break;
        case 38: // Yukarı
            if (karakterY > yuzmeAlaniY) { // 📌 Yüzme alanının üst sınırını tam olarak algıla
                karakterY -= hareketMesafesi;
                hareketEtti = true;
            }
            break;
        case 40: // Aşağı
            if (karakterY < (yuzmeAlaniY + yuzmeAlaniYukseklik - karakterKoordinatlar[animasyonKare][3])) {
                karakterY += hareketMesafesi;
                hareketEtti = true;
            }
            break;
        }
    }
    else {
        int ekranMerkezi = pencereGenislik / 2;
        switch (tus) {
        case 37: // Sol
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
        case 39: // Sağ
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
            if (!yuzmede) {
                ziplamaAktif = true;
                std::thread([]() {
                    int baslangicY = karakterY;

                    while (karakterY > baslangicY - ziplamaYuksekligi) {
                        karakterY -= ziplamaHizi;
                        ekraniCiz();
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    while (karakterY < baslangicY) {
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
        ICG_printf(MouseLogBox, "Karakter Konum: X=%d, Y=%d, ArkaplanX=%d, Yüzme: %d\n", karakterX, karakterY, arkaplanPosX, yuzmede);
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
    if (!ReadImage("karakter.bmp", yüzme)) {
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
    if (!ReadImage("bat.bmp", yarasa)) { // Düzeltildi: yarasa sprite'ı doğru yükleniyor
        MessageBox(NULL, "Yarasa resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }
    if (!ReadImage("gold1.bmp", gold)) { // Düzeltildi: yarasa sprite'ı doğru yükleniyor
        MessageBox(NULL, "Gold resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }


    ekraniCiz(); // İlk ekran çizimi

   /* // Threadleri başlat
    HANDLE threadKus = CreateThread(NULL, 0, KusHareket, NULL, 0, NULL);
    /*if (!threadKus) {
        MessageBox(NULL, "Kuş hareket thread'i başlatılamadı.", "Hata", MB_OK);
        return;
    }*/

    /*HANDLE threadBalik = CreateThread(NULL, 0, BalikHareket, NULL, 0, NULL);
    /*if (!threadBalik) {
        MessageBox(NULL, "Balık hareket thread'i başlatılamadı.", "Hata", MB_OK);
        return;
    }*/

    /*HANDLE threadYarasa = CreateThread(NULL, 0, YarasaHareket, NULL, 0, NULL);
    /*if (!threadYarasa) {
        MessageBox(NULL, "Yarasa hareket thread'i başlatılamadı.", "Hata", MB_OK);
        return;
    }*/

    /*if (!threadKus || !threadBalik || !threadYarasa) {
        MessageBox(NULL, "Hareket thread'i başlatılamadı.", "Hata", MB_OK);
        return;
    }*/

    //Klavye girdisini bağla
    ICG_SetOnKeyPressed(klavyeGirdisi);

    MouseLogBox = ICG_MLEditSunken(10, 700, 600, 80, "", SCROLLBAR_V);
}