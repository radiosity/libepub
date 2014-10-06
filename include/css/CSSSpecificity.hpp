/*
Copyright (c) 2014, Richard Martin
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Richard Martin nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL RICHARD MARTIN BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CSS_SPECIFICITY_HEADER
#define CSS_SPECIFICITY_HEADER

class CSSSpecificity {

	private:
		unsigned int a;
		unsigned int b;
		unsigned int c;
		unsigned int d;

	public:
		CSSSpecificity();
		CSSSpecificity(const unsigned int _a, const unsigned int _b, const unsigned int _c, const unsigned int _d);

		CSSSpecificity(CSSSpecificity const & cpy);
		CSSSpecificity(CSSSpecificity && mv) ;
		CSSSpecificity & operator =(const CSSSpecificity & cpy);
		CSSSpecificity & operator =(CSSSpecificity && mv) ;

		~CSSSpecificity();

		friend inline bool operator==(const CSSSpecificity & lhs, const CSSSpecificity & rhs);
		friend inline bool operator!=(const CSSSpecificity & lhs, const CSSSpecificity & rhs);

		friend inline bool operator< (const CSSSpecificity & lhs, const CSSSpecificity & rhs);
		friend inline bool operator> (const CSSSpecificity & lhs, const CSSSpecificity & rhs);
		friend inline bool operator<=(const CSSSpecificity & lhs, const CSSSpecificity & rhs);
		friend inline bool operator>=(const CSSSpecificity & lhs, const CSSSpecificity & rhs);

};

inline bool operator==(const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	if(lhs.d != rhs.d) {
		return false;
	}

	if(lhs.c != rhs.c) {
		return false;
	}

	if(lhs.b != rhs.b) {
		return false;
	}

	if(lhs.a != rhs.a) {
		return false;
	}

	return true;
}

inline bool operator!=(const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	return !(rhs == lhs);
}

inline bool operator< (const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	// Is 1,0,0,0 less than 0,1,0,0? No.
	// Is 0,1,0,0 less than 1,0,0,0? Yes

	// Check greater or less for a, then b etc.
	// Only cascade through if the values are equal.

	if(lhs.a < rhs.a) {
		return true;
	}

	if(lhs.a > rhs.a) {
		return false;
	}

	if(lhs.b < rhs.b) {
		return true;
	}

	if(lhs.b > rhs.b) {
		return false;
	}

	if(lhs.c < rhs.c) {
		return true;
	}

	if(lhs.c > rhs.c) {
		return false;
	}

	if(lhs.d < rhs.d) {
		return true;
	}

	if(lhs.d > rhs.d) {
		return false;
	}

	return false;

}
inline bool operator> (const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	return rhs < lhs;
}
inline bool operator<=(const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	return !(lhs > rhs);
}
inline bool operator>=(const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	return !(lhs < rhs);
}

#endif