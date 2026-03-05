# Implementation Plan: SDL2 Cleanup System

## Overview

Цей план описує покрокову імплементацію системи для безпечного очищення проекту "Punic Wars: Castra" від застарілих SDL2 файлів та документації. Система складається з 6 компонентів (Backup Manager, File Cleaner, Git Manager, Validation Engine, Report Generator, Cleanup Orchestrator), які виконуються послідовно з можливістю відкату змін.

## Tasks

- [x] 1. Створити структуру проекту та базові моделі даних
  - Створити директорію `.kiro/specs/project-cleanup-sdl2-removal/src/`
  - Створити файл `models.py` з dataclass моделями: CleanupConfig, CleanupResult, FileReference, ValidationResult
  - Створити файл `exceptions.py` з класами помилок: CleanupError, FileSystemError, GitError, ValidationError, BackupError
  - Створити файл `__init__.py` для Python пакету
  - _Requirements: 1.1, 1.2, 2.5, 8.1_

- [x] 2. Імплементувати Backup Manager
  - [x] 2.1 Створити клас BackupManager з методами для резервного копіювання
    - Імплементувати метод `create_safety_commit()` для створення git commit
    - Імплементувати метод `create_file_backups()` для копіювання файлів
    - Імплементувати метод `cleanup_backups()` для видалення резервних копій
    - Імплементувати метод `rollback_changes()` для відкату змін
    - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5_
  
  - [x]* 2.2 Написати property тест для BackupManager
    - **Property 3: Резервне копіювання перед модифікацією**
    - **Validates: Requirements 2.5, 8.2**
    - Перевірити, що для будь-якого файлу створюється резервна копія з ідентичним вмістом
  
  - [x]* 2.3 Написати unit тести для BackupManager
    - Тест `test_safety_commit_created()` для перевірки створення commit
    - Тест `test_backups_in_temp_directory()` для перевірки збереження резервних копій
    - Тест `test_rollback_on_error()` для перевірки відкату при помилці
    - Тест `test_cleanup_backups_after_success()` для перевірки видалення резервних копій
    - _Requirements: 8.1, 8.3, 8.4, 8.5_

- [x] 3. Імплементувати File Cleaner
  - [x] 3.1 Створити клас FileCleaner з методами для роботи з файлами
    - Імплементувати метод `find_sdl2_references()` для пошуку SDL2 згадок
    - Імплементувати метод `delete_sdl2_files()` для видалення SDL2 файлів
    - Імплементувати метод `clean_documentation()` для очищення документації
    - Імплементувати метод `find_legacy_files()` для пошуку застарілих файлів
    - _Requirements: 1.1, 1.2, 2.1, 2.2, 2.3, 5.1, 5.2_
  
  - [x]* 3.2 Написати property тест для пошуку SDL2 згадок
    - **Property 2: Повнота пошуку SDL2 згадок**
    - **Validates: Requirements 2.1, 5.2, 5.4**
    - Перевірити, що будь-який файл з SDL2 згадками буде знайдено
  
  - [x]* 3.3 Написати property тест для збереження незмінних файлів
    - **Property 1: Збереження незмінних файлів**
    - **Validates: Requirements 1.3, 1.4, 3.4**
    - Перевірити, що файли, які не є SDL2-специфічними, залишаються незмінними
  
  - [x]* 3.4 Написати unit тести для FileCleaner
    - Тест `test_delete_sdl2_migration_todo()` для видалення SDL2_MIGRATION_TODO.md
    - Тест `test_delete_quick_start_sdl2()` для видалення QUICK_START_SDL2.md
    - Тест `test_clean_tz_vid_groka()` для очищення файлу "ТЗ від грока"
    - Тест `test_legacy_files_list_created()` для створення списку застарілих файлів
    - _Requirements: 1.1, 1.2, 2.2, 5.3_

- [x] 4. Checkpoint - Перевірка базової функціональності
  - Ensure all tests pass, ask the user if questions arise.

- [x] 5. Імплементувати Git Manager
  - [x] 5.1 Створити клас GitManager з методами для git операцій
    - Імплементувати метод `get_current_branch()` для отримання поточної гілки
    - Імплементувати метод `branch_exists()` для перевірки існування гілки
    - Імплементувати метод `delete_local_branch()` для видалення локальної гілки
    - Імплементувати метод `delete_remote_branch()` для видалення віддаленої гілки
    - Імплементувати метод `verify_branch_integrity()` для перевірки цілісності гілки
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5_
  
  - [x]* 5.2 Написати unit тести для GitManager
    - Тест `test_current_branch_not_sdl2()` для перевірки поточної гілки
    - Тест `test_delete_local_sdl2_branch()` для видалення локальної гілки
    - Тест `test_branch_not_exists_no_error()` для edge case коли гілка не існує
    - _Requirements: 3.1, 3.2, 3.5_

