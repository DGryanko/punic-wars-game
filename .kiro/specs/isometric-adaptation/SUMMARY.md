# Isometric Adaptation - Summary

## What We're Doing

Адаптуємо всю гру "Punic Wars: Castra" для роботи з ізометричною системою координат. Всі юніти, будівлі та ресурси будуть використовувати grid координати замість екранних, з підтримкою ізометричних спрайтів та debug режиму.

## Key Changes

### 1. Coordinate System
- **Before:** `float x, y` (screen pixels)
- **After:** `GridCoords {row, col}` (grid cells)
- **Conversion:** Use `CoordinateConverter::gridToScreen()` for rendering

### 2. Rendering System
- **Sprites:** Load from `assets/sprites/isometric/`
- **Debug Mode:** Colored shapes when sprites not found
  - Units: Diamonds (32x16)
  - Buildings: Rectangles (64x32+)
  - Resources: Squares (24x24)

### 3. Depth Sorting
- **New:** RenderQueue sorts objects by row+col
- **Order:** Tilemap → Objects (sorted) → UI

### 4. Mouse Interaction
- **Clicks:** Convert screen → grid coordinates
- **Selection:** Find objects at grid position

## Files Created

### Specification
- `.kiro/specs/isometric-adaptation/requirements.md` - 12 requirements
- `.kiro/specs/isometric-adaptation/design.md` - Architecture and components
- `.kiro/specs/isometric-adaptation/tasks.md` - 13 implementation tasks

### Documentation
- `cpp/assets/ISOMETRIC_SPRITE_GUIDE.md` - Guide for artists
  - Sprite dimensions and formats
  - Anchor points and positioning
  - Debug mode specifications
  - File naming conventions

## Implementation Phases

1. **Core System** (2-3h) - Grid coordinates in all classes
2. **Debug Rendering** (1-2h) - Colored shapes for testing
3. **Sprite System** (2-3h) - Load and render sprites
4. **Depth Sorting** (1-2h) - Correct rendering order
5. **Mouse Interaction** (1-2h) - Grid-based selection
6. **Polish** (2-3h) - Testing and optimization

**Total: 9-15 hours**

## Debug Mode Specifications

### Units (Diamonds)
```
Size: 32x16 pixels
Colors:
  - Rome: RED (#FF0000)
  - Carthage: BLUE (#0000FF)
  - Slaves: YELLOW (#FFFF00)
Shape: Isometric diamond
Label: Unit type above shape
```

### Buildings (Rectangles)
```
Size: Based on footprint
  - 1x1: 64x32
  - 2x2: 128x64
  - 3x3: 192x96
Colors:
  - HQ: DARKGRAY
  - Barracks: BROWN
  - Storage: ORANGE
Label: Building type
```

### Resources (Squares)
```
Size: 24x24 pixels
Colors:
  - Food: GREEN with "F"
  - Gold: GOLD with "G"
```

## Sprite Specifications

### Units
- **Size:** 64x64 pixels (recommended)
- **Format:** PNG with transparency
- **Anchor:** Bottom center (32, 64)
- **Path:** `assets/sprites/isometric/units/{type}_{faction}.png`

### Buildings
- **Size:** Depends on footprint
  - 1x1: 128x96
  - 2x2: 256x160
  - 3x3: 384x224
- **Format:** PNG with transparency
- **Anchor:** Bottom center
- **Path:** `assets/sprites/isometric/buildings/{type}_{faction}.png`

### Resources
- **Size:** 48x48 pixels
- **Format:** PNG with transparency
- **Path:** `assets/sprites/isometric/resources/{type}_source.png`

## Benefits

✅ **Consistent coordinate system** - Everything uses grid
✅ **Proper depth sorting** - No z-fighting
✅ **Debug mode** - Test without sprites
✅ **Incremental sprites** - Add sprites one at a time
✅ **Better pathfinding** - Grid-aligned movement
✅ **Easier collision** - Grid-based checks

## Next Steps

1. Review requirements and design
2. Start with Phase 1 (Core System)
3. Test with debug rendering
4. Add sprites incrementally
5. Polish and optimize

## Documentation

- **For Developers:** See `design.md` and `tasks.md`
- **For Artists:** See `ISOMETRIC_SPRITE_GUIDE.md`
- **For Testing:** See test checklist in `design.md`

---

**Ready to start implementation!** 🚀
