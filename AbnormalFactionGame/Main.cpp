#include <windows.h>
#include "icb_gui.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

bool ucusAktif = false;
int hedefX = 66, hedefY = 280; // Karakterin başlangıç noktası
float ucusHizX = 0, ucusHizY = 0;
const float ucusHizCarpani = 0.1; // Hız katsayısı

int F1;
ICBYTES arkaplanilk, arkaplandevam, Character, Bird, Fish, Bat, Gold, Yuzme;
ICBYTES CharacterRun[3], CharacterSwim[4], BirdFly[4], FishSwim[2], BatFly[2];

bool oyunCalisiyor = true;
bool yuzmedenCikti = false; // Yüzmeden çıkış durumu

bool oyunBasladi = false; // Oyun başlama durumu
ICBYTES enterScreen;       // Giriş ekranı için resim

int MouseLogBox;

// Karakter değişkenleri
int karakterX = 66, karakterY = 280;
const int hareketMesafesi = 10;
bool yuzmede = false;
bool ziplamaAktif = false;
int animasyonKare = 0;

// Kuş, balık ve yarasa için animasyon kareleri
int birdAnimasyonKare = 0;
int fishAnimasyonKare = 0;
int batAnimasyonKare = 0;

// Merdiven koordinatları
const int merdivenSolX = 116, merdivenSagX = 130;
const int merdivenUstY = 280, merdivenAltY = 370;

// Yüzme alanı koordinatları
const int yuzmeAlaniX = 175, yuzmeAlaniY = 415;
const int yuzmeAlaniGenislik = 486, yuzmeAlaniYukseklik = 84;

// Objelerin başlangıç konumları
int birdX = 440, birdY = 247, birdYon = -1;
int fishX = 546, fishY = 432, fishYon = -1;
int batX = 309, batY = 160, batYon = -1;
const int hareketHizi = 5;

// Altın konumu
int goldX = 500, goldY = 390;
bool goldGorunur = true;

int loadAssetsButton, startGameButton; // Butonları kontrol etmek için ID'ler
bool assetsYuklendi = false; // Varlıkların yüklenip yüklenmediğini takip eder


bool altinAlindi = false;
bool oyunDurdu = false; // Oyun durdu mu?

bool mesajGorunur = false; // "One Life Extra!" mesajı aktif mi?
int mesajGorunurSuresi = 2000; // Mesajın görünme süresi (milisaniye)
std::chrono::steady_clock::time_point mesajBaslangicZamani;

const int ekranGenisligi = 700; // Oyun penceresinin genişliği
const int toplamArkaplanGenisligi = 1400; // İki arkaplanın birleşimi (700px + 700px)

int arkaPlanX = 0;  // Arka planın X ekseninde kaymasını kontrol eden değişken
const int arkaPlanGecisBaslangici = 500; // Arka planın kaymaya başlayacağı X noktası
const int maksimumKayma = 100;

bool gecisBasladi = false;  // Arka plan geçişi başladı mı?
int gecisKaymaMiktari = 0;  // Arkaplan2'nin kayma miktarı
const int gecisHizi = 10;   // Her güncellemede kaç birim kayacak
const int gecisTamamlanmaMiktari = 200; // Geçiş tamamlanınca toplam kayma miktarı
bool ikinciArkaplanAktif = false; // arka plan


void ShowOneLifeExtraMessage() {
    ICG_SLEdit(300, 400, 200, 50, "One Life Extra!");
}

// Merdivende mi?
void BaslangicaUcusBaslat() {
    ucusAktif = true;

    // Eğer ekran kayıyorsa, başlangıca dönüşte bu kaymayı iptal et
    int gercekKarakterX = karakterX + (karakterX >= 356 ? gecisKaymaMiktari : 0);

    // SABİT BİR HIZ BELİRLE (YAVAŞ UÇMASINI ENGELLE)
    ucusHizX = (hedefX - gercekKarakterX) * 0.1; // X hızını hedefe göre ayarla
    ucusHizY = (hedefY - karakterY) * 0.1; // Y hızını hedefe göre ayarla

    ICG_printf(MouseLogBox, "🚀 Uçuş Başladı! Karakter X=%d (Gerçek: %d), Y=%d -> Hedef X=%d, Y=%d\n",
        karakterX, gercekKarakterX, karakterY, hedefX, hedefY);
}


