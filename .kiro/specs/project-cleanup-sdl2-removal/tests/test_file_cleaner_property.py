"""Property-Based Tests для FileCleaner

Цей модуль містить property-based тести для перевірки коректності FileCleaner.
Використовує бібліотеку Hypothesis для генерації тестових даних.

Feature: project-cleanup-sdl2-removal
"""

import os
import tempfile
from pathlib import Path
from typing import List

import pytest
from hypothesis import given, strategies as st, settings, assume

import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from src.file_cleaner import FileCleaner


# Feature: project-cleanup-sdl2-removal, Property 2: Повнота пошуку SDL2 згадок
# **Validates: Requirements 2.1, 5.2, 5.4**
@given(st.text(min_size=1, max_size=1000))
@settings(max_examples=100, deadline=None)
def test_property_sdl2_search_completeness(file_content):
    """For any файл, що містить SDL2 згадки, пошук має їх знайти
    
    Property: Для будь-якого файлу, який містить рядки "SDL2", "sdl2", "SDL", або "sdl",
    метод find_sdl2_references має включити цей файл у результати пошуку.
    
    Args:
        file_content: Випадковий вміст файлу
    """
    # Створити тимчасову директорію для тестування
    with tempfile.TemporaryDirectory() as temp_dir:
        # Змінити робочу директорію на тимчасову
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            # Створити FileCleaner з тимчасовою директорією
            file_cleaner = FileCleaner(project_root=temp_dir)
            
            # Створити тестовий файл
            test_file = "test_file.txt"
            with open(test_file, 'w', encoding='utf-8') as f:
                f.write(file_content)
            
            # Виконати пошук SDL2 згадок
            results = file_cleaner.find_sdl2_references([test_file])
            
            # Перевірити, чи містить файл SDL2 згадки (case-insensitive)
            content_lower = file_content.lower()
            has_sdl2 = any(term in content_lower for term in ["sdl2", "sdl"])
            
            if has_sdl2:
                # Якщо файл містить SDL2 згадки, він має бути у результатах
                assert test_file in results, \
                    f"Файл з SDL2 згадками має бути знайдено. Вміст: {file_content[:100]}"
                
                # Перевірити, що результати не порожні
                assert len(results[test_file]) > 0, \
                    f"Результати пошуку не мають бути порожніми для файлу з SDL2 згадками"
            else:
                # Якщо файл не містить SDL2 згадок, він не має бути у результатах
                assert test_file not in results, \
                    f"Файл без SDL2 згадок не має бути у результатах. Вміст: {file_content[:100]}"
        
        finally:
            # Повернутися до оригінальної директорії
            os.chdir(original_cwd)


@given(st.lists(
    st.tuples(
        st.text(min_size=1, max_size=30, alphabet=st.characters(
            whitelist_categories=('Lu', 'Ll', 'Nd'),
            whitelist_characters='_-'
        )),
        st.text(min_size=0, max_size=500)
    ),
    min_size=1,
    max_size=10
))
@settings(max_examples=50, deadline=None)
def test_property_sdl2_search_multiple_files(files_data):
    """Property: FileCleaner має знаходити SDL2 згадки у всіх файлах
    
    Перевіряє, що find_sdl2_references коректно обробляє множину файлів
    і знаходить SDL2 згадки у кожному файлі, який їх містить.
    
    Args:
        files_data: Список кортежів (ім'я_файлу, вміст_файлу)
    """
    with tempfile.TemporaryDirectory() as temp_dir:
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            file_cleaner = FileCleaner(project_root=temp_dir)
            
            # Створити тестові файли
            created_files = []
            files_with_sdl2 = []
            
            for idx, (filename, content) in enumerate(files_data):
                safe_filename = f"test_{idx}_{filename}.txt"
                
                try:
                    with open(safe_filename, 'w', encoding='utf-8') as f:
                        f.write(content)
                    
                    created_files.append(safe_filename)
                    
                    # Перевірити чи містить файл SDL2 згадки
                    content_lower = content.lower()
                    if any(term in content_lower for term in ["sdl2", "sdl"]):
                        files_with_sdl2.append(safe_filename)
                
                except (OSError, ValueError):
                    continue
            
            assume(len(created_files) > 0)
            
            # Виконати пошук
            results = file_cleaner.find_sdl2_references(created_files)
            
            # Перевірити, що всі файли з SDL2 згадками знайдено
            for file_with_sdl2 in files_with_sdl2:
                assert file_with_sdl2 in results, \
                    f"Файл {file_with_sdl2} з SDL2 згадками має бути у результатах"
            
            # Перевірити, що файли без SDL2 згадок не у результатах
            for created_file in created_files:
                if created_file not in files_with_sdl2:
                    assert created_file not in results, \
                        f"Файл {created_file} без SDL2 згадок не має бути у результатах"
        
        finally:
            os.chdir(original_cwd)


