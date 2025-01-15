#include "icb_gui.h"
#include "ic_media.h"

// Karakterin başlangıç pozisyonu
int karakterX = 100, karakterY = 100;
const int hareketMesafesi = 10; // Hareket mesafesi (piksel)

// Arka plan için değişkenler
int arkaplanPosX = 0; // Arka planın başlangıç pozisyonu
const int pencereGenislik = 800, pencereYukseklik = 600;
int arkaplanGenislik = 1600, arkaplanYukseklik = 600; // Arka plan boyutları
ICBYTES arkaplan; // Arka plan resmi

// Karakter animasyonu için değişkenler
ICBYTES karakter; // Karakter sprite dosyası
int animasyonKare = 0; // Şu anki animasyon karesi
const int karakterKoordinatlar[3][2] = { {85, 55}, {83, 47}, {92, 58} }; // Her karenin genişlik ve yüksekliği

int anaPencere;

// Karakteri ekrana çizen fonksiyon
void KarakterCiz(ICBYTES& ekran) {
    int kareX = 0; // Her karenin soldan başlangıç konumu
    int kareY = 0; // İlk satırdaki karelerin varsayılan başlangıcı
    int kareGenislik = karakterKoordinatlar[animasyonKare][0];
    int kareYukseklik = karakterKoordinatlar[animasyonKare][1];

    // Karakteri ekrana çiz
    for (int y = 0; y < kareYukseklik; y++) {
        for (int x = 0; x < kareGenislik; x++) {
            ekran.C(karakterX + x, karakterY + y, 0) = karakter.C(kareX + x, kareY + y, 0); // Kırmızı
            ekran.C(karakterX + x, karakterY + y, 1) = karakter.C(kareX + x, kareY + y, 1); // Yeşil
            ekran.C(karakterX + x, karakterY + y, 2) = karakter.C(kareX + x, kareY + y, 2); // Mavi
        }
    }

    // Bir sonraki animasyon karesine geç
    animasyonKare = (animasyonKare + 1) % 3; // 0 → 1 → 2 → 0
}

// Arka planı ve karakteri çizen fonksiyon
void ekraniCiz() {
    ICBYTES ekran;
    CreateMatrix(ekran, pencereGenislik, pencereYukseklik, 3, ICB_UCHAR); // 800x600 boyutunda renkli ekran
    ekran = 255; // Arka plan beyaz

    // Arka planı ekrana yerleştir
    for (int y = 0; y < pencereYukseklik; y++) {
        for (int x = 0; x < pencereGenislik; x++) {
            if (x + arkaplanPosX < arkaplanGenislik && y < arkaplanYukseklik) {
                ekran.C(x, y, 0) = arkaplan.C(x + arkaplanPosX, y, 0); // Kırmızı
                ekran.C(x, y, 1) = arkaplan.C(x + arkaplanPosX, y, 1); // Yeşil
                ekran.C(x, y, 2) = arkaplan.C(x + arkaplanPosX, y, 2); // Mavi
            }
        }
    }

    // Karakteri çiz
    KarakterCiz(ekran);

    // Ekranı göster
    DisplayImage(anaPencere, ekran);
}

// Klavye girdisini işleyen fonksiyon
void klavyeGirdisi(int tus) {
    switch (tus) {
    case 37: // Sol ok
        if (karakterX > 0) {
            karakterX -= hareketMesafesi;
            if (karakterX < pencereGenislik / 2 && arkaplanPosX > 0) {
                arkaplanPosX -= hareketMesafesi; // Arka planı sola kaydır
            }
        }
        break;
    case 39: // Sağ ok
        if (karakterX < pencereGenislik - karakterKoordinatlar[animasyonKare][0]) {
            karakterX += hareketMesafesi;
            if (karakterX > pencereGenislik / 2 && arkaplanPosX < arkaplanGenislik - pencereGenislik) {
                arkaplanPosX += hareketMesafesi; // Arka planı sağa kaydır
            }
        }
        break;
    case 38: // Yukarı ok
        if (karakterY > 0) karakterY -= hareketMesafesi;
        break;
    case 40: // Aşağı ok
        if (karakterY < pencereYukseklik - karakterKoordinatlar[animasyonKare][1]) karakterY += hareketMesafesi;
        break;
    }
    ekraniCiz(); // Yeni pozisyonları çiz
}

void ICGUI_Create() {
    ICG_MWSize(pencereGenislik, pencereYukseklik); // Pencere boyutları
    ICG_MWTitle("Animasyonlu Karakter"); // Pencere başlığı
    ICG_MWColor(255, 255, 255); // Arka plan rengi (beyaz)
}

void ICGUI_main() {
    anaPencere = ICG_FramePanel(0, 0, pencereGenislik, pencereYukseklik); // Ana pencere

    // Arka planı yükle
    if (!ReadImage("arkaplan.bmp", arkaplan)) {
        MessageBox(NULL, "Arkaplan resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }

    // Karakter sprite'ını yükle
    if (!ReadImage("karakter.bmp", karakter)) {
        MessageBox(NULL, "Karakter resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }

    ekraniCiz(); // İlk çizim
    ICG_SetOnKeyPressed(klavyeGirdisi); // Klavye girdisi için fonksiyonu bağla
}
