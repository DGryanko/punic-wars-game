# Implementation Plan: Building Placement System

## Overview

Імплементація системи розміщення будівель на тайловій сітці з блокуванням для патфайндингу, автоматичним спавном головних наметів, панеллю замовлення юнітів та виправленням відображення ресурсів і мапи.

## Tasks

- [x] 1. Створити BuildingPlacer для розміщення будівель на тайлах
  - Створити файл `cpp/src/building_placer.h` з класом BuildingPlacer
  - Реалізувати метод `placeBuilding(Building&, int row, int col)` з конвертацією координат
  - Реалізувати метод `removeBuilding(const Building&)` для розблокування тайлів
  - Реалізувати метод `isTileFree(int row, int col)` для перевірки доступності
  - Інтегрувати з PathfindingManager для блокування/розблокування тайлів
  - _Requirements: 1.1, 1.2, 1.3, 1.4, 2.1, 2.2, 2.4_

- [x] 1.1 Написати property test для розміщення будівель
  - **Property 1: Building placement on single tile**
  - **Validates: Requirements 1.1, 1.2, 1.4**

- [x] 1.2 Написати property test для блокування тайлів
  - **Property 2: Tile blocking consistency**
  - **Validates: Requirements 2.1, 2.2**

- [x] 2. Створити FactionSpawner для автоматичного спавну HQ
  - Створити файл `cpp/src/faction_spawner.h` з класом FactionSpawner
  - Реалізувати метод `findRandomFreeTile()` для пошуку вільних тайлів
  - Реалізувати метод `isMinDistanceSatisfied()` для перевірки відстані між HQ
  - Реалізувати метод `spawnFactionHQs()` для створення HQ обох фракцій
  - Використовувати BuildingPlacer для розміщення HQ на тайлах
  - _Requirements: 3.1, 3.2, 3.3, 3.4_

- [x] 2.1 Написати property test для мінімальної відстані між HQ
  - **Property 4: HQ minimum distance**
  - **Validates: Requirements 3.3**

- [x] 2.2 Написати property test для спавну HQ на прохідних тайлах
  - **Property 5: HQ spawns on passable tiles**
  - **Validates: Requirements 3.1, 3.2**

- [x] 3. Checkpoint - Перевірити розміщення будівель
  - Скомпілювати код
  - Запустити гру та перевірити що будівлі розміщуються на тайлах
  - Перевірити що HQ обох фракцій з'являються на карті
  - Ensure all tests pass, ask the user if questions arise.

- [x] 4. Створити UnitOrderPanel для замовлення юнітів
  - Створити файл `cpp/src/unit_order_panel.h` з класом UnitOrderPanel
  - Реалізувати метод `setSelectedBuilding(int index)` для встановлення вибраної будівлі
  - Реалізувати метод `draw()` для малювання панелі знизу екрану
  - Реалізувати метод `drawWorkerButton()` для малювання кнопки Worker
  - Реалізувати метод `handleClick(Vector2)` для обробки кліків на кнопку
  - Реалізувати метод `isVisible()` для перевірки видимості панелі
  - Інтегрувати з існуючою системою виробництва `Building::startProduction`
  - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5_

- [x] 4.1 Написати property test для видимості панелі
  - **Property 6: Unit order panel visibility**
  - **Validates: Requirements 4.1, 4.5**

- [x] 4.2 Написати property test для доступності кнопки Worker
  - **Property 7: Worker button availability**
  - **Validates: Requirements 4.3**

- [x] 5. Виправити ResourceDisplay для коректного відображення ресурсів
  - Створити файл `cpp/src/resource_display.h` з класом ResourceDisplay
  - Реалізувати метод `init(Texture2D, Faction)` для ініціалізації
  - Реалізувати метод `calculateTextPositions()` для розрахунку позицій тексту
  - Реалізувати метод `draw(int food, int money, int foodReserved, int moneyReserved)` для малювання
  - Видалити відображення загальної кількості юнітів з панелі
  - Оновити позиції тексту: їжа на (panelX + 250, 50), гроші на (panelX + 750, 50)
  - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_

- [x] 5.1 Написати property test для точності відображення ресурсів
  - **Property 8: Resource display accuracy**
  - **Validates: Requirements 5.3, 5.4**

- [x] 5.2 Написати property test для позиціонування ресурсів
  - **Property 9: Resource display positioning**
  - **Validates: Requirements 5.1, 5.2**

- [x] 6. Виправити viewport culling в IsometricRenderer
  - Відкрити файл `cpp/src/tilemap/isometric_renderer.cpp`
  - Оновити метод `calculateVisibleTiles()` для правильного розрахунку видимих тайлів
  - Використовувати `GetScreenToWorld2D()` для конвертації меж екрану
  - Додати culling margin 2 тайли за межами екрану
  - Видалити будь-які фіксовані обмеження на кількість тайлів (12 тайлів)
  - Оновити метод `render()` для малювання всіх тайлів в діапазоні [visible_start, visible_end]
  - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5_

- [x] 6.1 Написати property test для повноти viewport culling
  - **Property 10: Viewport culling completeness**
  - **Validates: Requirements 6.1, 6.2, 6.4**

- [x] 6.2 Написати property test для culling margin
  - **Property 11: Culling margin coverage**
  - **Validates: Requirements 6.3**

- [x] 6.3 Написати property test для відсутності фіксованого ліміту
  - **Property 12: No fixed tile limit**
  - **Validates: Requirements 6.5**

- [x] 7. Checkpoint - Перевірити UI та відображення
  - Скомпілювати код
  - Запустити гру та перевірити що панель замовлення з'являється при кліку на HQ
  - Перевірити що ресурси відображаються в правильних позиціях
  - Перевірити що вся мапа видима у в'юпорті
  - Ensure all tests pass, ask the user if questions arise.

- [x] 8. Інтегрувати всі компоненти в main.cpp
  - Оновити функцію `InitBuildings()` для використання FactionSpawner
  - Замінити ручне розміщення будівель на автоматичний спавн через BuildingPlacer
  - Додати UnitOrderPanel в ігровий цикл
  - Замінити існуючий код відображення ресурсів на ResourceDisplay
  - Оновити функцію `DrawGame()` для використання виправленого viewport culling
  - Видалити старий код `DrawUnitOrderPanel()` з main.cpp
  - _Requirements: 1.1, 2.4, 3.1, 4.1, 5.1, 6.1_

- [x] 8.1 Написати integration test для повного workflow
  - Тест: спавн HQ → клік на HQ → панель з'являється → замовлення Worker
  - _Requirements: 3.1, 4.1, 4.3, 4.4_

- [x] 9. Оптимізація та тестування продуктивності
  - Виміряти FPS на карті 50x50 з 50 будівлями
  - Виміряти час розміщення 100 будівель
  - Виміряти час розрахунку viewport culling
  - Оптимізувати якщо FPS < 60 або час > вимог
  - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5_

- [x] 9.1 Написати performance test для viewport culling
  - Перевірити що FPS >= 60 на карті 50x50
  - _Requirements: 7.5_

- [x] 10. Final checkpoint - Повне тестування системи
  - Запустити всі unit tests
  - Запустити всі property tests
  - Запустити integration tests
  - Перевірити всі вимоги вручну
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation
- Property tests validate universal correctness properties
- Unit tests validate specific examples and edge cases
- Існуючі компоненти (CoordinateConverter, IsometricRenderer, PathfindingManager) вже реалізовані та будуть використані
- Нові компоненти інтегруються з існуючою архітектурою без breaking changes
