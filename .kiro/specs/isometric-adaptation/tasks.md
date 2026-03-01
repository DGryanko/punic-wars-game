# Implementation Plan: Isometric Game Adaptation

## Overview

Адаптація всіх ігрових об'єктів для роботи з ізометричною системою координат з підтримкою спрайтів та debug режиму.

## Tasks

- [ ] 1. Create IsometricSprite class
  - Create `src/isometric_sprite.h` and `src/isometric_sprite.cpp`
  - Implement texture loading with fallback to debug mode
  - Implement render methods for sprites and debug shapes
  - _Requirements: 2.2, 2.3, 5.1, 5.2_

- [ ] 2. Implement debug rendering functions
  - [ ] 2.1 Create debug unit rendering (diamond shape)
    - Draw 32x16 pixel isometric diamond
    - Color based on faction (RED/BLUE/YELLOW)
    - Add black outline for visibility
    - Display unit type label above shape
    - _Requirements: 5.2, 5.3, 5.4_
  
  - [ ] 2.2 Create debug building rendering (rectangle)
    - Draw isometric rectangle based on footprint
    - Color based on building type
    - Add type label
    - _Requirements: 3.2, 3.3, 5.3_
  
  - [ ] 2.3 Create debug resource rendering (square with letter)
    - Draw 24x24 square
    - Add letter (F for food, G for gold)
    - Color: GREEN for food, GOLD for gold
    - _Requirements: 9.3, 5.3_

- [ ] 3. Update Unit class for grid coordinates
  - [ ] 3.1 Replace float x, y with GridCoords position
    - Change member variable from `float x, y` to `GridCoords position`
    - Update constructor to accept GridCoords
    - Add getScreenPosition() method
    - _Requirements: 1.1, 1.4_
  
  - [ ] 3.2 Add IsometricSprite member and loading
    - Add `IsometricSprite sprite` member
    - Load sprite in constructor from `assets/sprites/isometric/units/`
    - Fall back to debug rendering if sprite not found
    - _Requirements: 2.1, 2.2, 5.1, 12.1, 12.2_
  
  - [ ] 3.3 Update render() method
    - Convert grid position to screen coordinates
    - If sprite loaded, render sprite
    - Else render debug diamond
    - _Requirements: 2.1, 2.2, 2.3, 2.4_
  
  - [ ] 3.4 Update movement logic for grid
    - Update moveTo() to work with GridCoords
    - Keep interpolation for smooth movement
    - Update pathfinding integration
    - _Requirements: 7.1, 7.2, 7.3, 7.4_

- [ ] 4. Update Building class for grid coordinates
  - [ ] 4.1 Replace int x, y with GridCoords position
    - Change member variables
    - Add footprint (GridCoords for size)
    - Update constructor
    - _Requirements: 1.2, 1.4_
  
  - [ ] 4.2 Add IsometricSprite and loading
    - Add sprite member
    - Load from `assets/sprites/isometric/buildings/`
    - Implement fallback to debug
    - _Requirements: 3.1, 3.2, 5.1, 12.1_
  
  - [ ] 4.3 Update render() method
    - Convert position to screen coordinates
    - Render sprite or debug rectangle
    - Handle multi-tile footprint
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.6_
  
  - [ ] 4.4 Implement occupiesGridCell() method
    - Check if given grid cell is within building footprint
    - Used for collision detection
    - _Requirements: 8.5_

- [ ] 5. Update ResourcePoint class for grid coordinates
  - [ ] 5.1 Replace int x, y with GridCoords position
    - Update member variables
    - Update constructor
    - _Requirements: 1.3, 1.4_
  
  - [ ] 5.2 Add sprite/debug rendering
    - Load sprite or use debug square
    - Render at correct isometric position
    - _Requirements: 9.2, 9.3_

