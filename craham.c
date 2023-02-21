#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* defs */
#define LOGNAME craham.log
#define SIDE 8
#define MOVES 256
#define CAPTURED -1
#define DEPTH 5 /* ply number */
#define QDEPTH -32
#define TABLESIZE 65536
#define EXACT 0
#define NO_EVAL -999999
#define NO_HASH 0

#define LAST_ACTION { .m = 0 , .n = 0, .movem = 0, .moven = 0, .eval = 0 }
#define EMPTY_SQUARE { .type_val = 0, .colour = -1, .mc = -0 , .node = NULL}

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define HOMEROW(A) ((A) ? (7) : (0))
#define PAWNHOMEROW(A) ((A) ? (6) : (1))
#define DIRECTION(A) ((A) ? (-1) : (1))

enum { FALSE, TRUE };

enum { ALPHA=1, BETA=2 };

enum colour { BLACK, WHITE };

enum piece_type {
    KING = 1000,
    QUEEN = 900,
    ROOK = 500,
    BISHOP = 350,
    KNIGHT = 300,
    PAWN = 100,
    EMPTY = 0
};

/* types */
struct Pair {
    int a;
    int b;
};

struct PieceNode {
    int m;
    int n;
    void *next;
    void *prev;
};

struct Piece {
    int type_val;
    int colour;
    int mc;
    struct PieceNode *node;
};

struct Action {
    int m;
    int n;
    int movem;
    int moven;
    int eval;
    int type;
};

struct Board {
    struct Piece all_pieces[SIDE][SIDE];
    int kingm[2];
    int kingn[2];
    struct PieceNode *head_piece[2];
    struct PieceNode *tail_piece[2];
};

/* board */
struct PieceNode *search_list(struct PieceNode *head, int m, int n);
struct Board *copy_board(struct Board *old_board);
void copy_piece_list(struct PieceNode *src_head, struct Board *dest_board, 
	                 int colour);
void delete_piece_list(struct PieceNode *head);
void delete_board(struct Board *board);
void add_all_pieces(struct Board *board);
struct PieceNode *push_piece_node(int m, int n, struct Board *board);
struct PieceNode *pop_piece_node(int colour, struct Board *board);
void remove_piece_node(struct Board *board, struct PieceNode *piece_node);
int add_pawns(int row, int colour, struct Board *board);
int add_two_pieces(int column, int type_val, struct Board *board);
int add_empty_squares(struct Piece all_pieces[8][8]);
void execute_move(int m, int n, int movem, int moven, struct Board *board);
void execute_cap(int m, int n, int movem, int moven, struct Board *board);
void reset_enemy_pawns(int colour, struct Piece all_pieces[8][8]);
void execute_castle(int m, int moven, struct Board *board);
void capture_en_passant(int m, int n, int movem, int moven, 
	                    struct Board *board);

/* ui */
int proper_print_board(struct Piece all_pieces[8][8], int colour);
int print_board(struct Piece all_pieces[8][8]);
char get_piece_icon(int type_val, int colour);
int file_to_row(char f);
void proper_move_prompt(struct Board *board, int colour);
char *my_strcpy(char *dest, char *src, int len);
struct Action parse_input(const char *input);

/* rules */
int further_from_zero(int num);
int closer_to_zero(int num);
int is_in_bounds(int x);
int is_on_board(int m, int n);
int is_valid_target(int m, int n, int movem, int moven, 
                    struct Piece all_pieces[8][8]);
int diag_path_empty(int m, int n, int movem, int moven, 
                    struct Piece all_pieces[8][8]);
int straight_path_empty(int m, int n, int movem, int moven, 
                        struct Piece all_pieces[8][8]);
int is_lShaped(int m, int n);
int valid_knight_move(int m, int n, int movem, int moven, 
                      struct Piece all_pieces[8][8]);
int valid_bishop_move(int m, int n, int movem, int moven, 
                      struct Piece all_pieces[8][8]);
int valid_rook_move(int m, int n, int movem, int moven, 
                    struct Piece all_pieces[8][8]);
int valid_king_move(int m, int n, int movem, int moven, struct Board *board);
int kings_too_close(int m, int n, int movem, int moven, struct Board *board);
int valid_castle(int m, int n, int movem, int moven, struct Board *board);
int is_basic_pawn_move(int colour, int movem, int moven);
int is_pawn_forward_two(int m, int n, int movem, int moven, 
                        struct Piece all_pieces[8][8]);
int is_pawn_capture(int colour, int movem, int moven);
int valid_pawn_move(int m, int n, int movem, int moven, 
                    struct Piece all_pieces[8][8]);
int valid_en_passant(int m, int n, int movem, int moven, 
                     struct Piece all_pieces[8][8]);
int valid_move(int m, int n, int movem, int moven, struct Board *board);
int is_threatened(int m, int n, struct Board *board, int colour);
int in_check(int colour, struct Board *board);
int will_be_in_check(int m, int n, int movem, int moven, struct Board *board);

/* eval */
int total_val(struct Board *board);
int calc_index(int colour, int m, int n);
int eval_piece(int m, int n, struct Piece all_pieces[8][8]);
int eval_knight(int index);
int eval_bishop(int index);
int eval_rook(int index);
int eval_pawn(int index);
int eval_king(int index);
int king_safety(int m, int n, struct Piece all_pieces[8][8]);
int eval_queen(int index);

/* search */
struct Action find_most_epic_quiet_move(struct Pair ab, int depth, int colour, 
                                        struct Board *board);
struct Action find_most_epic_move(struct Pair ab, int depth, int colour, 
                                  struct Board *board);
int geq(int a, int b);
int leq(int a, int b);
struct Pair dont_explore(struct Pair ab, int colour, int eval);
struct Pair update_aB(struct Pair ab, int colour, int eval, int *node_type);
int is_last_action(struct Action *action);

/* movegen */
struct Action *q_sort_moves(int colour, struct Action *legal_moves, int len);
int w_compare(const void *a, const void *b);
int b_compare(const void *a, const void *b);
int greater_than(int a, int b);
int less_than(int a, int b);
int strongest_move_from_list(int colour, struct Action *legal_moves);
int add_all_legal_moves(int colour, struct Board *board, 
                        struct Action *legal_moves);
int add_one_legal_move(int index, int m, int n, int movem, int moven, 
                       struct Action *legal_moves, struct Board *board);
int add_all_legal_caps(int colour, struct Board *board, 
                       struct Action *legal_moves);
int add_legal_moves(int m, int n, struct Board *board, 
                    struct Action *legal_moves, int index);
int add_legal_caps(int m, int n, struct Board *board, 
struct Action *legal_moves, int index);
int legal_pawn_moves(int m, int n, struct Board *board, 
                     struct Action *legal_moves, int index);
int legal_pawn_caps(int m, int n, struct Board *board, 
                    struct Action *legal_moves, int index);
int legal_knight_moves(int m, int n, struct Board *board, 
                       struct Action *legal_moves, int index);
int knight_move_legality(int m, int n, int movem, int moven, 
                         struct Board *board, struct Action *legal_moves, 
                         int index);
int legal_knight_caps(int m, int n, struct Board *board, 
                      struct Action *legal_moves, int index);
int knight_cap_legality(int m, int n, int movem, int moven, struct Board *board,
                        struct Action *legal_moves, int index);
int legal_bishop_moves(int m, int n, struct Board *board, 
                       struct Action *legal_moves, int index);
int bishop_line_legality(int m, int n, int movem, int moven, 
                         struct Board *board, struct Action *legal_moves, 
                         int index);
int legal_bishop_caps(int m, int n, struct Board *board, 
                      struct Action *legal_moves, int index);
int bishop_cap_legality(int m, int n, int movem, int moven, struct Board *board, 
                        struct Action *legal_moves, int index);
int legal_rook_moves(int m, int n, struct Board *board, 
                     struct Action *legal_moves, int index);
