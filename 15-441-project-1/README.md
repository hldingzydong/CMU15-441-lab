## Note
- 使用Clion进行开发,请安装2020.2版本,该版本支持Makefile Application 
 
- 在parse.c中添加对`set_parsing_options`与`yyparse`的声明
```c
extern void set_parsing_options(char *buf, size_t siz, Request *parsing_request);
extern int yyparse();
```

- 在parser.y中,递归调用request_header可支持多头部parse
- 在macOS下,使用`xxd`查看文件的二进制表示
```shell script
xxd {filename}
```
- 在parse http request时,遵循RFC2616对Request格式的定义:
```
Request       = Request-Line                      ; Section 5.1
                        *(( general-header        ; Section 4.5
                         | request-header         ; Section 5.3
                         | entity-header ) CRLF)  ; Section 7.1
                        CRLF
                        [ message-body ]          ; Section 4.3

```
所以,message-body包含任意字符,包括crlf,因此无需解析,只需放入Request->body中