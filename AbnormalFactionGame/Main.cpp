#include "icb_gui.h"
#include "ic_media.h"

// Karakterin başlangıç pozisyonu
int karakterX = 66, karakterY = 280;
const int hareketMesafesi = 10; // Hareket mesafesi (piksel)

// Arka plan için değişkenler
int arkaplanPosX = 0; // Arka planın başlangıç pozisyonu
const int pencereGenislik = 800, pencereYukseklik = 600;
int arkaplanGenislik = 1600, arkaplanYukseklik = 600; // Arka plan boyutları
ICBYTES arkaplan, arkaplan2; // İki arka plan resmi

// Karakter animasyonu için değişkenler
ICBYTES karakter; // Karakter sprite dosyası
int animasyonKare = 0; // Şu anki animasyon karesi
int karakterKoordinatlar[3][4] = { {8, 9, 40, 60}, {58, 10, 40, 60}, {110, 10, 40, 60} }; // X, Y, Genişlik, Yükseklik

int anaPencere;

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
    animasyonKare = (animasyonKare + 1) % 3;
}

// Arka planı ve karakteri çizen fonksiyon
void ekraniCiz() {
    ICBYTES ekran;
    CreateMatrix(ekran, pencereGenislik, pencereYukseklik, 3, ICB_UCHAR);
    ekran = 255;

    for (int y = 0; y < pencereYukseklik; y++) {
        for (int x = 0; x < pencereGenislik; x++) {
            if (arkaplanPosX + x < arkaplanGenislik) {
                ekran.C(x, y, 0) = arkaplan.C(arkaplanPosX + x, y, 0);
                ekran.C(x, y, 1) = arkaplan.C(arkaplanPosX + x, y, 1);
                ekran.C(x, y, 2) = arkaplan.C(arkaplanPosX + x, y, 2);
            }
            else if (arkaplanPosX + x >= arkaplanGenislik && arkaplanPosX + x < 2 * arkaplanGenislik) {
                ekran.C(x, y, 0) = arkaplan2.C(arkaplanPosX + x - arkaplanGenislik, y, 0);
                ekran.C(x, y, 1) = arkaplan2.C(arkaplanPosX + x - arkaplanGenislik, y, 1);
                ekran.C(x, y, 2) = arkaplan2.C(arkaplanPosX + x - arkaplanGenislik, y, 2);
            }
        }
    }
    KarakterCiz(ekran);
    DisplayImage(anaPencere, ekran);
}

// Klavye girdisini işleyen fonksiyon
void klavyeGirdisi(int tus) {
    switch (tus) {
    case 37:
        if (karakterX > 0) {
            karakterX -= hareketMesafesi;
            if (karakterX < pencereGenislik / 2 && arkaplanPosX > 0) {
                arkaplanPosX -= hareketMesafesi;
            }
        }
        break;
    case 39:
        if (karakterX < pencereGenislik - karakterKoordinatlar[animasyonKare][2]) {
            karakterX += hareketMesafesi;
            if (karakterX > pencereGenislik / 2 && arkaplanPosX < 2 * arkaplanGenislik - pencereGenislik) {
                arkaplanPosX += hareketMesafesi;
            }
        }
        break;
    case 38:
        if (karakterY > 0) karakterY -= hareketMesafesi;
        break;
    case 40:
        if (karakterY < pencereYukseklik - karakterKoordinatlar[animasyonKare][3]) karakterY += hareketMesafesi;
        break;
    }
    ekraniCiz();
}

void ICGUI_Create() {
    ICG_MWSize(pencereGenislik, pencereYukseklik);
    ICG_MWTitle("Animasyonlu Karakter");
    ICG_MWColor(255, 255, 255);
}

void ICGUI_main() {
    anaPencere = ICG_FramePanel(0, 0, pencereGenislik, pencereYukseklik);

    if (!ReadImage("arkaplan.bmp", arkaplan)) {
        MessageBox(NULL, "Arkaplan resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }
    if (!ReadImage("arkaplan2.bmp", arkaplan2)) {
        MessageBox(NULL, "Arkaplan2 resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }
    if (!ReadImage("karakter.bmp", karakter)) {
        MessageBox(NULL, "Karakter resmi yüklenemedi.", "Hata", MB_OK);
        return;
    }
    ekraniCiz();
    ICG_SetOnKeyPressed(klavyeGirdisi);
}