#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
 
#define n_cards 4
#define solve_goal 24
#define max_digit 13
#define iterations 4
 
typedef struct { int num, denom; } frac_t, *frac;
typedef enum { C_NUM = 0, C_ADD, C_SUB, C_MUL, C_DIV } op_type;
 
typedef struct expr_t *expr;
typedef struct expr_t {
        op_type op;
        expr left, right;
        int value;
} expr_t;

struct card_info {
        int** all_cards;
        int start;
        int end;
};

int weights[max_digit];
int top1, top2, top3;
 
void show_expr(expr e, op_type prec, int is_right)
{
        const char * op;
        switch(e->op) {
        case C_NUM:     printf("%d", e->value);
                        return;
        case C_ADD:     op = " + "; break;
        case C_SUB:     op = " - "; break;
        case C_MUL:     op = " x "; break;
        case C_DIV:     op = " / "; break;
        }
 
        if ((e->op == prec && is_right) || e->op < prec) printf("(");
        show_expr(e->left, e->op, 0);
        printf("%s", op);
        show_expr(e->right, e->op, 1);
        if ((e->op == prec && is_right) || e->op < prec) printf(")");
}
 
void eval_expr(expr e, frac f)
{
        frac_t left, right;
        if (e->op == C_NUM) {
                f->num = e->value;
                f->denom = 1;
                return;
        }
        eval_expr(e->left, &left);
        eval_expr(e->right, &right);
        switch (e->op) {
        case C_ADD:
                f->num = left.num * right.denom + left.denom * right.num;
                f->denom = left.denom * right.denom;
                return;
        case C_SUB:
                f->num = left.num * right.denom - left.denom * right.num;
                f->denom = left.denom * right.denom;
                return;
        case C_MUL:
                f->num = left.num * right.num;
                f->denom = left.denom * right.denom;
                return;
        case C_DIV:
                f->num = left.num * right.denom;
                f->denom = left.denom * right.num;
                return;
        default:
                fprintf(stderr, "Unknown op: %d\n", e->op);
                return;
        }
}
int solve(expr ex_in[], int len, int target)
{
        int i, j;
        expr_t node;
        expr ex[n_cards];
        frac_t final;
 
        if (len == 1) {
                eval_expr(ex_in[0], &final);
                if (final.num == final.denom * target && final.denom) {
                        //if (target != 24) printf(" H(");
                        //show_expr(ex_in[0], 0, 0);
                        //if (target != 24) printf(") ");
                        return 1;
                }
                return 0;
        }
 
        for (i = 0; i < len - 1; i++) {
                for (j = i + 1; j < len; j++)
                        ex[j - 1] = ex_in[j];
                ex[i] = &node;
                for (j = i + 1; j < len; j++) {
                        node.left = ex_in[i];
                        node.right = ex_in[j];
                        for (node.op = C_ADD; node.op <= C_DIV; node.op++)
                                if (solve(ex, len - 1, target))
                                        return 1;
 
                        node.left = ex_in[j];
                        node.right = ex_in[i];
                        node.op = C_SUB;
                        if (solve(ex, len - 1, target)) return 1;
                        node.op = C_DIV;
                        if (solve(ex, len - 1, target)) return 1;
 
                        ex[j] = ex_in[j];
                }
                ex[i] = ex_in[i];
        }
 
        return 0;
}
 
int solve24(int n[], int number_of_cards, int target)
{
        int i;
        expr_t ex[number_of_cards];
        expr   e[number_of_cards];
        for (i = 0; i < number_of_cards; i++) {
                e[i] = ex + i;
                ex[i].op = C_NUM;
                ex[i].left = ex[i].right = 0;
                ex[i].value = n[i];
        }
        

        int x = solve(e, number_of_cards, target);
        return x;
}


void getTops() {
        top1 = top2 = top3 = 0;
        int i;
        for (i = 1; i < max_digit; i++) {
                if (weights[i] > weights[top1]) {
                        top3 = top2;
                        top2 = top1;
                        top1 = i;
                } else if (weights[i] > weights[top2]) {
                        top3 = top2;
                        top2 = i;
                } else if (weights[i] > weights[top3]) {
                        top3 = i;
                }
        }

}

