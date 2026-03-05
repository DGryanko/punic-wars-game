# Requirements Document

## Introduction

Цей документ описує вимоги до очищення проекту "Punic Wars: Castra" від застарілих файлів та документації, пов'язаних з невдалою міграцією на SDL2. Проект повністю працює на Raylib, але містить залишки документації про спробу міграції на SDL2, яка не була завершена. Мета - видалити всі непотрібні файли та згадки про SDL2, зберігаючи при цьому повну працездатність поточної версії гри.

## Glossary

- **Project**: Проект гри "Punic Wars: Castra"
- **SDL2_Documentation**: Файли документації, що описують міграцію на SDL2 (SDL2_MIGRATION_TODO.md, QUICK_START_SDL2.md)
- **SDL2_Branch**: Git гілка з назвою "SDL2", що містить код міграції
- **Legacy_Files**: Застарілі файли, що більше не використовуються в проекті
- **Working_Code**: Поточний робочий код гри на Raylib
- **Git_Repository**: Локальний та віддалений Git репозиторій проекту
- **Documentation_Files**: Файли документації в кореневій директорії та cpp/ директорії
- **Cleanup_System**: Система для безпечного видалення непотрібних файлів

## Requirements

### Requirement 1: Видалення SDL2 документації

**User Story:** Як розробник, я хочу видалити всю документацію про SDL2 міграцію, щоб уникнути плутанини та зберегти актуальну документацію тільки про Raylib.

#### Acceptance Criteria

1. THE Cleanup_System SHALL видалити файл cpp/SDL2_MIGRATION_TODO.md
2. THE Cleanup_System SHALL видалити файл cpp/QUICK_START_SDL2.md
3. WHEN файли SDL2_Documentation видалені, THE Project SHALL зберегти всі інші файли документації без змін
4. THE Cleanup_System SHALL перевірити, що файли cpp/README.md, cpp/SETUP.md та інші документи залишаються незмінними

### Requirement 2: Очищення згадок про SDL2 в документації

**User Story:** Як розробник, я хочу видалити всі згадки про SDL2 з існуючої документації, щоб документація відображала тільки поточну архітектуру на Raylib.

#### Acceptance Criteria

1. THE Cleanup_System SHALL знайти всі згадки про SDL2 в файлах Documentation_Files
2. WHEN згадка про SDL2 знайдена в файлі "ТЗ від грока", THE Cleanup_System SHALL видалити або оновити відповідний розділ
3. WHEN згадка про SDL2 знайдена в файлі "Переписка з грок вихідні дані.txt", THE Cleanup_System SHALL видалити або оновити відповідний розділ
4. THE Cleanup_System SHALL зберегти історичний контекст, якщо згадки про SDL2 є частиною історії розробки
5. THE Cleanup_System SHALL створити резервну копію файлів перед модифікацією

### Requirement 3: Видалення SDL2 гілки з Git

**User Story:** Як розробник, я хочу видалити гілку SDL2 з локального та віддаленого репозиторію, щоб уникнути випадкового використання застарілого коду.

#### Acceptance Criteria

1. THE Cleanup_System SHALL перевірити, що поточна активна гілка не є SDL2
2. WHEN поточна гілка не є SDL2, THE Cleanup_System SHALL видалити локальну гілку SDL2
3. THE Cleanup_System SHALL видалити віддалену гілку SDL2 з Git_Repository
4. WHEN гілка SDL2 видалена, THE Cleanup_System SHALL перевірити, що гілка master залишається незмінною
5. IF гілка SDL2 не існує локально або віддалено, THEN THE Cleanup_System SHALL повідомити про це без помилки

### Requirement 4: Перевірка працездатності після очищення

**User Story:** Як розробник, я хочу переконатися, що після видалення файлів гра залишається повністю працездатною, щоб уникнути випадкового видалення важливих файлів.

#### Acceptance Criteria

1. WHEN очищення завершено, THE Cleanup_System SHALL перевірити наявність всіх критичних файлів Working_Code
2. THE Cleanup_System SHALL перевірити, що файл cpp/src/main.cpp не містить включень SDL2
3. THE Cleanup_System SHALL перевірити, що файл cpp/compile.bat працює без помилок
4. THE Cleanup_System SHALL перевірити, що всі файли в cpp/src/ директорії не містять включень SDL2
5. WHEN перевірка завершена, THE Cleanup_System SHALL створити звіт про успішне очищення

### Requirement 5: Ідентифікація інших застарілих файлів

**User Story:** Як розробник, я хочу ідентифікувати інші потенційно застарілі файли, щоб підтримувати проект в чистому стані.

#### Acceptance Criteria

1. THE Cleanup_System SHALL проаналізувати всі файли в cpp/ директорії на наявність застарілих скриптів компіляції
2. THE Cleanup_System SHALL перевірити наявність файлів з назвами, що містять "sdl", "SDL", "sdl2", "SDL2"
3. WHEN знайдено потенційно застарілі файли, THE Cleanup_System SHALL створити список для ручного перегляду
4. THE Cleanup_System SHALL перевірити файли .bat та .sh на наявність команд компіляції з SDL2
5. THE Cleanup_System SHALL створити рекомендації щодо видалення або збереження кожного знайденого файлу

### Requirement 6: Оновлення основної документації

**User Story:** Як розробник, я хочу оновити основну документацію проекту, щоб вона відображала тільки поточний стан на Raylib без згадок про SDL2.

#### Acceptance Criteria

1. THE Cleanup_System SHALL перевірити файл cpp/README.md на наявність згадок про SDL2
2. WHEN файл cpp/README.md містить згадки про SDL2, THE Cleanup_System SHALL видалити ці згадки
3. THE Cleanup_System SHALL переконатися, що файл cpp/SETUP.md містить тільки інструкції для Raylib
4. THE Cleanup_System SHALL оновити файл SESSION_CONTEXT.md, якщо він містить застарілу інформацію про SDL2
5. WHEN оновлення завершено, THE Cleanup_System SHALL зберегти всю актуальну інформацію про Raylib

### Requirement 7: Створення звіту про очищення

**User Story:** Як розробник, я хочу отримати детальний звіт про виконане очищення, щоб знати, які саме зміни були внесені в проект.

#### Acceptance Criteria

1. THE Cleanup_System SHALL створити звіт з переліком всіх видалених файлів
2. THE Cleanup_System SHALL включити в звіт список всіх модифікованих файлів з описом змін
3. THE Cleanup_System SHALL включити в звіт інформацію про видалені git гілки
4. THE Cleanup_System SHALL включити в звіт результати перевірки працездатності
5. WHEN звіт створено, THE Cleanup_System SHALL зберегти його в директорії .kiro/specs/project-cleanup-sdl2-removal/

### Requirement 8: Безпечне видалення з можливістю відкату

**User Story:** Як розробник, я хочу мати можливість відкотити зміни, якщо щось піде не так під час очищення, щоб уникнути втрати важливих даних.

#### Acceptance Criteria

1. WHEN очищення починається, THE Cleanup_System SHALL створити git commit з поточним станом проекту
2. THE Cleanup_System SHALL створити резервні копії всіх файлів, що будуть модифіковані
3. THE Cleanup_System SHALL зберегти резервні копії в тимчасовій директорії
4. IF під час очищення виникає помилка, THEN THE Cleanup_System SHALL запропонувати відкотити зміни
5. WHEN очищення успішно завершено, THE Cleanup_System SHALL видалити тимчасові резервні копії після підтвердження користувача
