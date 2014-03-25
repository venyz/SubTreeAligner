/*
 *  ProbabilityType.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 18.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifdef SEP_PROB_TYPE

#ifndef __PROBABILITYTYPE
#define __PROBABILITYTYPE

namespace bg_zhechev_ventsislav {
	class ProbabilityType {
		static const doubleType zeroProb;
		static const doubleType nanProb;
		
		double value;
	public:
		inline ProbabilityType() : value(zeroProb) {}
		inline ProbabilityType(const ProbabilityType& source) : value(source.value) {}
		inline ProbabilityType(const doubleType& v, bool force = false) { assert(force || v <= 1.); v ? value = log(v) : value = zeroProb; }
	private:
		inline ProbabilityType(const doubleType& val, const int&) : value(val) {}
		
	public:
		inline ProbabilityType operator*(const ProbabilityType& factor) const {
			if (value < zeroProb)
				if (factor.value < zeroProb) return ProbabilityType(value + factor.value, int());
				else return ProbabilityType(factor.value, int());
			else
				return ProbabilityType(value, int());
		}
		inline ProbabilityType& operator*=(const ProbabilityType& factor) {
			if (value < zeroProb) {
				if (factor.value < zeroProb) value += factor.value;
				else value = zeroProb;
			}
			return *this;
		}
		inline ProbabilityType& operator*=(const doubleType& factor) {
			assert(factor <= 1.);
			if (value < zeroProb) {
				if (factor) value += log(factor);
				else value = zeroProb;
			}
			return *this;
		}
		inline ProbabilityType operator/(const ProbabilityType& divisor) const {
			if (value < zeroProb)
				if (divisor.value < zeroProb) return ProbabilityType(value - divisor.value, int());
				else return ProbabilityType(nanProb, int());
			else
				return ProbabilityType(value, int());
		}
		friend inline ProbabilityType operator/(const doubleType& numerator, const ProbabilityType& divisor) { return ProbabilityType(numerator)/divisor; }
		inline ProbabilityType& operator/=(const ProbabilityType& divisor) {
			if (value < zeroProb) {
				if (divisor.value < zeroProb) value -= divisor.value;
				else value = nanProb;
			}
			return *this;
		}
		inline ProbabilityType& operator=(const ProbabilityType& value) { this->value = value.value; return *this; }
		
		inline bool operator==(const ProbabilityType& value) const { return this->value == value.value; }
		inline bool operator!=(const ProbabilityType& value) const { return this->value != value.value; }
		inline bool operator!() const { return this->value >= zeroProb; }
		inline bool operator<(const ProbabilityType& value) const { return this->value < value.value; }
		inline bool operator<=(const ProbabilityType& value) const { return this->value <= value.value; }
		inline bool operator>(const ProbabilityType& value) const { return this->value > value.value; }
		inline bool operator>=(const ProbabilityType& value) const { return this->value >= value.value; }
		inline bool operator&&(const bool term) { return value < zeroProb && term; }
		friend inline bool operator&&(const bool term, const ProbabilityType& num) { return term && num.value < zeroProb; }
		inline bool operator||(const bool term) { return value < zeroProb || term; }
		friend inline bool operator||(const bool term, const ProbabilityType& num) { return term || num.value < zeroProb; }
		
		friend inline wostream& operator<<(wostream& out, const ProbabilityType& value) {
			if (value.value == ProbabilityType::zeroProb) out << 0;
			else if (value.value == ProbabilityType::nanProb) out << "NaN";
			else out << exp(value.value);
			return out;
		}
		
		inline doubleType getDouble() const { if (value == zeroProb) return 0.; if (value == nanProb) return 0.; return exp(value); }
		
		static const ProbabilityType Zero;
	};
}

#endif //__PROBABILITYTYPE	
#endif //SEP_PROB_TYPE