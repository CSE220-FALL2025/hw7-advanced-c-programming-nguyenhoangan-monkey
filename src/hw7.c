#include "hw7.h"

bst_sf* insert_bst_sf(matrix_sf *mat, bst_sf *root) {
    if (root == NULL) {
        bst_sf *new_node = malloc(sizeof(bst_sf));
        if (new_node == NULL) return NULL; // check malloc

        new_node->mat = mat; 
        new_node->left_child = NULL;
        new_node->right_child = NULL;
        return new_node;
    }
    else {
        if (mat->name < root->mat->name)
            root->left_child = insert_bst_sf(mat, root->left_child);
        else
            root->right_child = insert_bst_sf(mat, root->right_child);

        return root;
    }
}

matrix_sf* find_bst_sf(char name, bst_sf *root) {
    if (root == NULL) return NULL;

    if (name == root->mat->name)      return root->mat;
    else if (name < root->mat->name)  return find_bst_sf(name, root->left_child);
    else if (name > root->mat->name)  return find_bst_sf(name, root->right_child);
}

void free_bst_sf(bst_sf *root) {
    // no double free
    if (root == NULL)
        return;

    // recursively free children
    free_bst_sf(root->left_child);
    free_bst_sf(root->right_child);

    free(root->mat);
    free(root);
}

matrix_sf* add_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    // check requirements
    if (mat1->num_cols != mat2->num_cols)
        return NULL;
    if (mat1->num_rows != mat2->num_rows)
        return NULL;
     
    // allocate a sum matrix
    unsigned int num_elements = mat1->num_cols * mat1->num_rows;
    matrix_sf *sum = malloc(sizeof(matrix_sf) + (size_t)num_elements * sizeof(int));
    sum->name = 'S';
    sum->num_rows = mat1->num_rows;
    sum->num_cols = mat1->num_cols;

    // add element-wise
    for (unsigned int i = 0; i < num_elements; i++) {
        sum->values[i] = mat1->values[i] + mat2->values[i];
    }

    return sum;
}

matrix_sf* mult_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    // check requirements
    if (mat1->num_cols != mat2->num_rows)
        return NULL;

    // allocate a result matrix
    unsigned int num_elements = mat1->num_rows * mat2->num_cols;
    matrix_sf *result = malloc(sizeof(matrix_sf) + (size_t)num_elements * sizeof(int));
    result->name = 'R';
    result->num_rows = mat1->num_rows;
    result->num_cols = mat2->num_cols;

    // matrix multiplication
    unsigned int shared_dimension = mat1->num_cols;
    for (unsigned int i = 0; i < result->num_rows; i++) {
        for (unsigned int j = 0; j < result->num_cols; j++) {
            int sum = 0;
            for (unsigned int k = 0; k < shared_dimension; k++) {
                int index_1 = (i * mat1->num_cols) + k;
                int index_2 = (k * mat2->num_cols) + j;
                sum += mat1->values[index_1] * mat2->values[index_2];
            }
            int index_result = (i * result->num_cols) + j;
            result->values[index_result] = sum;
        }
    }

    return result;
}

matrix_sf* transpose_mat_sf(const matrix_sf *mat) {
    // allocate result matrix
    unsigned int num_elements = mat->num_rows * mat->num_cols;
    matrix_sf *result = malloc(sizeof(matrix_sf) + (size_t)num_elements * sizeof(int));
    result->name = 'R';
    result->num_rows = mat->num_cols; // transpose dimension
    result->num_cols = mat->num_rows; // transpose dimension

    // transposing an element in (row, column)
    // the index for source matrix is:
    //      index = row * M->num_cols + column
    //      row = index / M->num_cols
    //      column = index % M->num_cols
    //
    // the index of destination matrix is:
    //      index = column * M->num_rows + row
    // here it is M->num_rows because it is transposed.
    //
    // which means
    //      index = (index % M->num_cols) * M->num_rows + (index / M->num_cols)
    for (unsigned int index = 0; index < num_elements; index++) {
        unsigned int new_column = index % mat->num_cols;
        unsigned int new_row = index / mat->num_cols;
        unsigned int new_index = new_column * mat->num_rows + new_row;

        result->values[new_index] = mat->values[index];
    }

    return result;
}

