######################################################
#使用说明:
# 本makefile是一个智能的makefile文件，只需要配置好你项目中的
# 一些特定参数，然后将makefile文件和源码发在一个目录执行make命令就可以了。
#######################################################
# 配置参数
# 是否使用C++11 特性
USE_Cplusplus11 	= 1
#为程序指定预编译宏，每个宏之间用空格隔开
PRE_MARCO 	= LINUX _REENTRANT	
#扩展包含头文件目录，每个目录之间用空格隔开
ADD_INCLUDE 	=
#扩展的库目录引用目录，每个目录之间用空格隔开
ADD_LIBDIR	=
#要包含的库名，每个库名用空格隔开(库名字需要些lib前缀和后缀.a或.so)
ADD_LIB		=
#编译生存的obj文件存放路径
BIN_DIR		= ./bin/
#生成输出目录
OUTPUT_DIR	= ../output/
#生成的output的类型
# 1 代表生成可执行文件
# 2 代表生成静态库文件
# 3 代表生存动态库文件
OUTPUT_TYPE	= 1
#生文件的名字，不包括后缀
OUTPUT_NAME	= socketLib
# 需要扩展编译的子目录，每个目录以空格隔开
SUB_DIRS	= 
#使用的编译器
CC		= g++
######################################################################
#编译器编译的配置参数
FLAGS		= \
 $(shell if [ 0 -ne $(USE_Cplusplus11) ];then echo "-std=c++11"; fi) \
 $(addprefix -D, $(PRE_MARCO))\
 $(addprefix -I, $(ADD_INCLUDE))\
 $(addprefix -L, $(ADD_LIBDIR))\
 $(addprefix -l, $(ADD_LIB))\

################################################################
#这个参数是FLAGS的扩展，主要是针对output文件生存的配置，		
FLAGS_LINK	= $(FLAGS)  -Wl,--no-as-needed -pthread
#####################################################################
# 	手动配置参数到此为止，下面的参数不许要配置了。
#####################################################################
#完整的输出名字
#strip 是去掉字符串两头的空格

OUTPUT_FULL_NAME =\
$(strip $(OUTPUT_DIR))$(shell if [ 2 -eq $(OUTPUT_TYPE) -o 3 -eq $(OUTPUT_TYPE) ];then echo -n "lib";fi)$(strip $(OUTPUT_NAME))$(shell if [ 1 -eq $(OUTPUT_TYPE) ];then echo -n "";elif [ 2 -eq $(OUTPUT_TYPE) ];then echo -n ".a";elif [ 3 -eq $(OUTPUT_TYPE) ];then echo -n ".so";fi)


#需要编译的文件名，文件间用空格分隔
FILES		= $(shell ls *.c*) 	

obj_file    = $(addprefix $(BIN_DIR), $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(shell ls $(1)*.c*))))
#所有生成的obj名字
OBJECTS		= $(call obj_file,)
OBJECTS	       += $(foreach dirname, $(SUB_DIRS), $(call obj_file,$(dirname)))
#######################################################

all:pre_command $(OBJECTS)
	@if [ 1 -eq $(OUTPUT_TYPE) ];then\
		$(CC) $(FLAGS_LINK) $(OBJECTS) -o $(OUTPUT_FULL_NAME);\
	elif [ 2 -eq $(OUTPUT_TYPE) ];then\
		ar crs $(OUTPUT_FULL_NAME) $(OBJECTS);\
	elif [ 3 -eq $(OUTPUT_TYPE) ];then\
		$(CC) -fPIC -shared -o $(OUTPUT_FULL_NAME) $(OBJECTS);\
	else \
		echo "请选择选择正确 OUTPUT_TYPE";\
	fi

#先处理创建一些必要的目录
pre_command:
	$(shell if [ -n "$(BIN_DIR)" -a ! -e "$(BIN_DIR)" ];then mkdir -p $(BIN_DIR); fi) #创建obj输出目录 bin 目录
	$(shell if [ -n "$(OUTPUT_DIR)" -a ! -e "$(OUTPUT_DIR)" ];then mkdir -p $(OUTPUT_DIR); fi ) #创建output文件的输出目录

#自动匹配 OBJECTS 中需要的 *.o 目标文件	
$(BIN_DIR)%.o: %.c*
	$(CC) $(FLAGS) -c -fPIC $< -o $@

#指定伪目标
.PHONY:clean clean_objs

#清除所有的生成文件，包括输出文件和生成的obj文件
clean:
	-rm -f $(OUTPUT_FULL_NAME) $(OBJECTS)

#只清除生成obj文件，不删除生成的输出文件
clean_objs:
	-rm -f $(OBJECTS)

echo:
	echo $(OBJECTS)
	echo $(OUTPUT_FULL_NAME)
