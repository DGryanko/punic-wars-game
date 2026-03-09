# Implementation Plan: Slave Resource Gathering System Fixes

## Overview

Цей план виправляє три критичні баги в системі збору ресурсів:
1. Відсутність автоматичного циклу збору
2. Ігнорування будівель при pathfinding
3. Ненадійна здача ресурсів

Імплементація розділена на 5 фаз з інкрементальним тестуванням.

## Tasks

- [-] 1. Phase 1: TileMap Enhancement

  - [x] 1.1 Add setPassable() method to TileMap
    - Open `cpp/src/tilemap/tilemap.h`
    - Add public method: `void setPassable(int row, int col, bool passable)`
    - Implement bounds checking: `if (row >= 0 && row < height && col >= 0 && col < width)`
    - Set tile passability: `tiles[row][col].is_passable = passable`
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5_

  - [ ] 1.2 Write unit tests for TileMap.setPassable()
    - Create test file `cpp/tests/test_tilemap_passability.cpp`
    - Test case: setPassable(row, col, false) makes tile impassable
    - Test case: setPassable(row, col, true) makes tile passable
    - Test case: out of bounds coordinates are handled safely
    - Test case: isPassable() returns correct value after setPassable()
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5_

  - [ ] 1.3 Verify TileMap tests pass
    - Compile and run tests: `cd cpp/tests && make test_tilemap_passability`
    - Verify all test cases pass
    - Fix any issues found
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5_

- [-] 2. Phase 2: NavigationGrid Building Integration

  - [x] 2.1 Implement NavigationGrid.updateFromBuildings()
    - Open `cpp/src/pathfinding.h`
    - Locate `NavigationGrid::updateFromBuildings()` method
    - Add null check: `if (!tileMap) return;`
    - Iterate through all buildings
    - For each building, get GridCoords position and footprint
    - Mark all tiles under building as impassable using `const_cast<TileMap*>(tileMap)->setPassable(row, col, false)`
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5_

  - [x] 2.2 Call updateFromBuildings() after building placement
    - Open `cpp/src/main.cpp`
    - Find building placement code (likely in HandleClicks or BuildingPlacer)
    - After adding building to buildings vector, call `pathfindingManager.updateGrid(buildings)`
    - _Requirements: 2.1, 4.1_

  - [x] 2.3 Call updateFromBuildings() after building removal
    - Find building removal code in `cpp/src/main.cpp`
    - Before removing building, mark its tiles as passable
    - After removal, call `pathfindingManager.updateGrid(buildings)`
    - _Requirements: 2.2, 4.2_

  - [ ] 2.4 Write unit tests for NavigationGrid building integration
    - Create test file `cpp/tests/test_navigation_buildings.cpp`
    - Test case: Building at (10,10) with footprint 2x2 blocks tiles (10,10), (10,11), (11,10), (11,11)
    - Test case: Neighboring tiles remain passable
    - Test case: Pathfinding finds path around building
    - Test case: Removing building restores passability
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 4.1, 4.2, 4.3_

  - [ ] 2.5 Verify building obstacle tests pass
    - Compile and run tests
    - Verify pathfinding avoids buildings
    - Test in game: place building, verify units walk around it
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5_