void BaslangicaUcusGuncelle() {
    if (!ucusAktif) return;

    // Karakteri başlangıca doğru hızlı hareket ettir
    karakterX += ucusHizX;
    karakterY += ucusHizY;

    // Başlangıç noktasına ulaştığında dur
    if (karakterX <= hedefX || karakterY <= hedefY) {
        karakterX = hedefX;
        karakterY = hedefY;
        ucusAktif = false;  // Uçuşu kapat

        // Düzgün şekilde durdur ve sıfırla
        yuzmede = false;
        animasyonKare = 0;
        ICG_printf(MouseLogBox, "✅ Karakter Başlangıca Ulaştı! X=%d, Y=%d\n", karakterX, karakterY);
    }

    // Ekranın dışına çıkmasını önleyelim
    if (karakterX < 0) karakterX = 0;
    if (karakterX > 800) karakterX = 800;
    if (karakterY < 0) karakterY = 0;
    if (karakterY > 600) karakterY = 600;
}



bool BaliklaCarpistiMi() {
    int karakterGenislik = 20;
    int karakterYukseklik = 20;
    int balikGenislik = 40;
    int balikYukseklik = 25;

    // Eğer ekran kayması başladıysa karakterin gerçek X konumunu hesapla
    int gercekKarakterX = karakterX + (karakterX >= 356 ? gecisKaymaMiktari : 0);
    int gercekBalikX = fishX - gecisKaymaMiktari; // Balığın kaymasını da hesaba kat

    int karakterMerkezX = gercekKarakterX + karakterGenislik / 2;
    int karakterMerkezY = karakterY + karakterYukseklik / 2;
    int balikMerkezX = gercekBalikX + balikGenislik / 2;
    int balikMerkezY = fishY + balikYukseklik / 2;

    int carpismaMesafesiX = (karakterGenislik + balikGenislik) / 2 + 5;
    int carpismaMesafesiY = (karakterYukseklik + balikYukseklik) / 2 + 5;


    bool carpisti = abs(karakterMerkezX - balikMerkezX) < carpismaMesafesiX &&
        abs(karakterMerkezY - balikMerkezY) < carpismaMesafesiY;



    if (carpisti) {
        BaslangicaUcusBaslat();
        ICG_printf(MouseLogBox, "⚠ BALIKLA ÇARPIŞMA! Karakter başlangıca uçuyor... Karakter X=%d (Gerçek: %d), Y=%d\n",
            karakterX, gercekKarakterX, karakterY);
    }

    ICG_printf(MouseLogBox, "fishX: %d, gercekBalikX: %d\n", fishX, gercekBalikX);


    return carpisti;
}

bool GoldCarpistiMi() {
    if (!goldGorunur) return false;

    // Tam olarak X=516, Y=416 olduğunda altını al!
    if (abs(karakterX - 526) <= 2 && abs(karakterY - 460) <= 2)
    {
        goldGorunur = false;
        ICG_printf(MouseLogBox, "✅ ALTIN ALINDI! Karakter X=%d, Y=%d | Altın X=%d, Y=%d\n",
            karakterX, karakterY, goldX, goldY);
        return true;
    }
    return false;
}

bool merdivendeMi(int x, int y) {
    // Eğer karakter en altta (Y=370) ise artık merdivende değil
    if (y >= merdivenAltY) return false;

    return (x >= merdivenSolX && x <= merdivenSagX && y >= merdivenUstY && y <= merdivenAltY);
}


// Yüzme alanında mı?
bool yuzmeAlaniIcindeMi(int x, int y) {
    return (x >= yuzmeAlaniX && x <= yuzmeAlaniX + yuzmeAlaniGenislik - 10 &&
        y >= yuzmeAlaniY && y <= yuzmeAlaniY + yuzmeAlaniYukseklik - 5);
}



// Balık su alanında hareket etmeli
const int fishMinX = 250;  // Su içindeki minimum X koordinatı
const int fishMaxX = 500;  // Su içindeki maksimum X koordinatı
const int fishMinY = 430;  // Su seviyesine uygun minimum Y
const int fishMaxY = 450;  // Su içinde hafif yukarı aşağı hareket

void ICGUI_Create() {
    ICG_MWSize(800, 600);
    ICG_MWTitle("Abnormal Faction Game");
}

