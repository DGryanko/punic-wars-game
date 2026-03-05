# Implementation Plan

- [x] 1. Write bug condition exploration test
  - **Property 1: Fault Condition** - Coordinate Consistency for Faction HQs
  - **CRITICAL**: This test MUST FAIL on unfixed code - failure confirms the bug exists
  - **DO NOT attempt to fix the test or the code when it fails**
  - **NOTE**: This test encodes the expected behavior - it will validate the fix when it passes after implementation
  - **GOAL**: Surface counterexamples that demonstrate the coordinate mismatch bug exists
  - **Scoped PBT Approach**: Test concrete failing cases - Rome HQ at (5,5) and Carthage HQ at (45,45)
  - Test that for buildings created by FactionSpawner, getScreenPosition() returns coordinates that match x,y fields
  - Verify the 64-pixel X-axis offset discrepancy exists on unfixed code
  - Test implementation details from Fault Condition: `abs(screenPosFromPosition.x - screenPosFromXY.x) == 64`
  - Run test on UNFIXED code
  - **EXPECTED OUTCOME**: Test FAILS (this is correct - it proves the bug exists)
  - Document counterexamples found (e.g., "Rome HQ: getScreenPosition() returns (0, 320) but x,y is (-64, 320)")
  - Mark task complete when test is written, run, and failure is documented
  - _Requirements: 2.1, 2.3, 2.4_

- [x] 2. Write preservation property tests (BEFORE implementing fix)
  - **Property 2: Preservation** - Non-Faction Building Behavior
  - **IMPORTANT**: Follow observation-first methodology
  - Observe behavior on UNFIXED code for player-placed buildings (non-FactionSpawner cases)
  - Test that player building placement through BuildingPlacer works correctly
  - Test that pathfinding grid updates correctly when buildings are placed
  - Test that occupied tiles tracking works correctly
  - Write property-based tests capturing observed behavior patterns from Preservation Requirements
  - Property-based testing generates many test cases for stronger guarantees
  - Run tests on UNFIXED code
  - **EXPECTED OUTCOME**: Tests PASS (this confirms baseline behavior to preserve)
  - Mark task complete when tests are written, run, and passing on unfixed code
  - _Requirements: 3.2, 3.3, 3.5_

- [x] 3. Fix coordinate mismatch in BuildingPlacer

  - [x] 3.1 Implement the fix in BuildingPlacer::placeBuilding()
    - Remove direct x, y assignment using gridToScreenWithOffset()
    - Remove: `building.x = (int)screenPos.x;`
    - Remove: `building.y = (int)screenPos.y;`
    - Update building.position with passed grid coordinates: `building.position = GridCoords(row, col);`
    - Call syncScreenCoords() to synchronize x,y with position: `building.syncScreenCoords();`
    - Keep tile_row, tile_col updates for compatibility layer
    - _Bug_Condition: isBugCondition(building) where building was created by FactionSpawner AND BuildingPlacer::placeBuilding was called AND abs(screenPosFromPosition.x - screenPosFromXY.x) == 64_
    - _Expected_Behavior: getScreenPosition() returns screen coordinates that match x,y fields, eliminating 64-pixel offset_
    - _Preservation: Player-placed buildings, pathfinding grid updates, occupied tiles tracking remain unchanged_
    - _Requirements: 2.1, 2.3, 2.4, 3.2, 3.3, 3.5_

  - [x] 3.2 Verify bug condition exploration test now passes
    - **Property 1: Expected Behavior** - Coordinate Consistency for Faction HQs
    - **IMPORTANT**: Re-run the SAME test from task 1 - do NOT write a new test
    - The test from task 1 encodes the expected behavior
    - When this test passes, it confirms the expected behavior is satisfied
    - Run bug condition exploration test from step 1
    - **EXPECTED OUTCOME**: Test PASSES (confirms bug is fixed)
    - Verify getScreenPosition() now returns coordinates matching x,y fields
    - Verify 64-pixel offset is eliminated
    - _Requirements: 2.1, 2.3, 2.4_

  - [x] 3.3 Verify preservation tests still pass
    - **Property 2: Preservation** - Non-Faction Building Behavior
    - **IMPORTANT**: Re-run the SAME tests from task 2 - do NOT write new tests
    - Run preservation property tests from step 2
    - **EXPECTED OUTCOME**: Tests PASS (confirms no regressions)
    - Confirm player building placement still works correctly
    - Confirm pathfinding grid updates still work correctly
    - Confirm occupied tiles tracking still works correctly
    - _Requirements: 3.2, 3.3, 3.5_

- [x] 4. Checkpoint - Ensure all tests pass
  - Verify all exploration tests pass (coordinate consistency achieved)
  - Verify all preservation tests pass (no regressions in player building placement)
  - Verify faction HQs now render at correct positions on map
  - Ask the user if questions arise
