---
aliases: 
tags: 
 - json
 - 源码分析
created: 2022-12-06 19:06:06
modified: 2022-12-17 17:52:13
---

## 目的

学习 c 、json 及 cJSON

## 介绍

### json

**JSON**(**J**ava**S**cript **O**bject **N**otation, JavaScript 对象表示法))，是存储和交换文本信息的语法。其内容由属性和值组成。

基本数据类型

- 数值：十进制数，不能有前导 0，可以为负数，可以有小数部分。也可以用 e 或 E 表示指数部分。不能包含非数，如 NaN。不区分整数与浮点数。用双精度浮点数表示所有数值。
- 字符串：以双引号`""`括起来的零个或多个 Unicode 码。支持反斜杠开始的转义字符序列。
- 布尔值：表示为 `true` 和 `false`
- 数组：有序的零个或多个值。每个值可以为任意类型。数组使用方括号`[]`包裹。多个数组元素之间用逗号`,`分割，形如：`[value, value]`
- 对象：若干无序的“键-值对”（key-value pairs），其中键只能是字符串。建议但不强制要求对象种的键是独一无二的。对象以花括号`{}`包裹。多个键-值对之间使用逗号`,`。键与值之间用冒号`:`分割。
- 空值：值写为 null

token（6 种标点符号、字符串、数值、3 种字面量）之间可以存在有限的空白符并被忽略。四个特定字符被认为是空白符：空格符、水平制表符、回车符、换行符。
空白符不能出现在 token 内部（但空格符可以出现在字符串内部）。
JSON 交换之必须编码为 UTF-8。

### cJSON

https://github.com/DaveGamble/cJSON

## 使用

