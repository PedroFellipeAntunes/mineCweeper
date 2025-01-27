/*
mineCweeper:
    - Code gets highscore from file;
    - Code creates a 2d matrix with pointers;
    - Each pointer points to the cardinal directions;
    - Each pointer contains a binary interger that defines if it is a bomb or not;
    - Each pointer contains a interger with the amount of bombs around it;
    - Each pointer contains a binary interger that defines if it is hidden or not;
    - Each pointer contains a binary interger that defines if it is flagged or not;
    - When arrow keys pressed, move to that position int the matrix;
    - When enter key is pressed reveal if there is a bomb or not;
    - Code keeps track of play time;
    - If it's a bomb finish game and reveal score;
    - If all non-bomb spaces are revealed win the game and reveal score;
    - Score is calculated by taking: ((amount of spaces * 100 / amount of bombs) - difficulty) / time of play
    - Add new score to file;
gcc -o minecweeper minecweeper.c -lncurses
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

typedef struct nodeMAT NodeMAT;
struct nodeMAT {
    int is_BOMB;
    int num_BOMB;
    int is_hidden;
    int is_flagged;
    NodeMAT *up;
    NodeMAT *down;
    NodeMAT *left;
    NodeMAT *right;
};

typedef struct {
    NodeMAT **start;
    int amount_BOMB;
} MATRIX;

void start_mat(MATRIX *matrix){
    matrix->start = NULL;
    matrix->amount_BOMB = 0;
}

int is_empty_mat(MATRIX *matrix){
    return (matrix->start == NULL);
}

//Randomly decide if it is a bomb based on weight
int is_BOMB(int weight){
    int random_num = rand() % 100;

    if (random_num < weight){
        return 0;
    } else {
        return 1;
    }
}

//Allocate memory space for nodeMAT
NodeMAT* malloc_nodeMAT(){
    NodeMAT *new = (NodeMAT*)malloc(sizeof(NodeMAT));
    new->up = NULL;
    new->down = NULL;
    new->left = NULL;
    new->right = NULL;
    new->is_hidden = 1;
    new->num_BOMB = 0;
    new->is_flagged = 0;
    return new;
}

//Connect the nodeMAT to the matrix
void build_mat(MATRIX *matrix, int size){
    //Allocate the struct arrays
    matrix->start = (NodeMAT**)malloc(size * sizeof(NodeMAT*));

    for (int line = 0; line < size; line++){
        matrix->start[line] = (NodeMAT*)malloc(size * sizeof(NodeMAT));

        for (int col = 0; col < size; col++){
            matrix->start[line][col] = *malloc_nodeMAT();
        }
    }

    //Point the pointer
    for (int line = 0; line < size; line++){
        for (int col = 0; col < size; col++){
            //Connect bellow
            if (line > 0){
                matrix->start[line][col].up = &matrix->start[line-1][col];
            }

            //Connect up
            if (line < size - 1){
                matrix->start[line][col].down = &matrix->start[line+1][col];
            }

            //Connect left
            if (col > 0){
                matrix->start[line][col].left = &matrix->start[line][col-1];
            }

            //Connect right
            if (col < size - 1){
                matrix->start[line][col].right = &matrix->start[line][col+1];
            }
        }
    }
}

//Erase all from memory
void erase_mat(MATRIX *matrix, int size){
    if (!is_empty_mat(matrix)){
        for (int line = 0; line < size; line++){
            free(matrix->start[line]);
        }

        free(matrix->start);
        matrix->start = NULL;
        matrix->amount_BOMB = 0;
    }
}

void print_colors_mat_interactive(int num_BOMB, int is_selected){
    //Setup colors for numbers
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_BLACK);
    init_pair(8, COLOR_GREEN, COLOR_BLACK);

    if (is_selected == 1){
        printw("[");
    }

    attron(COLOR_PAIR(num_BOMB));
    
    if (num_BOMB != 0){
        attron(A_BOLD);
    }

    if (is_selected == 1){
        printw("%d", num_BOMB);
    } else {
        printw(" %d ", num_BOMB);
    }

    attroff(COLOR_PAIR(num_BOMB));
    attroff(A_BOLD);

    if (is_selected == 1){
        printw("]");
    }
}

//Print matrix for game
void print_mat_interactive(NodeMAT **matrix_position, NodeMAT *position, int size){
    for (int line = 0; line < size; line++){
        for (int col = 0; col < size; col++){
            //Check if position matches
            if (&matrix_position[line][col] == position){
                //Check if hidden or fllaged
                if (matrix_position[line][col].is_flagged == 1){
                    printw("[F]");
                } else if (matrix_position[line][col].is_hidden == 1){
                    printw("[.]");
                } else {
                    print_colors_mat_interactive(matrix_position[line][col].num_BOMB, 1);
                }
            } else {
                //Check if hidden or fllaged
                if (matrix_position[line][col].is_flagged == 1){
                    printw(" F ");
                } else if (matrix_position[line][col].is_hidden == 1){
                    printw(" . ");
                } else {
                    print_colors_mat_interactive(matrix_position[line][col].num_BOMB, 0);
                }
            }
        }

        printw("\n");
    }
}

//Check if there are bombs around the position
int count_bomb_square(NodeMAT *pos){
    int BOMB_counter = 0;

    if (pos->up != NULL){
        BOMB_counter += pos->up->is_BOMB;

        if (pos->up->right != NULL){
            BOMB_counter += pos->up->right->is_BOMB;
        }
    }

    if (pos->right != NULL){
        BOMB_counter += pos->right->is_BOMB;

        if (pos->right->down != NULL){
            BOMB_counter += pos->right->down->is_BOMB;
        }
    }

    if (pos->down != NULL){
        BOMB_counter += pos->down->is_BOMB;

        if (pos->down->left != NULL){
            BOMB_counter += pos->down->left->is_BOMB;
        }
    }

    if (pos->left != NULL){
        BOMB_counter += pos->left->is_BOMB;

        if (pos->left->up != NULL){
            BOMB_counter += pos->left->up->is_BOMB;
        }
    }

    return BOMB_counter;
}

//Separate print function that is not affected by the NCURSES screen
void print_mat_non_interactive(NodeMAT **matrix_position, int size){
    for (int line = 0; line < size; line++){
        for (int col = 0; col < size; col++){
            if (matrix_position[line][col].is_BOMB == 1){
                printf(" X ");
            } else {
                int BOMB_count = count_bomb_square(&matrix_position[line][col]);

                init_pair(1, COLOR_BLUE, COLOR_BLACK);
                init_pair(2, COLOR_GREEN, COLOR_BLACK);
                init_pair(3, COLOR_RED, COLOR_BLACK);
                init_pair(4, COLOR_YELLOW, COLOR_BLACK);
                init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
                init_pair(6, COLOR_CYAN, COLOR_BLACK);
                init_pair(7, COLOR_RED, COLOR_BLACK);
                init_pair(8, COLOR_GREEN, COLOR_BLACK);

                switch (BOMB_count){
                    case 1: //BLUE
                    printf("\033[1;34m");
                    break;

                    case 2: //GREEN
                    printf("\033[1;32m");
                    break;

                    case 3: //RED
                    printf("\033[1;31m");
                    break;

                    case 4: //YELLOW
                    printf("\033[1;33m");
                    break;

                    case 5: //PURPLE
                    printf("\033[1;35m");
                    break;

                    case 6: //CYAN
                    printf("\033[1;36m");
                    break;

                    case 7: //RED
                    printf("\033[1;31m");
                    break;

                    case 8: //GREEN
                    printf("\033[1;32m");
                    break;
                
                    default:
                    break;
                }

                printf(" %d ", BOMB_count);
                printf("\033[0;0m");
            }
        }

        printf("\n");
    }
}

//Reveal all the non-bombs around
int reveal_adjacent_zeros(NodeMAT *position){
    if (position == NULL || position->is_hidden == 0){
        return 0;
    }

    int clears = 1;
    
    position->is_hidden = 0;
    position->is_flagged = 0;

    if ((position->num_BOMB = count_bomb_square(position)) != 0){
        return clears;
    }
    
    //Recursive fuction calls
    clears += reveal_adjacent_zeros(position->up);
    clears += reveal_adjacent_zeros(position->down);
    clears += reveal_adjacent_zeros(position->left);
    clears += reveal_adjacent_zeros(position->right);

    if (position->up != NULL) {
        clears += reveal_adjacent_zeros(position->up->right);
        clears += reveal_adjacent_zeros(position->up->left);
    }

    if (position->down != NULL) {
        clears += reveal_adjacent_zeros(position->down->right);
        clears += reveal_adjacent_zeros(position->down->left);
    }

    return clears;
}

void set_if_BOMB_first_play(MATRIX *matrix, NodeMAT *position, int weight, int size){
    int line_pos, col_pos;

    for (int line = 0; line < size; line++){
        for (int col = 0; col < size; col++){
            //Makes sure that the first node selected won't be a bomb
            if (&matrix->start[line][col] == position){
                matrix->start[line][col].is_BOMB = 0;
                line_pos = line;
                col_pos = col;
            } else {
                matrix->start[line][col].is_BOMB = is_BOMB(weight);
                
                if (matrix->start[line][col].is_BOMB == 1){
                    matrix->amount_BOMB++;
                }
            }
        }
    }

    //Really dumb way to make all the squares around the first square not BOMBS    
    if (position->up != NULL){
        position->up->is_BOMB = 0;

        if (position->up->right != NULL){
            position->up->right->is_BOMB = 0;
        }
    }

    if (position->right != NULL){
        position->right->is_BOMB = 0;

        if (position->right->down != NULL){
            position->right->down->is_BOMB = 0;
        }
    }

    if (position->down != NULL){
        position->down->is_BOMB = 0;

        if (position->down->left != NULL){
            position->down->left->is_BOMB = 0;
        }
    }

    if (position->left != NULL){
        position->left->is_BOMB = 0;

        if (position->left->up != NULL){
            position->left->up->is_BOMB = 0;
        }
    }
}

//Move around the matrix
int mat_pointer(MATRIX *matrix, int weight, int size){
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    start_color();
    
    NodeMAT *temporary_nodeMAT = *matrix->start;

    int key, game_over = 0, clears = 0, first_play = 1;

    srand(time(NULL)); //Seed for random generation of bombs

    while (1){
        clear();
        
        //Proper spacing for header
        if (size == 12){
            printw("    ");
        } else if (size == 15){
            printw("        ");
        }

        init_pair(1, COLOR_BLUE, COLOR_BLACK);
        attron(COLOR_PAIR(1));
        attron(A_BOLD);
        printw("=======[mineCweeper]=======\n");
        attroff(A_BOLD);
        attroff(COLOR_PAIR(1));

        print_mat_interactive(matrix->start, temporary_nodeMAT, size);

        //Check if gameover after showing players mistake
        if (game_over == 1){
            endwin();
            system("clear");

            if (size == 12){
                printf("    ");
            } else if (size == 15){
                printf("        ");
            }

            printf("\033[1;34m");
            printf("=======[mineCweeper]=======\n");
            printf("\033[0;0m");

            print_mat_non_interactive(matrix->start, size);

            if (size == 12){
                printf("    ");
            } else if (size == 15){
                printf("        ");
            }

            printf("\033[1;31m");
            printf("========[GAME OVER]========\n");
            printf("\033[0;0m");
            
            return clears;
        } else if (((size * size) - matrix->amount_BOMB) == clears){
            endwin();
            system("clear");

            if (size == 12){
                printf("    ");
            } else if (size == 15){
                printf("        ");
            }

            printf("\033[1;34m");
            printf("=======[mineCweeper]=======\n");
            printf("\033[0;0m");

            print_mat_non_interactive(matrix->start, size);

            if (size == 12){
                printf("    ");
            } else if (size == 15){
                printf("        ");
            }

            printf("\033[0;32m");
            printf("=========[YOU WON]=========\n");
            printf("\033[0;0m");
            return clears;
        }

        //Key commands
        key = getch();

        switch (key){
            case KEY_UP:
            if (temporary_nodeMAT->up != NULL){
                temporary_nodeMAT = temporary_nodeMAT->up;
            }
            break;

            case KEY_DOWN:
            if (temporary_nodeMAT->down != NULL){
                temporary_nodeMAT = temporary_nodeMAT->down;
            }
            break;

            case KEY_LEFT:
            if (temporary_nodeMAT->left != NULL){
                temporary_nodeMAT = temporary_nodeMAT->left;
            }
            break;

            case KEY_RIGHT:
            if (temporary_nodeMAT->right != NULL){
                temporary_nodeMAT = temporary_nodeMAT->right;
            }
            break;

            case 'f':
            case 'F':
            if (temporary_nodeMAT->is_flagged == 0 && temporary_nodeMAT->is_hidden == 1){
                temporary_nodeMAT->is_flagged = 1;
            } else {
                temporary_nodeMAT->is_flagged = 0;
            }
            break;

            case 10: //For some reason KEY_ENTER doesn't work
            if (temporary_nodeMAT->is_hidden == 1){
                //Never be a bomb in first selected square
                if (first_play == 1){
                    set_if_BOMB_first_play(matrix, temporary_nodeMAT, weight, size);
                    clears += reveal_adjacent_zeros(temporary_nodeMAT);
                    first_play = 0;
                    break;
                }

                //Get number of bombs around
                temporary_nodeMAT->num_BOMB = count_bomb_square(temporary_nodeMAT);

                //Reveal squares around when 0
                if (temporary_nodeMAT->num_BOMB == 0){
                    clears += reveal_adjacent_zeros(temporary_nodeMAT);
                } else {
                    temporary_nodeMAT->is_hidden = 0;
                    temporary_nodeMAT->is_flagged = 0; //Removes flag
                }
                
                //Game over or add points
                if (temporary_nodeMAT->is_BOMB == 1){
                    game_over = 1;
                } else {
                    if (temporary_nodeMAT->num_BOMB != 0){
                        clears++;
                    }
                }
            }
            break;

            //Force exit
            case KEY_BACKSPACE:
            endwin();
            return -1; //-1 if force exit
            break;

            default:
            break;
        }

        refresh();
    }

    endwin();
}

//Calculate points
int point_calculation(int clears, int weight, int BOMB_amount, int play_time, int size){
    /*
    Get points based on difficulty, time and porcentage clear

    size*size-BOMB_amount = 100%
    clears          = x%
    clear% * weight
    weighted_points - play_time
    */
    int points = (((clears * 100) / ((size * size) - BOMB_amount)) * (100 - weight)) - play_time;
    
    if (points < 0){
        points = 0;
    }

    printf("[TIME]: %.3dsec\n[POINTS]: %d\n", play_time, points);
    return points;
}

