# 指定编译器（根据项目情况修改）
CC ?= gcc
STRIP ?= strip

# 指定编译模式（不需要修改）
BUILD_MODE ?= debug

# 指定生成的可执行文件名（根据项目情况修改）
TARGET = hs_ring_buffer_demo

# 指定源码顶层目录（根据项目情况修改）
SRC_TOP_DIR := .

# 指定输出文件存放路径（根据项目情况修改）
OUT_DIR_WITHOUT_BUILD_MODE  := ./output
OUT_DIR := $(OUT_DIR_WITHOUT_BUILD_MODE)/$(BUILD_MODE)

# 指定编译器参数（根据项目情况修改）
# -std=gnu99: 支持 C99 标准 这里不要使用 -std=c99，否则会报警告提示部分库函数未包含对应头文件
# -Wall: 打开所有警告信息
# -Werror: 警告报错误
CFLAGS := -std=gnu99 -Wall # -Werror
ifeq ($(BUILD_MODE), debug)
CFLAGS += -DDEBUG -O0 -g
else
CFLAGS += -DNDEBUG -O2
endif

# 指定头文件路径（根据项目情况修改）
# 包含源码顶层目录和子目录下有 .h 文件的文件夹，并且过滤隐藏文件夹
# -not -path: 排除掉匹配特定路径模式的文件或目录
CFLAGS += $(addprefix -I,$(shell find $(SRC_TOP_DIR)  -type f -name "*.h" \
						   -not -path "*/.*" \
						   -exec dirname {} \; | sort -u))

# 指定链接时库文件路径（根据项目情况修改）
LDFLAGS :=

# 指定运行时库文件路径（根据项目情况修改）
# -Wl,-rpath: 指定运行时库文件路径
LDFLAGS +=

# 指定需要链接的库（根据项目情况修改）
LIBS := -pthread

# 获取源码顶层目录下的所有的 .c 文件列表（不需要修改）
TARGET_SRC = $(shell find $(SRC_TOP_DIR) -name "*.c")
# 将所有 .c 文件替换为 .o 文件，并保留原有目录结构
TARGET_OBJS = $(patsubst %.c, $(OUT_DIR)/objs/%.c.o, $(TARGET_SRC))

# 以下为编译默认目标规则（不需要修改）
# Makefile 的默认目标
$(OUT_DIR)/bin/$(TARGET): $(TARGET_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@
	@if [ "$(BUILD_MODE)" = "debug" ]; then \
		echo "debug build mode, skip strip"; \
	elif command -v $(STRIP) > /dev/null 2>&1; then \
		echo "strip target"; \
		$(STRIP) $@; \
	else \
		echo "strip not found, skip strip"; \
	fi
	@echo "compile done, target file: $@"

# 生成 .c 文件对应的 .o 文件，并且自动处理头文件的依赖（不需要修改）
$(OUT_DIR)/objs/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# 自动包含生成的依赖文件（不需要修改）
-include $(TARGET_OBJS:.o=.d)

# 以下为清除规则（不需要修改）
# 清理目标设置为伪目标，防止外面有 clean 文件，阻止执行 clean
.PHONY:clean
clean:
	@rm $(OUT_DIR) -rf
	@echo "clean $(BUILD_MODE) mode all file done!"

# 清理目标设置为伪目标
.PHONY:distclean
distclean:
	@rm $(OUT_DIR_WITHOUT_BUILD_MODE) -rf
	@echo "clean all mode all file done!"
