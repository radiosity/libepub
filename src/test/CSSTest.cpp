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
	ASSERT_TRUE(test_a >= test_b);
	ASSERT_TRUE(test_b <= test_a);
	ASSERT_TRUE(test_b >= test_a);

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

TEST(CSSTest, Selector_Count)
{

	CSSSelector test_a("p");

	ASSERT_EQ(1, test_a.count());

	CSSSelector test_b("h1, h2, h3");

	ASSERT_EQ(3, test_b.count());

	CSSSelector test_c("p span");

	ASSERT_EQ(1, test_c.count());

	CSSSelector test_d("p.section");

	ASSERT_EQ(1, test_d.count());

	CSSSelector test_e(".bold");

	ASSERT_EQ(1, test_e.count());

	CSSSelector test_f("#id");

	ASSERT_EQ(1, test_f.count());

}


TEST(CSSTest, Selector_Matches)
{

	CSSSelector test_a("p");

	ASSERT_TRUE(test_a.matches("p"));

	CSSSelector test_b("h1, h2, h3");

	ASSERT_TRUE(test_b.matches("h1"));
	ASSERT_TRUE(test_b.matches("h2"));
	ASSERT_TRUE(test_b.matches("h3"));

	/*
	CSSSelector test_c("p span");

	ASSERT_EQ(1, test_c.count());
	*/

	CSSSelector test_d("p.section");

	ASSERT_TRUE(test_d.matches("p.section"));

	CSSSelector test_e(".bold");

	ASSERT_TRUE(test_e.matches(".bold"));

	CSSSelector test_f("#id");

	ASSERT_TRUE(test_f.matches("#id"));

	CSSSelector test_g("span#id");

	ASSERT_TRUE(test_g.matches("span#id"));


}

TEST(CSSTest, Selector_Specificity)
{

	CSSSelector selector_a("p");
	CSSSpecificity test_a(0, 0, 0, 1);

	ASSERT_TRUE(selector_a.specificity == test_a);

	CSSSelector selector_b("h1, h2, h3");
	CSSSpecificity test_b(0, 0, 0, 3);

	ASSERT_TRUE(selector_b.specificity == test_b);

	/*
	CSSSelector test_c("p span");

	ASSERT_EQ(1, test_c.count());
	*/

	CSSSelector selector_d("p.section");
	CSSSpecificity test_d(0, 0, 1, 1);

	ASSERT_TRUE(selector_d.specificity == test_d);

	CSSSelector selector_e(".bold");
	CSSSpecificity test_e(0, 0, 1, 0);

	ASSERT_TRUE(selector_e.specificity == test_e);

	CSSSelector selector_f("#id");
	CSSSpecificity test_f(0, 1, 0, 0);

	ASSERT_TRUE(selector_f.specificity == test_f);

	CSSSelector selector_g("span#id");
	CSSSpecificity test_g(0, 1, 0, 1);

	ASSERT_TRUE(selector_g.specificity == test_g);


}