typedef struct nodeLIST NodeLIST;
struct nodeLIST{
    int SCORE;
    char NAME[4];
    NodeLIST *next;
    NodeLIST *back;
};

typedef struct{
    NodeLIST *start;
    int size;
} LIST;

void start_list(LIST *list){
    list->start = NULL;
    list->size = 0;
}

int is_empty_list(LIST *list){
    return (list->start == NULL);
}

//Adds the nodes to the list
void add_line_to_list(LIST *list, char name[4], int score){
    NodeLIST *new = (NodeLIST*)malloc(sizeof(NodeLIST));
    strcpy(new->NAME, name);
    new->SCORE = score;
    new->next = NULL;
    new->back = NULL;

    if (list->start == NULL){
        list->start = new;
    } else {
        NodeLIST *temporary_node = list->start;
        
        while (temporary_node->next != NULL){
            temporary_node = temporary_node->next;
        }

        new->back = temporary_node;
        temporary_node->next = new;
    }
}

//Gets dada from file
void file_to_list(LIST *list){
    FILE *file_read = fopen("minecweeper_highscore.txt", "r");

    if (file_read == NULL){
        return;
    }

    int score;
    char name[4];

    printf("[HIGH-SCORE]:\n");
    printf("\033[0m");

    while (EOF != fscanf(file_read, "%3s %d", name, &score)){
        add_line_to_list(list, name, score);
        list->size++;
        printf("    %s - %.4d\n", name, score);
    }

    fclose(file_read);
}

