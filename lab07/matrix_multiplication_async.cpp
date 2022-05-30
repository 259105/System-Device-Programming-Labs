#include <iostream>
#include <vector>
#include <numeric>
#include <future>

using namespace std;

int computeElement(vector<int> &row, shared_future<vector<int>> &fCol, int n){
    vector<int> col = fCol.get(); // wait until the col is complete transposed
    return inner_product(row.begin(), row.end(), col.begin(), 0);
}

vector<int> transposeCol2Row(vector<vector<int>> &M, int nrow, int col){
    vector<int> v;
    for(int i=0; i<nrow; i++){
        v.emplace_back(M[i][col]);
    }
    return v;
}

vector<vector<int>> computeMatrix(vector<vector<int>> &A, vector<vector<int>> &B, int p, int n, int q){
    vector<shared_future<vector<int>>> sfBT(q);
    vector<vector<future<int>>> fC(p);
    vector<vector<int>> C(p);

    // transpose the matrix B
    for( int i=0 ; i<q; i++){
        sfBT[i] = ( async(launch::async, transposeCol2Row, ref(B), n, i) ).share();
        // sfBT[i] is a shared future
    }

    // compute each element
    for(int i=0; i<p; i++){
        fC[i].resize(q);
        for(int j=0; j<q; j++){
            fC[i][j] = async(launch::async, computeElement, ref(A[i]), ref(sfBT[j]), n);
            // it doesn't work because a future is got more than one time, so we need to change method
            // change 1 : use the shared futures
            // change 2 : i've used move(sfBT[j]) to pass the shared_future, but with move i lose that row,
            //          and i neee that row for the next iteration
        }
    }

    // retrieve the values
    for(int i=0; i<p; i++){
        C[i].resize(q);
        for(int j=0; j<q; j++){
            C[i][j] = fC[i][j].get();
        }
    }
    return C;
}

vector<vector<int>> generateMatrix(int nR, int nC){
    vector<vector<int>> M;
    srand((unsigned int)(nR*nC));
    for(int i=0;i<nR;i++){
        vector<int> row(nC);
        M.emplace_back(row);
        for(int j=0;j<nC;j++){
            M[i][j] = rand() % 9 + 1;
        }
    }
    return M;
}

void printMatrix(auto A){
    for(auto& row : A){
        for(auto& element : row){
            cout << element << " " ;
        }
        cout << endl;
    }
}

int main(){
    int p, q, n1, n2;

    cout << "Insert num rows of matrix A" << endl;
    cin >> p;
    cout << "Insert num columns of matrix A" << endl;
    cin >> n1;
    cout << "Insert num rows of matrix B" << endl;
    cin >> n2;
    if(n2 != n1){
        cerr << "Matrices are not compatible" << endl;
        return 1;
    }
    cout << "Insert num columns of matrix B" << endl;
    cin >> q;

    auto A = generateMatrix(p, n1);
    auto B = generateMatrix(n2, q);
    //printMatrix(A);
    //printMatrix(B);
    auto C = computeMatrix(A, B, p, n1, q);
    printMatrix(C);

    return 0;
}
