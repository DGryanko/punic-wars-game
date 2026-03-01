# Implementation Plan: Building Textures System

## Overview

Поетапна імплементація системи текстур для будівель. Кожен крок будує на попередніх, забезпечуючи інкрементальний прогрес з ранньою валідацією функціоналу.

## Tasks

- [x] 1. Підготовка ресурсів та структури
  - Скопіювати Praetorium.png з Assets/ до cpp/assets/sprites/
  - Створити заглушки для інших текстур будівель (fallback кольорові квадрати)
  - Перевірити, що файл існує та доступний
  - _Requirements: 1.1, 1.3_

- [ ] 2. Створити BuildingTextureManager
  - [x] 2.1 Створити header файл building_texture_manager.h
    - Оголосити singleton клас
    - Додати map для зберігання текстур
    - Оголосити методи loadAllTextures(), getTexture(), hasTexture()
    - _Requirements: 1.1, 1.4_

  - [x] 2.2 Реалізувати building_texture_manager.cpp
    - Реалізувати singleton pattern (getInstance)
    - Реалізувати loadTexture() з обробкою помилок
    - Реалізувати createFallbackTexture() для кожного типу
    - Реалізувати loadAllTextures() для всіх 5 типів будівель
    - _Requirements: 1.1, 1.2, 3.3_

  - [x] 2.3 Написати unit тести для BuildingTextureManager
    - Тест завантаження існуючого файлу
    - Тест fallback для неіснуючого файлу
    - Тест завантаження всіх 5 типів
    - _Requirements: 1.1, 1.2, 1.3_

  - [x] 2.4 Написати property тест для кешування текстур
    - **Property 1: Texture Caching**
    - **Validates: Requirements 1.4, 4.1**

- [ ] 3. Checkpoint - Перевірка завантаження текстур
  - Переконатися, що всі тести проходять
  - Запитати користувача, чи є питання

- [ ] 4. Розширити структуру Building
  - [x] 4.1 Додати нові поля до building.h
    - Додати bool use_texture
    - Додати Vector2 texture_offset
    - Додати float texture_scale
    - Оновити метод init() для ініціалізації нових полів
    - _Requirements: 3.1, 3.2_

  - [x] 4.2 Додати метод getTextureRect() до Building
    - Розрахувати Rectangle для текстури з урахуванням offset та scale
    - _Requirements: 2.3_

  - [x] 4.3 Написати unit тест для ініціалізації Building
    - Тест, що кожен тип будівлі має правильний texture path
    - Тест, що римські будівлі мають римські текстури
    - Тест, що карфагенські будівлі мають карфагенські текстури
    - _Requirements: 3.1, 3.2, 3.4, 3.5_

  - [x] 4.4 Написати property тест для автоматичного призначення текстур
    - **Property 2: Automatic Texture Assignment**
    - **Validates: Requirements 3.2**

  - [x] 4.5 Написати property тести для фракційних текстур
    - **Property 3: Faction-Specific Textures (Rome)**
    - **Property 4: Faction-Specific Textures (Carthage)**
    - **Validates: Requirements 3.4, 3.5**

- [ ] 5. Створити BuildingRenderer
  - [x] 5.1 Створити building_renderer.h та building_renderer.cpp
    - Реалізувати статичний метод drawBuilding()
    - Реалізувати drawWithTexture() для малювання з текстурою
    - Реалізувати drawSelectionIndicator() для виділення
    - Реалізувати drawProductionProgress() для прогрес-бару
    - _Requirements: 2.1, 2.2, 2.4_

  - [x] 5.2 Додати viewport culling до BuildingRenderer
    - Перевірка чи будівля в межах екрану перед малюванням
    - Використати Camera2D для розрахунку видимості
    - _Requirements: 4.3_

  - [x] 5.3 Написати property тест для viewport culling
    - **Property 5: Viewport Culling**
    - **Validates: Requirements 4.3**

- [ ] 6. Checkpoint - Перевірка рендерингу
  - Переконатися, що всі тести проходять
  - Запитати користувача, чи є питання

- [ ] 7. Інтеграція з main.cpp
  - [x] 7.1 Ініціалізувати BuildingTextureManager в main()
    - Викликати loadAllTextures() при старті гри
    - Додати обробку помилок завантаження
    - _Requirements: 1.1_

  - [x] 7.2 Оновити InitBuildings() для використання текстур
    - Встановити use_texture = true для всіх будівель
    - Встановити правильні offset та scale для кожного типу
    - _Requirements: 3.2_

  - [x] 7.3 Замінити Building::draw() на BuildingRenderer::drawBuilding()
    - Оновити game loop для використання нового рендерера
    - Зберегти fallback на кольорові прямокутники
    - _Requirements: 2.1, 5.2_

  - [x] 7.4 Написати integration тести
    - Тест повного циклу: ініціалізація → малювання → cleanup
    - Тест взаємодії BuildingTextureManager та Building
    - _Requirements: 5.1, 5.4_

- [ ] 8. Тестування зворотної сумісності
  - [x] 8.1 Перевірити всі існуючі функції Building
    - Тест методів init(), getRect(), getColor()
    - Тест виділення (selected)
    - Тест виробництва (startProduction, updateProduction)
    - Тест взаємодії (isClicked)
    - _Requirements: 5.1, 5.4_

  - [x] 8.2 Перевірити fallback на кольорові прямокутники
    - Тест малювання без текстури (use_texture = false)
    - Тест малювання з неіснуючою текстурою
    - _Requirements: 5.2_

  - [x] 8.3 Написати unit тести для зворотної сумісності
    - Тест всіх існуючих методів Building
    - Тест fallback поведінки
    - _Requirements: 5.1, 5.2, 5.4_

- [x] 9. Фінальний checkpoint
  - Запустити всі тести (unit + property + integration)
  - Перевірити продуктивність (60 FPS з 20 будівлями)
  - Запитати користувача про фінальний огляд

- [x] 10. Cleanup та документація
  - Видалити застарілий код (якщо є)
  - Додати коментарі до нових класів
  - Оновити README з інформацією про текстури
  - _Requirements: 5.3_

## Notes

- Кожне завдання посилається на конкретні вимоги для трасування
- Checkpoints забезпечують інкрементальну валідацію
- Property тести валідують універсальні властивості коректності
- Unit тести валідують конкретні приклади та edge cases