int next_node_list(NodeLIST *next, char name[4], int score){
    if (next == NULL){
        return 0;
    }

    next_node_list(next->next, next->NAME, next->SCORE);
    strcpy(next->NAME, name);
    next->SCORE = score;
}

//Adds new highscore
int add_to_high_score_list(LIST *list, int new_score){
    NodeLIST *temporary_pointer = list->start;
    
    int line = 0;
    char new_name[4];

    while (temporary_pointer != NULL){
        if (new_score > temporary_pointer->SCORE){
            printf("\033[1;34m");
            printf("=====[NEW HIGH-SCORE]======\n");
            printf("\033[0;0m");
            printf("[NAME]: ");
            scanf("%3s", new_name);

            if (list->size >= 5){
                next_node_list(temporary_pointer, temporary_pointer->NAME, temporary_pointer->SCORE);

                strcpy(temporary_pointer->NAME, new_name);
                temporary_pointer->SCORE = new_score;
            } else {
                NodeLIST *new_node = (NodeLIST *)malloc(sizeof(NodeLIST));

                strcpy(new_node->NAME, new_name);
                new_node->SCORE = new_score;
                new_node->next = temporary_pointer;
                new_node->back = temporary_pointer->back;

                if (temporary_pointer->back != NULL) {
                    temporary_pointer->back->next = new_node;
                } else {
                    list->start = new_node;
                }

                temporary_pointer->back = new_node;
                list->size++;
            }

            break;
        }

        temporary_pointer = temporary_pointer->next;
    }

    if (temporary_pointer == NULL){ //Its not greater than the list
        if (list->size < 5){
            printf("\033[1;34m");
            printf("=====[NEW HIGH-SCORE]======\n");
            printf("\033[0;0m");
            printf("[NAME]: ");
            scanf("%3s", new_name);

            add_line_to_list(list, new_name, new_score);

            goto skip_return; //SKIPS RETURN
        }

        return 0;
    }

    skip_return:

    temporary_pointer = list->start;

    while (temporary_pointer != NULL){
        printf("    %s - %.4d\n", temporary_pointer->NAME, temporary_pointer->SCORE);
        temporary_pointer = temporary_pointer->next;
    }

    return 1;
}