int rook_line_legality(int m, int n, int movem, int moven, struct Board *board, 
                       struct Action *legal_moves, int index);
int legal_rook_caps(int m, int n, struct Board *board, 
                    struct Action *legal_moves, int index);
int rook_cap_legality(int m, int n, int movem, int moven, struct Board *board, 
                      struct Action *legal_moves, int index);
int legal_queen_moves(int m, int n, struct Board *board, 
                      struct Action *legal_moves, int index);
int legal_queen_caps(int m, int n, struct Board *board, 
                     struct Action *legal_moves, int index);
int legal_king_moves(int m, int n, struct Board *board, 
                     struct Action *legal_moves, int index);
int king_move_legality(int m, int n, int movem, int moven, struct Board *board, 
                       struct Action *legal_moves, int index);
int legal_king_caps(int m, int n, struct Board *board, 
                    struct Action *legal_moves, int index);
int king_cap_legality(int m, int n, int movem, int moven, struct Board *board, 
                      struct Action *legal_moves, int index);

/* global consts */
const struct Action LASTACTION = LAST_ACTION;
const struct Piece EMPTYSQUARE = EMPTY_SQUARE;

/* performs a quick sort on moves based on eval */
struct Action *
q_sort_moves(int colour, struct Action *legal_moves, int len)
{
    int (*cmp)(const void *a, const void *b);

    if (colour) {
        cmp = &w_compare;
    } else {
        cmp = &b_compare;
    }
    qsort(legal_moves, len, sizeof(struct Action), cmp);
    return legal_moves;
}

/* compare two moves for white */
int
w_compare(const void *a, const void *b)
{
    struct Action *m = (struct Action*) a;
    struct Action *n = (struct Action*) b;

    if (m->eval == n->eval)
        return 0;
    else if (m->eval > n->eval)
        return -1;
    else
        return 1;
}

/* compare two moves for black */
int
b_compare(const void *a, const void *b)
{
    struct Action *m = (struct Action*) a;
    struct Action *n = (struct Action*) b;

    if (m->eval == n->eval)
        return 0;
    else if (m->eval > n->eval)
        return 1;
    else
        return -1;
}

inline int
greater_than(int a, int b)
{
	return a > b;
}

inline int
less_than(int a, int b)
{
	return a < b;
}

/* returns the action with the best eval */
int
strongest_move_from_list(int colour, struct Action *legal_moves)
{
    int i = 1, best = 0;
    int (*comp)(int, int);

    if (colour)
    	comp = greater_than;
    else
    	comp = less_than;
    while (!is_last_action(&legal_moves[i])) {
		if (comp(legal_moves[i].eval, legal_moves[best].eval))
			best = i;
		i++;
    }
    return best;
}

/* adds a legal move to the list of all legal moves */
int
add_one_legal_move(int index, int m, int n, int movem, int moven, 
struct Action *legal_moves, struct Board *board)
{
    struct Board *new_board = copy_board(board);

    legal_moves[index].m = m;
    legal_moves[index].n = n;
    legal_moves[index].movem = movem;
    legal_moves[index].moven = moven;
    execute_move(m,n,movem,moven,new_board);
    legal_moves[index].eval = total_val(new_board);
    delete_board(new_board);
    return ++index;
}

/* adds all the legal caps for one side */
int 
add_all_legal_caps(int colour, struct Board *board, struct Action *legal_moves)
{
    int index = 0;
    struct PieceNode *attacker = board->head_piece[colour];

    while (attacker) {
        index = add_legal_caps(attacker->m, attacker->n, board, legal_moves, 
        	index); 
        attacker = attacker->next;
    }
    return index;
}

/* adds all the legal moves for one side */
int 
add_all_legal_moves(int colour, struct Board *board, struct Action *legal_moves)
{
    int index = 0;
    struct PieceNode *attacker = board->head_piece[colour];

    while (attacker) {
        index = add_legal_moves(attacker->m, attacker->n, board, legal_moves, 
        	index);
        attacker = attacker->next;
    }
    return index;
}

/* adds all legal caps for one piece */
int 
add_legal_caps(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    switch(board->all_pieces[m][n].type_val) {
    case PAWN :
        return legal_pawn_caps(m, n, board, legal_moves, index);        
    case KNIGHT :
        return legal_knight_caps(m, n, board, legal_moves, index);
    case BISHOP :
        return legal_bishop_caps(m, n, board, legal_moves, index);
    case ROOK :
        return legal_rook_caps(m, n, board, legal_moves, index);
    case QUEEN :
        return legal_queen_caps(m, n, board, legal_moves, index);
    case KING :
        return legal_king_caps(m, n, board, legal_moves, index);
    default :
        return index; 
    }
}

/* adds all legal moves for one piece */
int 
add_legal_moves(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    switch(board->all_pieces[m][n].type_val) {
    case PAWN :
        return legal_pawn_moves(m, n, board, legal_moves, index);        
    case KNIGHT :
        return legal_knight_moves(m, n, board, legal_moves, index);
    case BISHOP :
        return legal_bishop_moves(m, n, board, legal_moves, index);
    case ROOK :
        return legal_rook_moves(m, n, board, legal_moves, index);
    case QUEEN :
        return legal_queen_moves(m, n, board, legal_moves, index);
    case KING :
        return legal_king_moves(m, n, board, legal_moves, index);
    default :
        return index; 
    }
}

/* adds legal moves for one pawn */
int
legal_pawn_moves(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    int colour = board->all_pieces[m][n].colour;
    int dir = DIRECTION(colour);
    
    if (board->all_pieces[m+dir][n].type_val == EMPTY && \
    !will_be_in_check(m, n, dir, 0, board)) {
        index = add_one_legal_move(index,m,n,dir,0,legal_moves,board);
        if (board->all_pieces[m+dir*2][n].type_val == EMPTY && \
        m == PAWNHOMEROW(board->all_pieces[m][n].colour)) {
            index = add_one_legal_move(index,m,n,dir*2,0,legal_moves,board);
        }
    }
    index = legal_pawn_caps(m,n,board,legal_moves,index);
    return index;
}

/* adds the legal captures a pawn can make */
int
legal_pawn_caps(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    int i;
    int colour = board->all_pieces[m][n].colour;
    int dir = DIRECTION(colour);
    struct Piece piece;

    for (i = -1; i <= 1; i += 2) {
        piece = board->all_pieces[m+dir][n+i];
        if (is_on_board(m+dir, n+i)) {
            if (piece.type_val != EMPTY && \
            board->all_pieces[m][n].colour != piece.colour && \
            !will_be_in_check(m, n, dir, i, board)) {
                index = add_one_legal_move(index,m,n,dir,i,legal_moves,board);
            }
            if (piece.type_val == EMPTY && \
            board->all_pieces[m][n+i].type_val == PAWN && \
            board->all_pieces[m][n+i].colour != colour \
            && board->all_pieces[m][n+i].mc == 2 && \
            !will_be_in_check(m, n, dir, i, board)) {
                index = add_one_legal_move(index,m,n,dir,i,legal_moves,board);
            }
        }
    }
    return index;
}

/* adds legal moves for one knight */
int
legal_knight_moves(int m, int n, struct Board *board, 
struct Action *legal_moves, int index)
{
    index = knight_move_legality(m, n,1,2,board,legal_moves,index);
    index = knight_move_legality(m, n,-1,2,board,legal_moves,index);
    index = knight_move_legality(m, n,1,-2,board,legal_moves,index);
    index = knight_move_legality(m, n,-1,-2,board,legal_moves,index);
    index = knight_move_legality(m, n,2,1,board,legal_moves,index);
    index = knight_move_legality(m, n,-2,1,board,legal_moves,index);
    index = knight_move_legality(m, n,2,-1,board,legal_moves,index);
    index = knight_move_legality(m, n,-2,-1,board,legal_moves,index);
    return index;
}