- [-] 3. Phase 3: Unit Resource Assignment

  - [x] 3.1 Add resource assignment fields to Unit class
    - Open `cpp/src/unit.h`
    - Add fields:
      - `GridCoords assigned_resource_position;`
      - `GridCoords assigned_dropoff_position;`
      - `bool has_assigned_resource;`
    - Initialize in constructor: `has_assigned_resource = false`
    - _Requirements: 5.1, 5.2_

  - [x] 3.2 Implement Unit.assignResource() method
    - Add method signature: `void assignResource(GridCoords resource_pos, GridCoords dropoff_pos)`
    - Set `assigned_resource_position = resource_pos`
    - Set `assigned_dropoff_position = dropoff_pos`
    - Set `has_assigned_resource = true`
    - _Requirements: 5.1, 5.2_

  - [x] 3.3 Implement Unit.clearResourceAssignment() method
    - Add method signature: `void clearResourceAssignment()`
    - Set `has_assigned_resource = false`
    - Set `assigned_resource_position = GridCoords(-1, -1)`
    - Set `assigned_dropoff_position = GridCoords(-1, -1)`
    - _Requirements: 5.4, 5.5_

  - [x] 3.4 Update Unit.hasAssignedResource() method
    - Change implementation to return `has_assigned_resource`
    - Remove old logic that checked assigned_resource_x/y
    - _Requirements: 5.1, 5.2, 5.3_

  - [ ] 3.5 Update HandleClicks to assign resource with dropoff
    - Open `cpp/src/main.cpp`
    - Find code where user right-clicks on resource
    - Find nearest Questorium or HQ for player faction
    - Call `unit.assignResource(resource_grid_pos, dropoff_grid_pos)`
    - Log assignment: `printf("Slave assigned to resource at grid(%d, %d), dropoff at grid(%d, %d)\n", ...)`
    - _Requirements: 1.1, 5.1, 5.2_

  - [ ] 3.6 Write unit tests for resource assignment
    - Create test file `cpp/tests/test_unit_resource_assignment.cpp`
    - Test case: assignResource() sets correct positions
    - Test case: hasAssignedResource() returns true after assignment
    - Test case: clearResourceAssignment() clears all fields
    - Test case: hasAssignedResource() returns false after clear
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_

- [x] 4. Phase 4: Harvest Cycle State Machine

  - [x] 4.1 Refactor ProcessResourceHarvesting() - State: Moving to Resource
    - Open `cpp/src/main.cpp`, find `ProcessResourceHarvesting()`
    - For each slave with `hasAssignedResource()`:
    - **State 1: Moving to Resource**
      - Check if slave is at resource position (distance < 2 tiles)
      - If yes, transition to HARVESTING state
      - If no, ensure slave is moving to resource position
    - _Requirements: 1.1, 1.2, 7.1_

  - [x] 4.2 Implement State: Harvesting
    - **State 2: Harvesting**
      - Set `unit.is_harvesting = true`
      - Set `unit.is_moving = false`
      - Find resource at assigned position
      - Harvest resource (increment carrying_food or carrying_gold)
      - Check if inventory full: `!unit.canCarryMore()`
      - If full, transition to MOVING_TO_DROPOFF state
    - _Requirements: 1.2, 1.3, 7.2_

  - [x] 4.3 Implement State: Moving to Dropoff
    - **State 3: Moving to Dropoff**
      - Set `unit.is_harvesting = false`
      - Set `unit.is_moving = true`
      - Get dropoff position from `unit.assigned_dropoff_position`
      - Calculate distance to dropoff building
      - If distance < 3.0 tiles, transition to DROPPING_RESOURCES state
      - If not moving to dropoff, call `unit.moveTo(assigned_dropoff_position)`
    - _Requirements: 1.3, 3.1, 3.2, 7.3_

  - [x] 4.4 Implement State: Dropping Resources
    - **State 4: Dropping Resources**
      - Call `unit.dropResources(food, gold)`
      - Add resources to faction: `rome_food += food; rome_money += gold;`
      - Log dropoff: `printf("[DROP] Resources dropped: food=%d, gold=%d\n", food, gold)`
      - Call `unit.moveTo(assigned_resource_position)` to return to resource
      - Transition back to MOVING_TO_RESOURCE state
    - _Requirements: 1.4, 1.5, 3.3, 3.4, 3.5, 3.6, 7.4_

  - [x] 4.5 Handle resource depletion
    - In HARVESTING state, check if resource is depleted
    - If depleted, call `unit.clearResourceAssignment()`
    - Set `unit.is_harvesting = false`
    - Log: `printf("[HARVEST] Resource depleted, stopping cycle\n")`
    - _Requirements: 1.6, 5.4_

  - [x] 4.6 Handle new movement commands
    - In HandleClicks, when slave receives new move command
    - Call `unit.clearResourceAssignment()`
    - Set `unit.is_harvesting = false`
    - This cancels the harvest cycle
    - _Requirements: 1.7, 5.5, 7.5_

  - [x] 4.7 Write integration test for harvest cycle
    - Create test file `cpp/tests/test_harvest_cycle.cpp`
    - Setup: Create slave, resource, Questorium on test map
    - Assign resource to slave
    - Simulate game loop for multiple frames
    - Verify slave goes through all states:
      - MOVING_TO_RESOURCE → HARVESTING → MOVING_TO_DROPOFF → DROPPING_RESOURCES → MOVING_TO_RESOURCE
    - Verify resources are added to faction
    - Verify slave inventory is cleared after dropoff
    - **SKIPPED**: Manual testing prioritized per user request
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5_

