#include "util.h"


int main(int argc, char **argv) {
    char *file_name = argv[1];
    if (!file_name) {
        fprintf(stderr, "Please enter the file name as an argument");
        exit(1);
    }
    FILE *file = fopen(file_name, "w");
    if (!file) {
        fprintf(stderr, "Could not open file for writing");
        exit(1);
    }
    char *file_content = read_file(file_name);
    mpc_ast_t *tree = text_to_tree(file_content);
    char *error = tree_to_file(tree, file);
    if (error)
        fprintf(stderr, "an error has occurred: %s", error);
    return 0;
}