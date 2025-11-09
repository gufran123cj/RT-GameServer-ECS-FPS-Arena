# Raylib Header Dosyası

## Kurulum

1. **Raylib'i İndirin:**
   - https://github.com/raysan5/raylib/releases
   - En son release'i indirin (örn: `raylib-5.0.0_win64_mingw-w64.zip`)

2. **Header Dosyasını Kopyalayın:**
   - İndirdiğiniz zip'ten `raylib.h` dosyasını bu klasöre kopyalayın
   - Veya `include/raylib.h` dosyasını buraya kopyalayın

3. **Library Dosyasını Kopyalayın (Opsiyonel):**
   - Eğer static library kullanacaksanız, `lib/libraylib.a` dosyasını MinGW lib klasörüne kopyalayın
   - Veya `-Lraylib/lib` flag'i ile path belirtin

## Alternatif: Header-Only Mode

Raylib header-only mode'da da çalışabilir. `RAYLIB_STANDALONE` define edin.

## Test

Kurulumdan sonra `build-gameclient.bat` çalıştırın.

