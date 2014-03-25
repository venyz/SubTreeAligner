/*
 *  test.cpp
 *  
 *
 *  Created by Венцислав Жечев on 05.11.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

//#include <string>
//#include <iostream>
//#include <sstream>
//#include <cmath>
using namespace std;

#include "ProbabilityType.h"
#include "TreePair.h"
//#include "LinkSet.h"
//#include "LinkPowerSetRow.h"

//#define STR_POS_TYPE unsigned int
//typedef STR_POS_TYPE str_pos_type;

int main() {
	wcout.unsetf(ios::fixed);
	wcout.precision(16);
	
	Tree::binariseMode = false;
//	TreePair treePair;
//	wstringstream in(L"( TOP ( ROOT ( XXX ( Pro Je ) ( VCONJS déclare ) ( Nom?S reprise ) ) ( XXX ( Det?? la ) ( NomFS session ) ( XXX ( Prep du ) ( XXX ( NomMS Parlement ) ( Adj?? européen ) ) ) ) ( XXX ( ProRel qui ) ( XXX ( VCONJS avait ) ( XXX ( PpaMSp été ) ( PpaFS interrompue ) ) ) ( XXX ( Det?? le ) ( NomXXDate vendredi ) ) ( XXX ( DetNum 17 ) ( Nom?S décembre ) ( Adj?? dernier ) ) ( CCoordVCONJS et ) ( Pro je ) ( Pro vous ) ( VCONJS renouvelle ) ) ( Pro tous ) ( XXX ( DetMP mes ) ( NomInc vux ) ) ( XXX ( Prep en ) ( XXX ( Ppr espérant ) ( CSub que ) ) ) ( XXX ( Pro vous ) ( VCONJP avez ) ( XXX ( PpaMS passé ) ( XXX ( Prep de ) ( XXX ( Adj?? bonnes ) ( Nom?P vacances ) ) ) ) ) )  ( EOS <eos> ) )\n( TOP ( ROOT ( XXX ( Pro I ) ( VCONJ declare ) ) ( Ppa resumed ) ( XXX ( Det the ) ( Nom?S session ) ( XXX ( Prep of ) ( XXX ( Det the ) ( Nom?S European ) ( Nom?S Parliament ) ( XXX ( Ppa adjourned ) ( XXX ( Prep on ) ( NomXXDate ( NomXXDate_mwu Friday ) ( NomXXDate_mwu 17 ) ( NomXXDate_mwu December ) ( NomXXDate_mwu 1999 ) ) ) ) ) ) ) ( Typo , ) ( CCoord and ) ( XXX ( Pro I ) ( VCONJ ( VCONJ_mwu would ) ( VCONJ_mwu like ) ) ( Adv once ) ( Adv again ) ( XXX ( Prep to ) ( XXX ( VINF wish ) ( Pro you ) ( Det a ) ( Adj happy ) ( Adj new ) ( NomXXDate year ) ) ) ) ( XXX ( Prep in ) ( XXX ( Det the ) ( Nom?S hope ) ) ) ( ProRel that ) ( XXX ( Pro you ) ( VCONJ enjoyed ) ( XXX ( Det a ) ( Adj pleasant ) ( Adj festive ) ( Nom?S period ) ) ) )  ( EOS <eos> ) )\n\n");
//	in >> treePair;
////	wcout << treePair << endl;
//	treePair.printBracketed(wcout);
	
	Tree tree;
//	wstringstream in(L"(NP (PRN (-LRB- () (NP (NNP TEU)) (-RRB- ))) (NN bla))");
	wstringstream in(L"(SQ (MD Could) (NP (DT the) (NNP Commission)) (VP (VB please) (S (VP (VB tell) (NP (NNP Parliament)) (NP (NP (DT the) (NNS circumstances)) (CC and) (NP (DT the) (NNS groups))) (PP (IN within) (NP (NP (NN Article) (CD 13)) (PRN (-LRB- () (NP (NNP TEU)) (-RRB- ))) (ADJP (JJ likely) (S (VP (TO to) (VP (VB be) (VP (VBN effected) (PP (IN by) (NP (PDT such) (DT an) (NN exemption))))))))))))) (? ?))");
	in >> tree;
	tree.printBracketed(wcout);
	
//	wstring str(L"blablabla");
//	str_pos_type i = str.find('b');
//	wcout << "wstring::npos is " << wstring::npos << endl;
//	if (i == wstring::npos)
//		return 1;
//	
//	float bla;
//	wstringstream stream(L"0.14285719");
//	stream >> bla;
//	wcout << "bla: " << bla << endl;
	
	/*double one = 0.008130099999999999;
//	double two = 0.0081301;
	long double two = 0.0006061 - (0.0006060 + 0.0000001);
	wcout << one << "->" << log(one) << "/" << two << "->" << log(two) << endl;
	wcout << (one == two ? "equal" : "not equal") << endl;
	
	wcout << one*one << "/" << two*two << endl;
	wcout << exp(2.*log(one)) << "/" << exp(2.*log(two)) << endl;
	
	wcout.setf(ios::fixed);

	unsigned long problemCount = 0, totalCount = 0;
	for (double three = 0.; three <= 1.; three += 0.0000001) {
		if (three*three != exp(log(three)+log(three))) {
//		if (three != exp(log(three))) {
//			wcout << "Problems with " << three << " !" << endl;
			++problemCount;
		} else {
//			wcout << "No problems with " << three << " ." << endl;
		}
		++totalCount;
		if (totalCount % 100000 == 0)
			wcout << "  " << totalCount << " so far, " << problemCount << " problems. Текущо число: " << three << endl;
	}
	wcout << problemCount << " problems out of " << totalCount << " total. (" << (1.*problemCount/totalCount)*100 << ")" << endl;*/
	
