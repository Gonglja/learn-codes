# 是否使用交叉编译工具链 A?=aaa 代表如果A没有被定义过，那么变量A的值就是aaa，反之A被定义过，这条语句什么都不做
CROSS_COMPILE ?=

# 生成的可执行文件名称
PROM = app

# 链接的三方库
LIBS = 
# 依赖的头文件，使用命令 find 搜索
DEPS = $(filter %.h %.hpp,$(shell find ./ -name "*.h*"))
# 源码文件，使用命令 find 搜索
SRCS = $(filter %.c %.cpp,$(shell find ./ -name "*.c*"))
# 过程文件， 变量替换技术：以静态模式定义的 ，依赖于被替换字串中有相同的模式 eg:1.cpp 2.cpp 3.cpp 将.cpp结尾替换为.o 1.o 2.o 3.o 
OBJS = $(SRCS:%.cpp=%.o) 

# 生成的二进制文件路径
BIN_DIR = ./bin
# CXXFLAGS -g 调试信息  --std=c++11 支持c++11新特性 添加头文件路径
CXXFLAGS= -g --std=c++11 -I./include

# 判断BIN_DIR是否是目录 A||B 如果A命令执行失败（返回假，$?==1）， 就执行B命令
$(shell [ -d ${BIN_DIR} ] || mkdir -p ${BIN_DIR})

# 伪目标：并不是一个文件，只是一个标签，make无法生成依赖关系和决定它是否要执行。 
# 只能通过显式地指明这个“目标”才能让其生效。伪目标不能和文件名重名，不然就失去了伪目标的意义
# 为避免和文件重名的情况，使用一个特殊的标记 .PHONY 显式地指明一个目标是伪目标，向make说明，不管是否有这个文件，这个目标就是伪目标 
.PHONY : all build clean 

all:build

# build 构建 依赖 $(OBJS) ，构建方法：g++ -o 执行文件 过程文件 依赖参数
build: $(OBJS)
	${CROSS_COMPILE}g++  $(LIBS) -o ${BIN_DIR}/$(PROM) $(OBJS)

# %.o 依赖 %.cpp ，所有的.o文件均会匹配到 1.o 2.o 会依赖 1.cpp 2.cpp 构建方法：g++ -c -o 执行文件 过程文件 依赖参数 -c只编译不链接 自动化变量$< 表示第一个依赖文件(比如 1.o:1.cpp,则$< 代表1.cpp) $@表示规则中的目标文件集(比如 1.o:1.cpp,则$< 代表1.o)
%.o: %.cpp $(DEPS) 
	${CROSS_COMPILE}g++ -c $(CXXFLAGS) $(LIBS) $< -o $@
		
clean:
	rm -rf $(OBJS) ${BIN_DIR}