- [ ] 6. Implement depth sorting system
  - [ ] 6.1 Create RenderQueue class
    - Create `src/render_queue.h` and `src/render_queue.cpp`
    - Implement addUnit(), addBuilding(), addResource()
    - Implement sort() using row+col as key
    - _Requirements: 11.1, 11.2_
  
  - [ ] 6.2 Implement render() method
    - Render objects in sorted order
    - Handle all object types
    - _Requirements: 11.3, 11.4_
  
  - [ ] 6.3 Integrate into DrawGame()
    - Replace direct rendering with RenderQueue
    - Ensure tilemap renders first
    - Ensure UI renders last
    - _Requirements: 11.4, 11.5_

- [ ] 7. Update mouse interaction for grid coordinates
  - [ ] 7.1 Update click handling
    - Convert mouse position to grid coordinates
    - Use CoordinateConverter::screenToGrid()
    - _Requirements: 6.1, 6.2_
  
  - [ ] 7.2 Implement grid-based selection
    - Create findUnitAtGrid() function
    - Create findBuildingAtGrid() function
    - Handle multi-tile buildings
    - _Requirements: 6.3, 6.4, 6.5_
  
  - [ ] 7.3 Update movement commands
    - Right-click converts to grid coordinates
    - Units move to grid positions
    - _Requirements: 7.1, 7.2_

- [ ] 8. Update building placement system
  - [ ] 8.1 Implement grid snapping
    - Snap building position to grid
    - Show preview at grid position
    - _Requirements: 8.1_
  
  - [ ] 8.2 Add placement validation
    - Check if cells are passable
    - Check if cells are not occupied
    - Check for water tiles
    - _Requirements: 8.2, 8.3, 8.6_
  
  - [ ] 8.3 Add placement preview
    - Show building outline at mouse position
    - Green tint if valid, red if invalid
    - _Requirements: 8.4_

- [ ] 9. Update pathfinding for grid system
  - Verify pathfinding works with GridCoords
  - Update path following to use grid positions
  - Test with isometric map obstacles
  - _Requirements: 7.4_

- [ ] 10. Create sprite loading infrastructure
  - [ ] 10.1 Create sprite directory structure
    - Create `assets/sprites/isometric/units/`
    - Create `assets/sprites/isometric/buildings/`
    - Create `assets/sprites/isometric/resources/`
    - _Requirements: 12.1_
  
  - [ ] 10.2 Implement sprite filename conventions
    - Units: `{type}_{faction}.png`
    - Buildings: `{type}_{faction}.png`
    - Resources: `{type}_source.png`
    - _Requirements: 12.5_
  
  - [ ] 10.3 Add logging for sprite loading
    - Log successful loads
    - Log fallback to debug mode
    - _Requirements: 12.3, 5.6_

- [ ] 11. Update compile script
  - Add isometric_sprite.cpp to compilation
  - Add render_queue.cpp to compilation
  - Test compilation succeeds
  - _Requirements: All_

- [ ] 12. Testing and validation
  - [ ] 12.1 Test coordinate conversion
    - Verify grid ↔ screen conversion works
    - Test at map boundaries
    - Test with camera pan and zoom
  
  - [ ] 12.2 Test debug rendering
    - Verify all objects render in debug mode
    - Check colors and shapes are correct
    - Verify labels are readable
  
  - [ ] 12.3 Test sprite loading
    - Test with sprites present
    - Test with sprites missing
    - Verify fallback works
  
  - [ ] 12.4 Test depth sorting
    - Place objects at different positions
    - Verify rendering order is correct
    - Test with overlapping objects
  
  - [ ] 12.5 Test mouse interaction
    - Click on units, buildings, resources
    - Verify correct object is selected
    - Test with overlapping objects
  
  - [ ] 12.6 Test movement
    - Command units to move
    - Verify they move to correct grid positions
    - Test pathfinding around obstacles
  
  - [ ] 12.7 Performance test
    - Test with 100+ units
    - Verify 60 FPS maintained
    - Check memory usage

- [ ] 13. Documentation
  - Update SESSION_CONTEXT.md with changes
  - Document new coordinate system usage
  - Add examples of sprite creation
  - _Requirements: All_

## Notes

- All tasks build on the existing tilemap system
- Debug mode allows testing without sprites
- Sprite system is optional - game works without sprites
- Focus on getting debug rendering working first
- Add sprites incrementally as they are created

