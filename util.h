#include "mpc.h"
#include <elf.h>
#include <llvm-c/Types.h>

#ifndef CONSTRUCTION_UTILREMAKE_H
#define CONSTRUCTION_UTILREMAKE_H
#endif

char *eval(mpc_ast_t *programme, FILE *file);

LLVMValueRef visit(LLVMModuleRef modulr, struct mpc_ast_t *tree);

void visit_statement(LLVMBuilderRef builder,LLVMModuleRef module, mpc_ast_t *tree);

LLVMValueRef visit_function_declaration(LLVMBuilderRef builder, LLVMModuleRef module, struct mpc_ast_t *tree);

LLVMTypeRef get_type(struct mpc_ast_t *tree);

LLVMTypeRef get_builtin_type(char *type_name);

LLVMTypeRef get_custom_type(char *type_name);

LLVMValueRef visit_number(LLVMModuleRef module, mpc_ast_t *tree);

LLVMValueRef visit_function_call(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree);

LLVMValueRef visit_expression(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree);

LLVMValueRef visit_assignment(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree);

mpc_ast_t *text_to_tree(char *code);

char *read_file(char *file_name);

LLVMModuleRef tree_to_module(mpc_ast_t *programme, char* name, int include_std_lib);

char *tree_to_file(mpc_ast_t *programme, FILE *file);

char *module_to_file(LLVMModuleRef module, FILE *file)