"""Unit Tests для GitManager

Цей модуль містить unit тести для перевірки конкретних прикладів та edge cases
для GitManager.

Feature: project-cleanup-sdl2-removal
Validates: Requirements 3.1, 3.2, 3.5
"""

import os
import tempfile
import shutil
import subprocess
from pathlib import Path

import pytest

import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from src.git_manager import GitManager
from src.exceptions import GitError


class TestGitManagerUnit:
    """Unit тести для GitManager"""
    
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
        
        # Створити початковий commit (необхідно для створення гілок)
        with open("initial.txt", 'w') as f:
            f.write("Initial content")
        subprocess.run(["git", "add", "initial.txt"], capture_output=True, check=True)
        subprocess.run(["git", "commit", "-m", "Initial commit"], 
                      capture_output=True, check=True)
        
        self.git_manager = GitManager()
    
    def teardown_method(self):
        """Очищення після кожного тесту"""
        os.chdir(self.original_cwd)
        shutil.rmtree(self.temp_dir, ignore_errors=True)
    
    def test_current_branch_not_sdl2(self):
        """Тест перевірки поточної гілки
        
        **Validates: Requirements 3.1**
        
        Перевіряє, що GitManager коректно отримує назву поточної гілки
        та може визначити, чи це гілка SDL2.
        """
        # Отримати поточну гілку
        current_branch = self.git_manager.get_current_branch()
        
        # Перевірити, що отримано назву гілки
        assert current_branch, "Назва гілки не має бути порожньою"
        
        # За замовчуванням git створює гілку master або main
        assert current_branch in ["master", "main"], \
            f"Поточна гілка має бути master або main, отримано: {current_branch}"
        
        # Перевірити, що поточна гілка не є SDL2
        assert current_branch != "SDL2", \
            "Поточна гілка не має бути SDL2"
    
    def test_get_current_branch_on_sdl2_branch(self):
        """Тест отримання назви гілки SDL2
        
        **Validates: Requirements 3.1**
        
        Перевіряє, що GitManager коректно визначає, коли поточна гілка є SDL2.
        """
        # Створити та перейти на гілку SDL2
        subprocess.run(["git", "checkout", "-b", "SDL2"], 
                      capture_output=True, check=True)
        
        # Отримати поточну гілку
        current_branch = self.git_manager.get_current_branch()
        
        # Перевірити, що це гілка SDL2
        assert current_branch == "SDL2", \
            f"Поточна гілка має бути SDL2, отримано: {current_branch}"
    
    def test_delete_local_sdl2_branch(self):
        """Тест видалення локальної SDL2 гілки
        
        **Validates: Requirements 3.2**
        
        Перевіряє, що GitManager коректно видаляє локальну гілку SDL2.
        """
        # Створити гілку SDL2
        subprocess.run(["git", "branch", "SDL2"], 
                      capture_output=True, check=True)
        
        # Перевірити, що гілка існує
        assert self.git_manager.branch_exists("SDL2", remote=False), \
            "Гілка SDL2 має існувати перед видаленням"
        
        # Переконатися, що ми не на гілці SDL2
        current_branch = self.git_manager.get_current_branch()
        assert current_branch != "SDL2", \
            "Не можна видалити поточну активну гілку"
        
        # Видалити гілку SDL2
        result = self.git_manager.delete_local_branch("SDL2")
        
        # Перевірити успішність операції
        assert result is True, "Видалення гілки має бути успішним"
        
        # Перевірити, що гілка більше не існує
        assert not self.git_manager.branch_exists("SDL2", remote=False), \
            "Гілка SDL2 не має існувати після видалення"
    
    def test_delete_local_branch_cannot_delete_current(self):
        """Тест неможливості видалення поточної активної гілки
        
        **Validates: Requirements 3.1, 3.2**
        
        Edge case: перевіряє, що GitManager не дозволяє видалити
        поточну активну гілку.
        """
        # Створити та перейти на гілку SDL2
        subprocess.run(["git", "checkout", "-b", "SDL2"], 
                      capture_output=True, check=True)
        
        # Спроба видалити поточну гілку має викликати помилку
        with pytest.raises(GitError) as exc_info:
            self.git_manager.delete_local_branch("SDL2")
        
        # Перевірити повідомлення про помилку
        assert "поточну активну гілку" in str(exc_info.value).lower() or \
               "current" in str(exc_info.value).lower(), \
            "Помилка має вказувати на спробу видалення поточної гілки"
    
    def test_branch_not_exists_no_error(self):
        """Тест видалення неіснуючої гілки без помилки
        
        **Validates: Requirements 3.5**
        
        Edge case: перевіряє, що GitManager не викликає помилку,
        якщо гілка SDL2 не існує (згідно з Requirements 3.5).
        """
        # Перевірити, що гілка SDL2 не існує
        assert not self.git_manager.branch_exists("SDL2", remote=False), \
            "Гілка SDL2 не має існувати на початку тесту"
        
        # Спроба видалити неіснуючу гілку не має викликати помилку
        result = self.git_manager.delete_local_branch("SDL2")
        
        # Операція має бути успішною (повернути True)
        assert result is True, \
            "Видалення неіснуючої гілки має повертати True (не помилку)"
    
    def test_branch_exists_local(self):
        """Тест перевірки існування локальної гілки
        
        **Validates: Requirements 3.1**
        
        Перевіряє, що GitManager коректно визначає існування локальної гілки.
        """
        # Створити тестову гілку
        subprocess.run(["git", "branch", "test-branch"], 
                      capture_output=True, check=True)
        
        # Перевірити, що гілка існує
        assert self.git_manager.branch_exists("test-branch", remote=False), \
            "Створена гілка має існувати"
        
        # Перевірити, що неіснуюча гілка не знайдена
        assert not self.git_manager.branch_exists("nonexistent-branch", remote=False), \
            "Неіснуюча гілка не має бути знайдена"
    
    def test_delete_multiple_branches(self):
        """Тест видалення множини гілок
        
        **Validates: Requirements 3.2**
        
        Перевіряє, що GitManager може видалити кілька гілок послідовно.
        """
        # Створити кілька гілок
        branches = ["branch1", "branch2", "branch3"]
        for branch in branches:
            subprocess.run(["git", "branch", branch], 
                          capture_output=True, check=True)
        
        # Перевірити, що всі гілки існують
        for branch in branches:
            assert self.git_manager.branch_exists(branch, remote=False), \
                f"Гілка {branch} має існувати"
        
        # Видалити всі гілки
        for branch in branches:
            result = self.git_manager.delete_local_branch(branch)
            assert result is True, f"Видалення гілки {branch} має бути успішним"
        
        # Перевірити, що всі гілки видалено
        for branch in branches:
            assert not self.git_manager.branch_exists(branch, remote=False), \
                f"Гілка {branch} не має існувати після видалення"
    
    def test_verify_branch_integrity(self):
        """Тест перевірки цілісності гілки
        
        **Validates: Requirements 3.4**
        
        Перевіряє, що GitManager може отримати commit hash для перевірки
        цілісності гілки.
        """
        # Отримати commit hash поточної гілки
        current_branch = self.git_manager.get_current_branch()
        commit_hash = self.git_manager.verify_branch_integrity(current_branch)
        
        # Перевірити, що це валідний SHA-1 hash
        assert commit_hash, "Commit hash не має бути порожнім"
        assert len(commit_hash) == 40, "Commit hash має бути 40 символів"
        assert all(c in '0123456789abcdef' for c in commit_hash), \
            "Commit hash має містити тільки hex символи"
        
        # Перевірити, що hash відповідає реальному commit
        result = subprocess.run(
            ["git", "rev-parse", current_branch],
            capture_output=True,
            text=True,
            check=True
        )
        expected_hash = result.stdout.strip()
        
        assert commit_hash == expected_hash, \
            f"Commit hash має співпадати: очікувалось {expected_hash}, отримано {commit_hash}"
    
    def test_verify_branch_integrity_nonexistent_branch(self):
        """Тест перевірки цілісності неіснуючої гілки
        
        **Validates: Requirements 3.4**
        
        Edge case: перевіряє, що GitManager викидає помилку при спробі
        перевірити цілісність неіснуючої гілки.
        """
        # Спроба перевірити неіснуючу гілку має викликати помилку
        with pytest.raises(GitError) as exc_info:
            self.git_manager.verify_branch_integrity("nonexistent-branch")
        
        # Перевірити повідомлення про помилку
        assert "не існує" in str(exc_info.value).lower() or \
               "not exist" in str(exc_info.value).lower(), \
            "Помилка має вказувати на відсутність гілки"
    
    def test_get_current_branch_detached_head(self):
        """Тест отримання гілки у стані detached HEAD
        
        **Validates: Requirements 3.1**
        
        Edge case: перевіряє поведінку GitManager у стані detached HEAD.
        """
        # Отримати commit hash
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            check=True
        )
        commit_hash = result.stdout.strip()
        
        # Перейти у стан detached HEAD
        subprocess.run(["git", "checkout", commit_hash], 
                      capture_output=True, check=True)
        
        # Спроба отримати поточну гілку має викликати помилку або повернути порожній рядок
        # (залежить від імплементації)
        try:
            current_branch = self.git_manager.get_current_branch()
            # Якщо не викинуто помилку, перевірити, що повернуто порожній рядок або помилку
            assert not current_branch or current_branch == "", \
                "У стані detached HEAD не має бути активної гілки"
        except GitError:
            # Це також прийнятна поведінка
            pass
    
    def test_delete_branch_with_unmerged_changes(self):
        """Тест видалення гілки з незлитими змінами
        
        **Validates: Requirements 3.2**
        
        Перевіряє, що GitManager може видалити гілку навіть якщо вона
        містить незлиті зміни (використовуючи примусове видалення).
        """
        # Створити нову гілку з змінами
        subprocess.run(["git", "checkout", "-b", "feature-branch"], 
                      capture_output=True, check=True)
        
        # Додати зміни у нову гілку
        with open("feature.txt", 'w') as f:
            f.write("Feature content")
        subprocess.run(["git", "add", "feature.txt"], capture_output=True, check=True)
        subprocess.run(["git", "commit", "-m", "Add feature"], 
                      capture_output=True, check=True)
        
        # Повернутися на master/main
        current_branch = self.git_manager.get_current_branch()
        if current_branch == "feature-branch":
            # Отримати назву основної гілки
            result = subprocess.run(
                ["git", "branch", "--list"],
                capture_output=True,
                text=True,
                check=True
            )
            branches = result.stdout.strip().split('\n')
            main_branch = None
            for branch in branches:
                branch = branch.strip().lstrip('* ')
                if branch in ["master", "main"]:
                    main_branch = branch
                    break
            
            if main_branch:
                subprocess.run(["git", "checkout", main_branch], 
                              capture_output=True, check=True)
        
        # Видалити гілку з незлитими змінами
        result = self.git_manager.delete_local_branch("feature-branch")
        
        # Операція має бути успішною (використовується -D для примусового видалення)
        assert result is True, \
            "Видалення гілки з незлитими змінами має бути успішним"
        
        # Перевірити, що гілка видалена
        assert not self.git_manager.branch_exists("feature-branch", remote=False), \
            "Гілка має бути видалена"