//Update file
void update_high_score_file(LIST *list){
    NodeLIST *temporary_node = list->start;

    FILE *file_write = fopen("minecweeper_highscore.txt", "w");

    if (file_write == NULL){
        return;
    }

    while (temporary_node != NULL){
        fprintf(file_write, "%s %.4d", temporary_node->NAME, temporary_node->SCORE);
        
        if (temporary_node->next != NULL){
            fprintf(file_write, "\n");
        }
        
        temporary_node = temporary_node->next;
    }

    fclose(file_write);
}

//Iterate through list to free
void recursive_erase(NodeLIST *node){
    if (node == NULL){
        return;
    }

    recursive_erase(node->next);
    free(node);
    node->next = NULL;
}

//Erase list from memory
void erase_list(LIST *list){
    if (!is_empty_list(list)){
        recursive_erase(list->start);
        list->start = NULL;
    }
}

int main(void){
    MATRIX matrix;
    LIST high_score_list;

    int update_file, weight, selection, clears, play_time, score, size;

    start_list(&high_score_list);
    start_mat(&matrix);

    //MENU
    system("clear");
    printf("\033[1;34m");
    printf("=======[mineCweeper]=======\n");

    file_to_list(&high_score_list);

    printf("\033[1;34m");
    printf("[CONTROLS]:\n");
    printf("\033[0m");
    printf("    UP      DOWN\n    LEFT    RIGHT\n    ENTER   BACKSPACE\n    F_KEY\n");
    printf("\033[1;34m");
    printf(    "[DIFFICULTY]:\n");
    printf("\033[0m");
    printf(    "    1 - Easy\n"
    "    2 - Medium\n"
    "    3 - Hard\n"
    "    4 - Impossible\n> ");
    scanf("%d", &selection);

    switch (selection){
        case 1:
        weight = 80;
        size = 9;
        break;

        case 2:
        weight = 80;
        size = 12;
        break;

        case 3:
        weight = 85;
        size = 15;
        break;

        case 4:
        weight = 20;
        size = 9;
        break;
    
        default:
        printf("\033[1;31m");
        printf("![CHOOSE A OPTION DUMBASS]!\n");
        return 1;
        break;
    }

    build_mat(&matrix, size);
    
    time_t start_time = time(NULL); //Get starting time
    
    clears = mat_pointer(&matrix, weight, size);

    //Check if force exit
    if (clears == -1){
        erase_mat(&matrix, size);
        return 0;
    }

    time_t end_time = time(NULL); //Get ending time
    play_time = end_time - start_time;

    score = point_calculation(clears, weight, matrix.amount_BOMB, play_time, size);
    erase_mat(&matrix, size);

    update_file = add_to_high_score_list(&high_score_list, score);

    if (update_file == 1){
        update_high_score_file(&high_score_list);
    }

    erase_list(&high_score_list);

    return 0;
}