/* checks and adds one knight move to legal moves */
int
knight_move_legality(int m, int n, int movem, int moven, struct Board *board, \
struct Action *legal_moves, int index)
{
    if (is_valid_target(m, n, movem, moven, board->all_pieces) &&
    !will_be_in_check(m, n, movem, moven, board))
        index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
    return index;
}

/* adds legal captures for one knight */
int
legal_knight_caps(int m, int n, struct Board *board, 
struct Action *legal_moves, int index)
{
    index = knight_cap_legality(m, n,1,2,board,legal_moves,index);
    index = knight_cap_legality(m, n,-1,2,board,legal_moves,index);
    index = knight_cap_legality(m, n,1,-2,board,legal_moves,index);
    index = knight_cap_legality(m, n,-1,-2,board,legal_moves,index);
    index = knight_cap_legality(m, n,2,1,board,legal_moves,index);
    index = knight_cap_legality(m, n,-2,1,board,legal_moves,index);
    index = knight_cap_legality(m, n,2,-1,board,legal_moves,index);
    index = knight_cap_legality(m, n,-2,-1,board,legal_moves,index);
    return index;
}

/* checks and adds one knight capture to legal moves */
int
knight_cap_legality(int m, int n, int movem, int moven, struct Board *board, \
struct Action *legal_moves, int index)
{
    if (is_on_board(m+movem, n+moven) &&
    board->all_pieces[m][n].colour != board->all_pieces[m+movem][n+moven].colour 
    && board->all_pieces[m+movem][n+moven].type_val != EMPTY
    && !will_be_in_check(m,n,movem,moven,board))
        index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
    return index;
}

/* adds legal moves for one bishop */
int
legal_bishop_moves(int m, int n, struct Board *board, 
struct Action *legal_moves, int index)
{
    index = bishop_line_legality(m,n,1,1,board,legal_moves,index);
    index = bishop_line_legality(m,n,-1,1,board,legal_moves,index);
    index = bishop_line_legality(m,n,1,-1,board,legal_moves,index);
    index = bishop_line_legality(m,n,-1,-1,board,legal_moves,index);
    return index;
}

/* checks and adds moves for one line of direction for a bishop */
int
bishop_line_legality(int m, int n, int movem, int moven, struct Board *board, \
struct Action *legal_moves, int index)
{
    while (is_on_board(m+movem, n+moven) && \
    board->all_pieces[m+movem][n+moven].type_val == EMPTY) {
        if (!will_be_in_check(m,n,movem,moven,board))
            index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
        movem = further_from_zero(movem);
        moven = further_from_zero(moven);
    }

    if (is_on_board(m+movem, n+moven) &&
    board->all_pieces[m+movem][n+moven].colour != 
    board->all_pieces[m][n].colour) {
        if (!will_be_in_check(m,n,movem,moven,board))
            index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
    }
    return index;
}

/* adds legal moves for one bishop */
int
legal_bishop_caps(int m, int n, struct Board *board, 
struct Action *legal_moves, int index)
{
    index = bishop_cap_legality(m,n,1,-1,board,legal_moves,index);
    index = bishop_cap_legality(m,n,1,1,board,legal_moves,index);
    index = bishop_cap_legality(m,n,-1,1,board,legal_moves,index);
    index = bishop_cap_legality(m,n,-1,-1,board,legal_moves,index);
    return index;
}

/* checks and adds a capture for one line of direction for a bishop */
int
bishop_cap_legality(int m, int n, int movem, int moven, struct Board *board, \
struct Action *legal_moves, int index)
{
    while (is_on_board(m+movem, n+moven) && \
    board->all_pieces[m+movem][n+moven].type_val == EMPTY) {
        movem = further_from_zero(movem);
        moven = further_from_zero(moven);
    }
    if (is_on_board(m+movem, n+moven) &&
    board->all_pieces[m+movem][n+moven].colour != 
    board->all_pieces[m][n].colour) {
        if (!will_be_in_check(m,n,movem,moven,board))
            index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
    }
    return index;
}

/* adds the legal moves one rook can make */
int
legal_rook_moves(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    index = rook_line_legality(m,n,1,0,board,legal_moves,index);
    index = rook_line_legality(m,n,-1,0,board,legal_moves,index);
    index = rook_line_legality(m,n,0,1,board,legal_moves,index);
    index = rook_line_legality(m,n,0,-1,board,legal_moves,index);
    return index;
}

/* checks and adds moves for one line of direction for a rook */
int
rook_line_legality(int m, int n, int movem, int moven, struct Board *board, \
struct Action *legal_moves, int index)
{
    while (is_on_board(m+movem, n+moven) && \
    board->all_pieces[m+movem][n+moven].type_val == EMPTY) {
        if (!will_be_in_check(m,n,movem,moven,board))
            index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
        if (movem == 0)
            moven = further_from_zero(moven);
        else
            movem = further_from_zero(movem);
    }
    if (is_on_board(m+movem, n+moven) &&
    board->all_pieces[m+movem][n+moven].colour != 
    board->all_pieces[m][n].colour) {
        if (!will_be_in_check(m,n,movem,moven,board))
            index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
    }
    return index;
}

/* adds the legal caps one rook can make */
int
legal_rook_caps(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    index = rook_cap_legality(m,n,1,0,board,legal_moves,index);
    index = rook_cap_legality(m,n,-1,0,board,legal_moves,index);
    index = rook_cap_legality(m,n,0,1,board,legal_moves,index);
    index = rook_cap_legality(m,n,0,-1,board,legal_moves,index);
    return index;
}
/* checks and adds caps for one line of direction for a rook */
int
rook_cap_legality(int m, int n, int movem, int moven, struct Board *board, \
struct Action *legal_moves, int index)
{
    while (is_on_board(m+movem, n+moven) && \
    board->all_pieces[m+movem][n+moven].type_val == EMPTY) {
        if (movem == 0)
            moven = further_from_zero(moven);
        else
            movem = further_from_zero(movem);
    }
    if (is_on_board(m+movem, n+moven) &&
    board->all_pieces[m+movem][n+moven].colour != 
    board->all_pieces[m][n].colour) {
        if (!will_be_in_check(m,n,movem,moven,board))
            index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
    }
    return index;
}

/* adds the legal moves one queen can make */
int
legal_queen_moves(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    index = legal_rook_moves(m, n, board, legal_moves, index);
    index = legal_bishop_moves(m, n, board, legal_moves, index);
    return index;
}

/* adds the legal moves one queen can make */
int
legal_queen_caps(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    index = legal_rook_caps(m, n, board, legal_moves, index);
    index = legal_bishop_caps(m, n, board, legal_moves, index);
    return index;
}

/* adds the legal moves one king can make */
int
legal_king_moves(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    int i;

    index = king_move_legality(m,n,1,1,board,legal_moves,index);
    index = king_move_legality(m,n,0,1,board,legal_moves,index);
    index = king_move_legality(m,n,1,0,board,legal_moves,index);
    index = king_move_legality(m,n,-1,-1,board,legal_moves,index);
    index = king_move_legality(m,n,1,-1,board,legal_moves,index);
    index = king_move_legality(m,n,-1,1,board,legal_moves,index);
    index = king_move_legality(m,n,0,-1,board,legal_moves,index);
    index = king_move_legality(m,n,-1,0,board,legal_moves,index);
    for (i = -2; i <= 2; i += 4) {
        if (valid_castle(m,n,0,i,board))
            index = add_one_legal_move(index,m,n,0,i,legal_moves,board);
    }
    return index;
}

/* checks and adds one move that the king can make */
int
king_move_legality(int m, int n, int movem, int moven, struct Board *board, \
struct Action *legal_moves, int index)
{
    if (is_valid_target(m,n,movem,moven,board->all_pieces) && \
    !will_be_in_check(m,n,movem,moven,board) && \
    !kings_too_close(m,n,movem,moven,board))
        index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
    return index;
}

