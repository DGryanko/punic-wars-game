"""Git Manager для SDL2 Cleanup System

Цей модуль надає функціональність для роботи з git репозиторієм:
- Отримання інформації про поточну гілку
- Перевірка існування гілок (локальних та віддалених)
- Видалення локальних та віддалених гілок
- Перевірка цілісності гілок
"""

import subprocess
from typing import Optional
from .exceptions import GitError


class GitManager:
    """Менеджер для git операцій
    
    Забезпечує безпечне виконання git команд з обробкою помилок.
    Всі методи викидають GitError у разі проблем з git операціями.
    """
    
    def __init__(self):
        """Ініціалізує GitManager"""
        pass
    
    def _run_git_command(self, args: list[str]) -> tuple[bool, str]:
        """Виконує git команду та повертає результат
        
        Args:
            args: Список аргументів для git команди
            
        Returns:
            (success, output): Кортеж з результатом виконання та виводом
        """
        try:
            result = subprocess.run(
                ['git'] + args,
                capture_output=True,
                text=True,
                check=False
            )
            return result.returncode == 0, result.stdout.strip()
        except FileNotFoundError:
            raise GitError("Git не знайдено в системі")
        except Exception as e:
            raise GitError(f"Помилка виконання git команди: {e}")
    
    def get_current_branch(self) -> str:
        """Отримує назву поточної активної гілки
        
        Returns:
            branch_name: Назва поточної гілки
            
        Raises:
            GitError: Якщо не вдалося отримати назву гілки
        """
        success, output = self._run_git_command(['branch', '--show-current'])
        
        if not success:
            raise GitError("Не вдалося отримати поточну гілку")
        
        if not output:
            raise GitError("Репозиторій не має активної гілки")
        
        return output
    
    def branch_exists(self, branch_name: str, remote: bool = False) -> bool:
        """Перевіряє існування гілки
        
        Args:
            branch_name: Назва гілки
            remote: Перевіряти віддалену гілку (за замовчуванням False)
            
        Returns:
            exists: True якщо гілка існує, False інакше
            
        Raises:
            GitError: Якщо виникла помилка при перевірці
        """
        if remote:
            # Перевірка віддаленої гілки
            # Спочатку оновлюємо інформацію про віддалені гілки
            self._run_git_command(['fetch', '--prune'])
            
            # Перевіряємо існування віддаленої гілки
            success, output = self._run_git_command(['ls-remote', '--heads', 'origin', branch_name])
            
            # Якщо вивід не порожній, гілка існує
            return bool(output)
        else:
            # Перевірка локальної гілки
            success, output = self._run_git_command(['branch', '--list', branch_name])
            
            # Якщо вивід не порожній, гілка існує
            return bool(output.strip())
    
    def delete_local_branch(self, branch_name: str) -> bool:
        """Видаляє локальну гілку
        
        Args:
            branch_name: Назва гілки для видалення
            
        Returns:
            success: True якщо операція успішна
            
        Raises:
            GitError: Якщо поточна гілка є гілкою для видалення
            GitError: Якщо виникла помилка при видаленні
        """
        # Перевірка, що поточна гілка не є гілкою для видалення
        current_branch = self.get_current_branch()
        if current_branch == branch_name:
            raise GitError(f"Неможливо видалити поточну активну гілку '{branch_name}'")
        
        # Перевірка існування гілки
        if not self.branch_exists(branch_name, remote=False):
            # Гілка не існує - це не помилка згідно з Requirements 3.5
            return True
        
        # Видалення гілки (використовуємо -D для примусового видалення)
        success, output = self._run_git_command(['branch', '-D', branch_name])
        
        if not success:
            raise GitError(f"Не вдалося видалити локальну гілку '{branch_name}': {output}")
        
        return True
    
    def delete_remote_branch(self, branch_name: str) -> bool:
        """Видаляє віддалену гілку
        
        Args:
            branch_name: Назва гілки для видалення
            
        Returns:
            success: True якщо операція успішна
            
        Raises:
            GitError: Якщо виникла помилка при видаленні
        """
        # Перевірка існування віддаленої гілки
        if not self.branch_exists(branch_name, remote=True):
            # Гілка не існує - це не помилка згідно з Requirements 3.5
            return True
        
        # Видалення віддаленої гілки
        success, output = self._run_git_command(['push', 'origin', '--delete', branch_name])
        
        if not success:
            # Перевірка, чи гілка вже видалена
            if "remote ref does not exist" in output.lower() or "unable to delete" in output.lower():
                return True
            raise GitError(f"Не вдалося видалити віддалену гілку '{branch_name}': {output}")
        
        return True
    
    def verify_branch_integrity(self, branch_name: str) -> str:
        """Перевіряє цілісність гілки
        
        Args:
            branch_name: Назва гілки для перевірки
            
        Returns:
            commit_hash: SHA хеш останнього commit
            
        Raises:
            GitError: Якщо гілка не існує або виникла помилка
        """
        # Перевірка існування гілки
        if not self.branch_exists(branch_name, remote=False):
            raise GitError(f"Гілка '{branch_name}' не існує")
        
        # Отримання SHA хешу останнього commit
        success, output = self._run_git_command(['rev-parse', branch_name])
        
        if not success:
            raise GitError(f"Не вдалося отримати commit hash для гілки '{branch_name}': {output}")
        
        if not output:
            raise GitError(f"Гілка '{branch_name}' не має commits")
        
        return output
