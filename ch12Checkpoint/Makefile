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


.PHONY: clean
clean:
	rm -f *.d *.o ast_printer generate_ast cll


-include $(DEPS)


define make_test
.PHONY: $(1)
$(1):
	@make cll >/dev/null
	@echo "testing cll with $(1).lox ..."
	@./cll tests/$(1).lox | diff -u --color tests/$(1).lox.expected -;
endef


define make_test_error
.PHONY: $(1)
$(1):
	@make cll >/dev/null
	@echo "testing cll with $(1).lox ..."
	@./cll tests/$(1).lox 2>&1 | diff -u --color tests/$(1).lox.expected -;
endef


TESTS = \
test-classes \
test-classes2 \
test-classes3 \
test-classes4 \
test-classes5 \
test-classes6 \
test-classes7 \
test-classes8 \
test-classes9 \
test-classes12 \


TEST_ERRORS = \
test-classes10 \
test-classes11 \
test-classes13 \


$(foreach test, $(TESTS), $(eval $(call make_test,$(test))))
$(foreach test, $(TEST_ERRORS), $(eval $(call make_test_error,$(test))))

.PHONY: test-all
test-all:
	@for test in $(TESTS) $(TEST_ERRORS); do \
		make -s $$test; \
	done
	