matrix_sf* create_matrix_sf(char name, const char *expr) {
    // parsing the first two integers and the rest of the string
    int NR, NC;
    const unsigned int REST_LEN = 4096;
    char rest[REST_LEN];
    if (sscanf(expr, " %d %d %[^\n]", &NR, &NC, rest) != 3)
        return NULL;

    // allocate matrix
    matrix_sf *M = malloc(sizeof(matrix_sf) + (NR*NC) * sizeof(int));
    if (M == NULL)
        return NULL;

    M->name = name;
    M->num_rows = (unsigned int)NR;
    M->num_cols = (unsigned int)NC;
    
    // replace invalid characters with space so strtol() can work
    // valid characters are 0-9, spaces and minus sign
    for (unsigned int i = 0; i < (unsigned int)REST_LEN; i++) {
        if (!isdigit(rest[i]) && !isspace(rest[i]) && !rest[i] == '-')
            rest[i] = ' ';
    }

    // scanning for integers up to capacity
    // this is a very fragile code but because the prompt said that
    // the input is always correct, we can trade off a lot of error
    // handling for this compact code
    char *rest_ptr = rest;
    char *endptr;
    for (unsigned int i = 0; i < NR*NC; i++) {
        long val = strtol(rest_ptr, &endptr, 10);
        if (rest_ptr == endptr) {
            free(M);
            return NULL;  // avoid infinite loop
        }

        M->values[i] = (int)val;
        rest_ptr = endptr;
    }

    return M;
}

int precedence(char op) {
    if (op == '\'') return 3;
    if (op == '*') return 2;
    if (op == '+') return 1;
    return 0;
}

char* infix2postfix_sf(char *infix) {
    int string_len = strlen(infix);
    char *postfix = malloc(string_len + 1);
    int k = 0;

    char *stack = malloc(string_len);
    int top = -1;

    for (int i = 0; i < string_len; i++) {
        char token = infix[i];

        if (isspace(token))
            continue;
        else if (isalnum(token)) {
            postfix[k] = token;
            k++;
        }
        else if (token == '(') {
            top++;
            stack[top] = token;
        }
        else if (token == ')') {
            // popping the stack
            while (top >= 0 && stack[top] != '(') {
                postfix[k] = stack[top];
                k++;
                top--;
            }
            if (top >= 0 && stack[top] == '(')
                top--;
        }
        // for operators, add to stack
        else if (token == '+' || token == '*' || token == '\'') {
            while (top >= 0 && stack[top] != '(' && precedence(stack[top]) >= precedence(token)) {
                postfix[k] = stack[top];
                k++;
                top--;
            }
            top++;
            stack[top] = token;
        }
    }

    // pop the remaining operators
    while (top >= 0) {
        postfix[k] = stack[top];
        k++;
        top--;
    }

    postfix[k] = '\0';
    free(stack);
    return postfix;
}

matrix_sf* evaluate_expr_sf(char name, char *expr, bst_sf *root) {
    char *postfix = infix2postfix_sf(expr);

    matrix_sf** stack = malloc(strlen(postfix) * sizeof(matrix_sf*));
    int top = -1;

    int len = strlen(postfix);
    for (int i = 0; i < len; i++) {
        char token = postfix[i];

        if (isalnum(token)) {
            matrix_sf *find_matrix = find_bst_sf(token, root);
            if (!find_matrix)
                continue;
            top++;
            stack[top] = find_matrix;
        }
        else if (token == '+') {
            matrix_sf *new_matrix = add_mats_sf(stack[top-1], stack[top]);

            // freeing the temporary matrix
            if (!isalpha(stack[top-1]->name))
                free(stack[top-1]);
            if (!isalpha(stack[top]->name))
                free(stack[top]);

            top--; // two matrices popped, one matrix pushed
            new_matrix->name = '?';
            stack[top] = new_matrix;
        }
        else if (token == '*') {
            matrix_sf *new_matrix = mult_mats_sf(stack[top-1], stack[top]);

            // freeing the temporary matrix
            if (!isalpha(stack[top-1]->name))
                free(stack[top-1]);
            if (!isalpha(stack[top]->name))
                free(stack[top]);

            top--; // two matrices popped, one matrix pushed
            new_matrix->name = '?';
            stack[top] = new_matrix;
        }
        else if (token == '\'') {
            matrix_sf *new_matrix = transpose_mat_sf(stack[top]);
            new_matrix->name = '?';
            stack[top] = new_matrix;
        }
    }

    // return the matrix and free malloc memory
    matrix_sf *result_matrix = stack[top];
    result_matrix->name = name;
    free(postfix);
    free(stack);
    return result_matrix;
}

