OBJS_DIR = .objs

# define all the executables
EXE_CHATROOM=chatroom
EXE_SERVER=server
EXES_APPLICATION=$(EXE_CHATROOM) $(EXE_SERVER)

# list object file dependencies for each
OBJS_CHATROOM=$(EXE_CHATROOM).o chat_window.o client.o utils.o
OBJS_SERVER=$(EXE_SERVER).o

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Werror=unused-result
CFLAGS_DEBUG   = -O0 $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -pthread -fPIE
CFLAGS_RELEASE = -O2 $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -pthread -fPIE
CFLAGS_TSAN    = $(CFLAGS_DEBUG) -fPIE
CFLAGS_TSAN   += -fsanitize=thread -fPIC -pie -fPIE

# set up linker
LD = gcc
LDFLAGS = -pthread -lncurses
LD_TSAN_FLAGS = -ltsan -fPIC -pie

.PHONY: all
all: release

# build types
# run clean before building debug so that all of the release executables
# disappear
.PHONY: debug
.PHONY: release
.PHONY: tsan

release: $(EXES_APPLICATION)
debug:   clean $(EXES_APPLICATION:%=%-debug)
tsan:    clean $(EXES_APPLICATION:%=%-tsan)

# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly
$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-tsan.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_TSAN) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
# you will need a triple of exe and exe-debug and exe-tsan for each exe (other
# than provided exes)

$(EXE_CHATROOM): $(OBJS_CHATROOM:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CHATROOM)-debug: $(OBJS_CHATROOM:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CHATROOM)-tsan: $(OBJS_CHATROOM:%.o=$(OBJS_DIR)/%-tsan.o)
	$(LD) $^ $(LD_TSAN_FLAGS) -o $@

$(EXE_SERVER): $(OBJS_SERVER:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_SERVER)-debug: $(OBJS_SERVER:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_SERVER)-tsan: $(OBJS_SERVER:%.o=$(OBJS_DIR)/%-tsan.o)
	$(LD) $^ $(LD_TSAN_FLAGS) -o $@


.PHONY: clean
clean:
	-rm -rf .objs $(EXES_APPLICATION) $(EXES_APPLICATION:%=%-tsan) $(EXES_APPLICATION:%=%-debug)