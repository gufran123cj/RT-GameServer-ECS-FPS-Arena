# 🎮 Top-Down 2D Game Client - Raylib Entegrasyonu

## 📋 Özet

Projeye **top-down 2D görselleştirme** eklendi. Raylib kullanarak basit bir 2D client oluşturuldu.

## 🚀 Hızlı Başlangıç

### 1. Raylib Kurulumu

**Yöntem 1: Pre-compiled (Önerilen)**

1. https://github.com/raysan5/raylib/releases adresinden en son release'i indirin
2. `raylib-5.x.x_win64_mingw-w64.zip` dosyasını açın
3. `include/raylib.h` dosyasını `raylib/` klasörüne kopyalayın
4. `lib/libraylib.a` dosyasını MinGW lib klasörüne kopyalayın (veya build script'te path belirtin)

**Yöntem 2: Header-Only (Daha Kolay)**

1. `raylib.h` dosyasını `raylib/` klasörüne koyun
2. `RAYLIB_STANDALONE` define edin (build script'te zaten var)

### 2. Derleme

```bash
build-gameclient.bat
```

### 3. Çalıştırma

```bash
GameClient.exe
# Veya özel server adresi:
GameClient.exe 127.0.0.1 7777
```

## 🎨 Özellikler

- ✅ **Top-down 2D görünüm** - Yukarıdan bakış açısı
- ✅ **Grid sistemi** - Dünya grid'i görselleştirme
- ✅ **Player rendering** - Renkli player circle'ları
- ✅ **Yön göstergesi** - Player'ların baktığı yön
- ✅ **Camera kontrolü** - Zoom in/out (+/- tuşları)
- ✅ **Real-time güncelleme** - Server'dan snapshot alımı
- ✅ **UI overlay** - Server bilgileri, tick, player sayısı

## 🎮 Kontroller

- **+ / =** : Zoom in
- **- / _** : Zoom out
- **ESC** : Çıkış

## 📁 Dosya Yapısı

```
src/
├── GameClient.cpp          # Top-down 2D client (Raylib)
├── TestClient.cpp          # Basit test client
└── MiniGameViewer.cpp      # ASCII map viewer

raylib/
└── raylib.h                # Raylib header (siz indirmelisiniz)

build-gameclient.bat        # Game client build script
```

## 🔧 Build Script Ayarları

Eğer Raylib library farklı bir yerdeyse, `build-gameclient.bat` dosyasını düzenleyin:

```batch
-Lraylib/lib                # Library path
-lraylib                    # Library name
```

## ⚠️ Notlar

- Raylib header dosyasını manuel olarak indirmeniz gerekiyor
- MinGW ile uyumlu pre-compiled library kullanın
- Eğer derleme hatası alırsanız, Raylib kurulumunu kontrol edin

## 🐛 Sorun Giderme

### "raylib.h: No such file"
- `raylib/raylib.h` dosyasının var olduğundan emin olun
- Build script'te `-Iraylib` flag'inin olduğunu kontrol edin

### Linker hataları
- `libraylib.a` dosyasının doğru yerde olduğundan emin olun
- MinGW lib klasörüne kopyalayın veya `-L` flag'i ile path belirtin

### OpenGL hataları
- `-lopengl32 -lgdi32 -lwinmm` flag'lerinin eklendiğinden emin olun