//	typedef map<wstringSet*, string> strangeType;
	
//	wcout << "size of int is " << sizeof(int) << endl;
//	wcout << "size of bool is " << sizeof(bool) << endl;
//	wcout << "size of LinkSet* is " << sizeof(LinkSet*) << endl;
//	wcout << "size of linkAddressPair* is " << sizeof(linkAddressPair*) << endl;
//	wcout << "size of linkAddressPair is " << sizeof(linkAddressPair) << endl;
//	wcout << "size of doubleType* is " << sizeof(doubleType*) << endl;
//	wcout << "size of doubleType is " << sizeof(doubleType) << endl;
//	wcout << "size of LinkSet is " << sizeof(LinkSet) << endl;
//	wcout << "size of setOfLinks is " << sizeof(setOfLinks) << endl;
//	wcout << "size of LinkPowerSetRow::value_type is " << sizeof(LinkPowerSetRow::value_type) << endl;
//	
//	doubleType factorial(doubleType);
//	unsigned total = 57;
//	unsigned crossings = 14;
//	wcout << endl << "For a total of " << total << " links with " << crossings << " crossings we have maximal middle row with " << factorial(total)/(factorial(total/2)*factorial(total - total/2)) << " elements." << endl;
//	wcout << "The total number of elements is " << exp2(total) - 1 << "." << endl;
//	wcout << "We have in all the following rows:" << endl;
//	for (unsigned i = 1; i <= total; ++i) {
//		wcout << "row №" << i << ": " << factorial(total)/(factorial(i)*factorial(total - i)) << endl;
//	}
	
	return 0;
}

//doubleType factorial(doubleType num) {
//	doubleType result = 1.;
//	for (doubleType i = 2.; i <= num; i += 1.)
//		result *= i;
//	return result;
//}

///*
// * combined.c
// *
// * This program combines what we saw before.  It calculates e and pi
// * and then integrates the x^2.  We also print out the elapsed time in
// * ms at several points in our program.  We have replaced the function y=x^2
// * with a more complex polynomial 3x^3 + 2x^2 + x.
// */
//
//#include <cstdio>
//#include <ctime> 
//
//#define num_steps 10000000 /* steps to use in Taylor expansions */ 
//#define int_steps (1<<30)  /* steps to use in integration */ 
//
//int main()
//{
//  double start, stop; /* times of beginning and end of procedure */
//	
//  /* Values for part 1 */
//  double e, pi, factorial, product;
//  int i;
//	
//  /* Values for part 2 */
//  double sum = 0;
//  double x;
//	
//  /* start the timer */
//  start = clock();
//	
//#ifdef _OPENMP
//#pragma omp parallel reduction(+: sum) 
//#endif
//  {
//#ifdef _OPENMP
//#pragma omp sections nowait 
//#endif
//    {
//#ifdef _OPENMP
//#pragma omp section 
//#endif
//      {
//        /* First we calculate e from its Taylor expansion */
//        printf("e started at %.0f\n", clock()-start);
//        e = 1;
//        factorial = 1;
//        for (i = 1; i<num_steps; i++) {
//          factorial *= i;
//          e += 1.0/factorial;
//        }
//        printf("e is %f\n", e);
//        printf("e done at %.0f\n", clock()-start);
//      }
//#ifdef _OPENMP
//#pragma omp section 
//#endif
//      {
//        /* Then we calculate pi from its Taylor expansion */
//        printf("pi started at %.0f\n", clock()-start);
//        
//        pi = 0;
//        for (i = 0; i < num_steps*20; i++) {
//          pi += 1.0/(i*4.0 + 1.0);
//          pi -= 1.0/(i*4.0 + 3.0);
//        }
//        pi = pi * 4.0;
//        printf("pi is %f\n", pi);
//        printf("pi done at %.0f\n", clock()-start);
//      }
//    } /* sections */
//		
//    /* Now we integrate the function */
//    printf("integration started at %.0f\n", clock()-start);
//    sum = 0;
//#ifdef _OPENMP
//#pragma omp for nowait 
//#endif
//    for (i = 0; i<int_steps; i++) {
//      x = 2.0 * (double)i / (double)(int_steps); /* value of x */
//      sum += ( 3*x*x*x + 2*x*x + x ) / int_steps;
//    }
//		
//#ifdef _OPENMP
//#pragma omp single /* we only need to print this once */ 
//#endif
//    printf("integration done at %.0f\n", clock()-start);
//		
//		
//#ifdef _OPENMP		
//#pragma omp barrier 
//#endif
//    /* make sure all threads are caught up before we do the multiplication */
//    product = e * pi;
//		
//  } /* omp parallel */
//	
//  /* we're done so stop the timer */
//  stop = clock();
//	
//  printf("Values: e*pi = %f,  integral = %f\n", product, sum);
//  printf("Total elapsed time: %.3f seconds\n", (stop-start)/1000);
//	
//	
//	wcout << "Size of const int is " << sizeof(const int) << endl;
//	wcout << "Size of const int& is " << sizeof(const int&) << endl;
//	
//  return 0;
//}