matrix_sf *execute_script_sf(char *filename) {
    // variables to help with reading file 
    char *str = NULL;
    FILE *file = fopen(filename, "r");
    size_t max_line_size = MAX_LINE_LEN;
    char str_buffer[MAX_LINE_LEN];

    matrix_sf *new_matrix = NULL; // last matrix to be assigned/calculated
    bst_sf *root = NULL; // new binary search tree

    // read file per line
    while (getline(&str, &max_line_size, file) != -1) {
        char matrix_name;
        if (sscanf(str, " %c = %[^\n] ", &matrix_name, str_buffer) != 2)
            continue; // should not happen with blank lines
        
        // check whether it is an expression or a matrix definition
        // an expression contains letters, matrix definitions don't
        // matrix definitions contain numbers, expressions don't
        int is_a_matrix = 0;
        int is_an_expression = 0;
        for (int i = 0; str_buffer[i] != '\0'; i++) {
            char check = (unsigned char)str_buffer[i];
            if (isalpha(check)) is_an_expression = 1;
            if (isdigit(check)) is_a_matrix = 1;
        }
        
        if (is_a_matrix && !is_an_expression) {
            new_matrix = create_matrix_sf(matrix_name, str_buffer);
            if (new_matrix == NULL) continue;
            new_matrix->name = matrix_name;
            
            matrix_sf *find_matrix = find_bst_sf(matrix_name, root);
            if (find_matrix == NULL) root = insert_bst_sf(new_matrix, root);
        }
        else if (!is_a_matrix && is_an_expression) {
            new_matrix = evaluate_expr_sf(matrix_name, str_buffer, root);
            if (new_matrix == NULL) continue;
            new_matrix->name = matrix_name;

            matrix_sf *find_matrix = find_bst_sf(matrix_name, root);
            if (find_matrix == NULL) root = insert_bst_sf(new_matrix, root);
        }
        else {
            continue;
        }
    }

    // copy the matrix to a different allocation
    matrix_sf *result_matrix = NULL;
    if (new_matrix != NULL) {
        result_matrix = copy_matrix(new_matrix->num_rows, new_matrix->num_cols, new_matrix->values);
        result_matrix->name = new_matrix->name;
    }

    fclose(file);
    free(str);
    free_bst_sf(root);
    return result_matrix;
}

// This is a utility function used during testing. Feel free to adapt the code to implement some of
// the assignment. Feel equally free to ignore it.
matrix_sf *copy_matrix(unsigned int num_rows, unsigned int num_cols, int values[]) {
    matrix_sf *m = malloc(sizeof(matrix_sf)+num_rows*num_cols*sizeof(int));
    m->name = '?';
    m->num_rows = num_rows;
    m->num_cols = num_cols;
    memcpy(m->values, values, num_rows*num_cols*sizeof(int));
    return m;
}

// Don't touch this function. It's used by the testing framework.
// It's been left here in case it helps you debug and test your code.
void print_matrix_sf(matrix_sf *mat) {
    assert(mat != NULL);
    assert(mat->num_rows <= 1000);
    assert(mat->num_cols <= 1000);
    printf("%d %d ", mat->num_rows, mat->num_cols);
    for (unsigned int i = 0; i < mat->num_rows*mat->num_cols; i++) {
        printf("%d", mat->values[i]);
        if (i < mat->num_rows*mat->num_cols-1)
            printf(" ");
    }
    printf("\n");
}
