/*
 * evaluator.c
 * -----------
 * Traverses the AST and executes the parsed MiniLang program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "evaluator.h"

// Pointer to global environment for function call scoping
static Environment* g_global_env = NULL;

Value value_make_number(double num) {
    Value val;
    val.type = VAL_NUMBER;
    val.as.number = num;
    return val;
}

Value value_make_string(const char* str) {
    Value val;
    val.type = VAL_STRING;
    val.as.string = strdup(str ? str : "");
    return val;
}

Value value_copy(Value val) {
    if (val.type == VAL_STRING) {
        return value_make_string(val.as.string);
    }
    return val;
}

void value_free(Value val) {
    if (val.type == VAL_STRING && val.as.string) {
        free(val.as.string);
    }
}

int value_is_truthy(Value val) {
    if (val.type == VAL_NUMBER) {
        return val.as.number != 0.0;
    }
    if (val.type == VAL_STRING) {
        return (val.as.string && val.as.string[0] != '\0');
    }
    return 0;
}

const char* value_type_name(ValueType type) {
    switch (type) {
        case VAL_NUMBER: return "number";
        case VAL_STRING: return "string";
        default: return "unknown";
    }
}

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
        for (int i = 0; i < env->var_count; i++) {
            value_free(env->vars[i].value);
        }
        free(env);
    }
}

int env_set(Environment* env, const char* name, Value value) {
    // Check if variable already declared in CURRENT scope
    for (int i = 0; i < env->var_count; i++) {
        if (strcmp(env->vars[i].name, name) == 0) {
            value_free(env->vars[i].value);
            env->vars[i].value = value_copy(value);
            return 1;
        }
    }

    if (env->var_count >= 256) {
        fprintf(stderr, "Runtime Error: variable limit exceeded in scope\n");
        return 0;
    }

    strncpy(env->vars[env->var_count].name, name, sizeof(env->vars[env->var_count].name) - 1);
    env->vars[env->var_count].name[sizeof(env->vars[env->var_count].name) - 1] = '\0';
    env->vars[env->var_count].value = value_copy(value);
    env->var_count++;
    return 1;
}

int env_get(Environment* env, const char* name, Value* out_value) {
    for (Environment* curr = env; curr != NULL; curr = curr->parent) {
        for (int i = 0; i < curr->var_count; i++) {
            if (strcmp(curr->vars[i].name, name) == 0) {
                *out_value = value_copy(curr->vars[i].value);
                return 1;
            }
        }
    }
    return 0; // Not found
}

int env_assign(Environment* env, const char* name, Value value) {
    for (Environment* curr = env; curr != NULL; curr = curr->parent) {
        for (int i = 0; i < curr->var_count; i++) {
            if (strcmp(curr->vars[i].name, name) == 0) {
                value_free(curr->vars[i].value);
                curr->vars[i].value = value_copy(value);
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

static void print_value(Value val) {
    if (val.type == VAL_STRING) {
        printf("%s\n", val.as.string ? val.as.string : "");
    } else {
        double d = val.as.number;
        if ((long long)d == d) {
            printf("%lld\n", (long long)d);
        } else {
            printf("%g\n", d);
        }
    }
}

EvalResult eval_node(ASTNode* node, Environment* env, FunctionTable* ft) {
    EvalResult result;
    result.is_return = 0;
    result.is_break = 0;
    result.is_continue = 0;
    result.value = value_make_number(0.0);

    if (!node) return result;

    switch (node->type) {
        case AST_NUMBER: {
            result.value = value_make_number(node->data.number.value);
            return result;
        }

        case AST_STRING: {
            result.value = value_make_string(node->data.string.value);
            return result;
        }

        case AST_VAR: {
            Value val = value_make_number(0.0);
            if (!env_get(env, node->data.var.name, &val)) {
                fprintf(stderr, "Runtime Error [Line %d]: undefined variable '%s'\n",
                        node->line, node->data.var.name);
            }
            result.value = val;
            return result;
        }

        case AST_UNARY_OP: {
            EvalResult op_res = eval_node(node->data.unary_op.operand, env, ft);
            switch (node->data.unary_op.op) {
                case UNARY_NEGATE:
                    if (op_res.value.type != VAL_NUMBER) {
                        fprintf(stderr, "Type Error [Line %d]: unary '-' requires number, but got %s\n",
                                node->line, value_type_name(op_res.value.type));
                        value_free(op_res.value);
                        result.value = value_make_number(0.0);
                    } else {
                        result.value = value_make_number(-op_res.value.as.number);
                    }
                    break;
                case UNARY_NOT:
                    // Logical NOT: !truthy -> 0, !falsy -> 1
                    result.value = value_make_number(value_is_truthy(op_res.value) ? 0.0 : 1.0);
                    value_free(op_res.value);
                    break;
            }
            return result;
        }

        case AST_BINARY_OP: {
            EvalResult left_res = eval_node(node->data.binary_op.left, env, ft);
            EvalResult right_res = eval_node(node->data.binary_op.right, env, ft);

            BinaryOpType op = node->data.binary_op.op;

            // Handle string operations first (concatenation with + and string comparisons)
            if (left_res.value.type == VAL_STRING || right_res.value.type == VAL_STRING) {
                if (op == OP_ADD) {
                    if (left_res.value.type == VAL_STRING && right_res.value.type == VAL_STRING) {
                        const char* s1 = left_res.value.as.string ? left_res.value.as.string : "";
                        const char* s2 = right_res.value.as.string ? right_res.value.as.string : "";
                        int total_len = strlen(s1) + strlen(s2) + 1;
                        char* combined = (char*)malloc(total_len);
                        strcpy(combined, s1);
                        strcat(combined, s2);
                        result.value = value_make_string(combined);
                        free(combined);
                    } else {
                        fprintf(stderr, "Type Error [Line %d]: type mismatch in '+': cannot add %s and %s\n",
                                node->line,
                                value_type_name(left_res.value.type),
                                value_type_name(right_res.value.type));
                        result.value = value_make_number(0.0);
                    }
                } else if (op == OP_EQ) {
                    if (left_res.value.type == VAL_STRING && right_res.value.type == VAL_STRING) {
                        int eq = (strcmp(left_res.value.as.string, right_res.value.as.string) == 0);
                        result.value = value_make_number(eq ? 1.0 : 0.0);
                    } else {
                        result.value = value_make_number(0.0);
                    }
                } else if (op == OP_NEQ) {
                    if (left_res.value.type == VAL_STRING && right_res.value.type == VAL_STRING) {
                        int neq = (strcmp(left_res.value.as.string, right_res.value.as.string) != 0);
                        result.value = value_make_number(neq ? 1.0 : 0.0);
                    } else {
                        result.value = value_make_number(1.0);
                    }
                } else {
                    fprintf(stderr, "Type Error [Line %d]: operator '%s' is not supported for string types\n",
                            node->line, binary_op_to_string(op));
                    result.value = value_make_number(0.0);
                }

                value_free(left_res.value);
                value_free(right_res.value);
                return result;
            }

            // Both operands are numbers
            double l_num = left_res.value.as.number;
            double r_num = right_res.value.as.number;

            switch (op) {
                case OP_ADD:
                    result.value = value_make_number(l_num + r_num);
                    break;
                case OP_SUB:
                    result.value = value_make_number(l_num - r_num);
                    break;
                case OP_MUL:
                    result.value = value_make_number(l_num * r_num);
                    break;
                case OP_DIV:
                    if (r_num == 0.0) {
                        fprintf(stderr, "Runtime Error [Line %d]: division by zero\n", node->line);
                        result.value = value_make_number(0.0);
                    } else {
                        result.value = value_make_number(l_num / r_num);
                    }
                    break;
                case OP_MOD:
                    if (r_num == 0.0) {
                        fprintf(stderr, "Runtime Error [Line %d]: modulo by zero\n", node->line);
                        result.value = value_make_number(0.0);
                    } else {
                        result.value = value_make_number(fmod(l_num, r_num));
                    }
                    break;
                case OP_EQ:
                    result.value = value_make_number((l_num == r_num) ? 1.0 : 0.0);
                    break;
                case OP_NEQ:
                    result.value = value_make_number((l_num != r_num) ? 1.0 : 0.0);
                    break;
                case OP_LT:
                    result.value = value_make_number((l_num < r_num) ? 1.0 : 0.0);
                    break;
                case OP_LTE:
                    result.value = value_make_number((l_num <= r_num) ? 1.0 : 0.0);
                    break;
                case OP_GT:
                    result.value = value_make_number((l_num > r_num) ? 1.0 : 0.0);
                    break;
                case OP_GTE:
                    result.value = value_make_number((l_num >= r_num) ? 1.0 : 0.0);
                    break;
                case OP_AND:
                    result.value = value_make_number((double)((long long)l_num & (long long)r_num));
                    break;
                case OP_OR:
                    result.value = value_make_number((double)((long long)l_num | (long long)r_num));
                    break;
                case OP_XOR:
                    result.value = value_make_number((double)((long long)l_num ^ (long long)r_num));
                    break;
                case OP_NOR: {
                    long long l = (long long)l_num;
                    long long r = (long long)r_num;
                    result.value = value_make_number((!l && !r) ? 1.0 : 0.0);
                    break;
                }
                case OP_NAND: {
                    long long l = (long long)l_num;
                    long long r = (long long)r_num;
                    result.value = value_make_number(!(l && r) ? 1.0 : 0.0);
                    break;
                }
            }

            value_free(left_res.value);
            value_free(right_res.value);
            return result;
        }

        case AST_PRINT: {
            EvalResult expr_res = eval_node(node->data.print_stmt.expr, env, ft);
            print_value(expr_res.value);
            value_free(expr_res.value);
            return result;
        }

        case AST_VAR_DECL: {
            Value val = value_make_number(0.0);
            if (node->data.var_decl.init_expr) {
                EvalResult expr_res = eval_node(node->data.var_decl.init_expr, env, ft);
                val = expr_res.value;
            }
            env_set(env, node->data.var_decl.name, val);
            value_free(val);
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

        case AST_INC_DEC: {
            Value current_val = value_make_number(0.0);
            if (!env_get(env, node->data.inc_dec.name, &current_val)) {
                fprintf(stderr, "Runtime Error [Line %d]: undefined variable '%s'\n",
                        node->line, node->data.inc_dec.name);
            }

            if (current_val.type != VAL_NUMBER) {
                fprintf(stderr, "Type Error [Line %d]: increment/decrement requires number, got %s\n",
                        node->line, value_type_name(current_val.type));
                value_free(current_val);
                result.value = value_make_number(0.0);
                return result;
            }

            double num = current_val.as.number;
            double new_num = num;
            double return_num = num;

            switch (node->data.inc_dec.op) {
                case OP_PRE_INC:
                    new_num = num + 1.0;
                    return_num = new_num;
                    break;
                case OP_POST_INC:
                    new_num = num + 1.0;
                    return_num = num;
                    break;
                case OP_PRE_DEC:
                    new_num = num - 1.0;
                    return_num = new_num;
                    break;
                case OP_POST_DEC:
                    new_num = num - 1.0;
                    return_num = num;
                    break;
            }

            Value new_val = value_make_number(new_num);
            if (!env_assign(env, node->data.inc_dec.name, new_val)) {
                fprintf(stderr, "Runtime Error [Line %d]: undefined variable '%s'\n",
                        node->line, node->data.inc_dec.name);
            }

            value_free(current_val);
            result.value = value_make_number(return_num);
            return result;
        }

        case AST_BLOCK:
        case AST_PROGRAM: {
            for (int i = 0; i < node->data.block.count; i++) {
                value_free(result.value);
                result = eval_node(node->data.block.statements[i], env, ft);
                if (result.is_return || result.is_break || result.is_continue) {
                    return result; // Propagate return/break/continue out of block
                }
            }
            return result;
        }

        case AST_IF: {
            EvalResult cond_res = eval_node(node->data.if_stmt.condition, env, ft);
            int is_true = value_is_truthy(cond_res.value);
            value_free(cond_res.value);

            if (is_true) {
                return eval_node(node->data.if_stmt.then_branch, env, ft);
            } else if (node->data.if_stmt.else_branch) {
                return eval_node(node->data.if_stmt.else_branch, env, ft);
            }
            return result;
        }

        case AST_WHILE: {
            while (1) {
                EvalResult cond_res = eval_node(node->data.while_stmt.condition, env, ft);
                int is_true = value_is_truthy(cond_res.value);
                value_free(cond_res.value);

                if (!is_true) {
                    break;
                }
                value_free(result.value);
                result = eval_node(node->data.while_stmt.body, env, ft);
                if (result.is_return) {
                    return result;
                }
                if (result.is_break) {
                    result.is_break = 0;
                    break;
                }
                if (result.is_continue) {
                    result.is_continue = 0;
                    continue;
                }
            }
            result.is_return = 0;
            result.is_break = 0;
            result.is_continue = 0;
            return result;
        }

        case AST_FOR: {
            if (node->data.for_stmt.init) {
                EvalResult init_res = eval_node(node->data.for_stmt.init, env, ft);
                value_free(init_res.value);
            }
            while (1) {
                if (node->data.for_stmt.condition) {
                    EvalResult cond_res = eval_node(node->data.for_stmt.condition, env, ft);
                    int is_true = value_is_truthy(cond_res.value);
                    value_free(cond_res.value);
                    if (!is_true) {
                        break;
                    }
                }
                value_free(result.value);
                result = eval_node(node->data.for_stmt.body, env, ft);
                if (result.is_return) {
                    return result;
                }
                if (result.is_break) {
                    result.is_break = 0;
                    break;
                }
                if (result.is_continue) {
                    result.is_continue = 0;
                    if (node->data.for_stmt.update) {
                        EvalResult upd_res = eval_node(node->data.for_stmt.update, env, ft);
                        value_free(upd_res.value);
                    }
                    continue;
                }
                if (node->data.for_stmt.update) {
                    EvalResult upd_res = eval_node(node->data.for_stmt.update, env, ft);
                    value_free(upd_res.value);
                }
            }
            result.is_return = 0;
            result.is_break = 0;
            result.is_continue = 0;
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
            Value* arg_vals = NULL;
            if (actual > 0) {
                arg_vals = (Value*)malloc(sizeof(Value) * actual);
                for (int i = 0; i < actual; i++) {
                    EvalResult arg_res = eval_node(node->data.call.args[i], env, ft);
                    arg_vals[i] = arg_res.value;
                }
            }

            // Create new activation frame (scope) for function execution
            Environment* local_env = env_create(g_global_env ? g_global_env : env);
            for (int i = 0; i < actual; i++) {
                env_set(local_env, func_decl->data.func_decl.params[i], arg_vals[i]);
                value_free(arg_vals[i]);
            }

            // Execute function body
            EvalResult body_res = eval_node(func_decl->data.func_decl.body, local_env, ft);

            if (arg_vals) free(arg_vals);
            env_free(local_env);

            if (body_res.is_break) {
                fprintf(stderr, "Runtime Error [Line %d]: 'break' outside of loop\n", node->line);
            } else if (body_res.is_continue) {
                fprintf(stderr, "Runtime Error [Line %d]: 'continue' outside of loop\n", node->line);
            }

            // Function call expression yields the returned value
            result.is_return = 0;
            result.is_break = 0;
            result.is_continue = 0;
            result.value = body_res.value;
            return result;
        }

        case AST_RETURN: {
            Value ret_val = value_make_number(0.0);
            if (node->data.return_stmt.expr) {
                EvalResult expr_res = eval_node(node->data.return_stmt.expr, env, ft);
                ret_val = expr_res.value;
            }
            result.is_return = 1;
            result.value = ret_val;
            return result;
        }

        case AST_BREAK: {
            result.is_break = 1;
            return result;
        }

        case AST_CONTINUE: {
            result.is_continue = 1;
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

    EvalResult res = eval_node(root, global_env, &ft);
    if (res.is_break) {
        fprintf(stderr, "Runtime Error: 'break' outside of loop\n");
    } else if (res.is_continue) {
        fprintf(stderr, "Runtime Error: 'continue' outside of loop\n");
    }

    value_free(res.value);
    env_free(global_env);
    g_global_env = NULL;
}
