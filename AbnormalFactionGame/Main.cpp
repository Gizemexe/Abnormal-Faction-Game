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
int karakterKoordinatlar[3][4] = { { 8, 9, 40, 60 }, { 58, 10, 40, 60 }, { 110, 10, 40, 60 } }; // X, Y, Genişlik, Yükseklik

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
int oyunFrame;

bool altinAlindi = false;

// Karakteri ekrana çizen fonksiyon
void KarakterCiz(ICBYTES& ekran) {
    if (yuzmede) return; // Eğer karakter yüzüyorsa, normal çizim yapılmamalı

    int kareX = karakterKoordinatlar[animasyonKare][0];
    int kareY = karakterKoordinatlar[animasyonKare][1];
    int kareGenislik = karakterKoordinatlar[animasyonKare][2];
    int kareYukseklik = karakterKoordinatlar[animasyonKare][3];

    // Geçici bir sprite matrisi oluştur
    ICBYTES gecici;
    CreateMatrix(gecici, kareGenislik, kareYukseklik, 4, ICB_UCHAR);

    // Karakter sprite'ını kopyala
    if (!Copy(karakter, kareX, kareY, kareGenislik, kareYukseklik, gecici)) {
       
        return;
    }

    // Karakter sprite'ını ekrana yapıştır
    if (!PasteNon0(gecici, karakterX, karakterY, ekran)) {
        
    }

    // Bellek temizleme
    Free(gecici);
}



// Kuşu ekrana çizen fonksiyon
void KusCiz(ICBYTES& ekran) {
    int kareX = kusKoordinatlar[kusAnimasyonKare][0];
    int kareY = kusKoordinatlar[kusAnimasyonKare][1];
    int kareGenislik = kusKoordinatlar[kusAnimasyonKare][2];
    int kareYukseklik = kusKoordinatlar[kusAnimasyonKare][3];

    ICBYTES gecici;
    CreateMatrix(gecici, kareGenislik, kareYukseklik, 4, ICB_UCHAR);

    if (!Copy(kus, kareX, kareY, kareGenislik, kareYukseklik, gecici)) {
        printf("HATA: Kuş sprite'ı kopyalanamadı!\n");
        return;
    }

    if (!PasteNon0(gecici, kusX, kusY, ekran)) {
        printf("HATA: Kuş sprite'ı ekrana yapıştırılamadı!\n");
    }

    Free(gecici);
}




// Balığı ekrana çizen fonksiyon
void BalikCiz(ICBYTES& ekran) {
    int kareX = balikKoordinatlar[balikYon == -1 ? 0 : 1][0];
    int kareY = balikKoordinatlar[balikYon == -1 ? 0 : 1][1];
    int kareGenislik = balikKoordinatlar[balikYon == -1 ? 0 : 1][2];
    int kareYukseklik = balikKoordinatlar[balikYon == -1 ? 0 : 1][3];

    // Geçici sprite matrisi oluştur
    ICBYTES gecici;
    CreateMatrix(gecici, kareGenislik, kareYukseklik, 4, ICB_UCHAR);

    // Balık sprite'ını kopyala
    if (!Copy(fish, kareX, kareY, kareGenislik, kareYukseklik, gecici)) {
        
        return;
    }

    // Balık sprite'ını ekrana yapıştır
    if (!PasteNon0(gecici, balikX, balikY, ekran)) {
       
    }

    // Bellek temizleme
    Free(gecici);
}



// Yarasa ekrana çizen fonksiyon
void YarasaCiz(ICBYTES& ekran) {
    int index = (yarasaYon == -1) ? 0 : 1;
    int kareX = yarasaKoordinatlar[index][0];
    int kareY = yarasaKoordinatlar[index][1];
    int kareGenislik = yarasaKoordinatlar[index][2];
    int kareYukseklik = yarasaKoordinatlar[index][3];

    // Geçici sprite matrisi oluştur
    ICBYTES gecici;
    CreateMatrix(gecici, kareGenislik, kareYukseklik, 4, ICB_UCHAR);

    // Yarasa sprite'ını kopyala
    if (!Copy(yarasa, kareX, kareY, kareGenislik, kareYukseklik, gecici)) {
       
        return;
    }

    // Yarasa sprite'ını ekrana yapıştır
    if (!PasteNon0(gecici, yarasaX, yarasaY, ekran)) {
    
    }

    // Bellek temizleme
    Free(gecici);
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
    else if (karakterX >= 486) {
        yuzmede = false;
        karakterY = 370;
        yüzmeAnimasyonKare = 0; //

    }
}

