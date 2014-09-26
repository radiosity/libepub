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

/*
Info:

Selectors : http://www.w3.org/TR/WD-css2-971104/selector.html

*/

#include <gtest/gtest.h>

#include "CSS.hpp"

TEST(CSSTest, Specificity_Equality)
{

	CSSSpecificity test_a(0, 0, 0, 0);
	CSSSpecificity test_b(0, 0, 0, 0);

	ASSERT_TRUE(test_a == test_b);

	CSSSpecificity test_c1(1, 0, 0, 0);
	CSSSpecificity test_c2(0, 1, 0, 0);
	CSSSpecificity test_c3(0, 0, 1, 0);
	CSSSpecificity test_c4(0, 0, 0, 1);

	ASSERT_TRUE(test_a != test_c1);
	ASSERT_TRUE(test_a != test_c2);
	ASSERT_TRUE(test_a != test_c3);
	ASSERT_TRUE(test_a != test_c4);

}

TEST(CSSTest, Specificity_Relational)
{

	CSSSpecificity test_a(0, 0, 0, 0);
	CSSSpecificity test_b(0, 0, 0, 0);

	ASSERT_TRUE(test_a <= test_b);

	test_a = CSSSpecificity(0, 0, 0, 0);
	test_b = CSSSpecificity(0, 0, 0, 1);

	ASSERT_TRUE(test_a < test_b);
	ASSERT_FALSE(test_a > test_b);

	CSSSpecificity test_c1(1, 0, 0, 0);
	CSSSpecificity test_c2(0, 1, 0, 0);
	CSSSpecificity test_c3(0, 0, 1, 0);
	CSSSpecificity test_c4(0, 0, 0, 1);

	ASSERT_TRUE(test_c1 != test_c2);
	ASSERT_FALSE(test_c1 < test_c2);
	ASSERT_TRUE(test_c1 > test_c2);

	ASSERT_TRUE(test_c2 != test_c3);
	ASSERT_FALSE(test_c2 < test_c3);
	ASSERT_TRUE(test_c2 > test_c3);

	ASSERT_TRUE(test_c3 != test_c4);
	ASSERT_FALSE(test_c3 < test_c4);
	ASSERT_TRUE(test_c3 > test_c4);

}

TEST(CSSTest, Specificity_CopyConstructor)
{

	CSSSpecificity test_a(0, 0, 0, 0);
	CSSSpecificity test_b(test_a);

	ASSERT_TRUE(test_a == test_b);

}

TEST(CSSTest, Specificity_CopyAssignment)
{

	CSSSpecificity test_a(0, 0, 0, 0);
	CSSSpecificity test_b = test_a;

	ASSERT_TRUE(test_a == test_b);

}

