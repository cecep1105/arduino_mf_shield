/*
  ============================================================
  Sketch Uji Coba Semua Sensor - Multi-Function Shield B04505
  + Display 4-Digit TM1650 (modul terpisah)
  ============================================================
  Board shield ini punya:
    - 2 tombol   (SW1 -> D2, SW2 -> D3)
    - 2 LED      (LED1 biru -> D13, LED2 merah -> D12)
    - RGB LED    (D9 = Merah, D10 = Hijau, D11 = Biru)
    - DHT11      (suhu & kelembapan -> D4)
    - Buzzer     (-> D5)
    - IR Receiver(-> D6)
    - Potensio   (Rotation -> A0)
    - LDR        (Light -> A1)
    - LM35       (suhu analog -> A2)

  DISPLAY TAMBAHAN (modul terpisah, TM1650 4-Digit Display):
    Modul ini pakai chip TM1650 (label di chip: TM1650), header pin-nya
    tertulis "VCC SCL SDA GND" -- persis sama urutannya dengan header
    "GND VCC SDA SCL" di pojok kiri atas shield, jadi tinggal disambung
    pin-ke-pin apa adanya pakai kabel jumper:
      Modul VCC -> Shield VCC
      Modul SCL -> Shield SCL
      Modul SDA -> Shield SDA
      Modul GND -> Shield GND
    (Header ini terhubung ke pin hardware I2C Arduino UNO, yaitu
     SDA = A4 dan SCL = A5. TM1650, sama seperti TM1637, bukan protokol
     I2C sungguhan -- tidak perlu resistor pull-up tambahan, pin ini
     dipakai sebagai jalur DIO/CLK biasa yang di-bit-bang oleh library.)

    Cara kerja tombol:
      - TAP SW2 (tekan-lepas cepat) -> ganti mode tampilan:
        SUHU <-> KODE IR (bergantian).
      - TAP SW1 (tekan-lepas cepat, < 0.7 detik) -> ganti format
        angka kode IR: HEX <-> DESIMAL. (Format ini cuma kelihatan
        efeknya waktu mode tampilan lagi di KODE IR, tapi bisa
        diganti kapan saja.)
      - TAHAN SW1 (>= 0.7 detik) -> nyala/matikan RGB LED.
        Ini berguna untuk tes apakah RGB LED yang nyala bareng-bareng
        bikin layar TM1650 jadi redup/berkedip (dugaan drop tegangan
        5V pas komponen lain aktif bersamaan) -- matikan RGB LED lalu
        lihat apakah layarnya jadi lebih stabil/terang.
      - TAHAN SW2 (>= 0.7 detik) -> jalankan TES KECERAHAN DISPLAY.
        Layar akan bergantian menampilkan "----" (1 segmen/digit,
        arus paling kecil), "1111" (2 segmen/digit), lalu "8888"
        (7 segmen/digit, arus paling besar), masing-masing selama
        4 detik. Tes ini juga otomatis jalan sekali waktu board baru
        nyala/di-reset. Gunanya: ukur tegangan VCC-GND di modul
        TM1650 pakai multimeter di setiap pola -- kalau tegangannya
        kelihatan turun pas pola "8888" dibanding "----", itu artinya
        penyebab redupnya adalah drop tegangan/arus di jalur kabel
        VCC-GND ke modul (bukan software, bukan RGB LED).
      - Buzzer HANYA bunyi saat IR Receiver berhasil menerima kode
        dari remote (bukan bunyi berkala lagi).

  LIBRARY YANG HARUS DI-INSTALL (via Library Manager):
    1. "DHT sensor library" by Adafruit   (+ dependency "Adafruit Unified Sensor")
    2. "IRremote" by Armin Joachimsmeyer  (versi 4.x)
    3. "TM16xx" by Maxint R&D -- library ini mendukung banyak chip
       (TM1637, TM1650, TM1638, dll), kita pakai class TM1650-nya.
       (CATATAN: library "TM1637Display" yang dipakai sebelumnya SUDAH
        TIDAK DIPAKAI LAGI, boleh di-uninstall kalau mau beres-beres.)

  Cara pakai:
    - Upload sketch ini, lalu buka Serial Monitor, baud rate 9600.
    - Tap SW2 untuk pindah antara tampilan suhu dan tampilan kode IR.
    - Tap SW1 untuk ganti format kode IR antara HEX dan DESIMAL.
    - Tahan SW1 sebentar (>= 0.7 detik) untuk mati/nyalakan RGB LED.
    - Tahan SW2 sebentar (>= 0.7 detik) untuk mengulang tes kecerahan
      display (pola ---- / 1111 / 8888) kapan saja tanpa perlu reset.
    - Arahkan remote IR apapun ke Receiver -> buzzer akan bunyi
      pendek dan kode IR-nya muncul di layar (kalau sedang di mode
      KODE IR).
    - Putar potensiometer & tutup LDR dengan tangan untuk lihat
      nilainya berubah di Serial Monitor.
    - LED1/LED2 tetap berkedip otomatis sebagai tanda board hidup.

  CATATAN SOAL LAYAR REDUP:
    Di shield ini RGB LED disambung langsung ke pin digital (bukan
    lewat transistor yang narik banyak arus dari jalur 5V), jadi
    kemungkinan besar bukan RGB LED sendirian penyebabnya. Kalau
    setelah RGB dimatikan layar masih suka redup/berkedip, kemungkinan
    lain: sumber daya USB laptop yang pas-pasan (coba pakai adaptor
    5V terpisah), atau proses decode IRremote/DHT11 yang sesaat
    menghentikan interrupt sehingga jalur bit-bang DIO/CLK ke TM1650
    sedikit terganggu.
  ============================================================
*/

