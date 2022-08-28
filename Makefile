.PHONY: help format lint lint_copyright lint_line_length fmt

help:
	@echo "Targets:"
	@echo "make fmt          - reformat everything using clang-format"
	@echo "make lint         - run code checks"

format:
	@cmake -P cmake/format.cmake

lint_copyright:
	@cmake -P cmake/copyright.cmake

lint_line_length:
	@cmake -P cmake/lint_line_length.cmake

fmt: format lint_line_length

lint: lint_copyright lint_line_length
