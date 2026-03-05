# Bug Condition Exploration Test Results

## Test Execution Date
Task 1 completed - Bug condition exploration test

## Test File
`cpp/tests/test_faction_buildings_visibility.cpp`

## Test Status
✗ **TESTS FAILED (as expected on unfixed code)**

This confirms the bug exists and validates our root cause analysis.

## Counterexamples Found

### Counterexample 1: Rome HQ at tile (5, 5)
- **Grid position**: (5, 5)
- **After Building::init()**: x=0, y=320 (using gridToScreen)
- **After BuildingPlacer::placeBuilding()**: x=-64, y=320 (using gridToScreenWithOffset)
- **getScreenPosition() returns**: (0, 320)
- **x,y fields contain**: (-64, 320)
- **Discrepancy**: 64 pixels on X-axis, 0 pixels on Y-axis

### Counterexample 2: Carthage HQ at tile (45, 45)
- **Grid position**: (45, 45)
- **After Building::init()**: x=0, y=2880 (using gridToScreen)
- **After BuildingPlacer::placeBuilding()**: x=-64, y=2880 (using gridToScreenWithOffset)
- **getScreenPosition() returns**: (0, 2880)
- **x,y fields contain**: (-64, 2880)
- **Discrepancy**: 64 pixels on X-axis, 0 pixels on Y-axis

## Root Cause Confirmed

The test confirms the root cause identified in design.md:

1. **Building::init()** sets `position` and calls `syncScreenCoords()`, which uses `gridToScreen()` to set `x, y`
2. **BuildingPlacer::placeBuilding()** overwrites `x, y` using `gridToScreenWithOffset()`, which subtracts 64 pixels from X
3. **BuildingPlacer does NOT update `position`** - this is the bug!
4. **Building::getScreenPosition()** converts `position` using `gridToScreen()`, returning coordinates that differ from `x, y` by 64 pixels

## Impact

Buildings created by FactionSpawner and placed by BuildingPlacer have inconsistent coordinates:
- `getScreenPosition()` returns one set of coordinates (from `position`)
- `x, y` fields contain different coordinates (64 pixels offset)
- `Building::draw()` uses `getScreenPosition()`, so buildings render at wrong positions
- Buildings may render off-screen or at incorrect map locations

## Next Steps

1. Implement the fix in BuildingPlacer::placeBuilding() (Task 3.1)
2. Re-run this test to verify it passes after the fix (Task 3.2)
3. Run preservation tests to ensure no regressions (Task 3.3)