#include <DHT.h>
#include <IRremote.hpp>
#include <TM1650.h>

// ---------- Definisi Pin ----------
#define PIN_SW1      2
#define PIN_SW2      3
#define PIN_DHT      4
#define PIN_BUZZER   5
#define PIN_IR_RECV  6
#define PIN_RGB_R    9
#define PIN_RGB_G    10
#define PIN_RGB_B    11
#define PIN_LED2     12   // merah
#define PIN_LED1     13   // biru

#define PIN_ROTATION A0
#define PIN_LIGHT    A1
#define PIN_LM35     A2

// Header GND VCC SDA SCL -> pin I2C hardware bawaan Uno
#define PIN_DISPLAY_DIO A4   // SDA
#define PIN_DISPLAY_CLK A5   // SCL

#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// PENTING: urutan parameter TM1650 kebalik dari TM1637Display!
// TM1637Display: (CLK, DIO)  <->  TM1650: (DIO, CLK)
TM1650 display(PIN_DISPLAY_DIO, PIN_DISPLAY_CLK);

// ---------- Variabel waktu (non-blocking) ----------
unsigned long tSensorRead   = 0;
unsigned long tDHTRead      = 0;
unsigned long tLedBlink     = 0;
unsigned long tRgbCycle     = 0;
unsigned long tButtonSW1    = 0;
unsigned long tButtonSW2    = 0;
unsigned long tDisplayUpdate = 0;

bool led1State = false;
int  rgbStep   = 0;

bool lastSW1 = HIGH;
bool lastSW2 = HIGH;

// 0 = tampilkan suhu, 1 = tampilkan kode IR (di-toggle dengan tap SW2)
int displayMode = 0;

// true = tampilkan kode IR dalam HEX, false = DESIMAL (di-toggle dengan tap SW1)
bool irHexFormat = true;

// true = RGB LED aktif berputar warna, false = RGB LED dimatikan
// (di-toggle dengan tekan-tahan SW1 >= LONG_PRESS_MS)
bool rgbEnabled = true;

const unsigned long LONG_PRESS_MS = 700;
unsigned long tSW1PressStart = 0;
bool sw1LongPressTriggered = false;

unsigned long tSW2PressStart = 0;
bool sw2LongPressTriggered = false;

uint32_t lastIRCode = 0;
bool hasIRCode = false;

