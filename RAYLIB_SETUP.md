# Raylib Kurulum Rehberi (Windows MinGW)

## Hızlı Kurulum

### Yöntem 1: Pre-compiled Library (Önerilen)

1. **Raylib'i İndirin:**
   - https://github.com/raysan5/raylib/releases adresinden en son release'i indirin
   - `raylib-5.x.x_win64_mingw-w64.zip` dosyasını indirin

2. **Klasör Yapısı:**
   ```
   D:\Real-Time Game Server ECS (FPS-lite Arena)/
   ├── raylib/
   │   ├── include/
   │   │   └── raylib.h
   │   └── lib/
   │       └── libraylib.a
   ```

3. **Alternatif: Header-Only (Daha Kolay)**
   - `raylib.h` dosyasını `raylib/` klasörüne koyun
   - `RAYLIB_STANDALONE` define edin (header-only mode)

### Yöntem 2: Manuel Derleme

Eğer pre-compiled yoksa, kendiniz derleyebilirsiniz (daha karmaşık).

## Test

Kurulumdan sonra `build-gameclient.bat` çalıştırın.

