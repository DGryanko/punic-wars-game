# Testing Guide: Slave Resource Gathering Fixes

## Status: READY FOR TESTING ✅

All implementation phases are complete. The game compiles successfully.

## What Was Fixed

1. **Automatic Resource Cycle**: Slaves now automatically gather → dropoff → return without manual commands
2. **Building Avoidance**: Pathfinding now avoids buildings when moving
3. **Reliable Resource Dropoff**: Resources are credited within 20 pixels of warehouse

## How to Test

### Start the Game
```bash
cd cpp
.\punic_wars.exe
```

### Test Scenario 1: Basic Automatic Cycle
1. Build a Praetorium (slave production building)
2. Create a slave
3. Right-click on a resource point (food or gold)
4. **Expected**: Slave should:
   - Move to resource
   - Start harvesting (yellow circle appears)
   - When full, automatically move to nearest Questorium/HQ
   - Drop resources (watch resource counter increase)
   - Automatically return to resource and repeat

### Test Scenario 2: Building Obstacle Avoidance
1. Create a slave
2. Place a Questorium between the slave and a resource
3. Right-click on the resource
4. **Expected**: Slave should walk AROUND the Questorium, not through it

### Test Scenario 3: Resource Dropoff Distance
1. Create a slave and send to resource
2. Wait for slave to fill inventory (yellow bar under unit)
3. Watch slave move to Questorium/HQ
4. **Expected**: Resources should be credited when slave is within ~20 pixels of building center

### Test Scenario 4: Resource Depletion
1. Send slave to a small resource point
2. Wait for resource to deplete completely
3. **Expected**: Slave should stop harvesting and become idle

### Test Scenario 5: Manual Command Cancels Cycle
1. Send slave to resource (starts automatic cycle)
2. While slave is harvesting, right-click somewhere else
3. **Expected**: Slave should stop harvesting and move to new location

## What to Watch For

- **Resource Counter**: Should increase when slave reaches warehouse
- **Yellow Circle**: Indicates slave is harvesting
- **Yellow Bar**: Shows how full slave's inventory is
- **Movement**: Slaves should smoothly navigate around buildings
- **Console Logs**: Check for messages like:
  - `[HARVEST] Slave harvesting. Carrying: X/15`
  - `[DROP] Resources dropped: food=X, gold=Y`
  - `[MOVE_TO_DROPOFF] Moving to dropoff`
  - `[RETURN] Returning to resource`

## Known Limitations

- Dropoff distance is 20 pixels from building center (not collision-based)
- Multiple slaves may overlap at same resource point
- Pathfinding is basic and may not find optimal routes

## If You Find Issues

Please note:
1. What you were doing
2. What you expected to happen
3. What actually happened
4. Any console log messages

## Implementation Details

### Files Modified
- `cpp/src/tilemap/tilemap.h` - Added `setPassable()` method
- `cpp/src/tilemap/tilemap.cpp` - Implemented dynamic obstacle system
- `cpp/src/pathfinding.h` - Added `updateFromBuildings()` 
- `cpp/src/unit.h` - Added resource assignment fields
- `cpp/src/main.cpp` - Refactored `ProcessResourceHarvesting()` with state machine

### State Machine Logic
```
STATE 1: If inventory full → Move to dropoff
STATE 2: If at dropoff → Drop resources → Return to resource  
STATE 3: If not full → Move to resource
STATE 4: If at resource → Harvest
```

### Dropoff Logic
- Distance check: `sqrt((row1-row2)^2 + (col1-col2)^2) < 20.0/64.0` (grid units)
- Searches for Questorium first, then HQ
- Credits resources to player faction immediately

---

**Ready to test!** Run `.\punic_wars.exe` and try the scenarios above.
