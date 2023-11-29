CXX      := g++
CXXFLAGS := -ggdb -std=c++17
CPPFLAGS := -MMD

COMPILE  := $(CXX) $(CXXFLAGS) $(CPPFLAGS)

SRCS     := ASTPrinter.cpp GenerateAST.cpp cLL.cpp LoxFunction.cpp
DEPS     := $(SRCS:.cpp=.d)


cll: Expr.h Stmt.h cLL.o LoxFunction.o
	@$(COMPILE) cLL.o LoxFunction.o -o $@


ast_printer: Expr.h Stmt.h ASTPrinter.cpp 
	@$(COMPILE) ASTPrinter.cpp -o $@


generate_ast: GenerateAST.o
	@$(COMPILE) $< -o $@


Expr.h Stmt.h: generate_ast
	@./generate_ast .


.PHONY: clean
clean:
	rm -f *.d *.o ast_printer generate_ast jlox cll

# Include dependencies
-include $(DEPS)


.PHONY: clean
clean:
	rm -f *.d *.o ast_printer generate_ast jlox


-include $(DEPS)


define make_test
.PHONY: $(1)
$(1):
	@make cll >/dev/null
	@echo "testing cll with $(1).lox ..."
	@./cll tests/$(1).lox | diff -u --color tests/$(1).lox.expected -;
endef


TESTS = \
test-statements \
test-statements2 \
test-statements3 \
test-statements4 \
test-statements5 \
test-statements6 \
test-control-flow \
test-control-flow2 \
test-functions \
test-functions2 \
test-functions3 \
test-functions4 \


$(foreach test, $(TESTS), $(eval $(call make_test,$(test))))


.PHONY: test-all
test-all:
	@for test in $(TESTS); do \
		make -s $$test; \
	done