void LoadGameAssets() {
    ReadImage("enter.bmp", enterScreen);
    ReadImage("dene.bmp", arkaplanilk);
    ReadImage("arkaplan2.bmp", arkaplandevam);
    ReadImage("karakter.bmp", Character);
    ReadImage("karakter.bmp", Yuzme);
    ReadImage("kus.bmp", Bird);
    ReadImage("fishh.bmp", Fish);
    ReadImage("batt.bmp", Bat);
    ReadImage("gold1.bmp", Gold);

    // Koşma Animasyonu
    ICBYTES charRunCoords{ {8, 9, 40, 60}, {58, 10, 40, 60}, {110, 10, 40, 60} };
    for (int i = 0; i < 3; i++) {
        Copy(Character, charRunCoords.I(1, i + 1), charRunCoords.I(2, i + 1),
            charRunCoords.I(3, i + 1), charRunCoords.I(4, i + 1), CharacterRun[i]);
    }

    // Yüzme Animasyonu
    ICBYTES swimCoords{ {8, 85, 52, 30}, {66, 85, 56, 33}, {130, 85, 53, 31}, {187, 85, 57, 32} };
    for (int i = 0; i < 4; i++) {
        Copy(Yuzme, swimCoords.I(1, i + 1), swimCoords.I(2, i + 1),
            swimCoords.I(3, i + 1), swimCoords.I(4, i + 1), CharacterSwim[i]);
    }

    // Kuş Animasyonu
    ICBYTES birdCoords{ {6, 2, 39, 24}, { 55,11,39,23 }, { 9,35,38,26 }, { 49,61,40,30 } };
    for (int i = 0; i < 4; i++) {
        Copy(Bird, birdCoords.I(1, i + 1), birdCoords.I(2, i + 1),
            birdCoords.I(3, i + 1), birdCoords.I(4, i + 1), BirdFly[i]);
    }

    // Balık Animasyonu
    ICBYTES fishCoords{ {16,30,53,37},{16,73,53,39},{84,30,51,37},{84,74,49,39} };
    for (int i = 0; i < 4; i++) {
        Copy(Fish, fishCoords.I(1, i + 1), fishCoords.I(2, i + 1),
            fishCoords.I(3, i + 1), fishCoords.I(4, i + 1), FishSwim[i]);
    }

    // Yarasa Animasyonu
    ICBYTES batCoords{ {5, 24, 49, 43}, {56, 24, 41, 27} };
    for (int i = 0; i < 2; i++) {
        Copy(Bat, batCoords.I(1, i + 1), batCoords.I(2, i + 1),
            batCoords.I(3, i + 1), batCoords.I(4, i + 1), BatFly[i]);
    }

    DisplayImage(F1, enterScreen);

    ICG_DestroyWidget(loadAssetsButton);

    assetsYuklendi = true;
}

void UpdateScreen() {

    BaslangicaUcusGuncelle();  // Eğer uçuş devam ediyorsa güncelle

    if (mesajGorunur) {
        ShowOneLifeExtraMessage();
    }

    if (karakterX >= 356) {
        gecisBasladi = true;
    }

    if (gecisKaymaMiktari > 700) {
        gecisKaymaMiktari = 700;
    }
    else if (gecisBasladi) {
        gecisKaymaMiktari = karakterX - 356;
    }

    ReadImage("dene.bmp", arkaplanilk);
    ReadImage("arkaplan2.bmp", arkaplandevam);

    ICBYTES geciciArkaPlan;
    Copy(arkaplanilk, 0, 0, 700, 500, geciciArkaPlan);
    FillRect(geciciArkaPlan, 0, 0, 700, 500, 0);

    if (gecisKaymaMiktari < 700) {
        PasteNon0(arkaplanilk, -gecisKaymaMiktari, 0, geciciArkaPlan);
    }

    PasteNon0(arkaplandevam, 700 - gecisKaymaMiktari, 0, geciciArkaPlan);

    int karakterEkranX = karakterX - gecisKaymaMiktari;

    if (yuzmede) {
        PasteNon0(CharacterSwim[animasyonKare % 4], karakterEkranX, karakterY, geciciArkaPlan);
    }
    else {
        PasteNon0(CharacterRun[animasyonKare % 3], karakterEkranX, karakterY, geciciArkaPlan);
    }

    PasteNon0(BirdFly[birdAnimasyonKare % 4], birdX - gecisKaymaMiktari, birdY, geciciArkaPlan);
    PasteNon0(FishSwim[fishAnimasyonKare % 2], fishX - gecisKaymaMiktari, fishY, geciciArkaPlan);
    PasteNon0(BatFly[batAnimasyonKare % 2], batX - gecisKaymaMiktari, batY, geciciArkaPlan);

    if (goldGorunur) {
        PasteNon0(Gold, goldX - gecisKaymaMiktari, goldY, geciciArkaPlan);
    }

    //  Gold ile çarpışmayı kontrol et
    if (GoldCarpistiMi()) {
        ICG_printf(MouseLogBox, "Altınla çarpışma algılandı! \n");
        ICG_SLEdit(500, 400, 100, 25, "One Life Extra!");
    }
    if (BaliklaCarpistiMi()) {
        ICG_printf(MouseLogBox, "Altınla çarpışma algılandı! \n");
        ICG_SLEdit(500, 400, 100, 25, "LIFE 0!");
    }
    DisplayImage(F1, geciciArkaPlan);
}