详见 [cJSON_test.c](https://github.com/Gonglja/learn-json/blob/1f0af282f6e3a676301ca47effc21b1dd615121d/app/cJSON_test.c)

## 源码分析

cJSON 是一个 c 语言编写的 JSON 格式数据解析器，把 JSON 数据看成是**层次树**，**同级之间用双向链表相连，上下级用单项链表形成树状结构**。
(注：同级之间通过 `cJSON`结构中的 `next`和`prev`指针连接，上下级则通过`child`连接)

### cJSON 结构体

```c
/* The cJSON structure: */
typedef struct cJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next; // 指向 下一个 cJSON 结构体
    struct cJSON *prev; // 指向 上一个 cJSON 结构体
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cJSON *child; // 指向 孩子 cJSON 结构体

    /* The type of the item, as above. */
    int type;            // 类型，分为 7 种

    /* The item's string, if type==cJSON_String  and type == cJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use cJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==cJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;         // key-value 的 key
} cJSON;

/* cJSON Types: */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7) /* raw json */
```

### 创建对象/空节点等

在创建 `Object/Null/True/False/Number/Array/String` 等的过程中，调用 `cJSON_New_Item`，创建一个新项（实际也就是通过 allocate 去分配堆内存，并且将其初始化为 0，也就是相当于 calloc ），并且将 cJSON 的类型指定为 `cJSON_Object/Null/True/False/Number/Array/String`

在 internal_hooks 可以自己实现 alloc、free，此处在初始化时被指定为 `malloc`、`free`，所以当调用 `cJSON_New_Item` 时，调用的也就是 `malloc` 和 `free`

```c
CJSON_PUBLIC(cJSON *) cJSON_CreateObject(void)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_Object;
    }

    return item;
}

/* Internal constructor. */
static cJSON *cJSON_New_Item(const internal_hooks * const hooks)
{
    cJSON* node = (cJSON*)hooks->allocate(sizeof(cJSON));
    if (node)
    {
        memset(node, '\0', sizeof(cJSON));
    }

    return node;
}


typedef struct internal_hooks
{
    void *(CJSON_CDECL *allocate)(size_t size);
    void (CJSON_CDECL *deallocate)(void *pointer);
    void *(CJSON_CDECL *reallocate)(void *pointer, size_t size);
} internal_hooks;

static internal_hooks global_hooks = { internal_malloc, internal_free, internal_realloc };


CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks)
{
    if (hooks == NULL)
    {
        /* Reset hooks */
        global_hooks.allocate = malloc;
        global_hooks.deallocate = free;
        global_hooks.reallocate = realloc;
        return;
    }

    global_hooks.allocate = malloc;
    if (hooks->malloc_fn != NULL)
    {
        global_hooks.allocate = hooks->malloc_fn;
    }

    global_hooks.deallocate = free;
    if (hooks->free_fn != NULL)
    {
        global_hooks.deallocate = hooks->free_fn;
    }

    /* use realloc only if both free and malloc are used */
    global_hooks.reallocate = NULL;
    if ((global_hooks.allocate == malloc) && (global_hooks.deallocate == free))
    {
        global_hooks.reallocate = realloc;
    }
}
```

### 创建节点数组

```c
CJSON_PUBLIC(cJSON *) cJSON_CreateIntArray(const int *numbers, int count)
{
    size_t i = 0;
    cJSON *n = NULL;
    cJSON *p = NULL;
    cJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
        return NULL;
	// 创建数组头节点
    a = cJSON_CreateArray();

    for(i = 0; a && (i < (size_t)count); i++) {
        n = cJSON_CreateNumber(numbers[i]); // 创建 number 节点，创建不成功则删除头节点
        if (!n) {
            cJSON_Delete(a);
            return NULL;
        }
        if(!i) { // 如果是0，也就是第一个 number 节点，将头节点的child指向 该number 节点
            a->child = n;
        } else { // 不是头节点，则将当前节点 与 前一个 number 节点双向链接
            suffix_object(p, n);
        }
        p = n;   // 更新前一个 number 节点
    }

    if (a && a->child) { // 最后如果 有数组并该数组有子节点，那么将数组头节点的prev 指向最后一个节点，形成双向链表
        a->child->prev = n;
    }

    return a;
}


CJSON_PUBLIC(cJSON *) cJSON_CreateArray(void)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type=cJSON_Array;
    }

    return item;
}
```

### 添加孩子节点到 Array/Object 上

不管添加 String/Null/Object 等到 Array/Object 中，最终调用的都是该 add_item_to_object 接口

```c
CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string)
{
    cJSON *string_item = cJSON_CreateString(string);
    if (add_item_to_object(object, name, string_item, &global_hooks, false)) {
        return string_item;
    }

    cJSON_Delete(string_item);
    return NULL;
}

CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item) {
    return add_item_to_object(object, string, item, &global_hooks, false);
}
// 添加条目到对象中：该函数主要是构建新key和type，最后调用 add_item_to_array
// 该函数有两种模式：常量key，非常量key
// 在常量 key下，通过 cast_away_const 去 const 化，并且标记类型为 cJSON_StringIsConst
// 非常量 key 下，则通过 cJSON_strdup 函数重新 alloc 一个 新key，并且拷贝过去。
static cJSON_bool add_item_to_object(cJSON * const object, const char * const string, cJSON * const item, const internal_hooks * const hooks, const cJSON_bool constant_key) {
    char *new_key = NULL;
    int new_type = cJSON_Invalid;

    if ((object == NULL) || (string == NULL) || (item == NULL) || (object == item)) {
        return false;
    }

    if (constant_key) {
        new_key = (char*)cast_away_const(string);
        new_type = item->type | cJSON_StringIsConst;
    } else {
        new_key = (char*)cJSON_strdup((const unsigned char*)string, hooks);
        if (new_key == NULL) {
            return false;
        }

        new_type = item->type & ~cJSON_StringIsConst;
    }

    if (!(item->type & cJSON_StringIsConst) && (item->string != NULL)) {
        hooks->deallocate(item->string);
    }

    item->string = new_key;
    item->type = new_type;

    return add_item_to_array(object, item);
}

// array 作为头节点，如果头节点的child 为空，则说明该对象还没有链接到其它节点，所以此时可以直接 链接 item ，并且将item的prev 指向自身，next 为空。
// 如果头节点的 child 不为空，则说明该对象已经链接到其它节点，在添加节点只需要追加到最后就可以了。此处的精髓，在于双向链表如果在尾部插入时，知道头部了，所以最后一个节点也就是 头部--> prev ，而无需在从头遍历到尾
static cJSON_bool add_item_to_array(cJSON *array, cJSON *item)
{
    cJSON *child = NULL;

    if ((item == NULL) || (array == NULL) || (array == item)) {
        return false;
    }

    child = array->child;
    /*
     * To find the last item in array quickly, we use prev in array
     */
    if (child == NULL) {
        /* list is empty, start new one */
        array->child = item;
        item->prev = item;
        item->next = NULL;
    } else {
        /* append to the end */
        if (child->prev) {
            suffix_object(child->prev, item);
            array->child->prev = item;
        }
    }

    return true;
}
```

### 创建引用节点

cJSON_CreateStringReference 与 cJSON_CreateString 的区别在于，Reference 引用，**无拷贝**，直接就是外部的数据，数据的生命周期由**外部**决定

```c
CJSON_PUBLIC(cJSON *) cJSON_CreateStringReference(const char *string)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if (item != NULL)
    {
        item->type = cJSON_String | cJSON_IsReference;
        item->valuestring = (char*)cast_away_const(string);
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateString(const char *string)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item)
    {
        item->type = cJSON_String;
        item->valuestring = (char*)cJSON_strdup((const unsigned char*)string, &global_hooks);
        if(!item->valuestring)
        {
            cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}
```

### 创建数组引用

都创建了一个新的条目并标记为数组，返回新条目地址，
区别在于 child 节点由外部管理（引用）还是内部管理（直接拷贝）

```c
CJSON_PUBLIC(cJSON *) cJSON_CreateArrayReference(const cJSON *child) {
    cJSON *item = cJSON_New_Item(&global_hooks);
    if (item != NULL) {
        item->type = cJSON_Array | cJSON_IsReference;
        item->child = (cJSON*)cast_away_const(child);
    }

    return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateArray(void)
{
    cJSON *item = cJSON_New_Item(&global_hooks);
    if(item) {
        item->type=cJSON_Array;
    }

    return item;
}
```

### cJSON_Parse

cJSON_Parse 是 cJSON 中解析字符串的一个函数，通过解析字符串中的数据，产生一个存储 json 数据的链表。

```c
CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value)
{
    return cJSON_ParseWithOpts(value, 0, 0);
}
```

第一个参数为传入的 json 字符串，后两个参数传入为 0，暂时不用管。

```c
// 输入是json字符串，将其长度+1，进入 cJSON_ParseWithLengthOpts 函数
CJSON_PUBLIC(cJSON *) cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated) {
    size_t buffer_length;

    if (NULL == value) {
        return NULL;
    }

    /* Adding null character size due to require_null_terminated. */
    buffer_length = strlen(value) + sizeof("");

    return cJSON_ParseWithLengthOpts(value, buffer_length, return_parse_end, require_null_terminated);
}
```

第一个参数传入的 json 字符串，第二个为 buffer 的长度，字符串长度+1，另外两个参数不用管。如果解析成功，则返回 cJSON 链表。

```c
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, cJSON_bool require_null_terminated)
{
    parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    cJSON *item = NULL;

    /* reset error position */
    global_error.json = NULL;
    global_error.position = 0;

    if (value == NULL || 0 == buffer_length) {
        goto fail;
    }

    buffer.content = (const unsigned char*)value;
    buffer.length = buffer_length;
    buffer.offset = 0;
    buffer.hooks = global_hooks;

    item = cJSON_New_Item(&global_hooks);
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }

    if (!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        /* parse failure. ep is set. */
        goto fail;
    }

    /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (require_null_terminated)
    {
        buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
        {
            goto fail;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char*)buffer_at_offset(&buffer);
    }

    return item;

fail:
    if (item != NULL)
    {
        cJSON_Delete(item);
    }

    if (value != NULL)
    {
        error local_error;
        local_error.json = (const unsigned char*)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length)
        {
            local_error.position = buffer.offset;
        }
        else if (buffer.length > 0)
        {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL)
        {
            *return_parse_end = (const char*)local_error.json + local_error.position;
        }

        global_error = local_error;
    }

    return NULL;
}
```

#### parse_buffer

看一下 parse_buffer 处理缓冲区的结构

```c
typedef struct
{
    const unsigned char *content; // json 字符串
    size_t length; // 字符串总长度
    size_t offset; // 当前解析的位置
    size_t depth;  // json 对象嵌套的层级
    internal_hooks hooks; // 分配内存相关的函数
} parse_buffer;
```

为了更方便的使用改结构体，cJSON 提供几个宏来操作，在接下来的代码中会经常出现。

```c
// 检查 buffer 能否读取从 offset 开始的 size 个数据
#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))

// 检查 buffer 能否从 offset 开始读 index 个字节的数据
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
// 返回 buffer 的 offset 的字符串指针
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)
```

#### skip_utf8_bom()

[BOM](https://zh.wikipedia.org/wiki/%E4%BD%8D%E5%85%83%E7%B5%84%E9%A0%86%E5%BA%8F%E8%A8%98%E8%99%9F)(byte-order mark, 字节顺序标记)，常被用来当作标示文件是以 UTF-8、UTF-16、UTF-32 编码的标记。
字节顺序标记的使用是选择性的，它的存在会干扰不希望文件开头出现非 ASCII 字符、但可以用其它方式处理文字流的软件对于 UTF-8 的使用。

因此在此处，如果开头碰见这个标记则会跳过。

```c
static parse_buffer *skip_utf8_bom(parse_buffer * const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0))
    {
        return NULL;
    }
	// 如果可以访问从 offset 开始的第4个字符，并且前三个字符为 0xEF、0xBB、0xBF则跳过并+3
    if (can_access_at_index(buffer, 4) && (strncmp((const char*)buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) == 0))
    {
        buffer->offset += 3;
    }

    return buffer;
}
```

#### buffer_skip_whitespace

该函数用于跳过特殊字符（ASCII 码值小于 32）

```c
/* Utility to jump whitespace and cr/lf */
static parse_buffer *buffer_skip_whitespace(parse_buffer * const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
    {
        return NULL;
    }

    if (cannot_access_at_index(buffer, 0))
    {
        return buffer;
    }
	// ASCII 码值小于 32的都是不可显示的，特殊功能的码
    while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
    {
       buffer->offset++; // 标记跳过
    }
	// 如果字符串的尾部，则退回一个
    if (buffer->offset == buffer->length)
    {
        buffer->offset--;
    }

    return buffer;
}
```

#### parse_value

对象使用`{` 作开头，`}`作结尾，里面的每一个元素都是键值对的无序组合，键和值用`:`分割，使用`,`分隔每一个元素；
数组使用`[`作开头，`]`作结尾，里面的元素都是有序的值组成的集合，使用`,`作分隔符。

因此，我们得知：

- 对于简单的 null、true、false 只需要判断字符串是否相等；
- 对于其它类型的 item，判断第一个字符；
  - 数字类型的，判断符号位及 ASCII 码`0-9`
  - 字符串类型的，判断是否有`"`;
  - 数组类型的，判断 `[`
  - 对象类型的，判断`{`

```c
/* Parser core - when encountering text, process appropriately. */
static cJSON_bool parse_value(cJSON * const item, parse_buffer * const input_buffer)
{   // 输入buffer为空
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false; /* no input */
    }

    /* parse the different types of values */
    // 如果可以访问从 offset 开始的4个字符，并且为 null
    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "null", 4) == 0))
    {   // 则该类型就是 cJSON_NULL，跳过4个字符继续
        item->type = cJSON_NULL;
        input_buffer->offset += 4;
        return true;
    }
    // 如果可以访问从 offset 开始的5个字符，并且为 false
    if (can_read(input_buffer, 5) && (strncmp((const char*)buffer_at_offset(input_buffer), "false", 5) == 0))
    {   // 则该类型就是 cJSON_False，跳过5个字符继续
        item->type = cJSON_False;
        input_buffer->offset += 5;
        return true;
    }
    // 如果可以访问从 offset 开始的4个字符，并且为 true
    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "true", 4) == 0))
    {   // 则该类型就是 cJSON_False，跳过4个字符继续
        item->type = cJSON_True;
        item->valueint = 1;
        input_buffer->offset += 4;
        return true;
    }
    // 如果可以访问从 offset 开始读0个字节数据，也就是读当前数据，如果当前数据为 `\"`，则表示为遇到字符串开头了。
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"'))
    {   // 进入处理字符串函数
        return parse_string(item, input_buffer);
    }
    // 如果可以访问从 offset 开始读0个字节数据，也就是读当前数据，如果当前数据为 `-` 或者大于 `0`且小于`9`，则表示为遇到数字了。
    if (can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))
    {   // 进入处理数字函数
        return parse_number(item, input_buffer);
    }
    // 如果可以访问从 offset 开始读0个字节数据，也就是读当前数据，如果当前数据为 `[` ，表示遇到数组了。
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
    {   // 进入处理数组函数
        return parse_array(item, input_buffer);
    }
    // 如果可以访问从 offset 开始读0个字节数据，也就是读当前数据，如果当前数据为 `{` ，表示遇到对象了。
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {   // 进入处理对象函数
        return parse_object(item, input_buffer);
    }

    return false;
}
```

#### parse_string

从 json 字符串中解析字符串。

```c
static cJSON_bool parse_string(cJSON * const item, parse_buffer * const input_buffer)
{
    const unsigned char *input_pointer = buffer_at_offset(input_buffer) + 1; // 字符 '\"'的下一个位置
    const unsigned char *input_end = buffer_at_offset(input_buffer) + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (buffer_at_offset(input_buffer)[0] != '\"')
    {
        goto fail;
    }

    {
        // 计算要分配的字符的长度和跳过字节数
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        // 遍历 字符串，从"的下一个字符开始，到字符串结束，如果是转义字符，则跳过
        while (((size_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
        {
            // 如果是转义字符，对解析无用，则跳过改字符
            if (input_end[0] == '\\')
            {   // 防止当最后一个字符是\时，内存溢出
                if ((size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
                {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        // 正常情况下，input_end 此时应该是结尾的"，且长度不超过 length，再次检查
        if (((size_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
        {
            goto fail; /* string ended unexpectedly */
        }

        // 总的长度：输入字符串尾部-输入字符串头部-跳过的字节数
        allocation_length = (size_t) (input_end - buffer_at_offset(input_buffer)) - skipped_bytes;
        // 分配内存
        output = (unsigned char*)input_buffer->hooks.allocate(allocation_length + sizeof(""));
        if (output == NULL)
        {
            goto fail; /* allocation failure */
        }
    }
	// 复制字符串中的有效内容到 output 中
    output_pointer = output;
    /* loop through the string literal */
    while (input_pointer < input_end)
    {   // 如果不是转义字符，则原样拷贝
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        /* escape sequence */
        else // 是转义字符，则根据第二个字符，设置输出的字符 将字符串中两个字节对应的特殊符号转为1个字节的符号 "\r" 转为 `\r`
        {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1)
            {
                goto fail;
            }

            switch (input_pointer[1])
            {
                case 'b':
                    *output_pointer++ = '\b';
                    break;
                case 'f':
                    *output_pointer++ = '\f';
                    break;
                case 'n':
                    *output_pointer++ = '\n';
                    break;
                case 'r':
                    *output_pointer++ = '\r';
                    break;
                case 't':
                    *output_pointer++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *output_pointer++ = input_pointer[1];
                    break;

                /* UTF-16 literal */
                case 'u':
                    sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
                    if (sequence_length == 0)
                    {
                        /* failed to convert UTF16-literal to UTF-8 */
                        goto fail;
                    }
                    break;

                default:
                    goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    // 字符串结束符
    *output_pointer = '\0';
    // 设置item类型为字符串及输出字符串
    item->type = cJSON_String;
    item->valuestring = (char*)output;
    // 更新偏移
    input_buffer->offset = (size_t) (input_end - input_buffer->content);
    input_buffer->offset++;

    return true;

fail:
    if (output != NULL)
    {
        input_buffer->hooks.deallocate(output);
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    return false;
}
```

#### parse_number

```c
static cJSON_bool parse_number(cJSON * const item, parse_buffer * const input_buffer)
{
    double number = 0;
    unsigned char *after_end = NULL;
    unsigned char number_c_string[64];
    unsigned char decimal_point = get_decimal_point();
    size_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false;
    }

    // 判断最后63个字符是否为数字或小数点
    for (i = 0; (i < (sizeof(number_c_string) - 1)) && can_access_at_index(input_buffer, i); i++)
    {
        switch (buffer_at_offset(input_buffer)[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case 'e':
            case 'E':
	            // 以上情况直接复制到 number_c_string
                number_c_string[i] = buffer_at_offset(input_buffer)[i];
                break;

            case '.':
                // 小数点的情况特殊处理
                number_c_string[i] = decimal_point;
                break;

            default:
                // 不是数字或小数点，则说明已结束
                goto loop_end;
        }
    }
loop_end:
    // 赋予字符串终止符
    number_c_string[i] = '\0';
    // 这个才是核心，str to double，将字符串转为double
    number = strtod((const char*)number_c_string, (char**)&after_end);
    if (number_c_string == after_end)
    {
        return false; /* parse_error */
    }
	// item 赋值
    item->valuedouble = number;

    // 判断是否越界
    if (number >= INT_MAX)
    {
        item->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        item->valueint = INT_MIN;
    }
    else
    {
        item->valueint = (int)number;
    }

    item->type = cJSON_Number;

    input_buffer->offset += (size_t)(after_end - number_c_string);
    return true;
}
```

#### parse_array

> [1, 2, 3, 5] 转换为 cJSON 的链表，一共有 4 个节点，每个节点的类型都是 number，进入 parse_value 后都是往 parse_number 中跑。

```c
/* Build an array from input text. */
static cJSON_bool parse_array(cJSON * const item, parse_buffer * const input_buffer)
{
    cJSON *head = NULL; /* head of the linked list */
    cJSON *current_item = NULL;
    // json 深度限制
    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;
    // 如果开头不是 `[`, 说明不是 array
    if (buffer_at_offset(input_buffer)[0] != '[')
    {
        /* not an array */
        goto fail;
    }
    // 跳过首元素 `[` 和白块
    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    // 如果当前 offset 处的值为 `]`,则说明为空数组
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
    {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    // 返回到首元素处
    input_buffer->offset--;
    // 循环创建 item，
    do
    {
        /* allocate next item */
        cJSON *new_item = cJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            // 新链表开始
            current_item = head = new_item;
        }
        else
        {
            // 将新成员插入链表
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse next value */
        input_buffer->offset++;
        // 跳过数组中的元素前面可能存在的 特殊字符，比如`[  1 ,2,]`，此处则跳过`1`前的空白
        buffer_skip_whitespace(input_buffer);
        // 处理 value
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
         // 跳过数组中的元素后面可能存在的 特殊字符，比如`[  1 ,2,]`，此处则跳过`1`后`,`前的空白
        buffer_skip_whitespace(input_buffer);
    } // `,`表示存在下一个item，没有则退出
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));
	// 正常情况下，数组的最后一个元素解析完，下一个字符就是 `[`
    if (cannot_access_at_index(input_buffer, 0) || buffer_at_offset(input_buffer)[0] != ']')
    {
        goto fail; /* expected end of array */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }
    // 将数组的链表头挂在item的子节点上
    item->type = cJSON_Array;
    item->child = head;

    input_buffer->offset++;

    return true;

fail:
	// 失败则释放内存
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return false;
}
```

#### parse_object

从 json 字符串中解析 object。
该函数与 上一个函数非常像，不同之处在于，解析 json 对象时，当解析完`{`后，需要先解出 item 的**键**，在调用 parse_value 解析 item 的**值**

```c
static cJSON_bool parse_object(cJSON * const item, parse_buffer * const input_buffer)
{
    cJSON *head = NULL; /* linked list head */
    cJSON *current_item = NULL;
    // json 深度限制
    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false; /* to deeply nested */
    }
    input_buffer->depth++;
    // 对象是以 `{}` 包裹
    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '{'))
    {
        goto fail; /* not an object */
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    // 是否是空对象
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    // 往前退一字节
    input_buffer->offset--;
    // 循环创建 item ，`{` 之后一定是 键
    do
    {
        /* allocate next item */
        cJSON *new_item = cJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse the name of the child */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        // 解析 key，key 一定是string 类型
        if (!parse_string(current_item, input_buffer))
        {
            goto fail; /* failed to parse name */
        }
        buffer_skip_whitespace(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        current_item->string = current_item->valuestring;
        current_item->valuestring = NULL;
		// 检查 key 后是否是 `:`
        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        // 解析 value，而 value 可以是任何类型
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    }
    // 每个item是`,` 最后一个item 没有逗号
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));
	// 检查大括号是否和上
    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail; /* expected end of object */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = cJSON_Object;
    item->child = head;

    input_buffer->offset++;
    return true;

fail:
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return false;
}
```

到此，cJSON 基本上是分析完了。

## 参考

1. [JSON wiki](https://zh.wikipedia.org/wiki/JSON)
2. [零基础学习 cJSON 源码详解与应用](https://blog.csdn.net/weixin_44821644/article/details/110481533)