/* adds the legal moves one king can make */
int
legal_king_caps(int m, int n, struct Board *board, struct Action *legal_moves, 
int index)
{
    index = king_cap_legality(m,n,1,1,board,legal_moves,index);
    index = king_cap_legality(m,n,0,1,board,legal_moves,index);
    index = king_cap_legality(m,n,1,0,board,legal_moves,index);
    index = king_cap_legality(m,n,-1,-1,board,legal_moves,index);
    index = king_cap_legality(m,n,1,-1,board,legal_moves,index);
    index = king_cap_legality(m,n,-1,1,board,legal_moves,index);
    index = king_cap_legality(m,n,0,-1,board,legal_moves,index);
    index = king_cap_legality(m,n,-1,0,board,legal_moves,index);
    return index;
}

/* checks and adds one move that the king can make */
int
king_cap_legality(int m, int n, int movem, int moven, struct Board *board, \
struct Action *legal_moves, int index)
{
    if (is_on_board(m+movem, n+moven) && \
    board->all_pieces[m][n].colour != \
    board->all_pieces[m+movem][n+moven].colour && \
    board->all_pieces[m+movem][n+moven].type_val != EMPTY && \
    !will_be_in_check(m,n,movem,moven,board) && 
    !kings_too_close(m,n,movem,moven,board))
        index = add_one_legal_move(index,m,n,movem,moven,legal_moves,board);
    return index;
}

/* finds the strongest move for a colour in a position */
struct Action 
find_most_epic_quiet_move(struct Pair ab, int depth, int colour, 
struct Board *board)
{
    struct Action action;
    struct Action legal_moves[MOVES];
    int index;
    struct Board *new_board;
    int i = 0, eval;
    struct Pair explore;
    int node_type;
    
    index = add_all_legal_caps(colour,board,legal_moves);
    if (index == 0) {
        action.eval = total_val(board);
        return action;
    }
    legal_moves[index] = LASTACTION;
    q_sort_moves(colour, legal_moves, index);
    eval = total_val(board);
    explore = dont_explore(ab, colour, eval);
    if (explore.a) {
        legal_moves[0].eval = explore.b;
        legal_moves[1] = LASTACTION;
    } else {
	    ab = update_aB(ab,colour,eval,&node_type);
	    while (!is_last_action(&legal_moves[i])) {
	        new_board = copy_board(board); 
	        execute_cap(legal_moves[i].m, legal_moves[i].n, legal_moves[i].movem, \
	            legal_moves[i].moven, new_board);
	        action = find_most_epic_quiet_move(ab, depth-1, !colour, new_board);
	        delete_board(new_board);
	        explore = dont_explore(ab, colour, action.eval);
	        if (explore.a) {
	            legal_moves[i].eval = explore.b;
	            legal_moves[i+1] = LASTACTION;
	        	return legal_moves[i];
	        } else {
	            legal_moves[i].eval = action.eval;
	            ab = update_aB(ab,colour,action.eval,&node_type);
	            i++;
	        }
	    }
	}
    index = strongest_move_from_list(colour, legal_moves);
    return legal_moves[index];
}

/* finds the strongest move for a colour in a position */
struct Action 
find_most_epic_move(struct Pair ab, int depth, int colour, struct Board *board)
{
    struct Action action;
    struct Action legal_moves[MOVES];
    int index, i = 0;
	struct Board *new_board;
	struct Pair explore;
	int node_type = ALPHA;

    index = add_all_legal_moves(colour,board,legal_moves);
    if (index == 0) {
        if (colour) {
            action.eval = -1000000;
        } else {
            action.eval = 1000000;
        }
        return action;
    }
    if (depth == 0) {
        action = find_most_epic_quiet_move(ab,depth,colour,board);
        action.type = EXACT;
        return action;
    }
    legal_moves[index] = LASTACTION;
    q_sort_moves(colour, legal_moves, index);
    while (!is_last_action(&legal_moves[i])) {
        new_board = copy_board(board);
        execute_move(legal_moves[i].m, legal_moves[i].n, legal_moves[i].movem,  
            legal_moves[i].moven, new_board);
        action = find_most_epic_move(ab, depth-1, !colour, new_board);
        delete_board(new_board);
        explore = dont_explore(ab, colour, action.eval);
        if (explore.a) {
            legal_moves[i].eval = explore.b;
            legal_moves[i+1] = LASTACTION;
			legal_moves[i].type = BETA;
			return legal_moves[i];
        } else {
            legal_moves[i].eval = action.eval;
            ab = update_aB(ab,colour,action.eval,&node_type);
            i++;
        }
    }
    index = strongest_move_from_list(colour, legal_moves);
    legal_moves[index].type = node_type;
    return legal_moves[index];
}

inline int
geq(int a, int b)
{
	return a >= b;
}

inline int
leq(int a, int b)
{
	return a <= b;
}

/* returns whether a line should be explored and if not what eval to use */
struct Pair
dont_explore(struct Pair ab, int colour, int eval)
{
    struct Pair explore;
    int (*comp)(int, int);
    int new_eval;

    if (colour) {
    	comp = geq;
    	new_eval = ab.b;
    } else {
    	comp = leq;
    	new_eval = ab.a;
    }
    if (comp(eval, new_eval)) {
		explore.a = 1;
		explore.b = new_eval;
    } else
		explore.a = 0;
    return explore;
}

/* updates alpha and beta based on last eval */
struct Pair
update_aB(struct Pair ab, int colour, int eval, int *node_type)
{
    int (*comp)(int, int);
    int *bound;

	if (colour) {
		comp = greater_than;
		bound = &ab.a;
	} else {
		comp = less_than;
		bound = &ab.b;
	}
	if (comp(eval, *bound)) {
		*bound = eval;
		*node_type = EXACT;
	}
    return ab;
}

/* checks if an action is the last action marker */
int
is_last_action(struct Action *action)
{
    return action->m == LASTACTION.m && action->n == LASTACTION.n && \
    action->movem == LASTACTION.movem && action->moven == LASTACTION.moven && \
    action->eval == LASTACTION.eval;
}

/* returns the eval for a board */
int 
total_val(struct Board *board)
{
    int eval = 0;
    struct PieceNode *piece;

    piece = board->head_piece[WHITE];
    while (piece) {
		eval += board->all_pieces[piece->m][piece->n].type_val;
		eval += eval_piece(piece->m, piece->n, board->all_pieces);
		piece = piece->next;
    }
    piece = board->head_piece[BLACK];
    while (piece) {
		eval -= board->all_pieces[piece->m][piece->n].type_val;
		eval -= eval_piece(piece->m, piece->n, board->all_pieces);
		piece = piece->next;
    }
    return eval;
}


/* calculates the index of the pos for use with a bitboard */
int 
calc_index(int colour, int m, int n)
{
    if (colour == BLACK)
        m = 7 - m;
    return m * 8 + n;
}


/* returns the positional eval for a piece */
int
eval_piece(int m, int n, struct Piece all_pieces[8][8])
{
    switch(all_pieces[m][n].type_val) {
    case PAWN:
        return eval_pawn(calc_index(all_pieces[m][n].colour, m, n));
    case BISHOP:
        return eval_bishop(calc_index(all_pieces[m][n].colour, m, n));
    case KNIGHT:
        return eval_knight(calc_index(all_pieces[m][n].colour, m, n));
    case ROOK:
        return eval_rook(calc_index(all_pieces[m][n].colour, m, n));
    case KING:
        return eval_king(calc_index(all_pieces[m][n].colour, m, n)) +
		       king_safety(m, n, all_pieces);
    case QUEEN:
        return eval_queen(calc_index(all_pieces[m][n].colour, m, n));
    default :
        return 0;
    }
}

