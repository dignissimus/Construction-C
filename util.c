
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "util.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Linker.h>

#define STD_LIB_DIR "lib-construction/"

typedef struct {
    char *name;
    LLVMTypeRef type;
    int typed;
    unsigned int index;

} method_parameter;

typedef struct _list_node {
    void *value;
    struct _list_node *next;
} list_node;

typedef struct {
    struct _list_node *first_item;
} list;

list *create_list() {
    list *_list = malloc(sizeof(list));
    _list->first_item = NULL;
    return _list;
}

unsigned int list_len(list *_list) {
    unsigned int len = 0;
    list_node *node = _list->first_item;
    while (node) {
        len++;
        node = node->next;
    }
    return len;
}

void **list_to_array(list *_list) {
    int len = list_len(_list);
    void **array = malloc(sizeof(void *) * len);
    list_node *node = _list->first_item;
    for (int i = 0; i < len; i++) {
        array[i] = node->value;
        node = node->next;
    }
    return array;
}

list_node *list_end(list *_list) {
    list_node *node = _list->first_item;
    while (node) {
        list_node *next_node = node->next;
        if (next_node)
            node = next_node;
        else
            break;
    }
    return node;
}

void list_add(list *_list, void *value) {
    list_node *node = malloc(sizeof(list_node));
    node->value = value;
    node->next = NULL;
    list_node *end = list_end(_list);
    if (end)
        end->next = node;
    else
        _list->first_item = node;
}


mpc_ast_t *text_to_tree(char *code) {
    mpc_parser_t *UnaryOperatorPostfix = mpc_new("postunop");
    mpc_parser_t *UnaryOperatorPrefix = mpc_new("preunop");
    mpc_parser_t *BinaryOperator = mpc_new("binop");
    mpc_parser_t *BinaryOperation = mpc_new("binoperation");
    mpc_parser_t *Operation = mpc_new("operation");
    mpc_parser_t *Name = mpc_new("name");
    mpc_parser_t *Section = mpc_new("section");
    mpc_parser_t *Import = mpc_new("import");
    mpc_parser_t *KeyWordType = mpc_new("keywordtype");
    mpc_parser_t *Builtin = mpc_new("builtin");
    mpc_parser_t *CustomType = mpc_new("customtype");
    mpc_parser_t *Block = mpc_new("block"); // block?
    mpc_parser_t *Typed = mpc_new("typed");
    mpc_parser_t *Variable = mpc_new("var");
    mpc_parser_t *Assignment = mpc_new("assignment");
    mpc_parser_t *String = mpc_new("string");
    mpc_parser_t *BinaryNumber = mpc_new("binnum");
    mpc_parser_t *DecimalNumber = mpc_new("decnum");
    mpc_parser_t *HexNumber = mpc_new("hexnum");
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Expression = mpc_new("expr");
    mpc_parser_t *Call = mpc_new("call");
    mpc_parser_t *Terminator = mpc_new("terminator");
    mpc_parser_t *Statement = mpc_new("statement");
    mpc_parser_t *Function = mpc_new("function");
    mpc_parser_t *Programme = mpc_new("programme");


    mpc_err_t *error = mpca_lang(MPCA_LANG_DEFAULT, // TODO: move?
                                 "name: /[A-z._0-9]+/;"
                                         "import: \"import\" /[A-z.]*/" // TODO: change?
                                         "binop: ('+' | '-' | '/' | '*' | \"&&\" | '||');"
                                         "binaryoperation: <expr> <binop> <expr>;"
                                         "operation: (<binaryoperation>)" // TODO: add more operations
                                         "section: (\"section\" | \"sec\" | \"segment\" | \"seg\") /[A-z]+/;" // TODO: remove?
                                         "keywordtype: ( \"int\" );"
                                         "builtin: \"Type\";"
                                         "customtype: \"Struct\";"
                                         "block: '{' (<assignment>|<function>)* '}';" // TODO: remove?
                                         "typed: ':' ( ( (<type>|<builtin>) \"<\" <name> \">\") | <keywordtype>);"
                                         "var: <name><typed>?;"
                                         "assignment: <var> '=' <expr>;"
                                         "string: '\"' /[^\\r\\n\"]*/ '\"';"
                                         "binnum: \"0b\" /[01]+/;"
                                         "decnum: /[1-9][0-9]*/;"
                                         "hexnum: \"0x\" /[A-Fa-f0-9]+/;"
                                         "number: <hexnum> | <decnum> | <binnum>;"
                                         "expr: <string> | <function> | <call> | <number> | <assignment> | <var>;"
                                         "function: <name>?<typed>?('(' (<var>(','<var>)*)? ')')? '{' <statement>* '}';"
                                         "call: <name> '('  (<expr> (','<expr>)*)? ')';"
                                         "terminator:  ( /$/ | /[\\n;]+/ );"
                                         "statement: <expr> <terminator>;"
                                         "programme: /^/ ((<section> | <function> | <assignment>) <terminator>)* /$/;",
                                 Name, BinaryOperator, BinaryOperation, Operation, Section, KeyWordType, Builtin,
                                 CustomType, Block, Typed, Variable, Assignment,
                                 String, BinaryNumber, DecimalNumber, HexNumber, Number, Expression,
                                 Function, Call, Terminator, Statement, Programme, NULL);
    mpc_result_t r;
    if (mpc_parse("code", code, Programme, &r)) {
        mpc_ast_t *tree = r.output;
        return tree;
        mpc_ast_delete(r.output);
    }
    else {
        fprintf(stderr, "Unable to parse the string\n");
        /* Otherwise print and delete the Error */
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
    }
}