void YuzmeCiz(ICBYTES& ekran) {
    if (!yuzmede) return; // Eğer yüzme modunda değilse, çizme işlemini yapma

    // 🎯 **Animasyon karesini sürekli değiştir**
    yüzmeAnimasyonKare = (yüzmeAnimasyonKare + 1) % 4;

    int kareX = yüzmeKoordinatlar[yüzmeAnimasyonKare][0];
    int kareY = yüzmeKoordinatlar[yüzmeAnimasyonKare][1];
    int kareGenislik = yüzmeKoordinatlar[yüzmeAnimasyonKare][2];
    int kareYukseklik = yüzmeKoordinatlar[yüzmeAnimasyonKare][3];

    // 🎯 **Karakterin alt kısmından başlaması için ayarla**
    int cizimY = karakterY - (kareYukseklik / 2);

    // Geçici bir sprite matrisi oluştur
    ICBYTES gecici;
    CreateMatrix(gecici, kareGenislik, kareYukseklik, 4, ICB_UCHAR);

    // **Yüzme sprite'ını kopyala**
    Copy(yüzme, kareX, kareY, kareGenislik, kareYukseklik, gecici);

    // **Yüzme sprite'ını ekrana yapıştır**
    PasteNon0(gecici, karakterX, cizimY, ekran);

    // **Bellek temizleme**
    Free(gecici);
}


void YerCekimi() {
    if (yuzmede || ziplamaAktif) return;

    if (!merdivendeMi(karakterX, karakterY)) {
        karakterY += hareketMesafesi / 2; // Yer çekimi yavaşlatıldı
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

    // Ekranı siyah (0, 0, 0) ile doldur
    for (int y = 0; y < pencereYukseklik; y++) {
        for (int x = 0; x < pencereGenislik; x++) {
            ekran.C(x, y, 0) = -1; // Kırmızı kanal
            ekran.C(x, y, 1) = 0; // Yeşil kanal
            ekran.C(x, y, 2) = 0; // Mavi kanal
            ekran.C(x, y, 3) = 0x00;
        }
    }


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

    KusCiz(ekran);
    BalikCiz(ekran);
    YarasaCiz(ekran);
    GoldCiz(ekran);
    DisplayImage(anaPencere, ekran);
}

// Klavye girdisini işleyen fonksiyon
void klavyeGirdisi(int tus) {
    bool hareketEtti = false;
    int ekranMerkezi = pencereGenislik / 2;

    yuzmeModuGuncelle(); // **Yüzme durumunu güncelle**

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
        switch (tus) {
        case 37: // Sol
            if (karakterX > yuzmeAlaniX) {
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
            if (karakterY > yuzmeAlaniY) {
                karakterY -= hareketMesafesi;
                hareketEtti = true;
            }
            break;
        case 40: // Aşağı
            if (karakterY < (yuzmeAlaniY + (yuzmeAlaniYukseklik - 30))) {
                karakterY += hareketMesafesi;
                hareketEtti = true;
            }
            break;
        }

        if (karakterX >= 486) {
            yuzmede = false;
            karakterY = 370;
            yüzmeAnimasyonKare = 0;
            hareketEtti = true;
        }
    }
    else {
        switch (tus) {
        case 37: // **Sol**
            if (karakterX > 100) {
                if (arkaplanPosX > 0) {
                    arkaplanPosX -= hareketMesafesi;
                }
                else {
                    karakterX -= hareketMesafesi;
                }
               // hareketEtti = true;
            }
            break;
        case 39: // **Sağ**
            if (karakterX >= ekranMerkezi && arkaplanPosX + pencereGenislik < arkaplanGenislik) {
                arkaplanPosX += hareketMesafesi; // Ekranı sola kaydır
            }
            else {
                karakterX += hareketMesafesi; // Ekran kayamazsa karakteri ilerlet
            }

            // 🎯 Yüzmeden çıktıktan sonra bile arka plan kaymaya devam etmeli
            if (!yuzmede && karakterX >= ekranMerkezi && arkaplanPosX < arkaplanGenislik / 2) {
                arkaplanPosX += hareketMesafesi;
            }


            hareketEtti = true;
            break;

        case 32: // **Space (Zıplama)**
            if (!ziplamaAktif && !yuzmede) {
                ziplamaAktif = true;

                std::thread([]() {
                    int baslangicY = karakterY;

                    // Yukarı çıkış
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
    if (karakterX > pencereGenislik / 2 && !ziplamaAktif) {
        arkaplanPosX += hareketMesafesi / 2;
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

     //Klavye girdisini bağla
    ICG_SetOnKeyPressed(klavyeGirdisi);

    MouseLogBox = ICG_MLEditSunken(10, 700, 600, 80, "", SCROLLBAR_V);
}