/* returns the positional eval for a knight */ 
int
eval_knight(int index)
{
    static const int squares[] = {-50,-40,-30,-30,-30,-30,-40,-50,
                                -40,-20,  0,  0,  0,  0,-20,-40,
                                -30,  0, 10, 15, 15, 10,  0,-30,
                                -30,  5, 15, 10, 10, 15,  5,-30,
                                -30,  0, 15, 20, 20, 15,  0,-30,
                                -30,  5, 10, 15, 15, 12,  5,-30,
                                -40,-20,  0,  5,  5,  0,-20,-40,
                                -50,-40,-30,-30,-30,-30,-40,-50};

    return squares[index];
}

/* returns the positional eval for a bishop */
int
eval_bishop(int index)
{
    static const int squares[] = {-20,-10,-10,-10,-10,-10,-10,-20,
                                -10,  0,  0,  0,  0,  0,  0,-10,
                                -10,  0,  5, 10, 10,  5,  0,-10,
                                -10, 10,  5, 10, 10,  5, 10,-10,
                                -10,  0, 10, 10, 10, 10,  0,-10,
                                -10, 10, 10, -0, -0, 10, 10,-10,
                                -10, 10,  5,  5,  5,  5, 10,-10,
                                -20,-10,-10,-10,-10,-10,-10,-20};
    
    return squares[index];
}

/* returns the position eval for a rook */
int
eval_rook(int index)
{
     static const int squares[] = { 0,  0,  0,  0,  0,  0,  0,  0,
                                  5, 10, 10, 10, 10, 10, 10,  5,
                                 -5,  0,  0,  0,  0,  0,  0, -5,
                                 -5,  0,  0,  0,  0,  0,  0, -5,
                                 -5,  0,  0,  0,  0,  0,  0, -5,
                                 -5,  0,  0,  0,  0,  0,  0, -5,
                                 -5,  0,  0,  0,  0,  0,  0, -5,
                                  0,  0,  0,  5,  5,  0,  0,  0};

    return squares[index];
}

/* returns the position eval for a pawn */
int 
eval_pawn(int index)
{
   static const int squares[] = { 0,  0,  0,  0,  0,  0,  0,  0,
                                50, 50, 50, 50, 50, 50, 50, 50,
                                10, 10, 20, 30, 30, 20, 10, 10,
                                 5,  5, 10, 20, 20, 10,  5,  5,
                                 0,  0,  0, 20, 20,  0,  0,  0,
                                 0,  0,  0,  0,  0,  0,  0,  0,
                                 5, 10, 10,-20,-20, 10, 10,  5,
                                 0,  0,  0,  0,  0,  0,  0,  0};

    return squares[index];
}

/* returns the position eval for a king */
int
eval_king(int index)
{
    static const int squares[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0,-20,-20,-20,0,0,
                                  0, 0,-10,-20,-20,-20,0,0};

    return squares[index];
}

/* returns the integrity of the pawns protecting the king */
int
king_safety(int m, int n, struct Piece all_pieces[8][8])
{
    int total = 0, dir = DIRECTION(all_pieces[m][n].colour), i;

    for (i = -1; i < 2; i++) {
        if (n+i <= 2 || n+i >= 5) {
            if (all_pieces[m+dir][n+i].type_val == PAWN && \
            all_pieces[m][n].colour == all_pieces[m+dir][n+i].colour) {
                total += 20;
            }
        }
    }
    return total;
}

/* returns the position eval for a queen */
int
eval_queen(int index)
{
    static const int squares[] = {-20,-10,-10, -5, -5,-10,-10,-20,
                                -10,  0,  0,  0,  0,  0,  0,-10,
                                -10,  0,  0,  0,  0,  0,  0,-10,
                                 -5,  0,  0,  0,  0,  0,  0, -5,
                                  0,  0,  0, -0, -0,  0,  0, -5,
                                -10,  0,  0, -0, -0,  0,  0,-10,
                                -10,  0,  0,  0,  0,  0,  0,-10,
                                -20,-10,-10, -5, -5,-10,-10,-20};

    return squares[index];
}

/* returns an int one further from zero than the int passed */
int
further_from_zero(int num)
{
    if (num < 0)
        return num - 1;
    else if (num > 0)
        return num + 1;
    else 
        return num;
}

/* returns an int one closer to zero than the int passed */
int
closer_to_zero(int num)
{
    if (num < 0)
        return num + 1;
    else if (num > 0)
        return num - 1;
    else
        return num;
}

/* returns whether an integer is whithin the bounds of a chess board */
int
is_in_bounds(int x)
{
    return x >= 0 && x <= 7;
}

/* returns whether a position is on a chess board */
int
is_on_board(int m, int n)
{
    return (is_in_bounds(m) && is_in_bounds(n));
}

/* returns whether a move obides by the universal chess rules */
int
is_valid_target(int m, int n, int movem, int moven, 
struct Piece all_pieces[8][8])
{
    return is_on_board(m+movem, n+moven) && 
    	(all_pieces[m+movem][n+moven].type_val == EMPTY || 
    	all_pieces[m+movem][n+moven].colour != all_pieces[m][n].colour);
}

/* returns whether a diagonal move path is clear or not */
int
diag_path_empty(int m, int n, int movem, int moven, 
struct Piece all_pieces[8][8])
{
    movem = closer_to_zero(movem);
    moven = closer_to_zero(moven);
    while (movem != 0) {
        if (all_pieces[m+movem][n+moven].type_val != EMPTY)
            return FALSE;
        movem = closer_to_zero(movem);
        moven = closer_to_zero(moven);
    }
    return TRUE;
}

/* returns whether a straight move path is clear or not */
int
straight_path_empty(int m, int n, int movem, int moven, \
struct Piece all_pieces[8][8])
{
    movem = closer_to_zero(movem);
    moven = closer_to_zero(moven);
    while (movem != moven) {
        if (all_pieces[m+movem][n+moven].type_val != EMPTY)
            return FALSE;
        if (movem == 0) {
            moven = closer_to_zero(moven);
        } else {
            movem = closer_to_zero(movem);
        }
    }
    return TRUE;
}

/* returns whether a move is L shaped or not */
int
is_lShaped(int m, int n)
{
    int am = abs(m);
    int an = abs(n);

    return (am == 2 && an == 1) || (am == 1 && an == 2);
}

/* returns whether a knight move is valid or not*/
int
valid_knight_move(int m, int n, int movem, int moven, 
struct Piece all_pieces[8][8])
{
    return is_lShaped(movem, moven) && 
    	is_valid_target(m, n, movem, moven, all_pieces);
}

/* returns whether a bishop move is valid or not*/
int
valid_bishop_move(int m, int n, int movem, int moven, struct Piece all_pieces[8][8])
{
    return abs(movem) == abs(moven) && 
	    is_valid_target(m, n, movem, moven, all_pieces)
	    && diag_path_empty(m, n, movem, moven, all_pieces);
}

/* returns whether a rook move is valid or not */
int
valid_rook_move(int m, int n, int movem, int moven,
struct Piece all_pieces[8][8])
{
    return (movem == 0 || moven == 0) && 
	    is_valid_target(m, n, movem, moven, all_pieces)
	    && straight_path_empty(m, n, movem, moven, all_pieces);
}

/* returns whether a king move is valid or not
 * not including castling */
int
valid_king_move(int m, int n, int movem, int moven, struct Board *board)
{
    if (abs(movem) > 1 || abs(moven) > 1)
        return 0;
    if (!is_valid_target(m, n, movem, moven, board->all_pieces))
        return 0;
    return !kings_too_close(m,n,movem,moven,board);
}

/* returns whether the opposing kings are too close to each other */
int
kings_too_close(int m, int n, int movem, int moven, struct Board *board)
{
    int colour, diffm, diffn, enemyn, enemym;

    colour = !board->all_pieces[m][n].colour;
    enemym = board->kingm[colour];
    enemyn = board->kingn[colour];
    diffm = (m+movem) - enemym;
    diffn = (n+moven) - enemyn;
    return abs(diffm) <= 1 && abs(diffn) <= 1;
}