char *read_file(char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Could not read file '%s'\n", file_name);
        exit(1);
    }


    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc((size_t) length + 1);
    if (buffer) {
        fread(buffer, 1, (size_t) length, file);
        buffer[length] = 0;
        printf("r: %s\n", buffer);
    }
    fclose(file);
    return buffer;
}

LLVMModuleRef tree_to_module(mpc_ast_t *programme, char *module_name, int include_std_lib) {
    LLVMModuleRef module = LLVMModuleCreateWithName(module_name);

    if (include_std_lib) {
        struct dirent **files = NULL;
        int items = scandir(STD_LIB_DIR, &files, NULL, alphasort);
        while (items--) {
            char *file_name = files[items]->d_name;
            if (file_name[0] == '.') {
                continue;
            }

            char *full_path = malloc(sizeof(char) * (strlen(STD_LIB_DIR) + strlen(file_name)) + 1);

            strcpy(full_path, STD_LIB_DIR);
            strcat(full_path, file_name);

            char *contents = read_file(full_path);
            mpc_ast_t *tree = text_to_tree(contents);
            LLVMModuleRef library = tree_to_module(tree, file_name, 0);
            LLVMLinkModules2(module, library);
        }
    }

    if (!programme) {
        fprintf(stderr, "The provided ast tree was was null\n");
        exit(1);
    }

    for (int i = 0; i < programme->children_num; i++) {
        mpc_ast_t *child = programme->children[i];
        visit(module, child);
    }

    return module;
}

char *tree_to_file(mpc_ast_t *programme, FILE *file) {
    LLVMModuleRef module = tree_to_module(programme, "Programme", 1);
    return module_to_file(module, file);
}

char *module_to_file(LLVMModuleRef module, FILE *file) {
    char *ERROR = NULL;
    LLVMVerifyModule(module, LLVMPrintMessageAction, &ERROR);
    if (ERROR) {
        printf("%s", ERROR);
        LLVMDisposeMessage(ERROR);
    }

    if (LLVMWriteBitcodeToFileHandle(module, fileno(file)) != 0) {
        fprintf(stderr, "error writing bitcode to file, skipping\n");
    }
    fflush(file);
    fclose(file);

    return ERROR;
}

LLVMValueRef visit(LLVMModuleRef module, mpc_ast_t *tree) {
    if (strstr(tree->tag, "function")) {
        return visit_function_declaration(NULL, module, tree);
    }
}

LLVMValueRef visit_number(LLVMModuleRef module, mpc_ast_t *tree) {
    int base = 0;
    if (strstr(tree->tag, "decnum"))
        base = 10;
    if (strstr(tree->tag, "hexnum"))
        base = 16;
    if (strstr(tree->tag, "binnum"))
        base = 2;
    if (base)
        return LLVMConstInt(LLVMInt32Type(), strtoull(tree->contents, NULL, base), 0); // TODO: NULL
    return NULL;


}

LLVMValueRef visit_function_call(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree) {
    char *function_name = tree->children[0]->contents;
    list *arg_list = create_list();
    int place = 2; // skip '('
    while (1) {
        if (strcmp(tree->children[place]->contents, ")") == 0)
            break;
        list_add(arg_list, visit_expression(builder, module, tree->children[place]));
        place++;
    }
    return LLVMBuildCall(builder, LLVMGetNamedFunction(module, function_name),
                         (LLVMValueRef *) list_to_array(arg_list), list_len(arg_list), "return");
}