// ---------- Tes kecerahan display (diagnosa drop tegangan) ----------
// Menampilkan pola dengan jumlah segmen berbeda-beda secara berurutan,
// supaya bisa dibandingkan kecerahannya / diukur tegangannya dengan
// multimeter di tiap pola. Fungsi ini BLOCKING (pakai delay), sengaja,
// karena memang dipakai sesaat saja (bukan bagian dari loop utama).
void runBrightnessDiagnostic() {
  Serial.println(F("=== TES KECERAHAN DISPLAY ==="));
  Serial.println(F("Ukur tegangan VCC-GND di modul TM1650 pas tiap pola tampil."));

  display.setupDisplay(true, 7); // true = nyala, intensity 0-7 (7 = paling terang)

  display.setDisplayToString("----");
  Serial.println(F("Pola 1/3: \"----\" (1 segmen/digit, arus paling kecil) -> ukur sekarang"));
  delay(4000);

  display.setDisplayToDecNumber(1111, 0, true);
  Serial.println(F("Pola 2/3: \"1111\" (2 segmen/digit) -> ukur sekarang"));
  delay(4000);

  display.setDisplayToDecNumber(8888, 0, true);
  Serial.println(F("Pola 3/3: \"8888\" (7 segmen/digit, arus paling besar) -> ukur sekarang"));
  delay(4000);

  Serial.println(F("=== Tes selesai, kembali ke tampilan normal ==="));
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("=== Uji Coba Shield B04505 dimulai ==="));

  pinMode(PIN_SW1, INPUT_PULLUP);
  pinMode(PIN_SW2, INPUT_PULLUP);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_RGB_R, OUTPUT);
  pinMode(PIN_RGB_G, OUTPUT);
  pinMode(PIN_RGB_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  dht.begin();
  IrReceiver.begin(PIN_IR_RECV, ENABLE_LED_FEEDBACK);

  display.setupDisplay(true, 7); // true = nyala, intensity 0-7 (7 = paling terang)
  display.clearDisplay();

  // Nyala semua LED sesaat sebagai tanda board OK (buzzer TIDAK dibunyikan
  // di sini, karena buzzer sekarang khusus untuk notifikasi IR diterima)
  digitalWrite(PIN_LED1, HIGH);
  digitalWrite(PIN_LED2, HIGH);
  delay(200);
  digitalWrite(PIN_LED1, LOW);
  digitalWrite(PIN_LED2, LOW);

  // Jalankan tes kecerahan display sekali di awal (bisa diulang kapan
  // saja nanti dengan tahan SW2 >= 0.7 detik)
  runBrightnessDiagnostic();
}