- [x] 6. Імплементувати Validation Engine
  - [x] 6.1 Створити клас ValidationEngine з методами для перевірки
    - Імплементувати метод `verify_critical_files()` для перевірки критичних файлів
    - Імплементувати метод `check_sdl2_includes()` для перевірки SDL2 включень
    - Імплементувати метод `test_compilation()` для тестування компіляції
    - Імплементувати метод `run_validation_suite()` для повного набору перевірок
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5_
  
  - [x]* 6.2 Написати property тест для відсутності SDL2 включень
    - **Property 4: Відсутність SDL2 включень після очищення**
    - **Validates: Requirements 4.1, 4.4**
    - Перевірити, що після очищення жоден .cpp/.h файл не містить SDL2 включень
  
  - [x]* 6.3 Написати unit тести для ValidationEngine
    - Тест `test_main_cpp_no_sdl2_includes()` для перевірки main.cpp
    - Тест `test_compile_bat_works()` для перевірки compile.bat
    - Тест `test_readme_sdl2_check()` для перевірки README.md
    - _Requirements: 4.2, 4.3, 6.1_

- [x] 7. Імплементувати Report Generator
  - [x] 7.1 Створити клас ReportGenerator з методами для звітності
    - Імплементувати метод `add_deleted_files()` для додавання видалених файлів
    - Імплементувати метод `add_modified_files()` для додавання модифікованих файлів
    - Імплементувати метод `add_git_operations()` для додавання git операцій
    - Імплементувати метод `add_validation_results()` для додавання результатів валідації
    - Імплементувати метод `generate_report()` для генерації звіту
    - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5_
  
  - [x]* 7.2 Написати property тест для повноти звіту
    - **Property 5: Повнота звіту**
    - **Validates: Requirements 7.1, 7.2**
    - Перевірити, що будь-яка виконана операція присутня у звіті
  
  - [x]* 7.3 Написати unit тести для ReportGenerator
    - Тест `test_create_cleanup_report()` для створення звіту
    - Тест `test_report_deleted_files()` для наявності видалених файлів у звіті
    - Тест `test_report_validation_results()` для наявності результатів валідації
    - Тест `test_report_saved_to_correct_location()` для перевірки місця збереження
    - _Requirements: 4.5, 7.3, 7.4, 7.5_

- [x] 8. Checkpoint - Перевірка всіх компонентів
  - Ensure all tests pass, ask the user if questions arise.

- [x] 9. Імплементувати Cleanup Orchestrator
  - [x] 9.1 Створити клас CleanupOrchestrator для координації операцій
    - Ініціалізувати всі компоненти (BackupManager, FileCleaner, GitManager, ValidationEngine, ReportGenerator)
    - Імплементувати метод `execute_cleanup()` для виконання повного циклу очищення
    - Інтегрувати механізм `safe_execute_with_rollback()` для безпечного виконання
    - Додати обробку помилок та логування
    - _Requirements: 1.1, 1.2, 2.1, 3.2, 4.1, 7.1, 8.1_
  
  - [x]* 9.2 Написати інтеграційний тест для повного workflow
    - Тест `test_full_cleanup_workflow()` для перевірки повного циклу
    - Перевірити видалення файлів, оновлення документації, видалення гілки, валідацію
    - _Requirements: 1.1, 1.2, 3.2, 4.1, 7.1_

- [x] 10. Створити CLI інтерфейс та документацію
  - [x] 10.1 Створити головний скрипт cleanup.py
    - Додати argparse для обробки аргументів командного рядка
    - Додати опції: --dry-run, --no-backup, --skip-git, --verbose
    - Додати інтерактивне підтвердження перед виконанням операцій
    - Додати кольоровий вивід для зручності користувача
    - _Requirements: 8.1, 8.4_
  
  - [x] 10.2 Оновити документацію проекту
    - Оновити cpp/README.md - видалити згадки про SDL2
    - Оновити SESSION_CONTEXT.md - видалити застарілу інформацію
    - Створити README.md для cleanup системи з інструкціями використання
    - _Requirements: 6.1, 6.2, 6.4_
  
  - [x]* 10.3 Написати unit тести для CLI
    - Тест для перевірки аргументів командного рядка
    - Тест для dry-run режиму
    - Тест для інтерактивного підтвердження

- [-] 11. Фінальний checkpoint - Повна перевірка системи
  - Ensure all tests pass, ask the user if questions arise.
  - Виконати ручну перевірку: компіляція гри, запуск гри, перевірка документації
  - Переконатися, що всі 8 requirements виконані

## Notes

- Задачі, позначені `*`, є опціональними і можуть бути пропущені для швидшого MVP
- Кожна задача посилається на конкретні requirements для трасування
- Checkpoints забезпечують інкрементальну валідацію
- Property тести перевіряють універсальні властивості коректності (5 властивостей з design.md)
- Unit тести перевіряють конкретні приклади та edge cases
- Система використовує Python для імплементації
- Всі операції виконуються з можливістю відкату через Backup Manager
- Використовується бібліотека Hypothesis для property-based тестування
