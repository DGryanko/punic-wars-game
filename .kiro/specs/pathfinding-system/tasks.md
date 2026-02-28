# Implementation Plan: Pathfinding System

## Overview

Реалізація системи pathfinding з використанням алгоритму A* для гри Punic Wars: Castra. Система включає навігаційну сітку, пошук шляху, кешування та механізм виходу з застряглих ситуацій.

## Tasks

- [x] 1. Create pathfinding.h header file with core structures
  - Create NavigationGrid class with grid data structure
  - Create GridNode structure for A* algorithm
  - Create AStarPathfinder class interface
  - Create PathfindingManager class interface
  - Add necessary includes and forward declarations
  - _Requirements: 1.1, 1.4, 2.1_

- [x] 2. Implement NavigationGrid class
  - [x] 2.1 Implement grid initialization and memory allocation
    - Initialize grid with map dimensions
    - Allocate cell array
    - Set default walkability for all cells
    - _Requirements: 1.1_
  
  - [x] 2.2 Implement coordinate conversion methods
    - Implement worldToGrid() conversion
    - Implement gridToWorld() conversion
    - Add bounds checking
    - _Requirements: 1.4_
  
  - [x] 2.3 Implement obstacle marking methods
    - Implement markObstacle() for single cell
    - Implement updateFromBuildings() for all buildings
    - Handle building rectangles to grid cells conversion
    - _Requirements: 1.2, 1.3_
  
  - [ ] 2.4 Implement neighbor retrieval for A*
    - Get 8-directional neighbors
    - Filter out non-walkable neighbors
    - Return valid neighbor list
    - _Requirements: 2.1_

- [ ] 3. Implement A* pathfinding algorithm
  - [ ] 3.1 Implement heuristic function
    - Calculate Manhattan distance
    - Return float value
    - _Requirements: 2.2_
  
  - [x] 3.2 Implement main A* search
    - Initialize open and closed lists
    - Implement main search loop
    - Handle node expansion
    - Limit to 500 nodes maximum
    - Return path or empty vector
    - _Requirements: 2.1, 2.3, 2.4, 8.2_
  
  - [x] 3.3 Implement path reconstruction
    - Trace back from goal to start
    - Convert grid coordinates to world coordinates
    - Return waypoint list
    - _Requirements: 2.1_
  
  - [x] 3.4 Implement path smoothing
    - Remove collinear waypoints
    - Use line-of-sight checks
    - Reduce waypoint count
    - _Requirements: 2.1_

- [x] 4. Implement PathfindingManager
  - [x] 4.1 Implement path request queueing
    - Add requestPath() method
    - Store requests in priority queue
    - Handle priority sorting
    - _Requirements: 5.3_
  
  - [x] 4.2 Implement update loop
    - Process up to 5 requests per frame
    - Calculate paths using A*
    - Store results in cache
    - _Requirements: 5.3, 8.1_
  
  - [x] 4.3 Implement path cache
    - Store calculated paths by unit ID
    - Add timestamp to paths
    - Implement cache retrieval
    - _Requirements: 5.1_
  
  - [x] 4.4 Implement cache invalidation
    - Invalidate paths in area
    - Clear old paths (>10 seconds)
    - _Requirements: 5.2, 5.5_
  
  - [x] 4.5 Implement grid update method
    - Update grid from buildings
    - Invalidate affected paths
    - _Requirements: 1.5, 5.2_

- [x] 5. Integrate pathfinding with Unit structure
  - [x] 5.1 Add pathfinding fields to Unit
    - Add path vector
    - Add currentWaypointIndex
    - Add stuckTimer and lastPosition
    - Add unstuckAttempts counter
    - _Requirements: 3.1, 6.1_
  
  - [x] 5.2 Implement setPath() method
    - Store new path
    - Reset waypoint index
    - Clear stuck timer
    - _Requirements: 3.1_
  
  - [x] 5.3 Implement followPath() method
    - Move towards current waypoint
    - Check if waypoint reached
    - Advance to next waypoint
    - Stop at final waypoint
    - _Requirements: 3.1, 3.2, 3.3, 3.4_
  
  - [x] 5.4 Implement stuck detection
    - Track position changes
    - Increment stuck timer if not moving
    - Reset timer if moving
    - _Requirements: 6.1_
  
  - [x] 5.5 Implement unstuck mechanism
    - Try moving to adjacent free cells
    - Attempt up to 3 times
    - Recalculate path if all attempts fail
    - Log stuck situations
    - _Requirements: 6.2, 6.3, 6.4, 6.5_

- [x] 6. Update main.cpp integration
  - [x] 6.1 Create global PathfindingManager instance
    - Declare global variable
    - Initialize in main()
    - _Requirements: 7.1_
  
  - [x] 6.2 Initialize navigation grid
    - Call grid initialization in InitBuildings()
    - Update grid with initial buildings
    - _Requirements: 1.3, 7.1_
  
  - [x] 6.3 Replace unit movement logic
    - Remove old collision avoidance code
    - Request paths through PathfindingManager
    - Call followPath() in unit update
    - _Requirements: 7.1, 7.2, 7.3_
  
  - [x] 6.4 Update grid when buildings change
    - Call updateGrid() when buildings added/removed
    - _Requirements: 1.5, 7.1_
  
  - [x] 6.5 Handle dynamic unit obstacles
    - Implement temporary obstacle marking for units
    - Update grid each frame with unit positions
    - Clear temporary obstacles before next frame
    - _Requirements: 4.1, 4.2, 4.3, 4.4_

- [ ] 7. Add debug visualization (optional)
  - [ ] 7.1 Draw navigation grid
    - Draw grid cells
    - Color blocked cells differently
    - Toggle with debug key
    - _Requirements: None (debugging aid)_
  
  - [ ] 7.2 Draw unit paths
    - Draw lines between waypoints
    - Show current waypoint
    - _Requirements: None (debugging aid)_

- [x] 8. Testing and optimization
  - [x] 8.1 Test with single unit
    - Verify path calculation
    - Verify path following
    - Verify obstacle avoidance
    - _Requirements: 2.1, 3.1, 3.2, 3.3_
  
  - [x] 8.2 Test with multiple units
    - Verify 10+ units pathfinding
    - Check for performance issues
    - Verify units don't collide
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 8.1_
  
  - [x] 8.3 Test stuck scenarios
    - Create blocked situations
    - Verify unstuck mechanism works
    - Verify path recalculation
    - _Requirements: 6.1, 6.2, 6.3_
  
  - [x] 8.4 Performance profiling
    - Measure pathfinding time
    - Verify 60 FPS with 50 units
    - Check memory usage
    - _Requirements: 8.1, 8.2, 8.3, 8.4_
  
  - [x] 8.5 Integration testing
    - Test with resource harvesting
    - Test with combat
    - Test with AI units
    - Verify backward compatibility
    - _Requirements: 7.2, 7.3, 7.4, 7.5_

- [x] 9. Final cleanup and documentation
  - Remove old collision code
  - Add code comments
  - Update README if needed
  - _Requirements: All_

## Notes

- Кожна задача будується на попередніх
- Тестування проводиться після кожного основного компонента
- Debug візуалізація допоможе при налагодженні
- Оптимізація виконується після базової реалізації