@given(st.sampled_from(["SDL2", "sdl2", "SDL", "sdl", "Sdl2", "SdL", "sDl2"]))
@settings(max_examples=50, deadline=None)
def test_property_sdl2_search_case_insensitive(sdl_variant):
    """Property: FileCleaner має знаходити SDL2 згадки незалежно від регістру
    
    Перевіряє, що пошук працює case-insensitive для всіх варіантів написання SDL/SDL2.
    
    Args:
        sdl_variant: Варіант написання SDL/SDL2
    """
    with tempfile.TemporaryDirectory() as temp_dir:
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            file_cleaner = FileCleaner(project_root=temp_dir)
            
            # Створити файл з варіантом SDL
            test_file = "test_case.txt"
            content = f"This file mentions {sdl_variant} library"
            
            with open(test_file, 'w', encoding='utf-8') as f:
                f.write(content)
            
            # Виконати пошук
            results = file_cleaner.find_sdl2_references([test_file])
            
            # Файл має бути знайдено незалежно від регістру
            assert test_file in results, \
                f"Файл з '{sdl_variant}' має бути знайдено (case-insensitive)"
            
            assert len(results[test_file]) > 0, \
                f"Результати не мають бути порожніми для '{sdl_variant}'"
        
        finally:
            os.chdir(original_cwd)


@given(st.text(min_size=1, max_size=50, alphabet=st.characters(
    whitelist_categories=('Lu', 'Ll', 'Nd'),
    whitelist_characters='_-'
)))
@settings(max_examples=50, deadline=None)
def test_property_sdl2_search_no_false_positives(random_text):
    """Property: FileCleaner не має давати false positives
    
    Перевіряє, що файли без SDL/SDL2 згадок не потрапляють у результати пошуку.
    
    Args:
        random_text: Випадковий текст без SDL згадок
    """
    # Переконатися, що текст не містить SDL згадок
    assume("sdl" not in random_text.lower())
    
    with tempfile.TemporaryDirectory() as temp_dir:
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            file_cleaner = FileCleaner(project_root=temp_dir)
            
            # Створити файл без SDL згадок
            test_file = "test_no_sdl.txt"
            with open(test_file, 'w', encoding='utf-8') as f:
                f.write(random_text)
            
            # Виконати пошук
            results = file_cleaner.find_sdl2_references([test_file])
            
            # Файл не має бути у результатах
            assert test_file not in results, \
                f"Файл без SDL згадок не має бути у результатах. Текст: {random_text}"
        
        finally:
            os.chdir(original_cwd)


@given(st.lists(st.text(min_size=1, max_size=100), min_size=1, max_size=20))
@settings(max_examples=50, deadline=None)
def test_property_sdl2_search_multiline_files(lines):
    """Property: FileCleaner має знаходити SDL2 згадки у багаторядкових файлах
    
    Перевіряє, що пошук коректно працює з файлами, що містять багато рядків.
    
    Args:
        lines: Список рядків для створення файлу
    """
    with tempfile.TemporaryDirectory() as temp_dir:
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            file_cleaner = FileCleaner(project_root=temp_dir)
            
            # Створити багаторядковий файл
            test_file = "test_multiline.txt"
            content = "\n".join(lines)
            
            with open(test_file, 'w', encoding='utf-8') as f:
                f.write(content)
            
            # Перевірити чи містить файл SDL згадки
            content_lower = content.lower()
            has_sdl = any(term in content_lower for term in ["sdl2", "sdl"])
            
            # Виконати пошук
            results = file_cleaner.find_sdl2_references([test_file])
            
            if has_sdl:
                assert test_file in results, \
                    "Багаторядковий файл з SDL згадками має бути знайдено"
                
                # Перевірити, що знайдено правильну кількість рядків
                sdl_lines_count = sum(1 for line in lines if any(term in line.lower() for term in ["sdl2", "sdl"]))
                assert len(results[test_file]) >= sdl_lines_count, \
                    f"Має бути знайдено щонайменше {sdl_lines_count} рядків з SDL згадками"
            else:
                assert test_file not in results, \
                    "Багаторядковий файл без SDL згадок не має бути у результатах"
        
        finally:
            os.chdir(original_cwd)


