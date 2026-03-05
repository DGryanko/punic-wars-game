"""Unit Tests для BackupManager

Цей модуль містить unit тести для перевірки конкретних прикладів та edge cases
для BackupManager.

Feature: project-cleanup-sdl2-removal
Validates: Requirements 8.1, 8.3, 8.4, 8.5
"""

import os
import tempfile
import shutil
import subprocess
from pathlib import Path

import pytest

import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from src.backup_manager import BackupManager
from src.exceptions import BackupError, GitError


class TestBackupManagerUnit:
    """Unit тести для BackupManager"""
    
    def setup_method(self):
        """Налаштування перед кожним тестом"""
        self.temp_dir = tempfile.mkdtemp()
        self.original_cwd = os.getcwd()
        os.chdir(self.temp_dir)
        
        # Ініціалізувати git репозиторій для тестів
        subprocess.run(["git", "init"], capture_output=True, check=True)
        subprocess.run(["git", "config", "user.email", "test@example.com"], 
                      capture_output=True, check=True)
        subprocess.run(["git", "config", "user.name", "Test User"], 
                      capture_output=True, check=True)
        
        self.backup_manager = BackupManager(
            backup_directory=os.path.join(self.temp_dir, "test_backups")
        )
    
    def teardown_method(self):
        """Очищення після кожного тесту"""
        os.chdir(self.original_cwd)
        shutil.rmtree(self.temp_dir, ignore_errors=True)
    
    def test_safety_commit_created(self):
        """Тест створення safety commit
        
        **Validates: Requirements 8.1**
        
        Перевіряє, що BackupManager створює git commit з поточним станом проекту
        та повертає валідний commit hash.
        """
        # Створити тестовий файл
        test_file = "test_file.txt"
        with open(test_file, 'w') as f:
            f.write("Test content")
        
        # Додати файл до git
        subprocess.run(["git", "add", test_file], capture_output=True, check=True)
        
        # Створити safety commit
        commit_hash = self.backup_manager.create_safety_commit()
        
        # Перевірити, що commit hash не порожній
        assert commit_hash, "Commit hash не має бути порожнім"
        
        # Перевірити, що це валідний SHA-1 hash (40 символів)
        assert len(commit_hash) == 40, "Commit hash має бути 40 символів"
        assert all(c in '0123456789abcdef' for c in commit_hash), \
            "Commit hash має містити тільки hex символи"
        
        # Перевірити, що commit існує в git
        result = subprocess.run(
            ["git", "rev-parse", commit_hash],
            capture_output=True,
            text=True,
            check=True
        )
        assert result.stdout.strip() == commit_hash, \
            "Commit має існувати в git репозиторії"
    
    def test_safety_commit_with_no_changes(self):
        """Тест створення safety commit коли немає змін
        
        **Validates: Requirements 8.1**
        
        Перевіряє, що BackupManager коректно обробляє ситуацію,
        коли немає змін для commit (повертає поточний HEAD).
        """
        # Створити початковий commit
        test_file = "initial.txt"
        with open(test_file, 'w') as f:
            f.write("Initial content")
        subprocess.run(["git", "add", test_file], capture_output=True, check=True)
        subprocess.run(["git", "commit", "-m", "Initial commit"], 
                      capture_output=True, check=True)
        
        # Отримати поточний HEAD
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            check=True
        )
        current_head = result.stdout.strip()
        
        # Спробувати створити safety commit без змін
        commit_hash = self.backup_manager.create_safety_commit()
        
        # Має повернути поточний HEAD
        assert commit_hash == current_head, \
            "Без змін має повернутися поточний HEAD"
    
    def test_backups_in_temp_directory(self):
        """Тест збереження резервних копій у тимчасовій директорії
        
        **Validates: Requirements 8.3**
        
        Перевіряє, що BackupManager зберігає резервні копії файлів
        у вказаній тимчасовій директорії з правильною структурою.
        """
        # Створити тестові файли
        test_files = []
        for i in range(3):
            filename = f"test_file_{i}.txt"
            content = f"Content of file {i}"
            with open(filename, 'w') as f:
                f.write(content)
            test_files.append(filename)
        
        # Створити резервні копії
        backup_dir = self.backup_manager.create_file_backups(test_files)
        
        # Перевірити, що backup директорія існує
        assert os.path.exists(backup_dir), \
            "Backup директорія має існувати"
        assert os.path.isdir(backup_dir), \
            "Backup шлях має бути директорією"
        
        # Перевірити, що backup директорія знаходиться у правильному місці
        assert "test_backups" in backup_dir, \
            "Backup директорія має бути у вказаному місці"
        
        # Перевірити наявність кожного файлу в backup
        for test_file in test_files:
            backup_file = os.path.join(backup_dir, test_file)
            assert os.path.exists(backup_file), \
                f"Резервна копія має існувати: {backup_file}"
            
            # Перевірити вміст
            with open(test_file, 'r') as orig, open(backup_file, 'r') as backup:
                assert orig.read() == backup.read(), \
                    f"Вміст резервної копії має співпадати з оригіналом"
    
    def test_backups_with_subdirectories(self):
        """Тест резервного копіювання файлів у піддиректоріях
        
        **Validates: Requirements 8.3**
        
        Перевіряє, що BackupManager коректно зберігає структуру директорій
        при створенні резервних копій.
        """
        # Створити структуру директорій
        os.makedirs("subdir1/subdir2", exist_ok=True)
        
        # Створити файли у різних директоріях
        test_files = [
            "root_file.txt",
            "subdir1/file1.txt",
            "subdir1/subdir2/file2.txt"
        ]
        
        for filepath in test_files:
            with open(filepath, 'w') as f:
                f.write(f"Content of {filepath}")
        
        # Створити резервні копії
        backup_dir = self.backup_manager.create_file_backups(test_files)
        
        # Перевірити структуру директорій у backup
        for filepath in test_files:
            backup_file = os.path.join(backup_dir, filepath)
            assert os.path.exists(backup_file), \
                f"Резервна копія має існувати зі збереженням структури: {backup_file}"
            
            # Перевірити вміст
            with open(filepath, 'r') as orig, open(backup_file, 'r') as backup:
                assert orig.read() == backup.read(), \
                    f"Вміст має співпадати для {filepath}"
    
    def test_rollback_on_error(self):
        """Тест відкату при помилці
        
        **Validates: Requirements 8.4**
        
        Перевіряє, що BackupManager може відкотити зміни до попереднього стану
        використовуючи git reset та резервні копії.
        """
        # Створити початковий файл та commit
        original_file = "original.txt"
        original_content = "Original content"
        with open(original_file, 'w') as f:
            f.write(original_content)
        
        subprocess.run(["git", "add", original_file], capture_output=True, check=True)
        subprocess.run(["git", "commit", "-m", "Initial commit"], 
                      capture_output=True, check=True)
        
        # Отримати commit hash
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            check=True
        )
        initial_commit = result.stdout.strip()
        
        # Створити резервну копію
        backup_dir = self.backup_manager.create_file_backups([original_file])
        
        # Модифікувати файл
        modified_content = "Modified content"
        with open(original_file, 'w') as f:
            f.write(modified_content)
        
        subprocess.run(["git", "add", original_file], capture_output=True, check=True)
        subprocess.run(["git", "commit", "-m", "Modified commit"], 
                      capture_output=True, check=True)
        
        # Перевірити, що файл модифіковано
        with open(original_file, 'r') as f:
            assert f.read() == modified_content, "Файл має бути модифікованим"
        
        # Виконати rollback
        self.backup_manager.rollback_changes(initial_commit, backup_dir)
        
        # Перевірити, що файл відновлено
        with open(original_file, 'r') as f:
            content = f.read()
            assert content == original_content, \
                f"Файл має бути відновлено до оригінального вмісту. Очікувалось: '{original_content}', отримано: '{content}'"
        
        # Перевірити, що git відкочено до початкового commit
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            check=True
        )
        current_commit = result.stdout.strip()
        assert current_commit == initial_commit, \
            "Git має бути відкочено до початкового commit"
    
    def test_cleanup_backups_after_success(self):
        """Тест видалення резервних копій після успіху
        
        **Validates: Requirements 8.5**
        
        Перевіряє, що BackupManager видаляє тимчасові резервні копії
        після підтвердження успішного завершення операції.
        """
        # Створити тестові файли
        test_files = ["file1.txt", "file2.txt", "file3.txt"]
        for filename in test_files:
            with open(filename, 'w') as f:
                f.write(f"Content of {filename}")
        
        # Створити резервні копії
        backup_dir = self.backup_manager.create_file_backups(test_files)
        
        # Перевірити, що backup директорія існує
        assert os.path.exists(backup_dir), \
            "Backup директорія має існувати перед cleanup"
        
        # Перевірити наявність файлів у backup
        for filename in test_files:
            backup_file = os.path.join(backup_dir, filename)
            assert os.path.exists(backup_file), \
                f"Резервна копія має існувати перед cleanup: {backup_file}"
        
        # Виконати cleanup
        self.backup_manager.cleanup_backups(backup_dir)
        
        # Перевірити, що backup директорія видалена
        assert not os.path.exists(backup_dir), \
            "Backup директорія має бути видалена після cleanup"
    
    def test_cleanup_nonexistent_backup_directory(self):
        """Тест cleanup неіснуючої backup директорії
        
        **Validates: Requirements 8.5**
        
        Edge case: перевіряє, що cleanup не викликає помилку,
        якщо backup директорія вже не існує.
        """
        nonexistent_dir = os.path.join(self.temp_dir, "nonexistent_backup")
        
        # Cleanup не має викликати помилку
        self.backup_manager.cleanup_backups(nonexistent_dir)
        
        # Директорія все ще не має існувати
        assert not os.path.exists(nonexistent_dir)
    
    def test_safety_commit_without_git_repo(self):
        """Тест створення safety commit без git репозиторію
        
        **Validates: Requirements 8.1**
        
        Edge case: перевіряє, що BackupManager викидає GitError або BackupError,
        якщо поточна директорія не є git репозиторієм або не має commits.
        """
        # Створити нову тимчасову директорію повністю ізольовану від git
        isolated_temp = tempfile.mkdtemp()
        
        try:
            os.chdir(isolated_temp)
            
            # Ініціалізувати git але без commits (щоб не було HEAD)
            subprocess.run(["git", "init"], capture_output=True, check=True)
            subprocess.run(["git", "config", "user.email", "test@example.com"], 
                          capture_output=True, check=True)
            subprocess.run(["git", "config", "user.name", "Test User"], 
                          capture_output=True, check=True)
            
            backup_manager = BackupManager()
            
            # Має викинути BackupError через відсутність HEAD
            with pytest.raises((GitError, BackupError)) as exc_info:
                backup_manager.create_safety_commit()
            
            # Перевірити, що помилка пов'язана з git
            assert "HEAD" in str(exc_info.value) or "git" in str(exc_info.value).lower()
        finally:
            os.chdir(self.original_cwd)
            shutil.rmtree(isolated_temp, ignore_errors=True)
    
    def test_backup_empty_file_list(self):
        """Тест резервного копіювання порожнього списку файлів
        
        **Validates: Requirements 8.3**
        
        Edge case: перевіряє, що BackupManager коректно обробляє
        порожній список файлів для резервування.
        """
        # Створити backup з порожнім списком
        backup_dir = self.backup_manager.create_file_backups([])
        
        # Backup директорія має існувати
        assert os.path.exists(backup_dir), \
            "Backup директорія має існувати навіть для порожнього списку"
        
        # Cleanup має працювати
        self.backup_manager.cleanup_backups(backup_dir)
    
    def test_rollback_restores_multiple_files(self):
        """Тест відкату множини файлів
        
        **Validates: Requirements 8.4**
        
        Перевіряє, що rollback коректно відновлює всі файли
        з резервних копій.
        """
        # Створити множину файлів
        files = ["file1.txt", "file2.txt", "file3.txt"]
        original_contents = {}
        
        for filename in files:
            content = f"Original content of {filename}"
            original_contents[filename] = content
            with open(filename, 'w') as f:
                f.write(content)
        
        subprocess.run(["git", "add", "."], capture_output=True, check=True)
        subprocess.run(["git", "commit", "-m", "Initial commit"], 
                      capture_output=True, check=True)
        
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            check=True
        )
        initial_commit = result.stdout.strip()
        
        # Створити резервні копії
        backup_dir = self.backup_manager.create_file_backups(files)
        
        # Модифікувати всі файли
        for filename in files:
            with open(filename, 'w') as f:
                f.write(f"Modified content of {filename}")
        
        subprocess.run(["git", "add", "."], capture_output=True, check=True)
        subprocess.run(["git", "commit", "-m", "Modified commit"], 
                      capture_output=True, check=True)
        
        # Виконати rollback
        self.backup_manager.rollback_changes(initial_commit, backup_dir)
        
        # Перевірити, що всі файли відновлено
        for filename in files:
            with open(filename, 'r') as f:
                content = f.read()
                assert content == original_contents[filename], \
                    f"Файл {filename} має бути відновлено до оригінального вмісту"
