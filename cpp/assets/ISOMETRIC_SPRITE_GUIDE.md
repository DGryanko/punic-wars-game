# 🎨 Isometric Sprite Creation Guide

## Overview

Цей документ описує як створювати ізометричні спрайти для гри "Punic Wars: Castra".

## 📐 Isometric Projection Basics

### Ізометрична проекція (2:1 ratio)
```
        Top
         /\
        /  \
  Left /    \ Right
      /      \
      \      /
       \    /
        \  /
         \/
       Bottom
```

- **Кут**: 26.565° (arctg(0.5))
- **Ratio**: Ширина : Висота = 2 : 1
- **Tile size**: 64x32 пікселі

## 🎮 Unit Sprites

### Технічні вимоги

**Розмір файлу:**
- Рекомендований: **64x64 пікселі**
- Мінімальний: 32x32 пікселі
- Максимальний: 128x128 пікселі

**Формат:**
- PNG з прозорістю (альфа-канал)
- 32-bit RGBA

**Anchor Point:**
- Нижній центр спрайту
- Координати: (32, 64) для 64x64 спрайту
- Це точка, де юніт "стоїть" на тайлі

### Розміри юнітів

```
┌────────────────┐
│                │  ← 64px ширина
│     UNIT       │
│    SPRITE      │
│       ↓        │
└───────●────────┘  ← 64px висота
        ↑
   Anchor point (32, 64)
```

### Приклад структури юніта

**Легіонер (Legionary):**
- Висота фігури: ~48 пікселів
- Ширина фігури: ~32 пікселі
- Щит та зброя: в межах 64x64
- Тінь: опціонально, під ногами

**Раб (Slave):**
- Висота фігури: ~40 пікселів
- Ширина фігури: ~24 пікселі
- Простіша деталізація

**Фінікієць (Phoenician):**
- Висота фігури: ~48 пікселів
- Ширина фігури: ~32 пікселі
- Інша зброя та обладунки

### Напрямки (опціонально)

Для кращої анімації можна створити 8 напрямків:
```
  NW    N    NE
    \   |   /
  W --  ●  -- E
    /   |   \
  SW    S    SE
```

Назви файлів:
- `legionary_rome_n.png`
- `legionary_rome_ne.png`
- `legionary_rome_e.png`
- і т.д.

Якщо тільки один спрайт - використовується для всіх напрямків.

### Колірна схема

