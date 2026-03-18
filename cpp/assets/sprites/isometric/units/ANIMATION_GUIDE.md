# Animation Guide - Punic Wars: Castra

## Isometric axes

```
         BACK_RIGHT (up-right, col--)

BACK_LEFT (up-left)  <-->  FRONT_RIGHT (down-right)
   row--                        col++

         FRONT_LEFT (down-left, row++)
```

---

## Spritesheet format (recommended)

One PNG = one animation. Frames go left to right, then next row.

Examples for 64x64 frame size:
  8 frames in a row  -> 512x64 px
  4 frames in a row  -> 256x64 px
  8 frames, 2 rows   -> 512x128 px  (your variant!)

Frame count is calculated automatically: width/frameW * height/frameH

---

## Folder structure and file names

```
assets/sprites/isometric/units/
|
+-- legionary_rome/
|   +-- idle.png                  (1-4 frames, e.g. 256x64)
|   +-- walk_front_left.png       (4-8 frames, down-left movement)
|   +-- walk_front_right.png      (4-8 frames, down-right movement)
|   +-- walk_back_left.png        (4-8 frames, up-left movement)
|   +-- walk_back_right.png       (4-8 frames, up-right movement)
|   +-- attack_front_left.png     (4-8 frames)
|   +-- attack_front_right.png
|   +-- attack_back_left.png
|   +-- attack_back_right.png
|   +-- death.png                 (4-8 frames, no direction)
|
+-- slave_rome/
+-- slave_carthage/
+-- phoenician_carthage/
```

---

## Technical requirements

- Format: PNG with transparency (RGBA)
- Frame size: 64x64 px (or any consistent size)
- Anchor point: bottom center (character feet)
- All frames in one animation must be the same size
- Frame count is arbitrary - code counts automatically

---

## Animation FPS

| Animation | Recommended frames | FPS |
|-----------|-------------------|-----|
| idle      | 2-4               | 4   |
| walk      | 4-8               | 8   |
| attack    | 4-8               | 12  |
| death     | 4-8               | 6   |

---

## How to use in code

Unit::init() automatically calls:
  animator.loadSheets("assets/sprites/isometric/units/legionary_rome", 64, 64);

For different frame size (e.g. 96x96):
  animator.loadSheets(basePath, 96, 96);

---

## Fallback behavior

1. Spritesheets found -> use them
2. Separate files (0.png, 1.png...) found -> use them
3. legionary_rome.png (old format) found -> static sprite
4. Nothing -> debug diamond shape

You can add animations gradually - game won't break.

---

## Minimum to get started

```
legionary_rome/
  idle.png              (at least 1 frame = 64x64)
  walk_front_left.png   (4 frames = 256x64)
  walk_front_right.png
  walk_back_left.png
  walk_back_right.png
```