void* thread_solve24(void* cards) {
        struct card_info *my_cards = (struct card_info*) cards;
        int num_cards = my_cards->end - my_cards->start;
        int i, j, k;
        for(i = 0; i < num_cards; i++) {
                int *curr_card = my_cards->all_cards[my_cards->start + i];
                //printf("%d %d %d %d: ", curr_card[0], curr_card[1], curr_card[2], curr_card[3]);

                bool solved = false;
                //Cache
                for (j = 0; j < n_cards; j++) {
                        
                        if (curr_card[j] == top1 || curr_card[j] == top2 || curr_card[j] == top3) {
                                int cache_card[n_cards-1];
                                for (k = 0; k < n_cards - 1; k++) {
                                        if (k < j) {
                                                cache_card[k] = curr_card[k];
                                        } else {
                                                cache_card[k] = curr_card[k+1];
                                        }
                                        

                                }

                                if (solve24(cache_card, n_cards - 1, curr_card[j] * 24)) {
                                        for (k = 0; k < n_cards; k++) {
                                                weights[curr_card[k]]++;
                                                getTops();
                                        }
                                        //printf(" / %d \n", curr_card[j]);
                                        solved = true;
                                        break;
                                }
                                if (solve24(cache_card, n_cards - 1, curr_card[j] + 24)) {
                                        for (k = 0; k < n_cards; k++) {
                                                weights[curr_card[k]]++;
                                                getTops();
                                        }
                                        //printf(" - %d \n", curr_card[j]);
                                        solved = true;
                                        break;
                                        
                                }
                                if (solve24(cache_card, n_cards - 1, 24 - curr_card[j])) {
                                        for (k = 0; k < n_cards; k++) {
                                                weights[curr_card[k]]++;
                                                getTops();

                                        }
                                        //printf(" + %d \n", curr_card[j]);
                                        solved = true;
                                        break;
                                        
                                }
                                if (24 % curr_card[j] == 0) { //if it's not a decimal
                                        if (solve24(cache_card, n_cards - 1, 24/curr_card[j])) {
                                                for (k = 0; k < n_cards; k++) {
                                                        weights[curr_card[k]]++;
                                                        getTops();
                                                }
                                                //printf(" * %d \n", curr_card[j]);
                                                solved = true;
                                                break;
                                                
                                        }
                                }

                        } 
                }

                //Parallel
                if (!solved) {
                   if (solve24(curr_card, n_cards, 24)) {
                           for (j = 0; j < n_cards; j++) {
                                   weights[curr_card[j]]++;
                                   getTops();
                           }
                           //printf("\n");
                   }
                   //else printf("No solution\n");     
                }
                // printf("Weights: ");
                // int b;
                // for (b = 0; b < max_digit; b++) {
                //         printf("{%d, %d} , ", b, weights[b]);
                // }
                // printf("\n");
        

        }

        pthread_exit(NULL);
}
 
int main(int argc, char *argv[])
{
        
        if (argc != 2) {
                printf("Incorrect number of arguments. Remember to pass in number of threads.");
                return(0);
        }

        int i, j, k, rc;
        srand(time(0));

        for (i = 0; i < max_digit; i++) {
                weights[i] = 0;
        }
        top1 = 8; top2 = 3; top3 = 4;


        int num_threads = atoi(argv[1]);
        pthread_t threads[num_threads];
        struct card_info thread_cards[num_threads];

        int** all_cards = (int**) malloc (sizeof(int*)*iterations);
        for (k = 0; k < iterations; k++) {
                all_cards[k] = (int*) malloc(sizeof(int)*n_cards);
        }

        struct timespec start, stop; 
        double time;

        if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}
 
        for (j = 0; j < iterations; j++) {
                for (i = 0; i < n_cards; i++) {
                        all_cards[j][i] = 1 + (double) rand() * max_digit / RAND_MAX;
                        //printf(" %d", all_cards[j][i]);
                }
                //printf(":  ");
                // printf(solve24(n) ? "\n" : "No solution\n");
        }

        for (j = 0; j < num_threads; j++) {
                thread_cards[j].all_cards = all_cards;
                thread_cards[j].start = j * (iterations/num_threads);
                thread_cards[j].end = (j + 1) * (iterations/num_threads);
                rc = pthread_create(&threads[j], NULL, thread_solve24, (void *) &thread_cards[j]);
                if (rc) { printf("ERROR; return code from pthread_create() is %d\n", rc); exit(-1);}
        }
        for(j = 0; j < num_threads; j++) {        
                rc = pthread_join(threads[j], NULL);
                if (rc) { printf("ERROR; return code from pthread_join() is %d\n", rc); exit(-1);}
        }

        if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}           
        time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
        printf("Execution time for %d threads: %f ns\n", num_threads, time*1e9);

        // release memory
        // for (j = 0; j < iterations; j++) {
        //         free(all_cards[i]);
        // }
        // free(all_cards);
 
        return 0;
}