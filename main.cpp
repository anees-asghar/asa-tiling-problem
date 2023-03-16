#include <iostream>
#include <vector>
#include <stack>
#include <unordered_map>

typedef uint64_t u64;

using std::vector;
using std::cout;
using std::cin;
using std::string;
using std::stack;
typedef std::unordered_map<u64, u64> hashtable;


struct backtrace {
    int tileSize, cursorRow;
}; // represents tile of size tileSize has to be removed from the end of row cursorRow


class Grid {
public:
    vector<int> rowSizes;
    int cursor = 0;

    Grid(vector<int> rowSizes) : rowSizes(std::move(rowSizes)){ }

    void setCursorAtRow(int pos){
        cursor = pos;
    }

    bool isUnbranchable() const {
        // is unbranchable if area to be tiled is a single row, or if it is an L shape
        return rowSizes[rowSizes.size()-2] <= 1;
    }

    int getMaxTileSize() const {
        return std::min(rowSizes[cursor], (int)rowSizes.size()-cursor);
    }

    bool canRemoveTileOfsize(int n) const {
        if (cursor > 0){
            if (rowSizes[cursor-1] > rowSizes[cursor]-n) return false;
        }
        return rowSizes[cursor+n-1] == rowSizes[cursor];
    }

    void removeTileOfSize(int n){
        for (int i = cursor; i < cursor+n; i++){
            rowSizes[i] -= n;
        }
    }

    bool isEmpty() const {
        return rowSizes.size() == 0 || rowSizes.back() == 0;
    }

    void clean(){ // remove empty rows
        int i = 0;
        for (; i < (int)rowSizes.size() && rowSizes[i] == 0; i++);
        rowSizes.erase(rowSizes.begin(), rowSizes.begin() + i);
    }

    u64 hash() const {
        u64 seed = rowSizes.size();
        for(int x : rowSizes) {
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = (x >> 16) ^ x;
            seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }

    u64 hashWithBTStack(stack<backtrace> btStack){
        u64 seed = rowSizes.size() | btStack.size();
        for(int x : rowSizes) {
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = (x >> 16) ^ x;
            seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        while(!btStack.empty()){
            backtrace b = btStack.top();
            btStack.pop();
            seed ^= b.tileSize + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= b.cursorRow + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }

    bool tileRemovalIsValid(backtrace bt, int squareSize) const {
        // removal is valid if it does not affect the tile removal in backtrace
        return (squareSize + rowSizes[bt.cursorRow] <= rowSizes[bt.tileSize-1+bt.cursorRow]) &&
               ((cursor>0 && rowSizes[cursor]-squareSize>=rowSizes[cursor-1]) || cursor == 0);
    }
};


hashtable cache, cacheWithBTStack;
u64 solve(Grid& grid, stack<backtrace>& backTraceStack);


void readInput(vector<int>& rowSizes){
    int n, m; // n - rows, m - cols
    cin >> n >> m;
    rowSizes.resize(n);
    for (int i = 0; i < n; i++){
        cin >> rowSizes[i];
    }
}


int main(){
    vector<int> rowSizes;
    readInput(rowSizes);

    Grid grid(rowSizes);

    stack<backtrace> backTraceStack;
    if (grid.isEmpty()){
        cout << "0" << std::endl;
    } else {
        u64 result = solve(grid, backTraceStack);
        std::cout << result << std::endl;
    }
    return 0;
}


u64 inline solveAndStore(Grid& grid, stack<backtrace>& backTraceStack){
    u64 result = solve(grid, backTraceStack);
    u64 gridHash = grid.hash();
    if (cache.find(gridHash)==cache.end()){
        cache[gridHash] = result;
    }
    return result;
}


u64 inline solveAndStoreWithBTStack(Grid& grid, stack<backtrace>& backTraceStack){
    u64 result = solve(grid, backTraceStack);
    u64 gridHash = grid.hashWithBTStack(backTraceStack);
    if (cacheWithBTStack.find(gridHash)==cacheWithBTStack.end()){
        cacheWithBTStack[gridHash] = result;
    }
    return result;
}


u64 solve(Grid& grid, stack<backtrace>& backTraceStack) {
    grid.clean();

    if (grid.isEmpty() || grid.isUnbranchable())
        return 1;

    // if result of this problem is already stored in one of the caches, return result
    if (backTraceStack.empty()) {
        u64 gridHash = grid.hash();
        if (cache.find(gridHash) != cache.end()){
            return cache[gridHash];
        }
    } else {
        u64 gridHash = grid.hashWithBTStack(backTraceStack);
        if (cacheWithBTStack.find(gridHash) != cacheWithBTStack.end()){
            return cacheWithBTStack[gridHash];
        }
    }

    u64 permutations = 0;
    int maxTileSize = grid.getMaxTileSize();

    for (int size = 1; size <= maxTileSize; size++){
        if (!backTraceStack.empty() && !grid.tileRemovalIsValid(backTraceStack.top(), size))
            continue;
        
        Grid gridCopy = grid;

        if (!grid.canRemoveTileOfsize(size)){
            stack<backtrace> stackCopy;
            if (!backTraceStack.empty()) stackCopy = stack<backtrace>(backTraceStack);
            stackCopy.push({size, grid.cursor}); // push to stack for removal when possible

            // find the next row with rowSize > current row size
            for (int x = grid.cursor+1; x<(int)grid.rowSizes.size(); x++){
                if (grid.rowSizes[x] > grid.rowSizes[grid.cursor]){
                    gridCopy.setCursorAtRow(x);
                    break;
                }
            }
            permutations += solveAndStoreWithBTStack(gridCopy, stackCopy);
            continue;
        }

        // can remove tile of size 'size'
        gridCopy.removeTileOfSize(size);

        if (backTraceStack.empty()){
            permutations += solveAndStore(gridCopy, backTraceStack);
            continue;
        }

        // backtrace stack isn't empty
        stack<backtrace> stackCopy = stack<backtrace>(backTraceStack);
        int cursorRow = gridCopy.cursor; // store current cursorRow

        while (!stackCopy.empty()){
            backtrace bt = stackCopy.top();
            gridCopy.setCursorAtRow(bt.cursorRow);

            if (gridCopy.canRemoveTileOfsize(bt.tileSize)){
                stackCopy.pop();
                gridCopy.removeTileOfSize(bt.tileSize);
                cursorRow = bt.cursorRow;
                if (stackCopy.empty()){
                    permutations += solveAndStore(gridCopy, stackCopy);
                }
            } else {
                for (int x = cursorRow; x < (int)gridCopy.rowSizes.size(); x++){
                    if (gridCopy.rowSizes[x] > gridCopy.rowSizes[gridCopy.cursor]) {
                        gridCopy.setCursorAtRow(x);
                        break;
                    }
                }
                permutations += solveAndStoreWithBTStack(gridCopy, stackCopy);
                break;
            }
        }
        

    }

    return permutations;
}
