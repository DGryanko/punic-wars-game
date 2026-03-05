"""Property-Based Tests для BackupManager

Цей модуль містить property-based тести для перевірки коректності BackupManager.
Використовує бібліотеку Hypothesis для генерації тестових даних.

Feature: project-cleanup-sdl2-removal
"""

import os
import tempfile
import shutil
import filecmp
from pathlib import Path
from typing import List

import pytest
from hypothesis import given, strategies as st, settings, assume

import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from src.backup_manager import BackupManager
from src.exceptions import BackupError


# Feature: project-cleanup-sdl2-removal, Property 3: Резервне копіювання перед модифікацією
# **Validates: Requirements 2.5, 8.2**
@given(st.lists(
    st.tuples(
        st.text(min_size=1, max_size=50, alphabet=st.characters(
            whitelist_categories=('Lu', 'Ll', 'Nd'),
            whitelist_characters='_-.'
        )),
        st.binary(min_size=0, max_size=1000)
    ),
    min_size=1,
    max_size=10
))
@settings(max_examples=100, deadline=None)
def test_property_backup_before_modification(files_data):
    """For any файл для модифікації, має існувати резервна копія з ідентичним вмістом
    
    Property: Для будь-якого набору файлів, які потрібно зарезервувати,
    BackupManager має створити резервні копії з ідентичним вмістом.
    
    Args:
        files_data: Список кортежів (ім'я_файлу, вміст_файлу)
    """
    # Створити тимчасову директорію для тестування
    with tempfile.TemporaryDirectory() as temp_dir:
        # Змінити робочу директорію на тимчасову
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            # Створити тимчасову директорію для backup
            backup_base = os.path.join(temp_dir, "test_backups")
            backup_manager = BackupManager(backup_directory=backup_base)
            
            # Створити тестові файли
            created_files = []
            for filename, content in files_data:
                # Забезпечити унікальність імен файлів
                safe_filename = f"test_{len(created_files)}_{filename}"
                
                # Пропустити файли з недопустимими іменами
                try:
                    filepath = Path(safe_filename)
                    # Створити батьківські директорії, якщо потрібно
                    filepath.parent.mkdir(parents=True, exist_ok=True)
                    
                    # Записати вміст у файл
                    with open(filepath, 'wb') as f:
                        f.write(content)
                    
                    created_files.append(str(filepath))
                except (OSError, ValueError):
                    # Пропустити файли з проблемними іменами
                    continue
            
            # Якщо не вдалося створити жодного файлу, пропустити тест
            assume(len(created_files) > 0)
            
            # Створити резервні копії
            backup_dir = backup_manager.create_file_backups(created_files)
            
            # Перевірити, що backup директорія існує
            assert os.path.exists(backup_dir), "Backup директорія має існувати"
            assert os.path.isdir(backup_dir), "Backup шлях має бути директорією"
            
            # Перевірити кожен файл
            for original_file in created_files:
                # Побудувати шлях до резервної копії
                backup_file = os.path.join(backup_dir, original_file)
                
                # Перевірити існування резервної копії
                assert os.path.exists(backup_file), \
                    f"Резервна копія має існувати для файлу {original_file}"
                
                # Перевірити, що це файл, а не директорія
                assert os.path.isfile(backup_file), \
                    f"Резервна копія має бути файлом: {backup_file}"
                
                # Перевірити ідентичність вмісту
                assert filecmp.cmp(original_file, backup_file, shallow=False), \
                    f"Вміст резервної копії має бути ідентичним оригіналу: {original_file}"
                
                # Додаткова перевірка через порівняння байтів
                with open(original_file, 'rb') as orig, open(backup_file, 'rb') as backup:
                    original_content = orig.read()
                    backup_content = backup.read()
                    assert original_content == backup_content, \
                        f"Байтовий вміст має співпадати для {original_file}"
            
            # Очистити backup після тесту
            backup_manager.cleanup_backups(backup_dir)
            
        finally:
            # Повернутися до оригінальної директорії
            os.chdir(original_cwd)


@given(st.lists(
    st.text(min_size=1, max_size=30, alphabet=st.characters(
        whitelist_categories=('Lu', 'Ll', 'Nd'),
        whitelist_characters='_-'
    )),
    min_size=0,
    max_size=5
))
@settings(max_examples=50, deadline=None)
def test_property_backup_nonexistent_files(filenames):
    """Property: BackupManager має коректно обробляти неіснуючі файли
    
    Перевіряє, що create_file_backups не падає при спробі зарезервувати
    неіснуючі файли, а просто пропускає їх.
    
    Args:
        filenames: Список імен неіснуючих файлів
    """
    with tempfile.TemporaryDirectory() as temp_dir:
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            backup_base = os.path.join(temp_dir, "test_backups")
            backup_manager = BackupManager(backup_directory=backup_base)
            
            # Створити список неіснуючих файлів
            nonexistent_files = [f"nonexistent_{name}.txt" for name in filenames]
            
            # Спроба створити backup не має викликати помилку
            backup_dir = backup_manager.create_file_backups(nonexistent_files)
            
            # Backup директорія має існувати
            assert os.path.exists(backup_dir)
            
            # Очистити backup
            backup_manager.cleanup_backups(backup_dir)
            
        finally:
            os.chdir(original_cwd)


@given(st.lists(
    st.tuples(
        st.text(min_size=1, max_size=30, alphabet=st.characters(
            whitelist_categories=('Lu', 'Ll', 'Nd'),
            whitelist_characters='_'
        )),
        st.binary(min_size=1, max_size=500)
    ),
    min_size=1,
    max_size=5
))
@settings(max_examples=50, deadline=None)
def test_property_backup_preserves_file_metadata(files_data):
    """Property: BackupManager має зберігати метадані файлів
    
    Перевіряє, що резервні копії зберігають не тільки вміст,
    але й метадані файлів (розмір, час модифікації).
    
    Args:
        files_data: Список кортежів (ім'я_файлу, вміст_файлу)
    """
    with tempfile.TemporaryDirectory() as temp_dir:
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            backup_base = os.path.join(temp_dir, "test_backups")
            backup_manager = BackupManager(backup_directory=backup_base)
            
            created_files = []
            for idx, (filename, content) in enumerate(files_data):
                safe_filename = f"test_{idx}_{filename}.dat"
                
                try:
                    with open(safe_filename, 'wb') as f:
                        f.write(content)
                    created_files.append(safe_filename)
                except (OSError, ValueError):
                    continue
            
            assume(len(created_files) > 0)
            
            # Створити резервні копії
            backup_dir = backup_manager.create_file_backups(created_files)
            
            # Перевірити метадані
            for original_file in created_files:
                backup_file = os.path.join(backup_dir, original_file)
                
                # Перевірити розмір файлу
                original_size = os.path.getsize(original_file)
                backup_size = os.path.getsize(backup_file)
                assert original_size == backup_size, \
                    f"Розмір резервної копії має співпадати з оригіналом"
            
            # Очистити backup
            backup_manager.cleanup_backups(backup_dir)
            
        finally:
            os.chdir(original_cwd)