void yuzmeModuGuncelle() {
    int gercekKarakterX = karakterX + gecisKaymaMiktari;

    //  Yüzme alanına giriş kontrolü (Yüzme alanına ilk kez giriyorsa)
    if (!yuzmede && karakterY >= merdivenAltY &&
        gercekKarakterX >= yuzmeAlaniX &&
        gercekKarakterX <= yuzmeAlaniX + yuzmeAlaniGenislik - 20) {

        yuzmede = true;
        karakterY = yuzmeAlaniY + 5; // Karakter suyun içine girsin
        animasyonKare = 0;
        ICG_printf(MouseLogBox, "Karakter yüzme moduna girdi! X=%d (Gerçek: %d), Y=%d\n",
            karakterX, gercekKarakterX, karakterY);
    }

    // Eğer karakter yüzmedeyse ve çıkış bölgesine geldiyse çıkmasını sağla
    if (yuzmede && gercekKarakterX >= yuzmeAlaniX + yuzmeAlaniGenislik + gecisKaymaMiktari) {
        yuzmede = false;
        yuzmedenCikti = true;
        karakterY = merdivenAltY; // Karakteri tekrar yere koy
        gecisBasladi = true; // İkinci arka plan kaymasını başlat
        UpdateScreen();
        ICG_printf(MouseLogBox, "Karakter yüzmeden çıktı ve yürümeye başladı! X=%d (Gerçek: %d), Y=%d\n",
            karakterX, gercekKarakterX, karakterY);
    }
}


void StartGame(void*) {
    if (!assetsYuklendi) return;

    ICG_DestroyWidget(startGameButton);
    oyunBasladi = true;

    fishX = 574;
    fishY = 440;

    while (oyunCalisiyor) {
        static int frameCounter = 0;
        frameCounter++;

        //  Animasyonları 5 frame'de bir değiştir
        if (frameCounter % 5 == 0) {
            //  Kuş animasyonu yönüne göre değiştir
            if (birdYon == -1) {
                birdAnimasyonKare = (birdAnimasyonKare + 1) % 2;
            }
            else {
                birdAnimasyonKare = 2 + (birdAnimasyonKare + 1) % 2;
            }

            //  Balık animasyonu yönüne göre değiştir
            if (fishYon == -1) {
                fishAnimasyonKare = (fishAnimasyonKare + 1) % 2;
            }
            else {
                fishAnimasyonKare = 2 + (fishAnimasyonKare + 1) % 2;
            }

            //  Yarasa animasyonu (Bunun yönü değişmiyor)
            batAnimasyonKare = (batAnimasyonKare + 1) % 2;
        }

        //  Kuş hareketi
        birdX += birdYon * hareketHizi;
        if (birdX <= 100 || birdX >= 700) birdYon *= -1;

        //  Balık hareketi
        // Balık hareketi ve dönüş ayarı
        fishX += fishYon * hareketHizi;

        if (fishX <= fishMinX) fishYon = 1;
        if (fishX >= fishMaxX) fishYon = -1;


        //  Balık hafif dalgalanmalı
        fishY += (rand() % 3 - 1);
        if (fishY < 420) fishY = 420;
        if (fishY > 473) fishY = 473;

        // Yarasa hareketi
        batX += batYon * hareketHizi;
        if (batX <= 100 || batX >= 700) batYon *= -1;

        // Ekranı güncelle
        UpdateScreen();
        Sleep(100);
    }
}


