#include "config_info_collect.h"
#include "collect_code_block.h"
#include "error_report.h"
#include <stdlib.h>

/**
 *  1. 该模块从功能是xml搜集代码块，任何模板或者驱动函数都可以有
 *     相应的代码块，因此收集过程不应该涉及到具体模板或者驱动函数
 *     的模板数据结构
 *  2. 对于一个驱动函数而言，相同性质的代码块可能只有一个，例如preconditon
 *     函数，也可以有多个例如compute函数或者postprocess函数，为了是收集
 *     函数更通用，应该屏蔽这些细节；具体地使用一个变量来指定代码块的模式
 *     是单独存在的还是一个组的，如果是一个组的则每个代码块必须有id号且作为
 *     在这个组的唯一标识
 *  3. 如果是单独存在的则将收集到的代码块的地址返回，如果是一个组的那么内部
 *     申请一个相应大小的数组用于分别存放这些代码块的地址并将这个数组的首地
 *     址返回，void类型的指针就是用来屏蔽这些细节
 *  4. 如果相同性质的代码块是一个组的形式存在的，比如说这些代码块都是作为
 *     compute函数存在的，这些代码块都是包含在<compute_function>标签中,那么
 *     习惯用法是这些代码块都包含在<compute_functions>标签中，该标签中有一个
 *     length属性用来表示这个组的代码块的个数
 */


int collect_code_block
(
   char* op_name,
   char* code_block_name,
   int code_block_mode,
   struct code_block* cb
)
{
   void* op_context = find_device_operation(op_name);

   if (code_block_mode == SINGLE){
       return do_single_code_block_collection(op_context,
                                              code_block_name,
                                              cb);
   }else{
       char enclosing_elem_name[32];
       sprintf(enclosing_elem_name, "%ss", code_block_name);
       return do_group_code_blocks_collection(op_context,
                                              enclosing_elem_name,
                                              cb);
   }
}


static int do_group_code_blocks_collection
(
   void* op_context,
   const char* elem_name,
   struct code_block* gcb
)
{
   void* group_blocks = find_element_in_operation_context(op_context, elem_name);
   // 注意如果配置文件中没有length属性，将会报错这种是比较常见的模式
   // 应该有一个专门的报错函数，整理一下error_report
   int length = strtoul(get_element_data(group_blocks, "length"), NULL, 10); 
   const char** code_blocks_array = malloc(sizeof((void*)0) * length);
   gcb->code_blocks_src = code_blocks_array;
   gcb->num_block = length;

   printf("length is %d\n", length);

   int i;
   void* code_block = get_first_child(group_blocks);
   for (i=0; i<length; i++){
        
       install_code_block(code_blocks_array, length, code_block);

       printf("src is %s\n", get_element_data(code_block, "text_value"));
       printf("id is %s\n", get_element_data(code_block, "id"));
       
       code_block = get_next_sibling(code_block);
   }

   return SUCCESS;
}


static int do_single_code_block_collection
(
   void* op_context,
   const char* elem_name,
   struct code_block* scb
)
{ 
   void* single_block = find_element_in_operation_context(op_context, elem_name);
   scb->num_block = 1;
   scb->code_blocks_src = (void*)get_element_data(single_block, "text_value");

   return SUCCESS;
}


static void install_code_block
(
   const char** code_blocks_array,
   const int length,
   void* code_block
)
{
    const char* src_code = get_element_data(code_block, "text_value");
    unsigned int id =  strtoul(get_element_data(code_block, "id"), NULL, 10);
    
    if (id < length){
        code_blocks_array[id] = src_code; 
    }else{
        printf("Bad code block id\n");
        exit(1);
    }
}
