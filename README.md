# clib

[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/luzhixing12345/clib/blob/main/LICENSE)

高级语言(如C++、Python等)提供了丰富的标准库和数据结构, 但是在 C 中缺乏灵活且易于使用的内建数据结构, 导致开发者需要自行编写很多常用工具库

clib 是一个小型的 C 工具库, 用精简的方式实现了一些 C 标准库没有提供但是日常开发中常见的功能需求, 包括

- 日志
- 命令行参数解析
- hashtable
- 终端彩色打印
- 目录, 文件相关的处理(判断文件存在, 递归删除目录...)
- tqdm(进度条)
- 简易内嵌shell

本项目大部分代码是笔者从网络上搜集到的其他大佬开发好的轮子并修改封装为简单易用的 API, 绝赞维护更新中(๑˃̵ᴗ˂̵)و

## 示例

### 日志(thread safe)

方便好记, 和 printf 一样流畅的体验, 有颜色有位置, vscode 一步定位

```c
#include <clib/clib.h>

int main(int argc, char **argv) {
    DEBUG("this is debug message");
    INFO("this is info message");
    WARNING("this is warning message");
    ERR("this is error message");
    return 0;
}
```

![image](https://raw.githubusercontent.com/learner-lu/picbed/master/20250127142451.png)

### 参数解析

不要每次都花费大量的时间在处理参数上面了, 几个宏快速绑定参数, 自动生成 help 文档

```c
#include <clib/clib.h>

char *output = NULL;
char **include_path = NULL;

char **files = NULL;
char *optimize = NULL;
char **warning = NULL;
char **library_path = NULL;
char **library_name = NULL;
char *c_standard = NULL;

int debug = 0;

int main(int argc, const char **argv) {
    argparse_option options[] = {
        ARG_BOOLEAN(NULL, "-c", NULL, "Compile and assemble, but do not link.", NULL, "compile"),
        ARG_STR(&output, "-o", NULL, "Place the output into <file>.", " <file>", NULL),
        ARG_STRS(&include_path, "-I", NULL, "Add <dir> to the end of the main include path.", "<dir>", "include"),
        ARG_STRS(
            &library_path, "-L", NULL, "Add <dir> to the end of the main library path.", "<dir>", "library_path"),
        ARG_STRS(&library_name, "-l", NULL, "Search <lib> in library path", "<lib>", "library_name"),
        ARG_BOOLEAN(&debug, "-g", NULL, "Generate debug information in default format", NULL, "debug"),
        ARG_STRS(&warning, "-W", NULL, "Warning information option", NULL, NULL),
        ARG_STR(&optimize, "-O", "--optimize", "optimization level to <number>.", "<number>", NULL),
        ARG_STR(&c_standard, NULL, "--std", "C Compile standard, supported: {c99}", "=<standard>", NULL),
        ARG_BOOLEAN(NULL, "-h", "--help", "show help information", NULL, "help"),
        ARG_BOOLEAN(NULL, "-v", "--version", "show version", NULL, "version"),
        ARG_STRS_GROUP(&files, NULL, NULL, NULL, NULL, "src"),
        ARG_END()};

    argparse parser;
    argparse_init(
        &parser, options, ARGPARSE_ENABLE_ARG_STICK | ARGPARSE_ENABLE_STICK | ARGPARSE_ENABLE_EQUAL);
    argparse_describe(&parser,
                           "kcc",
                           "Kamilu's C Compiler",
                           "document:   <https://github.com/luzhixing12345/kcc>\nbug report: "
                           "<https://github.com/luzhixing12345/kcc/issues>");
    argparse_parse(&parser, argc, argv);

    if (arg_ismatch(&parser, "help")) {
        argparse_info(&parser);
    }

    free_argparse(&parser);
    return 0;
}
```
![20250130022533](https://raw.githubusercontent.com/learner-lu/picbed/master/20250130022533.png)

### hashtable(thread safe)

还在苦于 C 用不了 unordered_map? 给你把 C 的 API 封装好了, 直接用就可以了

```c
#include <clib/clib.h>

int main(int argc, const char* argv[]) {
	HashTable table;
	ht_setup(&table, sizeof(int), sizeof(double), 10);

	int x = 6;
	double y = 9;
	ht_insert(&table, &x, &y); // 插入

	if (ht_contains(&table, &x)) { // 判断
		y = *(double*)ht_lookup(&table, &x); // 查找
		printf("%d's value is: %f\n", x, y);
	}

	ht_erase(&table, &x); // 删除
    
	ht_destroy(&table);
}
```

### tqdm(进度条)

简单好用, 以及高度自定义的进度条

```c
#include <clib/clib.h>
#include <unistd.h>  // For usleep

int main() {
    int iter = 300;
    struct tqdm progress;

    init_tqdm(&progress, iter);
    for (int i = 0; i < iter; i++) {
        update_tqdm(&progress, i);
        usleep(10000);
    }
    return 0;
}
```

![iokl8](https://raw.githubusercontent.com/learner-lu/picbed/master/iokl8.gif)

### shell

想为你的程序添加一个内置的控制台? 只是简单处理一些命令, 一个完整的 shell 太过臃肿了, 来试试这个麻雀虽小五脏俱全的 shell 吧

```c
#include <clib/clib.h>

int main(int argc, char **argv) {
    struct shell *shell = create_shell("> ");
    init_shell(shell);

    while (shell_run(shell)) {
        struct shell_event *event = &shell->event;
        if (event->argc > 0 && !strcmp(event->argv[0], "exit")) {
            break;
        }
        for (int i = 0; i < event->argc; i++) {
            printf("%s ", event->argv[i]);
        }
        printf("\n");
    }

    free_shell(shell);
    return 0;
}
```

> event 获取到命令行参数之后可以用 argparse 解析哦

## 安装(TODO)

```bash
sudo apt install clib
```

## 源码编译

本库无第三方依赖, 且代码量相对较小, 可以轻松的嵌入到其他 C 项目中, 完全可以(推荐)直接将 clib/ 拷贝到其他项目中并整体参与链接

也可以将其作为静态库编译

```bash
make
```

然后链接 clib/libclib.a 即可

```Makefile
CFLAGS  += -Iclib/
LDFLAGS += -Lclib/ -lclib
```

代码中引用 `#include <clib/cilb.h>` 即可获取引用函数声明

## 代码实例

examples/ 下包含了所有的实例代码

```bash
cd example
bear -- make
```

每一个 .c 文件对应一项功能, 可以运行对应的可执行文件来查看

## 文档

[libc document](https://luzhixing12345.github.io/clib/)

## 测试

```bash
make
cd test
make
./test.sh
```

## 参考

- [星星说编程](https://space.bilibili.com/50657960/)
- man