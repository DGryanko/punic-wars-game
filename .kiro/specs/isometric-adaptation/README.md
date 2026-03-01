# Isometric Game Adaptation Specification

## Quick Links

- 📋 [Requirements](requirements.md) - 12 functional requirements
- 🏗️ [Design](design.md) - Architecture and implementation details
- ✅ [Tasks](tasks.md) - 13 implementation tasks
- 📊 [Summary](SUMMARY.md) - Quick overview
- 🎨 [Sprite Guide](../../cpp/assets/ISOMETRIC_SPRITE_GUIDE.md) - For artists

## What This Spec Covers

Повна адаптація гри для ізометричної системи координат:

1. **Grid-based coordinates** - Всі об'єкти використовують GridCoords
2. **Isometric rendering** - Спрайти або debug фігури
3. **Depth sorting** - Правильний порядок малювання
4. **Mouse interaction** - Клік та вибір в grid координатах
5. **Sprite system** - Завантаження з fallback до debug режиму

## Current Status

✅ Requirements complete (12 requirements)
✅ Design complete (architecture defined)
✅ Tasks complete (13 tasks)
✅ Sprite guide complete (for artists)
⏳ Implementation not started

## How to Use This Spec

### For Developers

1. Read [requirements.md](requirements.md) to understand what we're building
2. Read [design.md](design.md) for architecture and code structure
3. Follow [tasks.md](tasks.md) for step-by-step implementation
4. Refer to [SUMMARY.md](SUMMARY.md) for quick reference

### For Artists

1. Read [ISOMETRIC_SPRITE_GUIDE.md](../../cpp/assets/ISOMETRIC_SPRITE_GUIDE.md)
2. Follow sprite specifications (sizes, formats, anchors)
3. Place sprites in `cpp/assets/sprites/isometric/`
4. Game will automatically load them or use debug mode

### For Project Managers

1. Read [SUMMARY.md](SUMMARY.md) for overview
2. Estimated time: 9-15 hours
3. Can be done in 6 phases
4. Debug mode allows testing without sprites

## Key Concepts

### Grid Coordinates
```cpp
struct GridCoords {
    int row;  // Y-axis in grid
    int col;  // X-axis in grid
};
```

### Isometric Conversion
```cpp
// Grid → Screen
screenX = (col - row) * 64
screenY = (col + row) * 32

// Screen → Grid
col = (screenX / 64 + screenY / 32) / 2
row = (screenY / 32 - screenX / 64) / 2
```

### Debug Mode

When sprites not found, render colored shapes:
- **Units:** Diamonds (32x16) - RED/BLUE/YELLOW
- **Buildings:** Rectangles (64x32+) - GRAY/BROWN/ORANGE
- **Resources:** Squares (24x24) - GREEN/GOLD

## Dependencies

- Existing tilemap system (already implemented)
- CoordinateConverter (already exists)
- Raylib rendering (already available)

## Testing Strategy

1. **Unit tests** - Coordinate conversion, collision detection
2. **Integration tests** - Click-to-select, movement, rendering
3. **Manual tests** - Visual verification, performance

## Implementation Order

1. Core coordinate system (highest priority)
2. Debug rendering (enables testing)
3. Sprite system (polish)
4. Depth sorting (visual quality)
5. Mouse interaction (gameplay)
6. Polish and optimization (final)

## Success Criteria

✅ All objects use grid coordinates
✅ Debug mode works for all object types
✅ Sprites load and render correctly
✅ Objects render in correct depth order
✅ Mouse clicks select correct objects
✅ Game runs at 60 FPS
✅ No visual glitches or z-fighting

## Questions?

- Check [design.md](design.md) for technical details
- Check [ISOMETRIC_SPRITE_GUIDE.md](../../cpp/assets/ISOMETRIC_SPRITE_GUIDE.md) for sprite specs
- Check existing tilemap code in `cpp/src/tilemap/`

---

**Let's build an isometric RTS! 🎮🗺️**
