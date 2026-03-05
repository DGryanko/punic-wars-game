"""Unit тести для ReportGenerator"""

import pytest
import os
import tempfile
from datetime import datetime
from pathlib import Path

import sys
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from report_generator import ReportGenerator


class TestReportGenerator:
    """Тести для класу ReportGenerator"""
    
    def test_create_cleanup_report(self, tmp_path):
        """Тест створення звіту
        
        Validates: Requirements 4.5
        """
        # Arrange
        generator = ReportGenerator()
        output_path = tmp_path / "test_report.md"
        
        # Act
        generator.generate_report(str(output_path))
        
        # Assert
        assert output_path.exists()
        content = output_path.read_text(encoding='utf-8')
        assert "# SDL2 Cleanup Report" in content
        assert "Generated:" in content
    
    def test_report_deleted_files(self, tmp_path):
        """Тест наявності видалених файлів у звіті
        
        Validates: Requirements 7.3
        """
        # Arrange
        generator = ReportGenerator()
        deleted_files = [
            "cpp/SDL2_MIGRATION_TODO.md",
            "cpp/QUICK_START_SDL2.md"
        ]
        output_path = tmp_path / "test_report.md"
        
        # Act
        generator.add_deleted_files(deleted_files)
        generator.generate_report(str(output_path))
        
        # Assert
        content = output_path.read_text(encoding='utf-8')
        assert "## Deleted Files" in content
        for file in deleted_files:
            assert file in content
    
    def test_report_validation_results(self, tmp_path):
        """Тест наявності результатів валідації у звіті
        
        Validates: Requirements 7.4
        """
        # Arrange
        generator = ReportGenerator()
        validation_results = {
            "critical_files": True,
            "no_sdl2_includes": True,
            "compilation": False
        }
        output_path = tmp_path / "test_report.md"
        
        # Act
        generator.add_validation_results(validation_results)
        generator.generate_report(str(output_path))
        
        # Assert
        content = output_path.read_text(encoding='utf-8')
        assert "## Validation Results" in content
        assert "critical_files" in content
        assert "✓ PASSED" in content
        assert "✗ FAILED" in content
    
    def test_report_saved_to_correct_location(self, tmp_path):
        """Тест збереження звіту у правильній директорії
        
        Validates: Requirements 7.5
        """
        # Arrange
        generator = ReportGenerator()
        nested_dir = tmp_path / "nested" / "directory"
        output_path = nested_dir / "report.md"
        
        # Act
        generator.generate_report(str(output_path))
        
        # Assert
        assert output_path.exists()
        assert output_path.parent == nested_dir
    
    def test_add_modified_files(self, tmp_path):
        """Тест додавання модифікованих файлів до звіту
        
        Validates: Requirements 7.2
        """
        # Arrange
        generator = ReportGenerator()
        modifications = {
            "cpp/README.md": "Removed SDL2 references",
            "SESSION_CONTEXT.md": "Updated context information"
        }
        output_path = tmp_path / "test_report.md"
        
        # Act
        generator.add_modified_files(modifications)
        generator.generate_report(str(output_path))
        
        # Assert
        content = output_path.read_text(encoding='utf-8')
        assert "## Modified Files" in content
        for file, description in modifications.items():
            assert file in content
            assert description in content
    
    def test_add_git_operations(self, tmp_path):
        """Тест додавання git операцій до звіту
        
        Validates: Requirements 7.3
        """
        # Arrange
        generator = ReportGenerator()
        git_operations = [
            "Deleted local branch SDL2",
            "Deleted remote branch origin/SDL2",
            "Created safety commit abc123"
        ]
        output_path = tmp_path / "test_report.md"
        
        # Act
        generator.add_git_operations(git_operations)
        generator.generate_report(str(output_path))
        
        # Assert
        content = output_path.read_text(encoding='utf-8')
        assert "## Git Operations" in content
        for operation in git_operations:
            assert operation in content
    
    def test_empty_report(self, tmp_path):
        """Тест генерації порожнього звіту
        
        Edge case: звіт без жодних операцій
        """
        # Arrange
        generator = ReportGenerator()
        output_path = tmp_path / "empty_report.md"
        
        # Act
        generator.generate_report(str(output_path))
        
        # Assert
        content = output_path.read_text(encoding='utf-8')
        assert "# SDL2 Cleanup Report" in content
        assert "*No files were deleted*" in content
        assert "*No files were modified*" in content
        assert "*No git operations were performed*" in content
        assert "*No validation checks were performed*" in content
    
    def test_multiple_additions(self, tmp_path):
        """Тест множинних додавань до звіту
        
        Edge case: додавання даних кількома викликами
        """
        # Arrange
        generator = ReportGenerator()
        output_path = tmp_path / "test_report.md"
        
        # Act
        generator.add_deleted_files(["file1.txt"])
        generator.add_deleted_files(["file2.txt", "file3.txt"])
        generator.add_git_operations(["operation1"])
        generator.add_git_operations(["operation2"])
        generator.generate_report(str(output_path))
        
        # Assert
        content = output_path.read_text(encoding='utf-8')
        assert "file1.txt" in content
        assert "file2.txt" in content
        assert "file3.txt" in content
        assert "operation1" in content
        assert "operation2" in content
    
    def test_report_format_structure(self, tmp_path):
        """Тест структури формату звіту
        
        Перевіряє, що звіт має всі необхідні розділи
        """
        # Arrange
        generator = ReportGenerator()
        output_path = tmp_path / "test_report.md"
        
        # Act
        generator.generate_report(str(output_path))
        
        # Assert
        content = output_path.read_text(encoding='utf-8')
        assert "# SDL2 Cleanup Report" in content
        assert "## Deleted Files" in content
        assert "## Modified Files" in content
        assert "## Git Operations" in content
        assert "## Validation Results" in content
