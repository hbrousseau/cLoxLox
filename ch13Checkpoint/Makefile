CXX      := g++
CXXFLAGS := -ggdb -std=c++17
CPPFLAGS := -MMD

COMPILE  := $(CXX) $(CXXFLAGS) $(CPPFLAGS)

SRCS     := ASTPrinter.cpp GenerateAST.cpp cLL.cpp LoxFunction.cpp
DEPS     := $(SRCS:.cpp=.d)

cll: Expr.h Stmt.h cLL.o
	@$(COMPILE) cLL.o -o $@

LoxFunction.o: LoxFunction.cpp
	@$(COMPILE) -c $^ -o $@

ast_printer: Expr.h Stmt.h ASTPrinter.cpp
	@$(COMPILE) ASTPrinter.cpp -o $@

generate_ast: GenerateAST.o
	@$(COMPILE) $< -o $@

Expr.h Stmt.h: generate_ast
	@./generate_ast .

.PHONY: clean
clean:
	rm -f *.d *.o ast_printer generate_ast cll

# Include dependencies
-include $(DEPS)

OUTPUT_FILE := test_output.txt
TEST_DIR := tests

define make_test
.PHONY: $(1)

$(1):
	@echo "Testing cLoxLox with $(1)..."
	@echo "========================================" >> $(OUTPUT_FILE)
	@echo "Test: $(1)" >> $(OUTPUT_FILE)
	@./cll $(TEST_DIR)/$(1) >> $(OUTPUT_FILE);
	@echo "========================================" >> $(OUTPUT_FILE)
	@echo >> $(OUTPUT_FILE)  # Add a newline for better readability
endef

TESTS := $(patsubst $(TEST_DIR)/%,%,$(wildcard $(TEST_DIR)/**/*.lox))

$(foreach test, $(TESTS), $(eval $(call make_test,$(test))))

.PHONY: test-all
test-all:
	@for test in $(TESTS); do make $$test; done
