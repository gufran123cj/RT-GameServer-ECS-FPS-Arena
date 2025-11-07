# MinGW Versiyon Düşürme Rehberi

## 🎯 Amaç
MinGW 15.2.0'dan 13.2.0 veya 14.2.0'a geçiş yaparak C++20 standard library uyumsuzluk sorunlarını çözmek.

## ⚠️ ÖNEMLİ: Yedekleme

Önce mevcut MinGW'yi yedekleyin:

```powershell
# PowerShell'de (Yönetici olarak çalıştırın)
Copy-Item -Path "D:\MinGW" -Destination "D:\MinGW_backup_15.2.0" -Recurse
```

## 📥 Adım 1: Yeni MinGW İndirme

### Seçenek A: WinLibs (Önerilen - En Kolay)

1. **Web sitesine gidin:**
   - https://winlibs.com/ adresine gidin

2. **Doğru versiyonu seçin:**
   - **UCRT runtime** seçeneğini işaretleyin
   - **Release** seçeneğini seçin
   - **64-bit** seçeneğini seçin
   - **13.2.0** veya **14.2.0** versiyonunu seçin
   
   Örnek dosya adı:
   - `mingw-w64-ucrt-x86_64-13.2.0-16.0.6-11.0.0-ucrt-r1.zip`
   - veya `mingw-w64-ucrt-x86_64-14.2.0-16.0.6-11.0.0-ucrt-r1.zip`

3. **İndirme:**
   - ZIP dosyasını indirin (yaklaşık 200-300 MB)

### Seçenek B: MSYS2 (Paket Yöneticili)

Eğer MSYS2 kullanıyorsanız:

```bash
# MSYS2 UCRT64 terminalinde:
pacman -S mingw-w64-ucrt-x86_64-gcc=13.2.0
# veya
pacman -S mingw-w64-ucrt-x86_64-gcc=14.2.0
```

## 🔧 Adım 2: Kurulum

### WinLibs ile Kurulum:

1. **Mevcut MinGW'yi kaldırın veya yeniden adlandırın:**
   ```powershell
   # PowerShell'de (Yönetici olarak)
   Rename-Item -Path "D:\MinGW" -NewName "D:\MinGW_old_15.2.0"
   ```

2. **Yeni MinGW'yi kurun:**
   - İndirdiğiniz ZIP dosyasını açın
   - İçindeki `mingw64` klasörünü `D:\MinGW` olarak çıkarın
   - VEYA içeriği doğrudan `D:\MinGW` klasörüne çıkarın

3. **Klasör yapısını kontrol edin:**
   ```
   D:\MinGW\
   ├── bin\
   │   ├── g++.exe
   │   ├── gcc.exe
   │   └── ...
   ├── include\
   ├── lib\
   └── ...
   ```

## 🔄 Adım 3: PATH Ayarları

1. **Windows Ortam Değişkenlerini açın:**
   - Windows tuşu + R → `sysdm.cpl` yazın → Enter
   - "Gelişmiş" sekmesi → "Ortam Değişkenleri"

2. **PATH'i düzenleyin:**
   - "Sistem değişkenleri" altında `Path`'i seçin → "Düzenle"
   - Eski MinGW path'ini bulun (varsa): `D:\MinGW\bin` veya `D:\MinGW_old_15.2.0\bin`
   - Eski path'i silin veya düzenleyin
   - "Yeni" → `D:\MinGW\bin` yazın → Tamam
   - Tüm pencereleri kapatın

3. **Yeni terminal açın:**
   - VS Code'u tamamen kapatın
   - Yeni bir PowerShell/CMD penceresi açın

## ✅ Adım 4: Doğrulama

Yeni terminalde versiyonu kontrol edin:

```powershell
g++ --version
```

Beklenen çıktı:
```
g++.exe (MinGW-W64 x86_64-ucrt-posix-seh) 13.2.0
# veya
g++.exe (MinGW-W64 x86_64-ucrt-posix-seh) 14.2.0
```

## 🧪 Adım 5: Derleme Testi

Projeyi derleyin:

```powershell
cd "d:\Real-Time Game Server + ECS (FPS-lite  Arena)"
.\build.bat
```

VEYA CMake ile:

```powershell
cd "d:\Real-Time Game Server + ECS (FPS-lite  Arena)"
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path "build" | Out-Null
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

## 🔍 Sorun Giderme

### "g++ bulunamadı" hatası:
- PATH'e `D:\MinGW\bin` eklendiğinden emin olun
- VS Code'u tamamen kapatıp yeniden açın
- Yeni terminal penceresi açın

### Hala 15.2.0 görünüyor:
- Tüm terminal pencerelerini kapatın
- VS Code'u tamamen kapatın
- Bilgisayarı yeniden başlatın (gerekirse)
- Yeni terminal açıp tekrar kontrol edin

### Derleme hataları devam ediyor:
- `build` klasörünü silin ve CMake'i yeniden çalıştırın
- `build.bat` dosyasındaki `MINGW_PATH` değişkenini kontrol edin

## 📝 Notlar

- **13.2.0** daha stabil ve test edilmiş
- **14.2.0** daha yeni özellikler içerir ama biraz daha az test edilmiş olabilir
- Her iki versiyon da C++20'yi destekler
- WinLibs versiyonları Brecht Sanders tarafından derlenmiştir

## 🔄 Geri Dönüş

Eğer sorun yaşarsanız ve eski versiyona dönmek isterseniz:

```powershell
# Yeni MinGW'yi kaldırın
Remove-Item -Recurse -Force "D:\MinGW"

# Eski versiyonu geri yükleyin
Rename-Item -Path "D:\MinGW_old_15.2.0" -NewName "D:\MinGW"
```

---

**Son Güncelleme:** 2025-01-XX
**Hazırlayan:** AI Assistant