**Рим (Rome):**
- Червоний плащ (#DC143C)
- Золоті акценти (#FFD700)
- Сталевий обладунок (#C0C0C0)

**Карфаген (Carthage):**
- Фіолетовий/пурпурний (#8B008B)
- Золоті акценти (#FFD700)
- Бронзовий обладунок (#CD7F32)

**Раби (Slaves):**
- Коричневі/бежеві тони (#8B4513, #DEB887)
- Прості тканини

## 🏛️ Building Sprites

### Розміри будівель

Будівлі займають кілька тайлів. Розмір спрайту залежить від footprint:

**1x1 tile building:**
- Розмір: **128x96 пікселів**
- Footprint: 1 тайл (64x32)
- Висота: +64 пікселі для вертикального об'єму

**2x2 tile building:**
- Розмір: **256x160 пікселів**
- Footprint: 4 тайли (128x64)
- Висота: +96 пікселі для вертикального об'єму

**3x3 tile building:**
- Розмір: **384x224 пікселів**
- Footprint: 9 тайлів (192x96)
- Висота: +128 пікселі для вертикального об'єму

### Формула розміру

```
Width = footprint_cols * 64 + footprint_rows * 64
Height = footprint_cols * 32 + footprint_rows * 32 + vertical_height
```

Де:
- `footprint_cols` - кількість тайлів по горизонталі
- `footprint_rows` - кількість тайлів по вертикалі
- `vertical_height` - висота будівлі (64-128px)

### Anchor Point

Нижній центр основи будівлі:
```
┌─────────────────────┐
│                     │
│     BUILDING        │
│                     │
│        ╱╲           │
│       ╱  ╲          │
│      ╱____╲         │
└──────────●──────────┘
           ↑
      Anchor point
```

### Типи будівель

**Praetorium (HQ Rome) - 3x3:**
- Великий римський намет з орлом
- Червоно-золота колірна схема
- Розмір: 384x224

**Main Tent (HQ Carthage) - 3x3:**
- Фінікійський намет
- Фіолетово-золота колірна схема
- Розмір: 384x224

**Contubernium (Barracks Rome) - 2x2:**
- Римська казарма
- Розмір: 256x160

**Mercenary Camp (Barracks Carthage) - 2x2:**
- Табір найманців
- Розмір: 256x160

**Questorium (Storage Rome) - 2x2:**
- Склад з амфорами
- Розмір: 256x160

## 📦 Resource Point Sprites

### Розміри

**Розмір:** 48x48 пікселів (менше за юніта)

**Типи:**
- Food Source (їжа) - зелений
- Gold Source (золото) - жовтий

### Приклад

```
┌──────────┐
│   🌾     │  Food - колоски пшениці
│          │
└─────●────┘

┌──────────┐
│   💰     │  Gold - купа монет
│          │
└─────●────┘
```

## 🎨 Debug Mode Shapes

Коли спрайти не завантажені, система малює прості фігури:

### Debug Units (діаманти)

```cpp
// Розмір: 32x16 пікселів (половина тайлу)
// Форма: ізометричний ромб

     (16,0)
       /\
      /  \
(0,8)/    \(32,8)
     \    /
      \  /
       \/
    (16,16)
```

**Кольори:**
- Rome units: RED (#FF0000)
- Carthage units: BLUE (#0000FF)
- Slaves: YELLOW (#FFFF00)

### Debug Buildings (прямокутники)

```cpp
// Розмір: залежить від footprint
// Форма: ізометричний прямокутник

1x1: 64x32
2x2: 128x64
3x3: 192x96
```

**Кольори:**
- HQ: DARKGRAY (#404040)
- Barracks: BROWN (#8B4513)
- Storage: ORANGE (#FFA500)

### Debug Resources (квадрати)

```cpp
// Розмір: 24x24 пікселів
// Форма: квадрат з літерою

┌────┐
│ F  │  Food - GREEN
└────┘

┌────┐
│ G  │  Gold - GOLD
└────┘
```

## 📁 File Structure

```
cpp/assets/sprites/isometric/
├── units/
│   ├── legionary_rome.png
│   ├── phoenician_carthage.png
│   ├── slave.png
│   └── ... (8 directional variants optional)
├── buildings/
│   ├── hq_rome.png
│   ├── hq_carthage.png
│   ├── barracks_rome.png
│   ├── barracks_carthage.png
│   └── questorium_rome.png
└── resources/
    ├── food_source.png
    └── gold_source.png
```

## 🛠️ Creation Tools

### Recommended Software

**Pixel Art:**
- Aseprite (paid, best for pixel art)
- Piskel (free, web-based)
- GraphicsGale (free)

**Vector/Rendered:**
- Blender (3D → render to isometric)
- Inkscape (vector graphics)
- GIMP (raster graphics)

### Aseprite Workflow

1. Create new sprite: 64x64 pixels
2. Set up isometric grid (optional)
3. Draw character facing SE (default direction)
4. Add shading for depth
5. Export as PNG with transparency

### Blender Workflow

1. Model 3D character
2. Set camera to isometric (orthographic, 45° rotation)
3. Render at 64x64 or higher
4. Export with transparent background
5. Scale down if needed

## ✅ Quality Checklist

- [ ] PNG format with transparency
- [ ] Correct dimensions (64x64 for units, calculated for buildings)
- [ ] Anchor point at bottom center
- [ ] Transparent background (no white/black)
- [ ] Isometric perspective (2:1 ratio)
- [ ] Readable at game zoom levels (0.3x - 2.0x)
- [ ] Consistent art style across all sprites
- [ ] Faction colors clearly visible
- [ ] No anti-aliasing artifacts on edges (for pixel art)

## 🎯 Testing Your Sprites

1. Place sprite in correct folder
2. Run game
3. Check console for "[SPRITE] Loaded: filename.png"
4. Verify sprite appears at correct position
5. Test at different zoom levels
6. Check depth sorting with other objects

## 📝 Naming Convention

```
{type}_{faction}_{direction}.png

Examples:
- legionary_rome.png
- legionary_rome_n.png (with direction)
- hq_carthage.png
- barracks_rome.png
- slave.png (no faction)
- food_source.png
```

## 🎨 Example Palette

**Rome:**
```
#DC143C - Crimson (cloaks)
#FFD700 - Gold (accents)
#C0C0C0 - Silver (armor)
#8B4513 - Saddle Brown (leather)
```

**Carthage:**
```
#8B008B - Dark Magenta (purple)
#FFD700 - Gold (accents)
#CD7F32 - Bronze (armor)
#4B0082 - Indigo (cloth)
```

**Neutral:**
```
#8B4513 - Saddle Brown (wood)
#DEB887 - Burlywood (cloth)
#696969 - Dim Gray (stone)
#228B22 - Forest Green (grass)
```

## 💡 Tips

1. **Start simple** - Create basic shapes first, add details later
2. **Use references** - Look at real Roman/Carthaginian art
3. **Test early** - Put sprites in game as soon as possible
4. **Consistent lighting** - Light from top-left (standard isometric)
5. **Readable silhouettes** - Should be recognizable even at small size
6. **Faction distinction** - Make sure Rome and Carthage are clearly different

## 🚀 Quick Start

**Minimum viable sprites:**
1. One unit sprite per faction (3 total)
2. One building sprite per type (5 total)
3. Two resource sprites (2 total)

**Total: 10 sprites to get started!**

The rest can use debug mode until you create them.

---

**Questions?** Check the code in `src/unit.h`, `src/building.h` for implementation details.

**Happy sprite creating! 🎨✨**