LLVMValueRef visit_expression(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree) {
    if (strstr(tree->tag, "number"))
        return visit_number(module, tree);
    if (strstr(tree->tag, "call"))
        return visit_function_call(builder, module, tree);
    if (strstr(tree->tag, "function"))
        return visit_function_declaration(builder, module, tree);
    if (strstr(tree->tag, "assignment"))
        return visit_assignment(builder, module, tree);
    if (strstr(tree->tag, "operation"))
        return visit_operation(builder, module, tree);


}

LLVMValueRef visit_operation(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree) {
    if (strstr(tree->tag, "binaryoperation"))
        return visit_binary_operation(builder, module, tree);
}

LLVMValueRef visit_binary_operation(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree) {
    mpc_ast_t *left = tree->children[0];
    mpc_ast_t *right = tree->children[2];
    char *operator = tree->children[1]->contents;
    if (strcmp(operator, "+") == 0) {
        // TODO
    }
}

LLVMValueRef visit_assignment(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree) { // TODO: typed variables
    LLVMValueRef value = visit_expression(builder, module, tree->children[2]);
    char *name = tree->children[0]->contents;
    LLVMSetValueName(value, name);
    return value;

}

void visit_statement(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree) {
    mpc_ast_t *expression = tree->children[0];
    visit_expression(builder, module, expression);
}

LLVMValueRef visit_function_declaration(LLVMBuilderRef builder, LLVMModuleRef module, mpc_ast_t *tree) {
    char *function_name = tree->children[0]->contents;
    list *parameter_list = create_list();
    list *type_list = create_list();
    LLVMTypeRef return_type = LLVMVoidType();
    int offset = 1; // skip the '{'
    if (!strstr(tree->children[0]->tag, "name")) {
        function_name = "function";
        offset = 0;
    }
    if (strstr(tree->children[offset]->tag, "typed")) {
        return_type = get_type(tree->children[offset]);
        offset++; // skip this
    }

    if (strcmp(tree->children[offset]->contents, "(") == 0) { // TODO: checking for parameters
        unsigned int parameter_num = 0;
        offset++; // skip the '('
        for (int i = 0; i < tree->children_num; i++) {
            mpc_ast_t *child = tree->children[offset + i];
            if (strcmp(child->contents, ")") == 0) {
                offset += i;
                break;
            }
            if (strstr(child->tag, "var")) {
                mpc_ast_print(child);
                int typed = child->children_num > 1;
                LLVMTypeRef type = LLVMPointerType(LLVMInt8Type(), 0);
                char *parameter_name = NULL;
                if (typed) {
                    mpc_ast_t *type_tree = child->children[1];
                    type = get_type(type_tree);
                    parameter_name = child->children[0]->contents;
                }
                else {
                    parameter_name = child->contents;
                }
                method_parameter *parameter = malloc(sizeof(method_parameter));
                parameter->name = parameter_name;
                parameter->type = type;
                parameter->typed = typed;
                parameter->index = parameter_num++;
                list_add(parameter_list, parameter);
                list_add(type_list, type);
            }
        }
    }
    else {
        // No arguments
    }
    LLVMTypeRef function_type = LLVMFunctionType(return_type, (LLVMTypeRef *) list_to_array(type_list),
                                                 list_len(type_list), 0);

    LLVMValueRef function = LLVMAddFunction(module, function_name, function_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
    LLVMBuilderRef function_builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(function_builder, entry);


    method_parameter **parameter_array = (method_parameter **) list_to_array(parameter_list);
    for (int i = 0; i < list_len(parameter_list); i++) {
        method_parameter *parameter = parameter_array[i];
        LLVMSetValueName(LLVMGetParam(function, parameter->index), parameter->name);
    }

    for (int i = offset; i < tree->children_num; i++) {
        mpc_ast_t *statement = tree->children[i];
        if (strstr(statement->tag, "statement")) {
            visit_statement(function_builder, module, statement);
        }
    }

    LLVMBuildRetVoid(function_builder);
    return function;
}

LLVMTypeRef get_type(struct mpc_ast_t *tree) {
    char *type_type_tag = tree->children[1]->tag;
    if (strstr(type_type_tag, "builtin")) {
        char *type_name = tree->children[1]->contents;
        return get_builtin_type(type_name);
    }
    else if (strstr(type_type_tag, "struct")) {
        char *type_name = tree->children[1]->contents;
        return get_custom_type(type_name);
    }
    else if (strstr(type_type_tag, "keyword")) {
        char *type_name = tree->children[1]->contents;
        return get_builtin_type(type_name);
    }
}

LLVMTypeRef get_custom_type(char *type_name) {
    // TODO: implement
}

LLVMTypeRef get_builtin_type(char *type_name) {
    if (strcmp(type_name, "int") == 0) {
        return LLVMInt32Type();
    }
}