// Klavye Girişi
void KeyboardInput(int key) {
    if (mesajGorunur) {
        return; // Mesaj ekrandayken girişleri engelle
    }

    // Eğer oyun durduysa herhangi bir tuşa basınca devam et
    if (oyunDurdu) {
        oyunDurdu = false;  // Oyunu devam ettir
        ICG_printf(MouseLogBox, "Oyun devam ediyor!\n");
        UpdateScreen();
        return;
    }

    bool hareketEtti = false;


    //  Eğer karakter yüzme alanından çıkarsa yürümeye devam etmeli
    if (yuzmede && karakterX >= 566 && karakterY == 410) {
        yuzmede = false;
        yuzmedenCikti = true;
        karakterY = 370; // Karakteri yürüyüş moduna al
        ICG_printf(MouseLogBox, "Karakter yüzmeden çıktı ve yürümeye başladı! X=%d, Y=%d\n", karakterX, karakterY);
    }

    if (karakterX >= 400) {
        gecisBasladi = true;
    }

    if (key == 32 && !ziplamaAktif) { // 32 -> Space tuşu ASCII kodu
        ziplamaAktif = true;
        int ziplamaYuksekligi = 10; // Zıplama yüksekliği
        int ziplamaHizi = 5; // Her karede ne kadar yukarı çıksın

        for (int i = 0; i < ziplamaYuksekligi / ziplamaHizi; i++) {
            karakterY -= ziplamaHizi;
            UpdateScreen();
            Sleep(30); // Küçük bir gecikme ekleyerek animasyonu sağla
        }

        // Karakter tekrar aşağı insin
        for (int i = 0; i < ziplamaYuksekligi / ziplamaHizi; i++) {
            karakterY += ziplamaHizi;
            UpdateScreen();
            Sleep(30);
        }

        ziplamaAktif = false;
    }

    yuzmeModuGuncelle(); //  Yüzme modunu kontrol et

    //  Merdivende mi?
    if (merdivendeMi(karakterX, karakterY)) {
        if (key == 38 && karakterY > merdivenUstY) {
            karakterY -= hareketMesafesi;
            hareketEtti = true;
        }
        else if (key == 40 && karakterY < merdivenAltY) {
            karakterY += hareketMesafesi;
            hareketEtti = true;
        }

        if (karakterY >= 370) {
            karakterX += 10;
        }
    }
    //  Eğer yüzmede ise yüzme hareketlerini uygula
    else if (yuzmede && !yuzmedenCikti) {
        switch (key) {
        case 37: if (karakterX > yuzmeAlaniX) { karakterX -= hareketMesafesi; hareketEtti = true; } break;
        case 39: if (karakterX < yuzmeAlaniX + yuzmeAlaniGenislik - 40) { karakterX += hareketMesafesi; hareketEtti = true; } break;
        case 38: if (karakterY > yuzmeAlaniY) { karakterY -= hareketMesafesi; hareketEtti = true; } break;
        case 40: if (karakterY < yuzmeAlaniY + yuzmeAlaniYukseklik - 40) { karakterY += hareketMesafesi; hareketEtti = true; } break;
        }
    }
    // Eğer karakter yürümeye geçtiyse, ikinci arkaplana doğru ilerlemesini sağla
    else {
        if (key == 37 && karakterX > 50) { // Sol
            karakterX -= hareketMesafesi;
            hareketEtti = true;
        }
        else if (key == 39) { // Sağ
            karakterX += hareketMesafesi;
            hareketEtti = true;

            //  Eğer ikinci arkaplana geçtiyse, arkaplanı kaydır
            if (gecisBasladi && gecisKaymaMiktari < 800) {
                gecisKaymaMiktari += hareketMesafesi;
            }
        }
    }

    if (yuzmede && yuzmedenCikti) {
        yuzmede = false;
        karakterY = merdivenAltY;
        ICG_printf(MouseLogBox, "Karakter yüzmeden çıktı ve yürümeye başladı! X=%d, Y=%d\n", karakterX, karakterY);
    }


    if (hareketEtti) {
        animasyonKare = (animasyonKare + 1) % 4;
        ICG_printf(MouseLogBox, "Karakter Konum: X=%d, Y=%d, Yüzme: %d\n", karakterX, karakterY, yuzmede);
        UpdateScreen();
    }

    if (BaliklaCarpistiMi()) {
        ICG_printf(MouseLogBox, "Balıkla Çarpışma Algılandı!\n");
    }

}

void ICGUI_main() {
    PlaySound("Remembrance.wav", NULL, SND_ASYNC);

    F1 = ICG_FrameThin(5, 5, 800, 600);
    ICG_Button(270, 250, 150, 45, "Load Assets", LoadGameAssets);
    ICG_TButton(270, 295, 150, 45, "Start Game", StartGame, NULL);

    LoadGameAssets(); // İlk olarak giriş ekranını yükle
    DisplayImage(F1, enterScreen); // Giriş ekranını göster

    //MouseLogBox = ICG_MLEditSunken(10, 700, 600, 80, "", SCROLLBAR_V);

    ICG_SetOnKeyPressed(KeyboardInput);
}