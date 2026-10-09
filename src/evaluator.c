/*
 * evaluator.c
 * -----------
 * Traverses the AST and executes the parsed MiniLang program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "evaluator.h"

// Pointer to global environment for function call scoping
static Environment* g_global_env = NULL;

Environment* env_create(Environment* parent) {
    Environment* env = (Environment*)malloc(sizeof(Environment));
    if (!env) {
        fprintf(stderr, "Error: out of memory allocating Environment\n");
        exit(1);
    }
    env->var_count = 0;
    env->parent = parent;
    return env;
}

void env_free(Environment* env) {
    if (env) {
        free(env);
    }
}

int env_set(Environment* env, const char* name, double value) {
    // Check if variable already declared in CURRENT scope
    for (int i = 0; i < env->var_count; i++) {
        if (strcmp(env->vars[i].name, name) == 0) {
            env->vars[i].value = value;
            return 1;
        }
    }

    if (env->var_count >= 256) {
        fprintf(stderr, "Runtime Error: variable limit exceeded in scope\n");
        return 0;
    }

    strncpy(env->vars[env->var_count].name, name, sizeof(env->vars[env->var_count].name) - 1);
    env->vars[env->var_count].name[sizeof(env->vars[env->var_count].name) - 1] = '\0';
    env->vars[env->var_count].value = value;
    env->var_count++;
    return 1;
}

int env_get(Environment* env, const char* name, double* out_value) {
    for (Environment* curr = env; curr != NULL; curr = curr->parent) {
        for (int i = 0; i < curr->var_count; i++) {
            if (strcmp(curr->vars[i].name, name) == 0) {
                *out_value = curr->vars[i].value;
                return 1;
            }
        }
    }
    return 0; // Not found
}

int env_assign(Environment* env, const char* name, double value) {
    for (Environment* curr = env; curr != NULL; curr = curr->parent) {
        for (int i = 0; i < curr->var_count; i++) {
            if (strcmp(curr->vars[i].name, name) == 0) {
                curr->vars[i].value = value;
                return 1;
            }
        }
    }
    return 0; // Undefined variable
}

void function_table_init(FunctionTable* ft) {
    ft->function_count = 0;
}

int function_table_add(FunctionTable* ft, const char* name, ASTNode* decl) {
    for (int i = 0; i < ft->function_count; i++) {
        if (strcmp(ft->functions[i].name, name) == 0) {
            ft->functions[i].decl = decl; // update / overwrite
            return 1;
        }
    }
    if (ft->function_count >= 128) {
        fprintf(stderr, "Runtime Error: function limit exceeded\n");
        return 0;
    }
    strncpy(ft->functions[ft->function_count].name, name, sizeof(ft->functions[ft->function_count].name) - 1);
    ft->functions[ft->function_count].name[sizeof(ft->functions[ft->function_count].name) - 1] = '\0';
    ft->functions[ft->function_count].decl = decl;
    ft->function_count++;
    return 1;
}

ASTNode* function_table_get(FunctionTable* ft, const char* name) {
    for (int i = 0; i < ft->function_count; i++) {
        if (strcmp(ft->functions[i].name, name) == 0) {
            return ft->functions[i].decl;
        }
    }
    return NULL;
}

static void print_number(double val) {
    if ((long long)val == val) {
        printf("%lld\n", (long long)val);
    } else {
        printf("%g\n", val);
    }
}

EvalResult eval_node(ASTNode* node, Environment* env, FunctionTable* ft) {
    EvalResult result = { 0, 0.0 };
    if (!node) return result;

    switch (node->type) {
        case AST_NUMBER: {
            result.value = node->data.number.value;
            return result;
        }

        case AST_VAR: {
            double val = 0.0;
            if (!env_get(env, node->data.var.name, &val)) {
                fprintf(stderr, "Runtime Error [Line %d]: undefined variable '%s'\n",
                        node->line, node->data.var.name);
            }
            result.value = val;
            return result;
        }

        case AST_BINARY_OP: {
            EvalResult left_res = eval_node(node->data.binary_op.left, env, ft);
            EvalResult right_res = eval_node(node->data.binary_op.right, env, ft);

            switch (node->data.binary_op.op) {
                case OP_ADD:
                    result.value = left_res.value + right_res.value;
                    break;
                case OP_SUB:
                    result.value = left_res.value - right_res.value;
                    break;
                case OP_MUL:
                    result.value = left_res.value * right_res.value;
                    break;
                case OP_DIV:
                    if (right_res.value == 0.0) {
                        fprintf(stderr, "Runtime Error [Line %d]: division by zero\n", node->line);
                        result.value = 0.0;
                    } else {
                        result.value = left_res.value / right_res.value;
                    }
                    break;
                case OP_EQ:
                    result.value = (left_res.value == right_res.value) ? 1.0 : 0.0;
                    break;
                case OP_NEQ:
                    result.value = (left_res.value != right_res.value) ? 1.0 : 0.0;
                    break;
                case OP_LT:
                    result.value = (left_res.value < right_res.value) ? 1.0 : 0.0;
                    break;
                case OP_LTE:
                    result.value = (left_res.value <= right_res.value) ? 1.0 : 0.0;
                    break;
                case OP_GT:
                    result.value = (left_res.value > right_res.value) ? 1.0 : 0.0;
                    break;
                case OP_GTE:
                    result.value = (left_res.value >= right_res.value) ? 1.0 : 0.0;
                    break;
            }
            return result;
        }

        case AST_PRINT: {
            EvalResult expr_res = eval_node(node->data.print_stmt.expr, env, ft);
            print_number(expr_res.value);
            return result;
        }

        case AST_VAR_DECL: {
            double val = 0.0;
            if (node->data.var_decl.init_expr) {
                EvalResult expr_res = eval_node(node->data.var_decl.init_expr, env, ft);
                val = expr_res.value;
            }
            env_set(env, node->data.var_decl.name, val);
            return result;
        }

        case AST_ASSIGN: {
            EvalResult expr_res = eval_node(node->data.assign.expr, env, ft);
            if (!env_assign(env, node->data.assign.name, expr_res.value)) {
                fprintf(stderr, "Runtime Error [Line %d]: undefined variable '%s'\n",
                        node->line, node->data.assign.name);
            }
            result.value = expr_res.value;
            return result;
        }

        case AST_BLOCK:
        case AST_PROGRAM: {
            for (int i = 0; i < node->data.block.count; i++) {
                result = eval_node(node->data.block.statements[i], env, ft);
                if (result.is_return) {
                    return result; // Propagate return out of block
                }
            }
            return result;
        }

        case AST_IF: {
            EvalResult cond_res = eval_node(node->data.if_stmt.condition, env, ft);
            if (cond_res.value != 0.0) {
                return eval_node(node->data.if_stmt.then_branch, env, ft);
            } else if (node->data.if_stmt.else_branch) {
                return eval_node(node->data.if_stmt.else_branch, env, ft);
            }
            return result;
        }

        case AST_WHILE: {
            while (1) {
                EvalResult cond_res = eval_node(node->data.while_stmt.condition, env, ft);
                if (cond_res.value == 0.0) {
                    break;
                }
                result = eval_node(node->data.while_stmt.body, env, ft);
                if (result.is_return) {
                    return result;
                }
            }
            result.is_return = 0;
            return result;
        }

        case AST_FUNC_DECL: {
            function_table_add(ft, node->data.func_decl.name, node);
            return result;
        }

        case AST_CALL: {
            ASTNode* func_decl = function_table_get(ft, node->data.call.name);
            if (!func_decl) {
                fprintf(stderr, "Runtime Error [Line %d]: undefined function '%s'\n",
                        node->line, node->data.call.name);
                return result;
            }

            int expected = func_decl->data.func_decl.param_count;
            int actual = node->data.call.arg_count;
            if (expected != actual) {
                fprintf(stderr, "Runtime Error [Line %d]: function '%s' expects %d arguments, but got %d\n",
                        node->line, node->data.call.name, expected, actual);
                return result;
            }

            // Evaluate argument expressions in current environment
            double* arg_vals = NULL;
            if (actual > 0) {
                arg_vals = (double*)malloc(sizeof(double) * actual);
                for (int i = 0; i < actual; i++) {
                    EvalResult arg_res = eval_node(node->data.call.args[i], env, ft);
                    arg_vals[i] = arg_res.value;
                }
            }

            // Create new activation frame (scope) for function execution
            Environment* local_env = env_create(g_global_env ? g_global_env : env);
            for (int i = 0; i < actual; i++) {
                env_set(local_env, func_decl->data.func_decl.params[i], arg_vals[i]);
            }

            // Execute function body
            EvalResult body_res = eval_node(func_decl->data.func_decl.body, local_env, ft);

            if (arg_vals) free(arg_vals);
            env_free(local_env);

            // Function call expression yields the returned value
            result.is_return = 0;
            result.value = body_res.value;
            return result;
        }

        case AST_RETURN: {
            double ret_val = 0.0;
            if (node->data.return_stmt.expr) {
                EvalResult expr_res = eval_node(node->data.return_stmt.expr, env, ft);
                ret_val = expr_res.value;
            }
            result.is_return = 1;
            result.value = ret_val;
            return result;
        }
    }

    return result;
}

void evaluate(ASTNode* root) {
    if (!root) return;

    Environment* global_env = env_create(NULL);
    g_global_env = global_env;

    FunctionTable ft;
    function_table_init(&ft);

    eval_node(root, global_env, &ft);

    env_free(global_env);
    g_global_env = NULL;
}