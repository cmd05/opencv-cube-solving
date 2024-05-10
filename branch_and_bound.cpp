#include <iostream>
#include <queue>
constexpr int SIZE = 4;

// Branch and bound algorithm

// f = g + h
// score = depth(current level) + cost(to goal)
// prioritize low score node

struct Node {
    Node* parent;
    
    // store state of board
    int board[SIZE][SIZE];

    // coordinates of blank tile
    int x, y;

    // heuristic-cost (by number of misplaced tiles)
    int cost;

    int depth; // current level
};

void print_board(int board[SIZE][SIZE]) {
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++)
            std::cout << board[i][j] << ' ';
        std::cout << '\n';
    }
}

// allocate new node
// blank tile: x,y to new_x, new_y
// where x: row, y: col
Node* new_node(int board[SIZE][SIZE], int x, int y, int new_x, int new_y, int depth, Node* parent) {
    Node* node = new Node;

    node->parent = parent;

    // copy board to node
    memcpy(node->board, board, 4 * SIZE*SIZE); // 4 byte ints

    // swap specified tile and empty tile
    std::swap(node->board[x][y], node->board[new_x][new_y]);

    // set default cost
    node->cost = 16; // one more than maxm cost

    node->depth = depth;
    
    // save blank tile coordinates
    node->x = new_x;
    node->y = new_y;

    return node;
}

// Function to calculate the number of misplaced tiles
// ie. number of non-blank tiles not in their goal position
int calc_cost(int initial[SIZE][SIZE], int goal[SIZE][SIZE])
{
	int count = 0;
	for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (initial[i][j] && initial[i][j] != goal[i][j])
		        count++;
	return count;
}

// Function to check if (x, y) is a valid matrix move
// where x: row, y: col
bool is_valid_move(int x, int y)
{
    // valid row: 0 to SIZE-1
    // valid col: 0 to SIZE-1
	return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}

void print_path(Node* n) {
    if(!n)
        return;

    print_path(n->parent);
    print_board(n->board);
    std::cout << '\n';
}

int node_fcost(const Node* n) {
    return n->cost + n->depth;
}

// function object to compare nodes by cost
struct compare_nodes {
    bool operator()(const Node* lhs, const Node* rhs) const {
        return node_fcost(lhs) > node_fcost(rhs);
    }
};

struct Board_Info {
    int blank_x;
    int blank_y;
    bool solution_exists = false;
};

// If N is odd, then puzzle instance is solvable if number of inversions is even in the input state.
// If N is even, puzzle instance is solvable if 
//     the blank is on an even row counting from the bottom (second-last, fourth-last, etc.) and number of inversions is odd.
//     the blank is on an odd row counting from the bottom (last, third-last, fifth-last, etc.) and number of inversions is even.
// For all other cases, the puzzle instance is not solvable.

Board_Info solution_exists(int board[SIZE][SIZE]) {
    // flatten 2d array to 1d
    int arr[SIZE*SIZE];
    int blank_row = 0, blank_col = 0;

    for(int i = 0; i < SIZE; i++)
        for(int j = 0; j < SIZE; j++) {
            if(board[i][j] == 0) {
                blank_row = i;
                blank_col = j;
            }

            arr[i*SIZE + j] = board[i][j];
        }

    int inv = 0;

    for(int i = 0; i < SIZE*SIZE; i++)
        for(int j = i+1; j < SIZE*SIZE; j++)
            if(arr[i] && arr[j] && arr[i] > arr[j])
                inv++;

    bool sol = false;

    if(SIZE % 2 != 0) {
        // std::cout << inv;
        sol = (inv % 2 == 0);
    } else {
        int pos = SIZE-blank_row;
        sol = ((pos % 2 == 0 && inv % 2 != 0) || (pos % 2 != 0 && inv % 2 == 0));
    }

    return {blank_row, blank_col, sol};
}

void clean_pq(std::priority_queue<Node*, std::vector<Node*>, compare_nodes> pq) {
    while(!pq.empty()) {
        Node* n = pq.top();
        pq.pop();
        delete n;
    }
}

// solve n*n-1 puzzle by branch and bound method
void solve(int initial[SIZE][SIZE], int goal[SIZE][SIZE]) {
    Board_Info bi = solution_exists(initial);
    
    // check if solution exists first
    if(!bi.solution_exists)
        throw std::runtime_error("solution does not exist");

    int blank_x = bi.blank_x;
    int blank_y = bi.blank_y;

    // priority queue stores all current (live) nodes of search
    // sort by node_fcost
	// node with lowest cost has highest priority
    std::priority_queue<Node*, std::vector<Node*>, compare_nodes> pq;

	// start with root node
    // root node has no parent: nullptr
	Node* root = new_node(initial, blank_x, blank_y, blank_x, blank_y, 0, nullptr); // no swap initially
	root->cost = calc_cost(initial, goal);

	// add root to live tree
	pq.push(root);

    // change in row and column values for following moves:
    // bottom, left, top, right
    int row[] = { 1, 0, -1, 0 };
    int col[] = { 0, -1, 0, 1 };

    // find live nodes with least cost and add its children to list of live of nodes
    // till solution is found
	while (true)
	{
        // find live node with least cost
		Node* min = pq.top();

		// delete found node from pq
        // min still points to valid node
		pq.pop();

		// check if puzzle is solved
		if (min->cost == 0) {
            print_path(min);
            // clean_pq(pq); // free memory (freed on termination)
            return;
        }

        // check all possible children of min
        // i.e check all children of node with least cost in pq
		for (int i = 0; i < 4; i++)
		{
            int new_row = min->x + row[i];
            int new_col = min->y + col[i];

			if (is_valid_move(new_row, new_col))
			{
                
				Node* child = new_node(min->board, min->x, min->y, new_row, new_col, min->depth + 1, min);
				child->cost = calc_cost(child->board, goal);

				// Add child to list of live nodes
				pq.push(child);
			}
		}
	}
}


int main() {
    // change SIZE when using 3x3
    
	// int initial[SIZE][SIZE] =
	// {
	// 	{1, 2, 3},
	// 	{5, 6, 0},
	// 	{7, 8, 4}
	// };

	// int goal[SIZE][SIZE] =
	// {
	// 	{1, 2, 3},
	// 	{4, 5, 6},
	// 	{7, 8, 0}
	// };
    
    int initial[SIZE][SIZE] = {
        {10,6,4,3}, {1,5,8,0}, {2,13,7,15}, {14,9,12,11}
    };

    int goal[SIZE][SIZE] = {
        {1,2,3,4}, {5,6,7,8}, {9,10,11,12}, {13,14,15,0}
    };


    try {
	    solve(initial, goal);
    } catch(std::runtime_error& e) {
        std::cerr << e.what();
    }

	return 0;
}