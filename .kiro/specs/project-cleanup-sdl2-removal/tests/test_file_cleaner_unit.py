"""Unit Tests для FileCleaner

Цей модуль містить unit тести для перевірки конкретних прикладів та edge cases
для FileCleaner.

Feature: project-cleanup-sdl2-removal
Validates: Requirements 1.1, 1.2, 2.2, 5.3
"""

import os
import tempfile
import shutil
from pathlib import Path

import pytest

import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from src.file_cleaner import FileCleaner
from src.exceptions import FileSystemError


class TestFileCleanerUnit:
    """Unit тести для FileCleaner"""
    
    def setup_method(self):
        """Налаштування перед кожним тестом"""
        self.temp_dir = tempfile.mkdtemp()
        self.original_cwd = os.getcwd()
        os.chdir(self.temp_dir)
        
        self.file_cleaner = FileCleaner(project_root=self.temp_dir)
    
    def teardown_method(self):
        """Очищення після кожного тесту"""
        os.chdir(self.original_cwd)
        shutil.rmtree(self.temp_dir, ignore_errors=True)
    
    def test_delete_sdl2_migration_todo(self):
        """Тест видалення cpp/SDL2_MIGRATION_TODO.md
        
        **Validates: Requirements 1.1**
        
        Перевіряє, що FileCleaner коректно видаляє файл SDL2_MIGRATION_TODO.md
        з директорії cpp/.
        """
        # Створити структуру директорій
        os.makedirs("cpp", exist_ok=True)
        
        # Створити файл SDL2_MIGRATION_TODO.md
        sdl2_migration_file = "cpp/SDL2_MIGRATION_TODO.md"
        with open(sdl2_migration_file, 'w') as f:
            f.write("# SDL2 Migration TODO\n\nThis is a migration guide.")
        
        # Перевірити, що файл існує
        assert os.path.exists(sdl2_migration_file), \
            "Файл SDL2_MIGRATION_TODO.md має існувати перед видаленням"
        
        # Видалити SDL2 файли
        deleted_files = self.file_cleaner.delete_sdl2_files()
        
        # Перевірити, що файл видалено
        assert not os.path.exists(sdl2_migration_file), \
            "Файл SDL2_MIGRATION_TODO.md має бути видалено"
        
        # Перевірити, що файл у списку видалених
        assert "cpp/SDL2_MIGRATION_TODO.md" in deleted_files, \
            "SDL2_MIGRATION_TODO.md має бути у списку видалених файлів"
    
    def test_delete_quick_start_sdl2(self):
        """Тест видалення cpp/QUICK_START_SDL2.md
        
        **Validates: Requirements 1.2**
        
        Перевіряє, що FileCleaner коректно видаляє файл QUICK_START_SDL2.md
        з директорії cpp/.
        """
        # Створити структуру директорій
        os.makedirs("cpp", exist_ok=True)
        
        # Створити файл QUICK_START_SDL2.md
        quick_start_file = "cpp/QUICK_START_SDL2.md"
        with open(quick_start_file, 'w') as f:
            f.write("# Quick Start with SDL2\n\nHow to use SDL2 in this project.")
        
        # Перевірити, що файл існує
        assert os.path.exists(quick_start_file), \
            "Файл QUICK_START_SDL2.md має існувати перед видаленням"
        
        # Видалити SDL2 файли
        deleted_files = self.file_cleaner.delete_sdl2_files()
        
        # Перевірити, що файл видалено
        assert not os.path.exists(quick_start_file), \
            "Файл QUICK_START_SDL2.md має бути видалено"
        
        # Перевірити, що файл у списку видалених
        assert "cpp/QUICK_START_SDL2.md" in deleted_files, \
            "QUICK_START_SDL2.md має бути у списку видалених файлів"
    
    def test_delete_both_sdl2_files(self):
        """Тест видалення обох SDL2 файлів одночасно
        
        **Validates: Requirements 1.1, 1.2**
        
        Перевіряє, що FileCleaner може видалити обидва SDL2 файли
        за один виклик методу.
        """
        # Створити структуру директорій
        os.makedirs("cpp", exist_ok=True)
        
        # Створити обидва файли
        sdl2_files = [
            "cpp/SDL2_MIGRATION_TODO.md",
            "cpp/QUICK_START_SDL2.md"
        ]
        
        for filepath in sdl2_files:
            with open(filepath, 'w') as f:
                f.write(f"Content of {filepath}")
        
        # Перевірити, що обидва файли існують
        for filepath in sdl2_files:
            assert os.path.exists(filepath), \
                f"Файл {filepath} має існувати перед видаленням"
        
        # Видалити SDL2 файли
        deleted_files = self.file_cleaner.delete_sdl2_files()
        
        # Перевірити, що обидва файли видалено
        for filepath in sdl2_files:
            assert not os.path.exists(filepath), \
                f"Файл {filepath} має бути видалено"
        
        # Перевірити, що обидва файли у списку видалених
        assert len(deleted_files) == 2, \
            "Має бути видалено 2 файли"
        assert "cpp/SDL2_MIGRATION_TODO.md" in deleted_files
        assert "cpp/QUICK_START_SDL2.md" in deleted_files
    
    def test_delete_sdl2_files_when_not_exist(self):
        """Тест видалення SDL2 файлів, коли вони не існують
        
        **Validates: Requirements 1.1, 1.2**
        
        Edge case: перевіряє, що FileCleaner не викликає помилку,
        якщо SDL2 файли вже не існують.
        """
        # Не створюємо файли
        
        # Видалити SDL2 файли (які не існують)
        deleted_files = self.file_cleaner.delete_sdl2_files()
        
        # Список видалених файлів має бути порожнім
        assert len(deleted_files) == 0, \
            "Список видалених файлів має бути порожнім, якщо файли не існують"
    
    def test_clean_tz_vid_groka(self):
        """Тест очищення файлу "ТЗ від грока"
        
        **Validates: Requirements 2.2**
        
        Перевіряє, що FileCleaner коректно очищає згадки про SDL2
        з файлу "ТЗ від грока".
        """
        # Створити файл "ТЗ від грока" зі згадками SDL2
        tz_file = "ТЗ від грока"
        original_content = """# Технічне завдання від грока

## Графічна бібліотека
Використовуємо SDL2 для рендерингу.
SDL2 надає кросплатформенний API.

## Історія розробки
Раніше ми використовували SDL2, але перейшли на Raylib.

## Поточний стан
Зараз використовуємо Raylib для всього.
"""
        
        with open(tz_file, 'w', encoding='utf-8') as f:
            f.write(original_content)
        
        # Очистити документацію
        modifications = self.file_cleaner.clean_documentation([tz_file])
        
        # Перевірити, що файл модифіковано
        assert tz_file in modifications, \
            "Файл 'ТЗ від грока' має бути у списку модифікованих"
        
        # Перевірити, що були зміни
        assert "Видалено" in modifications[tz_file], \
            "Має бути повідомлення про видалення рядків"
        
        # Прочитати модифікований файл
        with open(tz_file, 'r', encoding='utf-8') as f:
            modified_content = f.read()
        
        # Перевірити, що неісторичні згадки SDL2 видалено
        lines = modified_content.split('\n')
        non_historical_lines = [
            line for line in lines 
            if not self.file_cleaner._is_historical_context(line)
        ]
        
        for line in non_historical_lines:
            # Перевірити, що SDL2 згадки видалено з неісторичних рядків
            if line.strip() and not line.startswith('#'):
                assert 'SDL2' not in line, \
                    f"Неісторичний рядок не має містити SDL2: {line}"
        
        # Перевірити, що історичний контекст збережено
        assert "Раніше ми використовували SDL2" in modified_content, \
            "Історичний контекст має бути збережено"
    
    def test_clean_documentation_preserves_historical_context(self):
        """Тест збереження історичного контексту при очищенні
        
        **Validates: Requirements 2.4**
        
        Перевіряє, що FileCleaner зберігає рядки з історичним контекстом,
        навіть якщо вони містять згадки про SDL2.
        """
        # Створити файл з історичним контекстом
        doc_file = "history.txt"
        content = """# Історія проекту

Раніше ми використовували SDL2 для рендерингу.
В минулому була спроба міграції на SDL2.
Архів старих версій містить SDL2 код.

Зараз використовуємо SDL2 для всього.
"""
        
        with open(doc_file, 'w', encoding='utf-8') as f:
            f.write(content)
        
        # Очистити документацію
        self.file_cleaner.clean_documentation([doc_file])
        
        # Прочитати модифікований файл
        with open(doc_file, 'r', encoding='utf-8') as f:
            modified_content = f.read()
        
        # Перевірити, що історичні рядки збережено
        assert "Раніше ми використовували SDL2" in modified_content, \
            "Історичний рядок з 'Раніше' має бути збережено"
        assert "В минулому була спроба міграції на SDL2" in modified_content, \
            "Історичний рядок з 'минулому' має бути збережено"
        assert "Архів старих версій містить SDL2 код" in modified_content, \
            "Історичний рядок з 'Архів' має бути збережено"
        
        # Перевірити, що неісторичний рядок видалено
        assert "Зараз використовуємо SDL2 для всього" not in modified_content, \
            "Неісторичний рядок з SDL2 має бути видалено"
    
    def test_clean_documentation_file_not_found(self):
        """Тест очищення неіснуючого файлу
        
        **Validates: Requirements 2.2**
        
        Edge case: перевіряє, що FileCleaner коректно обробляє
        спробу очищення неіснуючого файлу.
        """
        nonexistent_file = "nonexistent.txt"
        
        # Спроба очистити неіснуючий файл
        modifications = self.file_cleaner.clean_documentation([nonexistent_file])
        
        # Перевірити, що файл у результатах з повідомленням про помилку
        assert nonexistent_file in modifications, \
            "Неіснуючий файл має бути у результатах"
        assert "не знайдено" in modifications[nonexistent_file].lower(), \
            "Має бути повідомлення про те, що файл не знайдено"
    
    def test_legacy_files_list_created(self):
        """Тест створення списку застарілих файлів
        
        **Validates: Requirements 5.3**
        
        Перевіряє, що FileCleaner створює список потенційно застарілих файлів
        на основі імен файлів та їх вмісту.
        """
        # Створити структуру директорій
        os.makedirs("cpp/scripts", exist_ok=True)
        
        # Створити застарілі файли
        legacy_files_to_create = [
            ("cpp/old_sdl2_compile.bat", "g++ -lSDL2 main.cpp"),
            ("cpp/scripts/sdl_setup.sh", "#!/bin/bash\nsdl2-config --cflags"),
            ("cpp/backup_old.txt", "Some backup content"),
            ("cpp/deprecated_module.cpp", "// Old code")
        ]
        
        for filepath, content in legacy_files_to_create:
            with open(filepath, 'w') as f:
                f.write(content)
        
        # Створити нормальні файли (не застарілі)
        normal_files = [
            ("cpp/main.cpp", "#include <raylib.h>\nint main() {}"),
            ("cpp/README.md", "# Project README")
        ]
        
        for filepath, content in normal_files:
            with open(filepath, 'w') as f:
                f.write(content)
        
        # Знайти застарілі файли
        legacy_files = self.file_cleaner.find_legacy_files("cpp")
        
        # Перевірити, що список не порожній
        assert len(legacy_files) > 0, \
            "Список застарілих файлів не має бути порожнім"
        
        # Перевірити, що застарілі файли знайдено
        legacy_paths = [item['path'] for item in legacy_files]
        
        assert any('old_sdl2_compile.bat' in path for path in legacy_paths), \
            "Файл з 'sdl2' у назві має бути знайдено"
        assert any('sdl_setup.sh' in path for path in legacy_paths), \
            "Файл з 'sdl' у назві має бути знайдено"
        assert any('backup_old.txt' in path for path in legacy_paths), \
            "Файл з 'backup' та 'old' має бути знайдено"
        assert any('deprecated_module.cpp' in path for path in legacy_paths), \
            "Файл з 'deprecated' має бути знайдено"
        
        # Перевірити, що нормальні файли не у списку
        assert not any('main.cpp' in path for path in legacy_paths), \
            "Нормальний файл main.cpp не має бути у списку застарілих"
        assert not any('README.md' in path for path in legacy_paths), \
            "Нормальний файл README.md не має бути у списку застарілих"
        
        # Перевірити структуру елементів списку
        for item in legacy_files:
            assert 'path' in item, "Кожен елемент має містити 'path'"
            assert 'name' in item, "Кожен елемент має містити 'name'"
            assert 'reason' in item, "Кожен елемент має містити 'reason'"
            assert 'recommendation' in item, "Кожен елемент має містити 'recommendation'"
    
    def test_legacy_files_sdl2_commands_in_scripts(self):
        """Тест виявлення SDL2 команд у скриптах компіляції
        
        **Validates: Requirements 5.4**
        
        Перевіряє, що FileCleaner знаходить скрипти .bat та .sh,
        які містять команди компіляції з SDL2.
        """
        # Створити директорію
        os.makedirs("cpp", exist_ok=True)
        
        # Створити .bat файл з SDL2 командами
        bat_file = "cpp/compile_sdl2.bat"
        with open(bat_file, 'w') as f:
            f.write("@echo off\ng++ -o game.exe main.cpp -lSDL2 -lSDL2_image\n")
        
        # Створити .sh файл з SDL2 командами
        sh_file = "cpp/build_sdl2.sh"
        with open(sh_file, 'w') as f:
            f.write("#!/bin/bash\ng++ main.cpp `sdl2-config --cflags --libs`\n")
        
        # Створити .bat файл без SDL2 команд
        normal_bat = "cpp/compile_raylib.bat"
        with open(normal_bat, 'w') as f:
            f.write("@echo off\ng++ -o game.exe main.cpp -lraylib\n")
        
        # Знайти застарілі файли
        legacy_files = self.file_cleaner.find_legacy_files("cpp")
        
        # Перевірити, що скрипти з SDL2 командами знайдено
        legacy_paths = [item['path'] for item in legacy_files]
        legacy_reasons = {item['path']: item['reason'] for item in legacy_files}
        
        # Файли з SDL2 у назві будуть знайдено через ім'я
        assert any('compile_sdl2.bat' in path for path in legacy_paths), \
            "Bat файл з SDL2 командами має бути знайдено"
        assert any('build_sdl2.sh' in path for path in legacy_paths), \
            "Shell файл з SDL2 командами має бути знайдено"
        
        # Перевірити причину знаходження
        for path, reason in legacy_reasons.items():
            if 'compile_sdl2.bat' in path or 'build_sdl2.sh' in path:
                # Може бути знайдено або через ім'я (застарілі маркери), або через вміст (SDL2 команди)
                assert 'застарілі' in reason.lower() or 'SDL2' in reason or 'sdl' in reason.lower(), \
                    f"Причина має згадувати застарілі маркери або SDL2 для {path}"
    
    def test_legacy_files_empty_directory(self):
        """Тест пошуку застарілих файлів у порожній директорії
        
        **Validates: Requirements 5.3**
        
        Edge case: перевіряє, що FileCleaner коректно обробляє
        порожню директорію.
        """
        # Створити порожню директорію
        os.makedirs("empty_dir", exist_ok=True)
        
        # Знайти застарілі файли
        legacy_files = self.file_cleaner.find_legacy_files("empty_dir")
        
        # Список має бути порожнім
        assert len(legacy_files) == 0, \
            "Список застарілих файлів має бути порожнім для порожньої директорії"
    
    def test_legacy_files_nonexistent_directory(self):
        """Тест пошуку застарілих файлів у неіснуючій директорії
        
        **Validates: Requirements 5.3**
        
        Edge case: перевіряє, що FileCleaner коректно обробляє
        неіснуючу директорію.
        """
        # Знайти застарілі файли у неіснуючій директорії
        legacy_files = self.file_cleaner.find_legacy_files("nonexistent_dir")
        
        # Список має бути порожнім
        assert len(legacy_files) == 0, \
            "Список застарілих файлів має бути порожнім для неіснуючої директорії"
    
    def test_find_sdl2_references_in_multiple_files(self):
        """Тест пошуку SDL2 згадок у множині файлів
        
        **Validates: Requirements 2.1**
        
        Перевіряє, що FileCleaner знаходить SDL2 згадки у всіх файлах,
        які їх містять.
        """
        # Створити файли з SDL2 згадками
        files_with_sdl2 = {
            "file1.txt": "This project uses SDL2 library",
            "file2.md": "# SDL2 Integration Guide",
            "file3.cpp": "#include <SDL2/SDL.h>"
        }
        
        for filepath, content in files_with_sdl2.items():
            with open(filepath, 'w') as f:
                f.write(content)
        
        # Створити файли без SDL2 згадок
        files_without_sdl2 = {
            "normal1.txt": "This is a normal file",
            "normal2.md": "# Project Documentation"
        }
        
        for filepath, content in files_without_sdl2.items():
            with open(filepath, 'w') as f:
                f.write(content)
        
        # Знайти SDL2 згадки
        all_files = list(files_with_sdl2.keys()) + list(files_without_sdl2.keys())
        references = self.file_cleaner.find_sdl2_references(all_files)
        
        # Перевірити, що всі файли з SDL2 знайдено
        for filepath in files_with_sdl2.keys():
            assert filepath in references, \
                f"Файл {filepath} з SDL2 згадками має бути у результатах"
        
        # Перевірити, що файли без SDL2 не у результатах
        for filepath in files_without_sdl2.keys():
            assert filepath not in references, \
                f"Файл {filepath} без SDL2 згадок не має бути у результатах"
    
    def test_clean_documentation_no_sdl2_references(self):
        """Тест очищення файлу без SDL2 згадок
        
        **Validates: Requirements 2.2**
        
        Перевіряє, що FileCleaner коректно обробляє файли,
        які не містять SDL2 згадок.
        """
        # Створити файл без SDL2 згадок (уникаємо слова "SDL" навіть у контексті)
        clean_file = "clean_doc.txt"
        original_content = "This is a clean document about Raylib.\nNo graphics library mentions here."
        
        with open(clean_file, 'w', encoding='utf-8') as f:
            f.write(original_content)
        
        # Очистити документацію
        modifications = self.file_cleaner.clean_documentation([clean_file])
        
        # Перевірити, що файл у результатах
        assert clean_file in modifications, \
            "Файл має бути у результатах"
        
        # Перевірити повідомлення
        assert "не знайдено" in modifications[clean_file].lower(), \
            "Має бути повідомлення про відсутність SDL2 згадок"
        
        # Перевірити, що файл не змінився
        with open(clean_file, 'r', encoding='utf-8') as f:
            current_content = f.read()
        
        assert current_content == original_content, \
            "Файл без SDL2 згадок має залишитися незмінним"
