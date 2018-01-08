/******************************************
 * C++ code for finding sudoku            *
 * written by Sergey Kuznetsov            *
 * USAGE: ./binary <dat file> or ./run.sh *
 * Requires: GCC, make, CPLEX             *
 * Distributed under GNU Public License-3 *
 *****************************************/

#include <ilcplex/ilocplex.h>
#include<vector>
extern "C"{
#include<math.h>
}

//********** Cache optimization (GCC only) ***********//
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)

ILOSTLBEGIN

typedef IloArray<IloNumVarArray> twoDarray;
typedef IloArray<IloArray<IloNumVarArray> > threeDarray;


int main (int argc, char **argv)
{
  IloEnv env;
  try {
    IloInt i, j, k;
    IloInt p,q;
    IloInt h;

    IloNumArray Grow(env);
    IloNumArray Gcol(env);
    IloNumArray Gval(env);
    IloNumVarArray var(env);
    IloInt N, M;

    const char* filename;
    if (argc > 1) 
      filename = argv[1];
    ifstream file(filename);
    if (!file) {
      cerr << "usage:   " << argv[0] << " <your input file>" << endl;
      throw(-1);
    }
    // Read parameters:
    file >> N >> Grow >> Gcol >> Gval;
    M = sqrt(N);
    // Check for consistency:
    IloBool consistentData1 = (Grow.getSize() == Gcol.getSize());
    IloBool consistentData2 = (Grow.getSize() == Gcol.getSize());
    IloBool consistentData = (consistentData1 && consistentData2);
    if (!consistentData) {
      cerr << "ERROR: data file contains inconsistent data" << endl;
      throw(-1);
    }
    IloBool consistentProg = (M*M == N);
    if (!consistentProg) {
      cerr << "ERROR: Bad sudoku: input n {M != sqrt(N)}" << endl;
      throw(-1);
    }
    IloBool consistentSudoku = (N<10 && N>0);
    if (!consistentSudoku) {
      cerr << "ERROR: Bad sudoku: input n {n<=0 or n>=10}" << endl;
      throw(-1);
    }

    // Decision variables:
    threeDarray x(env, N+1);
    for (i=0; i<=N; ++i)
      x[i] = twoDarray(env, N+1);
    for (i=0; i<=N; ++i) {
      for (j=0; j<=N; ++j)
        x[i][j] = IloNumVarArray(env, N+1, 0, 1, ILOINT);
    }

    vector<vector<int> > sudoku;
    sudoku = vector<vector<int> >(N+1, vector<int> (N+1, 0));
    // Create model:
    IloModel model(env);
    // Add constraints:
    for(k=1; k<=N; ++k) {
      for(j=1; j<=N; ++j) {
      	IloExpr sum(env);
      	for(i=1; i<=N; ++i)
      	  sum += x[i][j][k];
      	model.add(sum == 1);  // (1)
      	sum.end();
      }
    }
    for(k=1; k<=N; ++k) {
      for(i=1; i<=N; ++i) {
      	IloExpr sum(env);
      	for(j=1; j<=N; ++j)
      	  sum += x[i][j][k]; // (2)
      	model.add(sum == 1);
      	sum.end();
      }
    }
    for(i=1; i<=N; ++i) {
      for(j=1; j<=N; ++j) {
      	IloExpr sum(env);
      	for(k=1; k<=N; ++k)
      	  sum += x[i][j][k]; // (4)
      	model.add(sum == 1);
      	sum.end();
      }
    }
    for(k=1; k<=N; ++k) {
      for(p=1; p<=M; ++p) {
      	for(q=1; q<=M; ++q) {
      	  IloExpr sum(env);
      	  for(i=(M*(p-1)+1); i<=(M*p); ++i) {
      	    for(j=(M*(q-1)+1); j<=(M*q); ++j)
      	      sum += x[i][j][k];
	        }
  	      model.add(sum == 1); // (3)
          sum.end();
        }
      }
    }
    for(h=0; h<Grow.getSize(); ++h) {
      i = Grow[h];
      j = Gcol[h];
      k = Gval[h];
      model.add(x[i][j][k] == 1); // (5)
    }
    // Objective function:
    model.add(IloMinimize(env, 0));
    // Run CPLEX:
    IloCplex cplex(env);
    cplex.extract(model);
    // For solution pool:
    cplex.setParam(IloCplex::SolnPoolAGap, 0.0);
    cplex.setParam(IloCplex::SolnPoolIntensity,4); 
    if ( !cplex.solve() ) {
      env.error() << "Failed to solve sudoku" << endl;
      throw(-1);
    }
    
    // Solution pool:
    cplex.populate();
    /* Get the number of solutions in the solution pool */

    int numsol = cplex.getSolnPoolNsolns();
    env.out() << "The solution pool contains " << numsol << " solutions." << endl;

    /* Some solutions are deleted from the pool because of the solution
       pool relative gap parameter */

    int numsolreplaced = cplex.getSolnPoolNreplaced();
    env.out() << numsolreplaced << " solutions were removed due to the "
      "solution pool relative gap parameter." << endl;

    env.out() << "In total, " << numsol + numsolreplaced <<
      " solutions were generated." << endl;
    /* Get the average objective value of solutions in the solution
       pool */
    
    env.out() << "The average objective value of the solutions is " <<
      cplex.getSolnPoolMeanObjValue() << "." << endl << endl;

    std::cout << "************************** Initial Sudoku ****************************"  << endl;
    for(h=0; h<Grow.getSize(); ++h) {
      i = Grow[h];
      j = Gcol[h];
      k = Gval[h];
      sudoku[i][j] = k;
    }
    for(j=1; j<=N; ++j) {
      for(i=1; i<=N; ++i)
      	std::cout << sudoku[i][j] << '\t';
      std::cout << endl;
    }
    
    std::cout << "**************************  Final  Sudoku ****************************" << endl;    
    for(k=1; k<=N; ++k) {
      for(j=1; j<=N; ++j) {
      	for(i=1; i<=N; ++i){
      	  if(unlikely(cplex.getValue(x[i][j][k])==1)) {
      	    sudoku[i][j] = k;
      	  }
      	}
      }
    }
    for(j=1; j<=N; ++j) {
      for(i=1; i<=N; ++i)
      	std::cout << sudoku[i][j] << '\t';
      std::cout << endl;
    }

/*************************************** Another Sudoku ********************************************/

    if (numsol>1){
      std::cout << "*MASSAGE*: There exists more than one possible sudoku" << endl;
      std::cout << "Indicate which one you want to display ( indexed from 0 to " << (numsol - 1) << " )" << endl;
      int index;
      std::cin >> index;
      if ( (index<0) || (index>(numsol-1)) ) {
      	cerr << "Invalid index" << endl;
      	throw(-1);
      }
      vector<vector<int> > another;
      another = vector<vector<int> >(N+1, vector<int> (N+1, 0));

      IloNumVarArray tmp(env, N*N*N);
      h=0;
      for(k=1; k<=N; ++k){
      	for(j=1; j<=N; ++j){
      	  for(i=1; i<=N; ++i){
      	    tmp[h] = x[i][j][k];
      	    ++h;
      	  }
      	}
      }
      IloNumArray val(env, N*N*N);
      cplex.getValues(val, tmp, index);
      h=0;
      for(k=1; k<=N; ++k){
      	for(j=1; j<=N; ++j){
      	  for(i=1; i<=N; ++i){
      	    if (unlikely(val[h]!=0)){
      	      another[i][j] = k;
      	    }
      	    ++h;
      	  }
      	}
      }
    

      std::cout << "************************** Another Sudoku ****************************" << endl;    
      for(j=1; j<=N; ++j) {
      	for(i=1; i<=N; ++i)
      	  std::cout << another[i][j] << '\t';
      	std::cout << endl;
      }
    }          
  }

  catch(IloException& e) {
    cerr  << " ERROR: " << e << endl;   
  }
  catch(...) {
    cerr  << " ERROR" << endl;   
  }
  env.end();

  return 0;  
}
