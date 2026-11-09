# PEPEL Glue — Master Glue Compressor / Saturator / Exciter

Плагин для мастер-канала: компрессия + сатурация + эксайтер с макро-интенсивностью и пресетами (Clean / Glue / Dirty / Air). Интерфейс в стиле пастельного 8-бит аниме + VHS.

## Системные требования
- Windows 10/11 (VST3), macOS 10.14+ (VST3 / AU), Linux (VST3)
- FL Studio 20.9+ (VST3)

## Как собрать локально
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```
Артефакты появятся в `build/PEPEL_artefacts/Release/`.

## Тест DSP (необязательно)
```bash
cmake -B build -DPEPEL_BUILD_TESTS=ON
cmake --build build --target PEPELDspTest
./build/PEPELDspTest_artefacts/Release/PEPELDspTest
```

## Режим скриншота (Standalone)
```bash
./PEPEL --screenshot=shot.png
```

## Параметры
- **Threshold / Ratio / Attack / Release / Knee / Makeup** — компрессор.
- **Drive / Character / Sat Mix** — сатуратор (tanh + чётные гармоники).
- **Exciter / Exc Freq** — воздушный эксайтер.
- **HP Filter** — фильтр низких частот на входе.
- **Mix** — dry/wet на выходе.
- **Intensity** — макро-ручка: чем выше, тем сильнее пресет "влияет" на звук.
- **Preset** — мгновенно задаёт все параметры под выбранный стиль.

## Как загрузить на GitHub (через браузер)
1. Зайди на [github.com/new](https://github.com/new) и создай публичный репозиторий, например `PEPEL-Glue`.
2. На странице репозитория нажми **"Add file" → "Upload files"**.
3. Перетащи **всё содержимое папки PEPEL-Glue** (CMakeLists.txt, Source/, Assets/, .github/ и т.д.) в окно загрузки. Важно: папку `.github` тоже загрузи (она может быть скрытой — включи отображение скрытых файлов).
4. Нажми **"Commit changes"**.
5. Перейди во вкладку **Actions** — убедись, что Actions включены (GitHub спросит разрешение при первом запуске — нажми **"I understand my workflows, go ahead and enable them"**).
6. Через несколько минут появится зелёный чек. Нажми на workflow-run → внизу раздел **Artifacts** → скачай архив `PEPEL-windows-latest` (или mac/linux).
7. Распакуй. Внутри `VST3/PEPEL Glue.vst3` — скопируй в `C:\Program Files\Common Files\VST3\` (Windows) или `~/Library/Audio/Plug-Ins/VST3/` (macOS).
8. В FL Studio зайди **Options → Manage plugins → Find plugins**.

## Лицензии шрифтов
- VT323 — [OFL](Assets/OFL-VT323.txt)
- Press Start 2P — [OFL](Assets/OFL-PressStart2P.txt)
- DotGothic16 — [OFL](Assets/OFL-DotGothic16.txt) (только как источник пиксельных глифов 灰愛 в шапке; сам шрифт в плагин не вшит)
