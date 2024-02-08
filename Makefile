
CC=gcc
AR=ar
ADEPT2-8=adept
SRCDIR=src/backend
OBJDIR=obj
C_SOURCES=$(wildcard $(SRCDIR)/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/AST/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/AST/EXPR/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/AST/POLY/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/AST/TYPE/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/AST/UTIL/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/BRIDGE/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/DRVR/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/LEX/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/PARSE/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/TOKEN/*.c) \
		$(wildcard $(SRCDIR)/INSIGHT/src/UTIL/*.c)
ADEPT_SOURCES=$(wildcard *.adept)
INCLUDE=include
INSIGHT_INCLUDE=$(SRCDIR)/INSIGHT/include
INSIGHT=obj/insight.a
C_OBJECTS=$(C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CFLAGS=-c -Wall -I"$(SRCDIR)/include" -I"$(SRCDIR)/INSIGHT/include" -O3 -DADEPT_INSIGHT_BUILD

release: $(INSIGHT) $(ADEPT_SOURCES)
	$(ADEPT2-8) main.adept

debug: $(INSIGHT) $(ADEPT_SOURCES)
	$(ADEPT2-8) debug.adept

$(INSIGHT): out-directories $(C_OBJECTS)
	$(AR) -rcs $(INSIGHT) $(C_OBJECTS)

$(C_OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
ifeq ($(OS), Windows_NT)
	rmdir /s /q obj/
else
	rm -rf ./obj/
endif

deepclean: clean

out-directories:
ifeq ($(OS), Windows_NT)
	@if not exist obj mkdir obj
	@if not exist obj\INSIGHT\src mkdir obj\INSIGHT\src
	@if not exist obj\INSIGHT\src\AST mkdir obj\INSIGHT\src\AST
	@if not exist obj\INSIGHT\src\AST\EXPR mkdir obj\INSIGHT\src\AST\EXPR
	@if not exist obj\INSIGHT\src\AST\POLY mkdir obj\INSIGHT\src\AST\POLY
	@if not exist obj\INSIGHT\src\AST\TYPE mkdir obj\INSIGHT\src\AST\TYPE
	@if not exist obj\INSIGHT\src\AST\UTIL mkdir obj\INSIGHT\src\AST\UTIL
	@if not exist obj\INSIGHT\src\BRIDGE mkdir obj\INSIGHT\src\BRIDGE
	@if not exist obj\INSIGHT\src\DRVR mkdir obj\INSIGHT\src\DRVR
	@if not exist obj\INSIGHT\src\LEX mkdir obj\INSIGHT\src\LEX
	@if not exist obj\INSIGHT\src\PARSE mkdir obj\INSIGHT\src\PARSE
	@if not exist obj\INSIGHT\src\TOKEN mkdir obj\INSIGHT\src\TOKEN
	@if not exist obj\INSIGHT\src\UTIL mkdir obj\INSIGHT\src\UTIL
else
	@mkdir -p obj
	@mkdir -p obj/INSIGHT/src
	@mkdir -p obj/INSIGHT/src/AST
	@mkdir -p obj/INSIGHT/src/AST/EXPR
	@mkdir -p obj/INSIGHT/src/AST/POLY
	@mkdir -p obj/INSIGHT/src/AST/TYPE
	@mkdir -p obj/INSIGHT/src/AST/UTIL
	@mkdir -p obj/INSIGHT/src/BRIDGE
	@mkdir -p obj/INSIGHT/src/DRVR
	@mkdir -p obj/INSIGHT/src/LEX
	@mkdir -p obj/INSIGHT/src/PARSE
	@mkdir -p obj/INSIGHT/src/TOKEN
	@mkdir -p obj/INSIGHT/src/UTIL
endif
