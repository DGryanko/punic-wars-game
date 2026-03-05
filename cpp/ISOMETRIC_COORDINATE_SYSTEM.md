# Ізометрична система координат

## Константи тайлів (CoordinateConverter)

```cpp
TILE_WIDTH = 128        // Повна ширина тайлу в пікселях
TILE_HEIGHT = 64        // Повна висота тайлу в пікселях
TILE_WIDTH_HALF = 64    // Половина ширини (128/2)
TILE_HEIGHT_HALF = 32   // Половина висоти (64/2)
```

## Важливо!

- **Горизонтальний розмір тайлу**: 128 пікселів (TILE_WIDTH)
- **Вертикальний розмір тайлу**: 64 пікселі (TILE_HEIGHT)
- **НЕ ПЛУТАТИ**: TILE_HEIGHT_HALF = 32 - це ПОЛОВИНА висоти, не повна!

## Конвертація координат

### Grid → Screen
```cpp
screenX = (col - row) * TILE_WIDTH_HALF   // (col - row) * 64
screenY = (col + row) * TILE_HEIGHT_HALF  // (col + row) * 32
```

### Screen → Grid
```cpp
col = (screenX / TILE_WIDTH_HALF + screenY / TILE_HEIGHT_HALF) / 2
row = (screenY / TILE_HEIGHT_HALF - screenX / TILE_WIDTH_HALF) / 2
```

## Footprint будівель

Всі будівлі мають footprint **2x2 тайли**:
- Ширина в пікселях: `2 * TILE_WIDTH_HALF = 128` пікселів
- Висота в пікселях: `2 * TILE_HEIGHT_HALF = 64` пікселі

## Позиціювання спрайтів

### IsometricSprite anchor point
- Спрайти мають anchor в **нижньому центрі**: `{width/2, height}`
- При малюванні: `drawPos = position - anchorPoint`

### Building offset
- Спрайти будівель (384x224) малюються з anchor в нижньому центрі
- Область кліку/колізії зміщена вгору на **TILE_HEIGHT (64 пікселі)**
- Це вирівнює інтерактивну область з основою спрайту

```cpp
float offsetY = -CoordinateConverter::TILE_HEIGHT;  // -64 пікселів
```

## Ромб для кліку

Формула перевірки точки всередині ізометричного ромба:
```cpp
|localX / halfWidth| + |localY / halfHeight| <= 1.0
```

Де:
- `halfWidth = footprint.col * TILE_WIDTH_HALF`
- `halfHeight = footprint.row * TILE_HEIGHT_HALF`
- `localX = mouseX - buildingX`
- `localY = mouseY - (buildingY + offsetY)`

## Типові помилки

❌ **НЕ РОБИТИ**: Використовувати хардкоджені значення (44, 22, 32)
✅ **РОБИТИ**: Використовувати константи з CoordinateConverter

❌ **НЕ РОБИТИ**: Плутати TILE_HEIGHT_HALF (32) з TILE_HEIGHT (64)
✅ **РОБИТИ**: Для повної висоти використовувати TILE_HEIGHT

❌ **НЕ РОБИТИ**: Змінювати anchorPoint або додавати offset в IsometricSprite
✅ **РОБИТИ**: Зміщувати області кліку/колізії в Building класі
