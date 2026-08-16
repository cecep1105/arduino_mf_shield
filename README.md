  Sketch Uji Coba Semua Sensor - Multi-Function Shield B04505
  + Display 4-Digit TM1637 (modul terpisah)
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
 
  DISPLAY TAMBAHAN (modul terpisah, TM1637 4-Digit Display):
    Sambungkan modul display ke header "GND VCC SDA SCL" di pojok
    kiri atas shield pakai kabel jumper:
      Modul GND -> Shield GND
      Modul VCC -> Shield VCC
      Modul CLK -> Shield SCL
      Modul DIO -> Shield SDA
    (Header ini terhubung ke pin hardware I2C Arduino UNO, yaitu
     SDA = A4 dan SCL = A5. TM1637 bukan sensor I2C sesungguhnya,
     tapi pin ini bisa dipakai sebagai jalur CLK/DIO biasa.)
 
    Cara kerja tombol:
      - TAP SW2 (tekan-lepas cepat) -> ganti mode tampilan:
        SUHU <-> KODE IR (bergantian).
      - TAP SW1 (tekan-lepas cepat, < 0.7 detik) -> ganti format
        angka kode IR: HEX <-> DESIMAL. (Format ini cuma kelihatan
        efeknya waktu mode tampilan lagi di KODE IR, tapi bisa
        diganti kapan saja.)
      - TAHAN SW1 (>= 0.7 detik) -> nyala/matikan RGB LED.
        Ini berguna untuk tes apakah RGB LED yang nyala bareng-bareng
        bikin layar TM1637 jadi redup/berkedip (dugaan drop tegangan
        5V pas komponen lain aktif bersamaan) -- matikan RGB LED lalu
        lihat apakah layarnya jadi lebih stabil/terang.
      - Buzzer HANYA bunyi saat IR Receiver berhasil menerima kode
        dari remote (bukan bunyi berkala lagi).
 
  LIBRARY YANG HARUS DI-INSTALL (via Library Manager):
    1. "DHT sensor library" by Adafruit   (+ dependency "Adafruit Unified Sensor")
    2. "IRremote" by Armin Joachimsmeyer  (versi 4.x)
    3. "TM1637Display" by Avishay Orpaz
 
  Cara pakai:
    - Upload sketch ini, lalu buka Serial Monitor, baud rate 9600.
    - Tap SW2 untuk pindah antara tampilan suhu dan tampilan kode IR.
    - Tap SW1 untuk ganti format kode IR antara HEX dan DESIMAL.
    - Tahan SW1 sebentar (>= 0.7 detik) untuk mati/nyalakan RGB LED.
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
    menghentikan interrupt sehingga jalur bit-bang CLK/DIO ke TM1637
    sedikit terganggu.
  ============================================================