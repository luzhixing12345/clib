
# shell

有时候我们希望在程序中嵌入一个小型的控制台, 实现一个 shell 太过复杂, 也完全没有必要, 因为往往我们只需要在控制台中支持自定义的命令, 也就是完成解析, 再跳转到对应的函数处理即可

下面是一个简单的示例, 输入 exit 或者 <kbd>ctrl</kbd> + <kbd>c</kbd> 退出

```c
#include <clib/clib.h>

int main(int argc, char **argv) {
    struct shell *shell = create_shell("> ");   // 初始化一个 shell 对象
    shell->history_file = ".shell_history";     // (可选) 将用户输入作为历史记录保存/导入
    init_shell(shell);                          // 初始化 shell

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

![shell](https://raw.githubusercontent.com/learner-lu/picbed/master/shell.gif)

支持

- 历史命令提示, <kbd>tab</kbd> 自动补全
- 上下历史记录回顾, 左右箭头移动, <kbd>backspace</kbd> 退格, <kbd>home</kbd> <kbd>end</kbd> 
- <kbd>ctrl</kbd> + <kbd>u</kbd> 清空光标之前, <kbd>ctrl</kbd> + <kbd>k</kbd> 清空光标之后
- <kbd>ctrl</kbd> + <kbd>l</kbd> 清屏

主循环中会在每一次 <kbd>enter</kbd> 之后返回, 其 event 对象包含两个属性 argc 和 argv 为当前命令解析结果

> [!NOTE]
> 推荐使用本项目中的 [argparse](./参数解析.md) 模块进行后续的参数解析哦