void loop() {
  unsigned long now = millis();

  // ---------- 1. Tombol SW1 & SW2 (tap untuk toggle) ----------
  bool sw1 = digitalRead(PIN_SW1);
  bool sw2 = digitalRead(PIN_SW2);

  if (sw1 != lastSW1 && now - tButtonSW1 > 50) { // debounce 50ms
    tButtonSW1 = now;
    lastSW1 = sw1;
    Serial.print(F("[TOMBOL] SW1 (D2): "));
    Serial.println(sw1 == LOW ? F("DITEKAN") : F("dilepas"));

    if (sw1 == LOW) {
      // Mulai hitung durasi tekan, untuk membedakan tap vs tahan
      tSW1PressStart = now;
      sw1LongPressTriggered = false;
    } else {
      // Dilepas: kalau belum sempat jadi long-press, berarti ini TAP biasa
      if (!sw1LongPressTriggered) {
        irHexFormat = !irHexFormat;
        Serial.print(F("[FORMAT] Kode IR sekarang: "));
        Serial.println(irHexFormat ? F("HEX") : F("DESIMAL"));
      }
    }
  }

  // Deteksi tekan-tahan SW1 (dicek tiap loop selama tombol masih ditekan)
  if (sw1 == LOW && !sw1LongPressTriggered && (now - tSW1PressStart) >= LONG_PRESS_MS) {
    sw1LongPressTriggered = true;
    rgbEnabled = !rgbEnabled;
    Serial.print(F("[RGB] LED RGB sekarang: "));
    Serial.println(rgbEnabled ? F("AKTIF") : F("MATI"));
  }
  if (sw2 != lastSW2 && now - tButtonSW2 > 50) {
    tButtonSW2 = now;
    lastSW2 = sw2;
    Serial.print(F("[TOMBOL] SW2 (D3): "));
    Serial.println(sw2 == LOW ? F("DITEKAN") : F("dilepas"));

    if (sw2 == LOW) {
      // Mulai hitung durasi tekan, untuk membedakan tap vs tahan
      tSW2PressStart = now;
      sw2LongPressTriggered = false;
    } else {
      // Dilepas: kalau belum sempat jadi long-press, berarti ini TAP biasa
      if (!sw2LongPressTriggered) {
        displayMode = !displayMode;
        Serial.print(F("[MODE] Tampilan sekarang: "));
        Serial.println(displayMode == 0 ? F("SUHU") : F("KODE IR"));
      }
    }
  }

  // Deteksi tekan-tahan SW2 (dicek tiap loop selama tombol masih ditekan)
  if (sw2 == LOW && !sw2LongPressTriggered && (now - tSW2PressStart) >= LONG_PRESS_MS) {
    sw2LongPressTriggered = true;
    runBrightnessDiagnostic(); // blocking ~12 detik, sengaja karena cuma dipicu manual
  }

  // ---------- 2. LED1 & LED2 berkedip bergantian (heartbeat) ----------
  if (now - tLedBlink >= 500) {
    tLedBlink = now;
    led1State = !led1State;
    digitalWrite(PIN_LED1, led1State);
    digitalWrite(PIN_LED2, !led1State);
  }

  // ---------- 3. RGB LED berganti warna (bisa dimatikan lewat tahan SW1) ----------
  // Catatan: kalau warnanya kebalik (menyala saat harusnya mati),
  // berarti RGB ini common-anode -> ganti HIGH/LOW di bawah ini.
  if (!rgbEnabled) {
    digitalWrite(PIN_RGB_R, LOW);
    digitalWrite(PIN_RGB_G, LOW);
    digitalWrite(PIN_RGB_B, LOW);
  } else if (now - tRgbCycle >= 800) {
    tRgbCycle = now;
    rgbStep = (rgbStep + 1) % 4;
    digitalWrite(PIN_RGB_R, rgbStep == 0 ? HIGH : LOW);
    digitalWrite(PIN_RGB_G, rgbStep == 1 ? HIGH : LOW);
    digitalWrite(PIN_RGB_B, rgbStep == 2 ? HIGH : LOW);
    // rgbStep == 3 -> RGB mati sejenak
  }

  // ---------- 4. IR Receiver (buzzer bunyi HANYA saat kode IR diterima) ----------
  if (IrReceiver.decode()) {
    lastIRCode = IrReceiver.decodedIRData.decodedRawData;
    hasIRCode = true;
    Serial.print(F("[IR] Kode diterima: 0x"));
    Serial.println(lastIRCode, HEX);

    tone(PIN_BUZZER, 2000, 100); // bip pendek sebagai notifikasi IR diterima

    IrReceiver.resume();
  }

  // ---------- 5. Sensor analog: Potensio, LDR, LM35 (tiap 1 detik) ----------
  float lm35TempC = 0;
  if (now - tSensorRead >= 1000) {
    tSensorRead = now;

    int rotVal   = analogRead(PIN_ROTATION);
    int lightVal = analogRead(PIN_LIGHT);
    int lm35Raw  = analogRead(PIN_LM35);
    lm35TempC = (lm35Raw * 5.0 / 1024.0) * 100.0;

    Serial.println(F("---- Sensor Analog ----"));
    Serial.print(F("Potensio (A0): "));
    Serial.println(rotVal);
    Serial.print(F("Cahaya/LDR (A1): "));
    Serial.println(lightVal);
    Serial.print(F("LM35 (A2): "));
    Serial.print(lm35TempC);
    Serial.println(F(" C"));
  }

  // ---------- 6. DHT11 (maksimal dibaca tiap 2 detik) ----------
  if (now - tDHTRead >= 2000) {
    tDHTRead = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    Serial.println(F("---- DHT11 ----"));
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Gagal membaca DHT11! Cek koneksi/posisi sensor."));
    } else {
      Serial.print(F("Suhu: "));
      Serial.print(t);
      Serial.print(F(" C, Kelembapan: "));
      Serial.print(h);
      Serial.println(F(" %"));
    }
  }

  // ---------- 7. Update layar 4-digit TM1650 sesuai mode ----------
  if (now - tDisplayUpdate >= 300) {
    tDisplayUpdate = now;

    if (displayMode == 0) {
      // Mode suhu: baca LM35 langsung supaya responsif (tidak perlu tunggu 2 detik seperti DHT11)
      int lm35Raw = analogRead(PIN_LM35);
      float tempC = (lm35Raw * 5.0 / 1024.0) * 100.0;
      display.setDisplayToDecNumber((int)tempC, 0, true); // contoh: suhu 24C -> "0024"
    } else {
      if (hasIRCode) {
        if (irHexFormat) {
          // Tampilkan 4 digit heksa terakhir dari kode IR, contoh: 0xFFA25D -> "A25D"
          char hexBuf[5];
          sprintf(hexBuf, "%04X", (unsigned int)(lastIRCode & 0xFFFF));
          display.setDisplayToString(hexBuf);
        } else {
          // Tampilkan 4 digit desimal terakhir dari kode IR, contoh: 4293848669 -> "8669"
          display.setDisplayToDecNumber((int)(lastIRCode % 10000UL), 0, true);
        }
      } else {
        // Belum ada kode IR yang diterima -> tampilkan strip "----"
        display.setDisplayToString("----");
      }
    }
  }
}
