//----------------------------------------------------------
// Copyright 2017 University of Oxford
// Written by Michael A. Boemo (mb915@cam.ac.uk)
// This software is licensed under GPL-2.0.  You should have
// received a copy of the license with this software.  If
// not, please Email the author.
//----------------------------------------------------------

#include <algorithm>
#include <exception>
#include <vector>
#include "poreModels.h"
#include "error_handling.h"
#define _USE_MATH_DEFINES

class NegativeLog : public std::exception {
	public:
		virtual const char * what () const throw () {
			return "Negative value passed to natural log function.";
		}
};


class DivideByZero : public std::exception {
	public:
		virtual const char * what () const throw () {
			return "lnQuot: Cannot divide by zero.";
		}
};


/*
Functions for computing probabilities in log space to improve numerical stability.  Most algorithms 
inspired by Numerically Stable Hidden Markov Model Implementation by Tobias P. Mann
(see http://bozeman.genome.washington.edu/compbio/mbt599_2006/hmm_scaling_revised.pdf).
*/

inline double eexp( double x ){
/*Map from log space back to linear space. */

	if ( std::isnan( x ) ) {
		return 0.0;
	}	
	else {
		return exp( x );
	}
}


inline double eln( double x ){
/*Map from linear space to log space. */    

	if (x == 0.0){
		return NAN;
	}
	else if (x > 0.0){
		return log( x );
	}
	else{
		throw NegativeLog();
	}
}


inline double lnSum( double ln_x, double ln_y ){
/*evalutes the quotient ln_x + ln_y */    
    
	/*if one of the arguments is NAN, go into special handling for that */
	if ( std::isnan( ln_x ) || std::isnan( ln_y ) ){

        	if ( std::isnan( ln_x ) && std::isnan( ln_y ) ){
			return NAN;
		}
		else if ( std::isnan( ln_x ) ){
			return ln_y;
		}
		else{
			return ln_x;
		}
	}
	/*Otherwise, compute the output.  For numerical stability, we always want to subtract the larger argument from the smaller argument in the exponential term. */
	else{
	
		if ( ln_x > ln_y ){
			return ln_x + eln( 1.0 + eexp( ln_y - ln_x ) );
		}
		else{
			return ln_y + eln( 1.0 + eexp( ln_x - ln_y ) );
		}
	}
}


inline double lnProd( double ln_x, double ln_y ){
/*evalutes the quotient ln_x*ln_y */

	if ( std::isnan( ln_x ) || std::isnan( ln_y ) ){
		return NAN;
	}
	else{
		return ln_x + ln_y;
	}
}


inline double lnQuot( double ln_x, double ln_y ){
/*evalutes the quotient ln_x/ln_y */

	if ( std::isnan( ln_y ) ){
        	throw DivideByZero();
	}
	else if ( std::isnan( ln_x ) ){
		return NAN;
	}
	else{
		return ln_x - ln_y;
	}
}


inline bool lnGreaterThan( double ln_x, double ln_y ){
/*evalutes whether ln_x is greater than ln_y, and returns a boolean */
    	
	if ( std::isnan( ln_x ) || std::isnan( ln_y ) ){
		
		if ( std::isnan( ln_x ) || std::isnan( ln_y ) == false ){
			return false;
		}
        	else if ( std::isnan( ln_x ) == false || std::isnan( ln_y ) ){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if ( ln_x > ln_y ){
			return true;
		}
		else{
			return false;
		}
	}
}


inline double uniformPDF( double lb, double ub, double x ){

	if ( x >= lb && x <= ub ){

		return 1.0/( ub - lb );
	}
	else {
		return 0.0;
	}
};


inline double normalPDF( double mu, double sigma, double x ){

	return ( 1.0/sqrt( 2.0*pow( sigma, 2.0 )*M_PI ) )*exp( -pow( x - mu , 2.0 )/( 2.0*pow( sigma, 2.0 ) ) );
}

inline double KLdivergence( double mu1, double sigma1, double mu2, double sigma2 ){

	return log(sigma2 / sigma1) + (pow(sigma1,2) + pow((mu1 - mu2),2))/(2.0*pow(sigma2,2)) - 0.5;
}