/* returns whether a move is a valid castle */
int
valid_castle(int m, int n, int movem, int moven, struct Board *board)
{
    int side, start, end, i;

    if (!(board->all_pieces[m][n].mc == 0 && movem == 0 && \
    m == HOMEROW(board->all_pieces[m][n].colour) && 
    n == 4))
        return FALSE;
    if (moven == 2)
        side = 1;
    else if (moven == -2)
        side = 0;
    else
        return FALSE;
    if (side) {
        if (board->all_pieces[m][7].mc != 0 || \
        board->all_pieces[m][7].type_val != ROOK)
            return FALSE;
        start = 4;
        end = 6;
    } else {
        if (board->all_pieces[m][0].mc != 0 || \
        board->all_pieces[m][0].type_val != ROOK) {
            return FALSE;
        }
        start = 1;
        end = 4;
    }
    for (i = start; i <= end; i++) {
        if (board->all_pieces[m][i].type_val != KING \
        && board->all_pieces[m][i].type_val != EMPTY) {
            return FALSE;
        }
        if (is_threatened(m, i, board, board->all_pieces[m][n].colour)) {
            return FALSE;
        }
    }
    return TRUE;
}

/* returns whether a move is the most basic kind of pawn move */
int
is_basic_pawn_move(int colour, int movem, int moven)
{
    return moven == 0 && movem == DIRECTION(colour);
}

/* returns whether a move is a pawn forward two move
 * assumes target is empty */
int
is_pawn_forward_two(int m, int n, int movem, int moven, 
struct Piece all_pieces[8][8])
{
    return all_pieces[m][n].mc == 0 && moven == 0 && \
        movem == 2 * DIRECTION(all_pieces[m][n].colour) && \
        all_pieces[m+DIRECTION(all_pieces[m][n].colour)][n].type_val == EMPTY;

}

/* returns whether the movement of a pawn is a capture */
int
is_pawn_capture(int colour, int movem, int moven)
{
    return movem == DIRECTION(colour) && abs(moven) == 1;
}

/* returns whether a pawn move is valid
 * not including en passant */
int
valid_pawn_move(int m, int n, int movem, int moven, 
struct Piece all_pieces[8][8])
{
    int targetm, targetn;

    targetm = m+movem;
    targetn = n+moven;
    if (all_pieces[targetm][targetn].colour == all_pieces[m][n].colour)
        return FALSE;
    if (!is_on_board(targetm, targetn))
        return FALSE;
    if (all_pieces[targetm][targetn].type_val == EMPTY) {
        if (is_basic_pawn_move(all_pieces[m][n].colour, movem, moven))
            return TRUE;
        else if (is_pawn_forward_two(m, n, movem, moven, all_pieces))
            return TRUE;
        else
            return FALSE;
    } else {
        if (all_pieces[m][n].colour != all_pieces[targetm][targetn].colour && \
        is_pawn_capture(all_pieces[m][n].colour, movem, moven))
            return TRUE;
        else
            return FALSE;
    }
    return FALSE;
}

/* returns whether a move is a valid en passant */
int
valid_en_passant(int m, int n, int movem, int moven, 
struct Piece all_pieces[8][8])
{
    return is_pawn_capture(all_pieces[m][n].colour, movem, moven) &&
        all_pieces[m][n+moven].type_val == PAWN &&
        all_pieces[m][n].colour != all_pieces[m][n+moven].colour &&
        all_pieces[m][n+moven].mc == 2;
}

/* returns whether a move is valid or not */
int
valid_move(int m, int n, int movem, int moven, struct Board *board)
{
    switch(board->all_pieces[m][n].type_val) {
    case PAWN :
        return valid_pawn_move(m, n, movem, moven, board->all_pieces) || \
            valid_en_passant(m, n, movem, moven, board->all_pieces);
    case KNIGHT :
        return valid_knight_move(m, n, movem, moven, board->all_pieces);
    case BISHOP :
        return valid_bishop_move(m, n, movem, moven, board->all_pieces);
    case ROOK :
        return valid_rook_move(m, n, movem, moven, board->all_pieces);
    case QUEEN :
        return valid_rook_move(m, n, movem, moven, board->all_pieces) || \
            valid_bishop_move(m, n, movem, moven, board->all_pieces);
    case KING :
        return valid_king_move(m, n, movem, moven, board) || \
            valid_castle(m, n, movem, moven, board);
    default :
        return FALSE;
    }
}

/* returns whether a piece is threatened 
 * only used to check for check and castle validation */
int
is_threatened(int m, int n, struct Board *board, int colour)
{
    struct PieceNode *attacker;
    
    attacker = board->head_piece[!colour];
    while (attacker) {
        if (board->all_pieces[attacker->m][attacker->n].type_val != KING && 
        valid_move(attacker->m, attacker->n, m - attacker->m, n - attacker->n, 
        board)) {
            return TRUE;
        }
        attacker = attacker->next;
    }
    return FALSE;
}

/* returns whether a side is in check or not */
int
in_check(int colour, struct Board *board)
{
    return is_threatened(board->kingm[colour],board->kingn[colour],board,colour);
}

/* returns whether a side will be in check after a move is made */
int
will_be_in_check(int m, int n, int movem, int moven, struct Board *board)
{
    int check;
    struct Board *new_board = copy_board(board);

    execute_move(m, n, movem, moven, new_board);
    check = in_check(board->all_pieces[m][n].colour, new_board);
    delete_board(new_board);
    return check;
}

int
proper_print_board(struct Piece all_pieces[8][8], int colour)
{
    struct Piece piece;
    int m, n;
    char files[9];
    char ranks[] = "87654321";
    int dir, start, end;

    if (colour) {
        start = 0;
        end = 8;
        dir = 1;
        strcpy(files, "ABCDEFGH");
    } else {
        start = 7;
        end = -1;
        dir = -1;
        strcpy(files, "HGFEDCBA");
    }
    printf("  ");
    for (m = 0; m < 8; m++) {
        printf("%c ", files[m]);
    }
    printf("\n");
    for (m = start; m != end; m+=dir) {
        printf("%c ", ranks[m]);
        for (n = start; n != end; n+=dir) {
            if ((m + n) % 2) {
                printf("\033[0;44m"); /* set colour to dark gray */
            } else {
                printf("\033[0;47m"); /* set colour to light gray */
            }
            piece = all_pieces[m][n];
            printf("%c", get_piece_icon(piece.type_val, piece.colour));
            printf(" ");
            printf("\033[0m"); /* reset colour */
        }
        printf("%c", ranks[m]);
        printf("\n");
    }
    printf("  ");
    for (m = 0; m < 8; m++) {
        printf("%c ", files[m]);
    }
    printf("\n");
    return m * n;
}

/* prints the board */
int 
print_board(struct Piece all_pieces[8][8])
{
    struct Piece piece;
    int m, n;

    printf("  ");
    for (m = 0; m < 8; m++) {
        printf("%d ", m);
    }
    printf("\n");
    for (m = 0; m < SIDE; m++) {
        printf("%d ", m);
        for (n = 0; n < SIDE; n++) {
            piece = all_pieces[m][n];
            printf("%c", get_piece_icon(piece.type_val, piece.colour));
            printf("\033[0m"); /* reset colour */
            printf(" ");
        }
        printf("\n");
    }
    return m * n;
}

/* returns the icon to be printed for a piece type */
char 
get_piece_icon(int type_val, int colour)
{
    if (colour) {
        printf("\033[1;31m"); /* set colour to white */
    } else {
        printf("\033[1;30m"); /* set colour to black */
    }
    switch(type_val) {
    case KING :
        return 'k';
    case QUEEN :
        return 'q';
    case ROOK :
        return 'r';
    case BISHOP :
        return 'b';
    case KNIGHT :
        return 'n';
    case PAWN :
        return 'p';
    case EMPTY :
        return ' ';
    default :
        return '?';
    }
    return '?';
}

