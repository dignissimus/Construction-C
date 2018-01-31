#include "util.h"


int main(int argc, char **argv) { // TODO: better argument parsing
    char *source_file = argv[1];
    char *output_file_name = argv[2] ? argv[2] : "out.bc";
    if (!source_file) {
        fprintf(stderr, "Please enter the source file name as an argument");
        exit(1);
    }
    FILE *output_file = fopen(output_file_name, "w+"); // Overwrite file but create when necessary
    if (!output_file) {
        fprintf(stderr, "Could not open file '%s' for reading", output_file_name);
        exit(1);
    }
    char *file_content = read_file(source_file);
    mpc_ast_t *tree = text_to_tree(file_content);
    char *error = tree_to_file(tree, output_file);
    if (error)
        fprintf(stderr, "an error has occurred: %s", error);
    return 0;
}