- [ ] 5. Phase 5: Integration and Testing

  - [ ] 5.1 Write property test: Harvest Cycle Completion
    - **Property 1: Harvest Cycle Completion**
    - Generate random resource and dropoff positions
    - Create slave and assign resource
    - Simulate game loop until cycle completes or timeout
    - Verify slave completed full cycle
    - Verify resources were added to faction
    - Run test 100 times with different random positions
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5_

  - [ ] 5.2 Write property test: Building Obstacle Avoidance
    - **Property 2: Building Obstacle Avoidance**
    - Generate random buildings on map
    - Call updateFromBuildings()
    - Generate random start/goal positions
    - Calculate path using pathfinding
    - Verify no tile in path is occupied by building
    - Run test 100 times with different random configurations
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5_

  - [ ] 5.3 Write property test: Resource Conservation
    - **Property 3: Resource Conservation**
    - Generate random carrying_food and carrying_gold values
    - Record initial faction resources
    - Execute dropoff logic
    - Verify delta equals carrying values
    - Verify inventory is empty after dropoff
    - Run test 100 times with different random values
    - _Requirements: 3.3, 3.4, 3.5_

  - [ ] 5.4 Write property test: TileMap Building Synchronization
    - **Property 4: TileMap Building Synchronization**
    - Generate random building positions and footprints
    - Call updateFromBuildings()
    - For each building, verify all tiles in footprint are impassable
    - Verify pathfinding avoids these tiles
    - Run test 100 times with different random buildings
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5_

  - [ ] 5.5 Write property test: Resource Assignment Persistence
    - **Property 5: Resource Assignment Persistence**
    - Assign random resource and dropoff positions
    - Simulate multiple game loop iterations
    - Verify positions remain unchanged during cycle
    - Verify positions are cleared when resource depletes
    - Verify positions are cleared when new command given
    - Run test 100 times with different scenarios
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_

  - [ ] 5.6 Manual testing in game
    - Compile game: `cd cpp && ./compile.bat`
    - Run game: `./punic_wars.exe`
    - Test scenario 1: Single slave gathering
      - Create slave from Praetorium
      - Right-click on resource
      - Verify slave automatically gathers and returns
      - Verify resources increase in UI
    - Test scenario 2: Building obstacle
      - Place Questorium between slave and resource
      - Send slave to resource
      - Verify slave walks around Questorium
    - Test scenario 3: Multiple slaves
      - Create 3 slaves
      - Send all to same resource
      - Verify all gather and dropoff without conflicts
    - Test scenario 4: Resource depletion
      - Send slave to small resource
      - Wait for resource to deplete
      - Verify slave stops gathering
    - _Requirements: All_

  - [ ] 5.7 Fix any bugs found during testing
    - Document bugs in comments
    - Fix bugs one by one
    - Re-run tests after each fix
    - Verify all tests pass
    - _Requirements: All_

- [ ] 6. Checkpoint - Ensure all tests pass
  - Run all unit tests
  - Run all property tests
  - Run all integration tests
  - Verify manual testing scenarios work
  - Ensure all tests pass, ask the user if questions arise

## Notes

- Each phase builds on the previous one
- Test after each phase before moving to next
- Property tests should run 100 iterations minimum
- Manual testing is critical for verifying user experience
- Log statements are important for debugging