void
proper_move_prompt(struct Board *board, int colour)
{
    char buffer[20]; 
    char input[5];
    int invalid = 1;
    struct Action move;

    while (invalid) {
        printf("Enter the move you wish to make (eg e2e4): ");
        scanf("%s", buffer);
        my_strcpy(input, buffer, 4);
        move = parse_input(input); 
        if (valid_move(move.m, move.n, move.movem, move.moven, board)) {
            printf("move is valid\n");
        }
        if (board->all_pieces[move.m][move.n].colour == colour && 
        is_on_board(move.m, move.n) && 
        is_on_board(move.m+move.movem, move.n+move.moven) && 
        valid_move(move.m, move.n, move.movem, move.moven, board) && 
        !will_be_in_check(move.m,move.n,move.movem,move.moven,board)) {
            invalid = 0;
        } else {
            printf("That is an illegal move...\n");
        }
    }
    execute_move(move.m, move.n, move.movem, move.moven, board);
}

char *
my_strcpy(char *dest, char *src, int len)
{
    int i;

    for (i = 0; i < len; i++)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}

struct Action
parse_input(const char *input)
{
    struct Action move;
    int targetm, targetn;

    if (input[0] == 'q')
    	exit(1);
    printf("%s\n", input);
    move.m = 8 - atoi(&input[1]);
    move.n = file_to_row(input[0]);
    targetm = 8 - atoi(&input[3]);
    move.movem = targetm - move.m;
    targetn = file_to_row(input[2]);
    move.moven = targetn - move.n;
    return move;
}

/* converts a file letter to a matrix row number */
int
file_to_row(char f)
{
    char c = tolower(f); 
    int ret;

    c -= 97;
    if (c < 0 || c > 7)
        ret = -1;
    else
        ret = c;
    return ret;
}

struct PieceNode *
search_list(struct PieceNode *head, int m, int n)
{
    while (head) {
        if (head->m == m && head->n == n) {
            return head;
        } else {
            head = head->next;
        }
    }
    return NULL;
}

struct Board *
copy_board(struct Board *old_board)
{
    int m, n;
    struct Board *new_board = malloc(sizeof(struct Board));

    for (m = 0; m < SIDE; m++) {
        for (n = 0; n < SIDE; n++) {
            new_board->all_pieces[m][n] = old_board->all_pieces[m][n]; 
            new_board->all_pieces[m][n].node = NULL;
        }
    }
    for (m = 0; m < 2; m++) {
        new_board->kingm[m] = old_board->kingm[m];
        new_board->kingn[m] = old_board->kingn[m];
        copy_piece_list(old_board->head_piece[m], new_board, m); 
    }
    return new_board;
}

/* note: expects dest_head to already be malloced */
void
copy_piece_list(struct PieceNode *src_head, struct Board *dest_board, int colour)
{
    struct PieceNode *prev_node = NULL; 
    struct PieceNode *dest_node;
    int m, n;

    /* if src list is empty return NULL */
    if (!src_head) {
        dest_board->head_piece[colour] = NULL;
        dest_board->tail_piece[colour] = NULL;
        return;
    }
    /* if list only has one element */
    if (!src_head->next) {
        dest_board->head_piece[colour] = malloc(sizeof(struct PieceNode));
        *dest_board->head_piece[colour] = *src_head;
        dest_board->head_piece[colour]->prev = NULL;
        dest_board->head_piece[colour]->next = NULL;
        dest_board->tail_piece[colour] = dest_board->head_piece[colour];
        m = dest_board->head_piece[colour]->m;
        n = dest_board->head_piece[colour]->n;
        dest_board->all_pieces[m][n].node = dest_board->head_piece[colour];
        return;
    }
    dest_board->head_piece[colour] = malloc(sizeof(struct PieceNode));
    dest_node = dest_board->head_piece[colour];
    /* if list has more than one element */
    while (src_head) {
        *dest_node = *src_head; /* copy m and n */
        dest_node->prev = prev_node;
        dest_board->all_pieces[dest_node->m][dest_node->n].node = dest_node;
        prev_node = dest_node;
        dest_node->next = malloc(sizeof(struct PieceNode)); 
        dest_node = dest_node->next;
        src_head = src_head->next;
    }
    free(dest_node);
    prev_node->next = NULL;
    dest_board->tail_piece[colour] = prev_node;
}

void
delete_piece_list(struct PieceNode *head)
{
    /* if list is empty */
    if (!head)
        return;
    while (head->next) {
        head = head->next;
        free(head->prev);
    }
    free(head);
}

void
delete_board(struct Board *board)
{
    int i;

    for (i = 0; i < 2; i++) {
        delete_piece_list(board->head_piece[i]);
        board->head_piece[i] = NULL;
        board->tail_piece[i] = NULL;
    }
    free(board);
}

/* adds pieces to a board */
void 
add_all_pieces(struct Board *board)
{
    int i;

    for (i = 0; i < 2; i++) {
        board->head_piece[i] = NULL;
        board->tail_piece[i] = NULL;
    }
    add_pawns(1, BLACK, board);
    add_pawns(6, WHITE, board);
    add_two_pieces(0, ROOK, board);
    add_two_pieces(1, KNIGHT, board);
    add_two_pieces(2, BISHOP, board);
    add_two_pieces(3, QUEEN, board);
    add_two_pieces(4, KING, board);
    add_two_pieces(5, BISHOP, board);
    add_two_pieces(6, KNIGHT, board);
    add_two_pieces(7, ROOK, board);
    add_empty_squares(board->all_pieces);
    board->kingm[WHITE] = 7;
    board->kingn[WHITE] = 4;
    board->kingm[BLACK] = 0;
    board->kingn[BLACK] = 4;
}

/* pushes a piece to the list */
struct PieceNode *
push_piece_node(int m, int n, struct Board *board)
{
    struct PieceNode *new_node = malloc(sizeof(struct PieceNode));
    int colour = board->all_pieces[m][n].colour;
    
    new_node->m = m;
    new_node->n = n;
    new_node->next = NULL;
    new_node->prev = board->tail_piece[colour];
    if (board->tail_piece[colour] == NULL)  /* if list is empty */
        board->head_piece[colour] = new_node;     
    else
        board->tail_piece[colour]->next = new_node;
    board->tail_piece[colour] = new_node;
    return new_node;
}

/* pops from the tail of the piece list
   note: data still needs freeing */
struct PieceNode *
pop_piece_node(int colour, struct Board *board)
{
    struct PieceNode *old_tail = board->tail_piece[colour];
    struct PieceNode *new_tail; 

    if (!old_tail) /* if list is empty */
        return NULL;
    new_tail = old_tail->prev;
    if (new_tail) /* if there was more than 1 element */
        new_tail->next = NULL;
    board->tail_piece[colour] = new_tail;
    return old_tail;
}

/* removes a list from the list and frees the mem */
void
remove_piece_node(struct Board *board, struct PieceNode *piece_node)
{
    struct PieceNode *neighbour;
    int colour = board->all_pieces[piece_node->m][piece_node->n].colour; 

    if (piece_node == board->head_piece[colour]) {/* if this piece is the head */
        neighbour = piece_node->next;
        neighbour->prev = NULL;
        board->head_piece[colour] = neighbour;
    } else if (piece_node == board->tail_piece[colour]) { /* if this piece is the tail */
        neighbour = piece_node->prev;
        neighbour->next = NULL;
        board->tail_piece[colour] = neighbour;
    } else {
        neighbour = piece_node->next;
        neighbour->prev = piece_node->prev;
        neighbour = piece_node->prev;
        neighbour->next = piece_node->next;
    }
    free(piece_node);
}