@given(st.text(min_size=1, max_size=30, alphabet=st.characters(
    whitelist_categories=('Lu', 'Ll', 'Nd'),
    whitelist_characters='_-'
)))
@settings(max_examples=30, deadline=None)
def test_property_sdl2_search_nonexistent_files(filename):
    """Property: FileCleaner має коректно обробляти неіснуючі файли
    
    Перевіряє, що find_sdl2_references не падає при спробі пошуку у неіснуючих файлах.
    
    Args:
        filename: Ім'я неіснуючого файлу
    """
    with tempfile.TemporaryDirectory() as temp_dir:
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            file_cleaner = FileCleaner(project_root=temp_dir)
            
            # Спроба пошуку у неіснуючому файлі не має викликати помилку
            nonexistent_file = f"nonexistent_{filename}.txt"
            results = file_cleaner.find_sdl2_references([nonexistent_file])
            
            # Результати мають бути порожніми або не містити неіснуючий файл
            assert nonexistent_file not in results, \
                "Неіснуючий файл не має бути у результатах"
        
        finally:
            os.chdir(original_cwd)


# Feature: project-cleanup-sdl2-removal, Property 1: Збереження незмінних файлів
# **Validates: Requirements 1.3, 1.4, 3.4**
@given(st.lists(
    st.tuples(
        st.text(min_size=1, max_size=30, alphabet=st.characters(
            whitelist_categories=('Lu', 'Ll', 'Nd'),
            whitelist_characters='_-.'
        )),
        st.text(min_size=1, max_size=500),
        st.booleans()
    ),
    min_size=1,
    max_size=10
))
@settings(max_examples=100, deadline=None)
def test_property_unchanged_files_preserved(files_data):
    """Property: Файли, які не є SDL2-специфічними, залишаються незмінними
    
    For any файл, який не є SDL2-специфічним та не міститься в списку files_to_clean,
    після виконання операції очищення вміст файлу має залишитися ідентичним вмісту
    до операції (перевірка через хеш-суму).
    
    Args:
        files_data: Список кортежів (ім'я_файлу, вміст_файлу, чи_містить_SDL2)
    """
    import hashlib
    
    with tempfile.TemporaryDirectory() as temp_dir:
        original_cwd = os.getcwd()
        os.chdir(temp_dir)
        
        try:
            file_cleaner = FileCleaner(project_root=temp_dir)
            
            # Створити тестові файли та зберегти їх хеші
            unchanged_files = {}  # {filename: hash}
            files_to_clean = []
            
            for idx, (filename, content, has_sdl2) in enumerate(files_data):
                safe_filename = f"test_{idx}_{filename}.txt"
                
                # Якщо файл має містити SDL2, додаємо згадку
                if has_sdl2:
                    content = f"This file uses SDL2 library\n{content}"
                    files_to_clean.append(safe_filename)
                
                try:
                    with open(safe_filename, 'w', encoding='utf-8', newline='') as f:
                        f.write(content)
                    
                    # Обчислити хеш для файлів без SDL2 (використовуємо binary mode)
                    if not has_sdl2:
                        with open(safe_filename, 'rb') as f:
                            content_hash = hashlib.sha256(f.read()).hexdigest()
                        unchanged_files[safe_filename] = content_hash
                
                except (OSError, ValueError):
                    continue
            
            assume(len(unchanged_files) > 0)
            
            # Виконати очищення документації для файлів з SDL2
            if files_to_clean:
                file_cleaner.clean_documentation(files_to_clean)
            
            # Перевірити, що файли без SDL2 залишилися незмінними
            for filename, original_hash in unchanged_files.items():
                # Перевірити, що файл існує
                assert os.path.exists(filename), \
                    f"Файл {filename} має існувати після очищення"
                
                # Обчислити поточний хеш (використовуємо binary mode)
                with open(filename, 'rb') as f:
                    current_hash = hashlib.sha256(f.read()).hexdigest()
                
                # Перевірити, що хеш не змінився
                assert current_hash == original_hash, \
                    f"Файл {filename} без SDL2 згадок має залишитися незмінним. " \
                    f"Оригінальний хеш: {original_hash}, поточний хеш: {current_hash}"
        
        finally:
            os.chdir(original_cwd)
