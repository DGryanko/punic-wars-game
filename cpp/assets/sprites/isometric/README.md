# Isometric Sprite Guide

This directory contains isometric sprites for the game. The game supports both sprite rendering and debug mode (colored shapes) as fallback.

## Directory Structure

```
isometric/
├── units/          - Unit sprites (legionaries, phoenicians, slaves)
├── buildings/      - Building sprites (HQ, barracks, questorium)
└── resources/      - Resource point sprites (food, gold sources)
```

## Sprite Specifications

### Units
- **Format**: PNG with transparency
- **Dimensions**: 64x64 pixels (recommended)
- **Anchor Point**: Bottom center (32, 64)
- **Perspective**: Isometric 2:1 ratio
- **Naming Convention**: `{type}_{faction}.png`
  - Examples: `legionary_rome.png`, `phoenician_carthage.png`, `slave_rome.png`

### Buildings
- **Format**: PNG with transparency
- **Dimensions** (based on footprint):
  - 1x1 tile: 128x96 pixels
  - 2x2 tiles: 256x160 pixels
  - 3x3 tiles: 384x224 pixels
- **Anchor Point**: Bottom center
- **Perspective**: Isometric 2:1 ratio
- **Naming Convention**: `{type}_{faction}.png`
  - Examples: `hq_rome.png`, `barracks_carthage.png`, `questorium_rome.png`

### Resources
- **Format**: PNG with transparency
- **Dimensions**: 64x64 pixels (recommended)
- **Anchor Point**: Bottom center
- **Perspective**: Isometric 2:1 ratio
- **Naming Convention**: `{type}_source.png`
  - Examples: `food_source.png`, `gold_source.png`

## Debug Mode

When sprite files are not found, the game automatically falls back to debug rendering:

### Unit Debug Shapes
- **Shape**: Isometric diamond (32x16 pixels)
- **Colors**:
  - Legionary: RED
  - Phoenician: BLUE
  - Slave: YELLOW
- **Features**: Black outline, type label above shape

### Building Debug Shapes
- **Shape**: Isometric rectangle (based on footprint)
- **Colors**: Varies by building type
- **Features**: Black outline, type label

### Resource Debug Shapes
- **Shape**: Square (24x24 pixels)
- **Colors**:
  - Food: GREEN with 'F' letter
  - Gold: GOLD with 'G' letter
- **Features**: Shows remaining amount

## Implementation Notes

1. **Sprite Loading**: Sprites are loaded during object initialization
2. **Fallback**: If sprite loading fails, debug mode is automatically enabled
3. **Logging**: All sprite loading attempts are logged to console
4. **Hot-Reload**: Not currently supported (requires game restart)

## Creating Sprites

### Tips for Artists
1. Use isometric perspective with 2:1 ratio (width:height)
2. Ensure transparent background (alpha channel)
3. Anchor point should be at bottom center for proper ground alignment
4. Consider multiple frames for animations (future feature)
5. Test with debug mode first to verify positioning

### Recommended Tools
- Aseprite (pixel art)
- Photoshop/GIMP (general)
- Blender (3D to isometric rendering)

## Current Status

- ✅ Directory structure created
- ✅ Debug rendering implemented
- ✅ Sprite loading system implemented
- ⏳ Sprite assets (to be created by artists)

## Future Enhancements

- [ ] Animated sprites (walking, attacking)
- [ ] Directional sprites (8 directions)
- [ ] Hot-reload support
- [ ] Sprite atlas for performance
- [ ] Shadow rendering