/* adds a row of pawns to a board */
int 
add_pawns(int row, int colour, struct Board *board)
{
    int n;

    for (n = 0; n < SIDE; n++) {
        board->all_pieces[row][n].type_val = PAWN;
        board->all_pieces[row][n].colour = colour;
        board->all_pieces[row][n].mc = 0;
        board->all_pieces[row][n].node = push_piece_node(row, n, board);
    }
    return n;
}

/* adds a piece two both sides */
int 
add_two_pieces(int column, int type_val, struct Board *board)
{
    struct Piece piece;

    piece.mc = 0;
    piece.type_val = type_val;
    board->all_pieces[0][column] = piece;
    board->all_pieces[0][column].colour = BLACK;
    board->all_pieces[0][column].node = push_piece_node(0, column, board);
    board->all_pieces[7][column] = piece;
    board->all_pieces[7][column].colour = WHITE;
    board->all_pieces[7][column].node = push_piece_node(7, column, board);
    return 0;
}

/* adds the empty squares to the board */
int 
add_empty_squares(struct Piece all_pieces[8][8])
{
    int m, n;

    for (m = 2; m <= 5; m++) {
        for (n = 0; n < SIDE; n++) {
            all_pieces[m][n].type_val = EMPTY;
            all_pieces[m][n].mc = -1;
            all_pieces[m][n].colour = -1;
        }
    }
    return m * n;
}

/* executes a move on the board and returns the new board
 * assumes move is legal */
void
execute_move(int m, int n, int movem, int moven, struct Board *board)
{
    int colour = board->all_pieces[m][n].colour;

    switch (board->all_pieces[m][n].type_val) {
    case PAWN:
        if (abs(movem) == 2) {
            board->all_pieces[m][n].mc = 2;
        } else if (m + movem == HOMEROW(!board->all_pieces[m][n].colour)) {
            board->all_pieces[m][n].type_val = QUEEN;
        }
        if (moven != 0 && 
        board->all_pieces[m+movem][n+moven].type_val == EMPTY) {
            capture_en_passant(m, n, movem, moven, board);
        } else {
            /* if a capture */
            if (board->all_pieces[m+movem][n+moven].type_val) { 
                remove_piece_node(board, 
                	board->all_pieces[m+movem][n+moven].node);
            }
            board->all_pieces[m][n].node->m = m+movem; 
            board->all_pieces[m][n].node->n = n+moven; 
            board->all_pieces[m+movem][n+moven] = board->all_pieces[m][n];
            board->all_pieces[m][n] = EMPTYSQUARE;
        }
        break;
    case KING:
        board->kingm[board->all_pieces[m][n].colour] = m + movem; 
        board->kingn[board->all_pieces[m][n].colour] = n + moven; 
        if (abs(moven) > 1) {
            execute_castle(m, moven, board);
            break;
        }
    default:
        board->all_pieces[m][n].mc++;
        /* if target square not empty */
        if (board->all_pieces[m+movem][n+moven].type_val) { 
            remove_piece_node(board, board->all_pieces[m+movem][n+moven].node);
        }
        board->all_pieces[m][n].node->m = m+movem; 
        board->all_pieces[m][n].node->n = n+moven; 
        board->all_pieces[m+movem][n+moven] = board->all_pieces[m][n];
        board->all_pieces[m][n] = EMPTYSQUARE;
    }
    reset_enemy_pawns(colour, board->all_pieces);
}

/* executes a cap on the board and returns the new board
 * assumes move is legal */
void
execute_cap(int m, int n, int movem, int moven, struct Board *board)
{
    int colour = board->all_pieces[m][n].colour;

    if (board->all_pieces[m][n].type_val == KING) {
        board->kingm[colour] = m + movem;
        board->kingn[colour] = n + moven;
    }
    if (board->all_pieces[m][n].type_val == PAWN && \
    m + movem == HOMEROW(!colour)) {
            board->all_pieces[m][n].type_val = QUEEN;
    }
    board->all_pieces[m][n].mc++;
    if (board->all_pieces[m][n].type_val == PAWN && moven != 0 && \
    board->all_pieces[m+movem][n+moven].type_val == EMPTY) {
        capture_en_passant(m, n, movem, moven, board);
    } else {
        remove_piece_node(board, board->all_pieces[m+movem][n+moven].node);
        board->all_pieces[m][n].node->m = m+movem; 
        board->all_pieces[m][n].node->n = n+moven; 
        board->all_pieces[m+movem][n+moven] = board->all_pieces[m][n];
        board->all_pieces[m][n] = EMPTYSQUARE;
    }
    reset_enemy_pawns(colour, board->all_pieces);
}

/* resets the move count for the enemy pawns */
void
reset_enemy_pawns(int colour, struct Piece all_pieces[8][8])
{
    int m, n;

    m = PAWNHOMEROW(!colour) + 2 * DIRECTION(!colour);
    for (n = 0; n < SIDE; n++)
        all_pieces[m][n].mc = 0; 
}

/* executes a castle on the board */
void
execute_castle(int m, int moven, struct Board *board)
{
    int rookn, rookmoven, colour;

    colour = board->all_pieces[m][7].colour;
    board->kingn[colour] = 4 + moven;
    board->all_pieces[m][4].node->n = 4 + moven;
    board->all_pieces[m][4].mc++;
    board->all_pieces[m][4+moven] = board->all_pieces[m][4];
    board->all_pieces[m][4] = EMPTYSQUARE;
    if (moven == 2) {
        rookn = 7;
        rookmoven = -2;
    } else {
        rookn = 0;
        rookmoven = 3;
    }
    board->all_pieces[m][rookn].mc++;
    board->all_pieces[m][rookn].node->n = rookn + rookmoven;
    board->all_pieces[m][rookn+rookmoven] = board->all_pieces[m][rookn];
    board->all_pieces[m][rookn] = EMPTYSQUARE;
}

/* executes an en passant capture on the board */
void
capture_en_passant(int m, int n, int movem, int moven, struct Board *board)
{
    remove_piece_node(board, board->all_pieces[m][n+moven].node);
    board->all_pieces[m][n].node->m = m+movem;
    board->all_pieces[m][n].node->n = n+moven;
    board->all_pieces[m+movem][n+moven] = board->all_pieces[m][n];
    board->all_pieces[m][n] = EMPTYSQUARE;
    board->all_pieces[m][n+moven] = EMPTYSQUARE;
}

int 
main(int argc, char *argv[])
{
    struct Board *board = malloc(sizeof(struct Board));
    struct Action best_move;
    struct Pair ab;
    int start, end;
    int orientation = WHITE;
    int selfplay;
    int computer_colour = WHITE;

    if (argc == 1) {
        selfplay = 1; 
    } else {
        selfplay = 0;
        if (!strcmp(argv[1], "w")) {
            printf("player is white\n");
            orientation = WHITE;
        } else if (!strcmp(argv[1], "b")) {
            printf("player is black\n");
            orientation = BLACK;
        } else {
            selfplay = 1;
        }
        computer_colour = orientation;
    }
    ab.a = -2000000;
    ab.b = 2000000;
    add_all_pieces(board);
    best_move.m = 0;
    best_move.n = 0;
    best_move.movem = 0;
    best_move.moven = 0;
    if (orientation == BLACK) {
        proper_print_board(board->all_pieces, orientation);
        best_move = find_most_epic_move(ab, DEPTH, !orientation, board);
        execute_move(best_move.m, best_move.n, best_move.movem, \
            best_move.moven, board);
    }
    while (1) {
        proper_print_board(board->all_pieces, orientation);
        if (!selfplay) {
            proper_move_prompt(board, orientation);
            proper_print_board(board->all_pieces, orientation);
        } else {
            computer_colour = !computer_colour;
        }
        start = clock();
        best_move = find_most_epic_move(ab, DEPTH, !computer_colour, board);
        end = clock();
        printf("%lds\n", (end - start) / CLOCKS_PER_SEC);
        execute_move(best_move.m, best_move.n, best_move.movem, \
            best_move.moven, board);
    }
    return 0;
}
