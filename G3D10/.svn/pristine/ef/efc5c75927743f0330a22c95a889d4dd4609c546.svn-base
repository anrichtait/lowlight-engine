/*
MIT License

Copyright (c) 2022 Roblox Research

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "G3D-base/G3DString.h"
#include "main.h"
#include "gtest/gtest.h"

#include <string>
#include <locale>

char sampleString[44] = "the quick brown fox jumps over the lazy dog";
size_t sampleStringSize = strlen(sampleString);
char sampleStringLarge[446] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
size_t sampleStringLargeSize = strlen(sampleStringLarge);
const char* findTestString = "abcabcabcabcabcabcabcdabcabcABCabc";
const char* findTestString2 = "abcabcabcabcabcabcabcdabcabcabcabcadsf";
const char* findTestString3 = "abcabcabcabcabcabcabcabcabcABCabc";

namespace G3D {
TEST(G3DStringTest, Construct)
{
    G3DString<64> g3dstring0;
    G3DString<64> g3dstring1('a');
    G3DString<64> g3dstring2("0123456789abcdefghijklmnopqrstuvwxyz");
    G3DString<64> g3dstring3(g3dstring2, 10);
    G3DString<64> g3dstring4(g3dstring2, 10, 10);
    G3DString<64> g3dstring5(g3dstring2.begin(), g3dstring2.begin() + 10);
    G3DString<64> g3dstring6{ G3DString<64>(sampleString) };
    G3DString<64> g3dstring7({ 'a', 'b', 'c' });

    std::string string1(sampleString);
    G3DString<64> g3dstring8(string1, 40);
    G3DString<64> g3dstring9(string1, 4, 5);
    G3DString<64> g3dstring10(10, 'b');
    G3DString<64> g3dstring11(g3dstring10);

    EXPECT_EQ(g3dstring0.size(), 0);
    EXPECT_STREQ(g3dstring0.c_str(), "");

    EXPECT_EQ(g3dstring1.size(), 1);
    EXPECT_STREQ(g3dstring1.c_str(), "a");

    EXPECT_EQ(g3dstring2.size(), 36);
    EXPECT_STREQ(g3dstring2.c_str(), "0123456789abcdefghijklmnopqrstuvwxyz");

    EXPECT_EQ(g3dstring3.size(), 26);
    EXPECT_STREQ(g3dstring3.c_str(), "abcdefghijklmnopqrstuvwxyz");

    EXPECT_EQ(g3dstring4.size(), 10);
    EXPECT_STREQ(g3dstring4.c_str(), "abcdefghij");

    EXPECT_EQ(g3dstring5.size(), 10);
    EXPECT_STREQ(g3dstring5.c_str(), "0123456789");

    EXPECT_EQ(g3dstring6.size(), 43);
    EXPECT_STREQ(g3dstring6.c_str(), sampleString);

    EXPECT_EQ(g3dstring7.size(), 3);
    EXPECT_STREQ(g3dstring7.c_str(), "abc");

    EXPECT_EQ(g3dstring8.size(), 3);
    EXPECT_STREQ(g3dstring8.c_str(), "dog");

    EXPECT_EQ(g3dstring9.size(), 5);
    EXPECT_STREQ(g3dstring9.c_str(), "quick");

    EXPECT_EQ(g3dstring10.size(), 10);
    EXPECT_STREQ(g3dstring10.c_str(), "bbbbbbbbbb");

    g3dstring10.insert(0, "0123456789");
    G3DString<64> g3dstring12(g3dstring10, 0, 5);

    EXPECT_EQ(g3dstring11.size(), 10);
    EXPECT_STREQ(g3dstring11.c_str(), "bbbbbbbbbb");

    EXPECT_EQ(g3dstring12.size(), 5);
    EXPECT_STREQ(g3dstring12.c_str(), "01234");

    G3DString<64> g3dstring13(string1.begin(), string1.end());
    EXPECT_EQ(g3dstring13.size(), string1.size());
    EXPECT_STREQ(g3dstring13.c_str(), string1.c_str());

    std::istringstream ss{ sampleStringLarge };
    std::istreambuf_iterator<char> it{ ss };
    G3DString<64> g3dstring14{ it, std::istreambuf_iterator<char>() };
    EXPECT_STREQ(g3dstring14.c_str(), sampleStringLarge);
    EXPECT_EQ(g3dstring14.size(), sampleStringLargeSize);
}

TEST(G3DStringTest, Assign)
{
    G3DString<64> g3dstring0;
    EXPECT_EQ(g3dstring0.size(), 0);
    EXPECT_STREQ(g3dstring0.c_str(), "");

    g3dstring0.assign(1, 'a');
    EXPECT_EQ(g3dstring0.size(), 1);
    EXPECT_STREQ(g3dstring0.c_str(), "a");

    G3DString<64> g3dstring1(sampleString, 26);

    G3DString<64> g3dstring2("0123456789abcdefghijklmnopqrstuvwxyz");

    g3dstring0.assign(g3dstring2);
    EXPECT_EQ(g3dstring0.size(), 36);
    EXPECT_STREQ(g3dstring0.c_str(), "0123456789abcdefghijklmnopqrstuvwxyz");

    g3dstring0.assign(g3dstring1, 16, 5);
    EXPECT_EQ(g3dstring0.size(), 5);
    EXPECT_STREQ(g3dstring0.c_str(), "fox j");

    g3dstring0.assign(g3dstring2, 10);
    EXPECT_EQ(g3dstring0.size(), 26);
    EXPECT_STREQ(g3dstring0.c_str(), "abcdefghijklmnopqrstuvwxyz");

    g3dstring0.assign(g3dstring2, 10, 10);
    EXPECT_EQ(g3dstring0.size(), 10);
    EXPECT_STREQ(g3dstring0.c_str(), "abcdefghij");

    g3dstring0.assign(g3dstring2.begin(), g3dstring2.begin() + 10);
    EXPECT_EQ(g3dstring0.size(), 10);
    EXPECT_STREQ(g3dstring0.c_str(), "0123456789");

    g3dstring0 = G3DString<64>(sampleString);
    EXPECT_EQ(g3dstring0.size(), 43);
    EXPECT_STREQ(g3dstring0.c_str(), sampleString);

    g3dstring0 = g3dstring0;
    EXPECT_EQ(g3dstring0.size(), 43);
    EXPECT_STREQ(g3dstring0.c_str(), sampleString);

    g3dstring0.assign({ 'a', 'b', 'c' });
    EXPECT_EQ(g3dstring0.size(), 3);
    EXPECT_STREQ(g3dstring0.c_str(), "abc");

    G3DString<64> g3dstring3(100, 'a');
    std::string string3(100, 'a');
    g3dstring0.assign(g3dstring3);
    EXPECT_EQ(g3dstring0.size(), string3.size());
    EXPECT_STREQ(g3dstring0.c_str(), string3.c_str());

    g3dstring0.assign(sampleString);
    EXPECT_EQ(g3dstring0.size(), 43);
    EXPECT_STREQ(g3dstring0.c_str(), sampleString);

    g3dstring0.assign(g3dstring2.c_str());
    EXPECT_EQ(g3dstring0.size(), g3dstring2.size());
    EXPECT_STREQ(g3dstring0.c_str(), g3dstring2.c_str());

    g3dstring0.assign(string3.c_str());
    EXPECT_EQ(g3dstring0.size(), string3.size());
    EXPECT_STREQ(g3dstring0.c_str(), string3.c_str());

    std::string string4(40, 'c');
    g3dstring0 = string4;
    EXPECT_EQ(g3dstring0.size(), string4.size());
    EXPECT_STREQ(g3dstring0.c_str(), string4.c_str());

    g3dstring0 = "";
    EXPECT_EQ(g3dstring0.size(), 0);
    EXPECT_STREQ(g3dstring0.c_str(), "");
}

TEST(G3DStringTest, Access)
{
    G3DString<64> g3dstring(sampleString);

    EXPECT_EQ(g3dstring.at(4), 'q');
    EXPECT_EQ(g3dstring[4], 'q');
    EXPECT_EQ(g3dstring.front(), 't');
    EXPECT_EQ(g3dstring.back(), 'g');
    EXPECT_STREQ(g3dstring.c_str(), sampleString);
    EXPECT_STREQ(g3dstring.data(), sampleString);

    const char* aString = "aaaaa";
    const char* constString1 = "baaaa";
    const char* constString2 = "abaaa";
    const char* constString3 = "aabaa";
    const char* constString4 = "aaaab";

    G3DString<64> g3dstring1(constString1);
    EXPECT_EQ(g3dstring1.c_str(), constString1);
    g3dstring1.front() = 'a';
    EXPECT_NE(g3dstring1.c_str(), constString1);
    EXPECT_STREQ(g3dstring1.c_str(), aString);

    G3DString<64> g3dstring2(constString2);
    EXPECT_EQ(g3dstring2.c_str(), constString2);
    g3dstring2[1] = 'a';
    EXPECT_NE(g3dstring2.c_str(), constString2);
    EXPECT_STREQ(g3dstring2.c_str(), aString);

    G3DString<64> g3dstring3(constString3);
    EXPECT_EQ(g3dstring3.c_str(), constString3);
    g3dstring3.at(2) = 'a';
    EXPECT_NE(g3dstring3.c_str(), constString3);
    EXPECT_STREQ(g3dstring3.c_str(), aString);

    G3DString<64> g3dstring4(constString4);
    EXPECT_EQ(g3dstring4.c_str(), constString4);
    g3dstring4.back() = 'a';
    EXPECT_NE(g3dstring4.c_str(), constString4);
    EXPECT_STREQ(g3dstring4.c_str(), aString);
}

TEST(G3DStringTest, Iterator)
{
    G3DString<64> g3dstring1(sampleString);
    std::string string1(sampleString);

    EXPECT_EQ(*g3dstring1.begin(), *string1.begin());
    EXPECT_EQ(*(g3dstring1.end() - 1), *(string1.end() - 1));
    EXPECT_EQ(*g3dstring1.cbegin(), *string1.cbegin());
    EXPECT_EQ(*(g3dstring1.cend() - 1), *(string1.cend() - 1));
    EXPECT_EQ(*g3dstring1.rbegin(), *string1.rbegin());
    EXPECT_EQ(*(g3dstring1.rend() - 1), *(string1.rend() - 1));
    EXPECT_EQ(*g3dstring1.crbegin(), *string1.crbegin());
    EXPECT_EQ(*(g3dstring1.crend() - 1), *(string1.crend() - 1));

    // test mutability
    *g3dstring1.begin() = 'a';
    *string1.begin() = 'a';
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());

    *(g3dstring1.end() - 16) = '1';
    *(string1.end() - 16) = '1';
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());

    *(g3dstring1.rbegin() + 10) = '5';
    *(string1.rbegin() + 10) = '5';
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());

    *(g3dstring1.rend() - 30) = 'T';
    *(string1.rend() - 30) = 'T';
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
}

TEST(G3DStringTest, Compare)
{
    std::string string1(5, 'a');
    std::string string2(string1, 1);
    std::string string3(6, 'b');
    std::string string4(5, 'a');

    G3DString<64> g3dstring1(5, 'a');
    G3DString<64> g3dstring2(g3dstring1, 1);
    G3DString<64> g3dstring3(6, 'b');
    G3DString<64> g3dstring4(5, 'a');

    // same string
    EXPECT_EQ(string1.compare(string1), g3dstring1.compare(g3dstring1));
    EXPECT_EQ(string1.compare(g3dstring1.c_str()), g3dstring1.compare(string1.c_str()));
    EXPECT_EQ(string1.compare(string4), g3dstring1.compare(g3dstring4));
    EXPECT_EQ(string1.compare(g3dstring4.c_str()), g3dstring1.compare(string4.c_str()));

    // one off
    EXPECT_EQ(string1.compare(string2), g3dstring1.compare(g3dstring2));
    EXPECT_EQ(string2.compare(string1), g3dstring2.compare(g3dstring1));
    EXPECT_EQ(string1.compare(g3dstring2.c_str()), g3dstring1.compare(string2.c_str()));
    EXPECT_EQ(string2.compare(g3dstring1.c_str()), g3dstring2.compare(string1.c_str()));

    // totally different
    EXPECT_EQ(string1.compare(string3), g3dstring1.compare(g3dstring3));
    EXPECT_EQ(string3.compare(string1), g3dstring3.compare(g3dstring1));
    EXPECT_EQ(string1.compare(g3dstring3.c_str()), g3dstring1.compare(string3.c_str()));
    EXPECT_EQ(string3.compare(g3dstring1.c_str()), g3dstring3.compare(string1.c_str()));
    EXPECT_EQ(string3.compare(string2), g3dstring3.compare(g3dstring2));
    EXPECT_EQ(string2.compare(string3), g3dstring2.compare(g3dstring3));
    EXPECT_EQ(string3.compare(g3dstring2.c_str()), g3dstring3.compare(string2.c_str()));
    EXPECT_EQ(string2.compare(g3dstring3.c_str()), g3dstring2.compare(string3.c_str()));

    // operators
    EXPECT_EQ(string1 < string2, g3dstring1 < g3dstring2);
    EXPECT_EQ(string2 < string1, g3dstring2 < g3dstring1);
    EXPECT_EQ(string1 < string3, g3dstring1 < g3dstring3);
    EXPECT_EQ(string3 < string1, g3dstring3 < g3dstring1);
    EXPECT_EQ(string3 < string2, g3dstring3 < g3dstring2);
    EXPECT_EQ(string2 < string3, g3dstring2 < g3dstring3);

    EXPECT_EQ(string1 <= string2, g3dstring1 <= g3dstring2);
    EXPECT_EQ(string2 <= string1, g3dstring2 <= g3dstring1);
    EXPECT_EQ(string1 <= string3, g3dstring1 <= g3dstring3);
    EXPECT_EQ(string3 <= string1, g3dstring3 <= g3dstring1);
    EXPECT_EQ(string3 <= string2, g3dstring3 <= g3dstring2);
    EXPECT_EQ(string2 <= string3, g3dstring2 <= g3dstring3);

    EXPECT_EQ(string1 > string2, g3dstring1 > g3dstring2);
    EXPECT_EQ(string2 > string1, g3dstring2 > g3dstring1);
    EXPECT_EQ(string1 > string3, g3dstring1 > g3dstring3);
    EXPECT_EQ(string3 > string1, g3dstring3 > g3dstring1);
    EXPECT_EQ(string3 > string2, g3dstring3 > g3dstring2);
    EXPECT_EQ(string2 > string3, g3dstring2 > g3dstring3);

    EXPECT_EQ(string1 >= string2, g3dstring1 >= g3dstring2);
    EXPECT_EQ(string2 >= string1, g3dstring2 >= g3dstring1);
    EXPECT_EQ(string1 >= string3, g3dstring1 >= g3dstring3);
    EXPECT_EQ(string3 >= string1, g3dstring3 >= g3dstring1);
    EXPECT_EQ(string3 >= string2, g3dstring3 >= g3dstring2);
    EXPECT_EQ(string2 >= string3, g3dstring2 >= g3dstring3);
    
    // compare with pos
    EXPECT_EQ(string1.compare(1, 2, string3, 1, 2) < 0, g3dstring1.compare(1, 2, g3dstring3, 1, 2) < 0);
    EXPECT_EQ(string3.compare(1, 2, string1, 1, 2) > 0, g3dstring3.compare(1, 2, g3dstring1, 1, 2) > 0);
    EXPECT_EQ(string1.compare(1, 2, g3dstring3.c_str(), 2) < 0, g3dstring1.compare(1, 2, string3.c_str(), 2) < 0);
    EXPECT_EQ(string3.compare(1, 2, g3dstring1.c_str(), 2) > 0, g3dstring3.compare(1, 2, string1.c_str(), 2) > 0);
}

TEST(G3DStringTest, Equality)
{
    G3DString<64> g3dstring1(5, 'a');
    G3DString<64> g3dstring2(6, 'a');
    G3DString<64> g3dstring3(6, 'b');
    G3DString<64> g3dstring4(5, 'a');
    std::string string1(5, 'a');
    std::string string2(6, 'a');
    std::string string3(6, 'b');

    EXPECT_TRUE(g3dstring1 == g3dstring1);
    EXPECT_TRUE(g3dstring2 == g3dstring2);
    EXPECT_TRUE(g3dstring3 == g3dstring3);
    EXPECT_TRUE(g3dstring1 == g3dstring4);

    EXPECT_FALSE(g3dstring1 == g3dstring2);
    EXPECT_FALSE(g3dstring2 == g3dstring1);

    EXPECT_FALSE(g3dstring3 == g3dstring2);
    EXPECT_FALSE(g3dstring2 == g3dstring3);

    EXPECT_FALSE(g3dstring1 == g3dstring3);
    EXPECT_FALSE(g3dstring3 == g3dstring1);

    EXPECT_TRUE(g3dstring1 == string1.c_str());
    EXPECT_TRUE(g3dstring2 == string2.c_str());
    EXPECT_TRUE(g3dstring3 == string3.c_str());

    EXPECT_FALSE(g3dstring1 == string2.c_str());
    EXPECT_FALSE(g3dstring2 == string1.c_str());

    EXPECT_FALSE(g3dstring3 == string2.c_str());
    EXPECT_FALSE(g3dstring2 == string3.c_str());

    EXPECT_FALSE(g3dstring1 == string3.c_str());
    EXPECT_FALSE(g3dstring3 == string1.c_str());

    EXPECT_FALSE(g3dstring3 == "");
}

TEST(G3DStringTest, Append)
{
    std::string string1(5, 'a');
    std::string string2(6, 'b');

    G3DString<64> g3dstring1(5, 'a');
    G3DString<64> g3dstring2(6, 'b');

    EXPECT_STREQ((string1 + "abc").c_str(), (g3dstring1 + "abc").c_str());
    EXPECT_STREQ((string1 + 'a').c_str(), (g3dstring1 + 'a').c_str());

    // append calls operator+=, so this tests that as well
    EXPECT_STREQ(string1.append(string2).c_str(), g3dstring1.append(g3dstring2).c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    EXPECT_STREQ(string1.append(string2, 2).c_str(), g3dstring1.append(g3dstring2, 2).c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    EXPECT_STREQ(string1.append(string2, 2, 2).c_str(), g3dstring1.append(g3dstring2, 2, 2).c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    EXPECT_STREQ(string1.append("0123456789").c_str(), g3dstring1.append("0123456789").c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    EXPECT_STREQ(string1.append(sampleString).c_str(), g3dstring1.append(sampleString).c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    EXPECT_STREQ(string1.append(6, ' ').c_str(), g3dstring1.append(6, ' ').c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    EXPECT_STREQ(string1.append({ 'd', '1', 'h' }).c_str(), g3dstring1.append({ 'd', '1', 'h' }).c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    EXPECT_STREQ(string1.append(string2.begin() + 1, string2.begin() + 3).c_str(), g3dstring1.append(g3dstring2.begin() + 1, g3dstring2.begin() + 3).c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    // test concat
    EXPECT_STREQ((string1 + string2).c_str(), (g3dstring1 + g3dstring2).c_str());
    EXPECT_STREQ((string1 + sampleString).c_str(), (g3dstring1 + sampleString).c_str());
    EXPECT_STREQ((string1 + 'a').c_str(), (g3dstring1 + 'a').c_str());
    EXPECT_STREQ((sampleString + string1).c_str(), (sampleString + g3dstring1).c_str());
    EXPECT_STREQ(('a' + string1).c_str(), ('a' + g3dstring1).c_str());

    // Test rvalue concat
    EXPECT_STREQ((sampleString + std::string(sampleString)).c_str(), (sampleString + G3DString<64>(sampleString)).c_str());
    EXPECT_STREQ(('a' + std::string(sampleString)).c_str(), ('a' + G3DString<64>(sampleString)).c_str());
    EXPECT_STREQ((string1 + std::string(sampleString)).c_str(), (g3dstring1 + G3DString<64>(sampleString)).c_str());
    EXPECT_STREQ((std::string(sampleString) + string2).c_str(), (G3DString<64>(sampleString) + g3dstring2).c_str());
    EXPECT_STREQ((std::string(sampleString) + sampleString).c_str(), (G3DString<64>(sampleString) + sampleString).c_str());
    EXPECT_STREQ((std::string(sampleString) + 'a').c_str(), (G3DString<64>(sampleString) + 'a').c_str());
    EXPECT_STREQ((std::string(sampleString) + std::string(sampleString)).c_str(), (G3DString<64>(sampleString) + G3DString<64>(sampleString)).c_str());

    // append using input iterator
    std::istringstream ss1{ sampleString };
    std::istreambuf_iterator<char> it1{ ss1 };
    std::istringstream ss2{ sampleString };
    std::istreambuf_iterator<char> it2{ ss2 };

    string1.append(it1, std::istreambuf_iterator<char>());
    g3dstring1.append(it2, std::istreambuf_iterator<char>());
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());
}

TEST(G3DStringTest, PushPopBack)
{
    G3DString<64> g3dstring1(sampleString);
    std::string string1(sampleString);

    string1.push_back('a');
    g3dstring1.push_back('a');

    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    string1.pop_back();
    g3dstring1.pop_back();
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    G3DString<64> g3dstring2;
    g3dstring2.push_back('a');
    EXPECT_STREQ(g3dstring2.c_str(), "a");
    EXPECT_EQ(g3dstring2.length(), 1);
}

TEST(G3DStringTest, Insert)
{
    std::string string1("0123456789");
    std::string string2("abcdefg");

    G3DString<64> g3dstring1("0123456789");
    G3DString<64> g3dstring2("abcdefg");

#define INSERT_TESTS(stdPos, g3dPos)\
  EXPECT_STREQ(string1.insert(stdPos, string2).c_str(), g3dstring1.insert(g3dPos, g3dstring2).c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());\
  EXPECT_STREQ(string1.insert(stdPos, string2, 2, 3).c_str(), g3dstring1.insert(g3dPos, g3dstring2, 2, 3).c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());\
  EXPECT_STREQ(string1.insert(stdPos, "hijklmnop").c_str(), g3dstring1.insert(g3dPos, "hijklmnop").c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());\
  EXPECT_STREQ(string1.insert(stdPos, "hijklmnop", 4).c_str(), g3dstring1.insert(g3dPos, "hijklmnop", 4).c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());\
  EXPECT_STREQ(string1.insert(stdPos, 6, 'z').c_str(), g3dstring1.insert(g3dPos, 6, 'z').c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());\

    // insert in the beginning
    INSERT_TESTS(0, 0)

        // insert in the middle 
        INSERT_TESTS(string1.size() / 2, g3dstring1.size() / 2)

        // insert near the end
        INSERT_TESTS(string1.size() - 1, g3dstring1.size() - 1)

        // insert at the end
        INSERT_TESTS(string1.size(), g3dstring1.size())

        // insert using iterators
        string1.insert(string1.begin() + 2, string2.begin() + 2, string2.begin() + 4);
    g3dstring1.insert(g3dstring1.begin() + 2, g3dstring2.begin() + 2, g3dstring2.begin() + 4);
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    string1.insert(string1.begin(), '/');
    g3dstring1.insert(g3dstring1.begin(), '/');
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    // insert using input iterator
    std::istringstream ss1{ sampleString };
    std::istreambuf_iterator<char> it1{ ss1 };
    std::istringstream ss2{ sampleString };
    std::istreambuf_iterator<char> it2{ ss2 };

    string1.insert(string1.begin() + 6, it1, std::istreambuf_iterator<char>());
    g3dstring1.insert(g3dstring1.begin() + 6, it2, std::istreambuf_iterator<char>());
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());
}

TEST(G3DStringTest, Replace)
{
    std::string string1("0123456789");
    std::string string2("abcdefg");

    G3DString<64> g3dstring1("0123456789");
    G3DString<64> g3dstring2("abcdefg");

    std::string string3("abcdefg");
    G3DString<64> g3dstring3("abcdefg");

#define REPLACE_TESTS(stdPos, g3dPos, count, count2)\
  EXPECT_STREQ(string1.replace(stdPos, count, string2).c_str(), g3dstring1.replace(g3dPos, count, g3dstring2).c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());\
  EXPECT_STREQ(string1.replace(stdPos, count, string2, 2, count2).c_str(), g3dstring1.replace(g3dPos, count, g3dstring2, 2, count2).c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());\
  EXPECT_STREQ(string1.replace(stdPos, count, "hijklmnop").c_str(), g3dstring1.replace(g3dPos, count, "hijklmnop").c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());\
  EXPECT_STREQ(string1.replace(stdPos, count, "hijklmnop", count2).c_str(), g3dstring1.replace(g3dPos, count, "hijklmnop", count2).c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());\
  EXPECT_STREQ(string1.replace(stdPos, count, count2, 'z').c_str(), g3dstring1.replace(g3dPos, count, count2, 'z').c_str());\
  EXPECT_EQ(string1.length(), g3dstring1.length());

    // replace at beginning, grow
    REPLACE_TESTS(0, 0, 1, 4)
        // replace at beginning, shrink
        REPLACE_TESTS(0, 0, 10, 4)
        // replace at beginning, same size
        REPLACE_TESTS(0, 0, 5, 5)

        // replace at middle, grow
        REPLACE_TESTS(5, 5, 1, 4)
        // replace at middle, shrink
        REPLACE_TESTS(5, 5, 10, 4)
        // replace at middle, same size
        REPLACE_TESTS(5, 5, 5, 5)

        // replace near the end, grow
        REPLACE_TESTS(string1.size() - 1, g3dstring1.size() - 1, 1, 4)
        // replace near the end, smaller size, should still grow
        REPLACE_TESTS(string1.size() - 1, g3dstring1.size() - 1, 10, 4)
        // replace near the end, same size, should still grow
        REPLACE_TESTS(string1.size() - 1, g3dstring1.size() - 1, 5, 5)

        // replace at the end, grow
        REPLACE_TESTS(string1.size(), g3dstring1.size(), 1, 4)
        // replace at the end, shrink
        REPLACE_TESTS(string1.size(), g3dstring1.size(), 10, 4)
        // replace at the end, same size
        REPLACE_TESTS(string1.size(), g3dstring1.size(), 5, 5)

        // replace buffer to heap
        EXPECT_STREQ(string3.replace(0, string1.size(), string1).c_str(), g3dstring3.replace(0, g3dstring1.size(), g3dstring1).c_str());
    EXPECT_EQ(string3.length(), g3dstring3.length());

    // replace using iterators, range
    string1.replace(string1.begin() + 2, string1.begin() + 6, string2.begin() + 2, string2.begin() + 4);
    g3dstring1.replace(g3dstring1.begin() + 2, g3dstring1.begin() + 6, g3dstring2.begin() + 2, g3dstring2.begin() + 4);
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    // replace using iterators, str
    string1.replace(string1.begin() + 2, string1.begin() + 6, string2);
    g3dstring1.replace(g3dstring1.begin() + 2, g3dstring1.begin() + 6, g3dstring2);
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    // replace using iterators, cstr
    string1.replace(string1.begin() + 2, string1.begin() + 6, "hijklmnop");
    g3dstring1.replace(g3dstring1.begin() + 2, g3dstring1.begin() + 6, "hijklmnop");
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    string1.replace(string1.begin() + 2, string1.begin() + 6, "hijklmnop", 3);
    g3dstring1.replace(g3dstring1.begin() + 2, g3dstring1.begin() + 6, "hijklmnop", 3);
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    // replace using iterators, char
    string1.replace(string1.begin() + 2, string1.begin() + 6, 6, 'z');
    g3dstring1.replace(g3dstring1.begin() + 2, g3dstring1.begin() + 6, 6, 'z');
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());

    // replace using input iterator
    std::istringstream ss1{ sampleString };
    std::istreambuf_iterator<char> it1{ ss1 };
    std::istringstream ss2{ sampleString };
    std::istreambuf_iterator<char> it2{ ss2 };

    string1.replace(string1.begin() + 2, string1.begin() + 6, it1, std::istreambuf_iterator<char>());
    g3dstring1.replace(g3dstring1.begin() + 2, g3dstring1.begin() + 6, it2, std::istreambuf_iterator<char>());
    EXPECT_STREQ(string1.c_str(), g3dstring1.c_str());
    EXPECT_EQ(string1.length(), g3dstring1.length());
}

TEST(G3DStringTest, ClearErase)
{
    G3DString<64> g3dstring1(sampleString);
    std::string string1(sampleString);

    EXPECT_EQ(g3dstring1.erase(6, 12).size(), string1.erase(6, 12).size());
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());

    string1.erase(string1.begin() + 5);
    g3dstring1.erase(g3dstring1.begin() + 5);
    EXPECT_EQ(g3dstring1.size(), string1.size());
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_FALSE(g3dstring1.empty());

    string1.erase(string1.begin(), string1.begin() + 5);
    g3dstring1.erase(g3dstring1.begin(), g3dstring1.begin() + 5);
    EXPECT_EQ(g3dstring1.size(), string1.size());
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_FALSE(g3dstring1.empty());

    g3dstring1.erase(g3dstring1.begin(), g3dstring1.end());
    EXPECT_EQ(g3dstring1.size(), 0);
    EXPECT_EQ(g3dstring1.length(), 0);
    EXPECT_STREQ(g3dstring1.c_str(), "");
    EXPECT_TRUE(g3dstring1.empty());

    G3DString<64> g3dstring2(sampleString);
    g3dstring2.clear();
    EXPECT_EQ(g3dstring2.size(), 0);
    EXPECT_EQ(g3dstring2.length(), 0);
    EXPECT_STREQ(g3dstring2.c_str(), "");
    EXPECT_TRUE(g3dstring2.empty());

    G3DString<64> g3dstring3(sampleString);
    std::string string3(sampleString);
    g3dstring3.erase(10, 1);
    string3.erase(10, 1);
    EXPECT_EQ(g3dstring3.size(), string3.size());
    EXPECT_STREQ(g3dstring3.c_str(), string3.c_str());
    EXPECT_FALSE(g3dstring3.empty());

    std::string string4(100, 'a');
    G3DString<64> g3dstring4(100, 'a');
    string4[75] = 'b';
    g3dstring4[75] = 'b';
    EXPECT_EQ(g3dstring4.size(), string4.size());
    EXPECT_STREQ(g3dstring4.c_str(), string4.c_str());
    EXPECT_FALSE(g3dstring4.empty());
}

TEST(G3DStringTest, Swap)
{
    // buffer alloc
    std::string string1(20, 'a');
    G3DString<64> g3dstring1(20, 'a');

    // const string
    std::string string2(sampleString);
    G3DString<64> g3dstring2(sampleString);

    // heap alloc
    std::string string3(100, 'b');
    G3DString<64> g3dstring3(100, 'b');

    g3dstring1.swap(g3dstring2);

    EXPECT_STREQ(g3dstring1.c_str(), string2.c_str());
    EXPECT_EQ(g3dstring1.size(), string2.size());
    EXPECT_STREQ(g3dstring2.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring2.size(), string1.size());

    // g3dstring1 = string2
    g3dstring1.swap(g3dstring3);
    EXPECT_STREQ(g3dstring1.c_str(), string3.c_str());
    EXPECT_EQ(g3dstring1.size(), string3.size());
    EXPECT_STREQ(g3dstring3.c_str(), string2.c_str());
    EXPECT_EQ(g3dstring3.size(), string2.size());

    G3DString<64> g3dstring4(G3DString<64>(26, 'z'));
    G3DString<64> g3dstring5(G3DString<64>(sampleString));
    G3DString<64> g3dstring6(G3DString<64>(200, 'z'));
}

TEST(G3DStringTest, Find)
{
    std::string string1(findTestString);
    G3DString<64> g3dstring1(findTestString);
    std::string substr("abc");
    G3DString<64> subg3dstr("abc");

    // find substring
    EXPECT_EQ(string1.find("abc"), g3dstring1.find("abc"));
    EXPECT_EQ(string1.find("abcd"), g3dstring1.find("abcd"));
    EXPECT_EQ(string1.find(substr), g3dstring1.find(subg3dstr));
    // find substring with pos
    EXPECT_EQ(string1.find("abc", 10), g3dstring1.find("abc", 10));
    EXPECT_EQ(string1.find("abcd", 10), g3dstring1.find("abcd", 10));
    EXPECT_EQ(string1.find(substr, 10), g3dstring1.find(subg3dstr, 10));
    // find substring with pos (abcd should not be found)
    EXPECT_EQ(string1.find("abc", 30), g3dstring1.find("abc", 30));
    EXPECT_EQ(string1.find("abcd", 30), g3dstring1.find("abcd", 30));
    EXPECT_EQ(string1.find(substr, 30), g3dstring1.find(subg3dstr, 30));

    // find char (4 is no match)
    EXPECT_EQ(string1.find('a'), g3dstring1.find('a'));
    EXPECT_EQ(string1.find('c'), g3dstring1.find('c'));
    EXPECT_EQ(string1.find('d'), g3dstring1.find('d'));
    EXPECT_EQ(string1.find('4'), g3dstring1.find('4'));
    // find char with  (4 is no match)
    EXPECT_EQ(string1.find('a', 10), g3dstring1.find('a', 10));
    EXPECT_EQ(string1.find('c', 10), g3dstring1.find('c', 10));
    EXPECT_EQ(string1.find('d', 10), g3dstring1.find('d', 10));
    EXPECT_EQ(string1.find('4', 10), g3dstring1.find('4', 10));
    // find char with pos (d should not be found)
    EXPECT_EQ(string1.find('a', 30), g3dstring1.find('a', 30));
    EXPECT_EQ(string1.find('c', 30), g3dstring1.find('c', 30));
    EXPECT_EQ(string1.find('d', 30), g3dstring1.find('d', 30));

    // find whole string
    EXPECT_EQ(string1.find(findTestString), g3dstring1.find(findTestString));
    // find whole string (no match)
    EXPECT_EQ(string1.find(findTestString2), g3dstring1.find(findTestString2));
    EXPECT_EQ(string1.find(findTestString3), g3dstring1.find(findTestString3));

    // buffer alloc
    std::string string2(20, 'a');
    G3DString<64> g3dstring2(20, 'a');
    string2[15] = 'b';
    g3dstring2[15] = 'b';
    string2.resize(11);
    g3dstring2.resize(11);

    // test edge case where b is definitely in an immediate, greater memory location
    EXPECT_EQ(string2.find('b', 6), g3dstring2.find('b', 6));
    EXPECT_EQ(string2.find("ab", 6), g3dstring2.find("ab", 6));
    EXPECT_EQ(string2.find('b', string2.size()), g3dstring2.find('b', g3dstring2.size()));
    EXPECT_EQ(string2.find("ab", string2.size()), g3dstring2.find("ab", g3dstring2.size()));
    // test empty string
    EXPECT_EQ(string2.find(""), g3dstring2.find(""));
    EXPECT_EQ(string2.find("", 6), g3dstring2.find("", 6));
    EXPECT_EQ(string2.find("", string2.size()), g3dstring2.find("", g3dstring2.size()));

    // test empty string
    G3DString<64> g3dstring3;
    std::string string3;
    EXPECT_EQ(string3.find('a'), g3dstring3.find('a'));
    EXPECT_EQ(string3.find("abcd"), g3dstring3.find("abcd"));

    // edge case null char
    G3DString<64> g3dstring4("test");
    g3dstring4 += '\0';
    g3dstring4 += "null";
    std::string string4("test");
    string4 += '\0';
    string4 += "null";
    EXPECT_EQ(string4.find('\0'), g3dstring4.find('\0'));
}

TEST(G3DStringTest, RFind)
{
    std::string string1(findTestString);
    G3DString<64> g3dstring1(findTestString);
    std::string substr("abc");
    G3DString<64> subg3dstr("abc");

    // find substring
    EXPECT_EQ(string1.rfind("abc"), g3dstring1.rfind("abc"));
    EXPECT_EQ(string1.rfind("abcd"), g3dstring1.rfind("abcd"));
    EXPECT_EQ(string1.rfind(substr), g3dstring1.rfind(subg3dstr));
    // find substring with pos
    EXPECT_EQ(string1.rfind("abc", 10), g3dstring1.rfind("abc", 10));
    EXPECT_EQ(string1.rfind("abcd", 10), g3dstring1.rfind("abcd", 10));
    EXPECT_EQ(string1.rfind(substr, 10), g3dstring1.rfind(subg3dstr, 10));
    // find substring with pos (abcd should not be found)
    EXPECT_EQ(string1.rfind("abc", 30), g3dstring1.rfind("abc", 30));
    EXPECT_EQ(string1.rfind("abcd", 30), g3dstring1.rfind("abcd", 30));
    EXPECT_EQ(string1.rfind(substr, 30), g3dstring1.rfind(subg3dstr, 30));

    // find char
    EXPECT_EQ(string1.rfind('c'), g3dstring1.rfind('c'));
    EXPECT_EQ(string1.rfind('d'), g3dstring1.rfind('d'));
    EXPECT_EQ(string1.rfind('A'), g3dstring1.rfind('A'));
    // find char with pos
    EXPECT_EQ(string1.rfind('c', 10), g3dstring1.rfind('c', 10));
    EXPECT_EQ(string1.rfind('d', 10), g3dstring1.rfind('d', 10));
    EXPECT_EQ(string1.rfind('A', 10), g3dstring1.rfind('A', 10));
    // find char with pos (d should not be found)
    EXPECT_EQ(string1.rfind('c', 30), g3dstring1.rfind('c', 30));
    EXPECT_EQ(string1.rfind('d', 30), g3dstring1.rfind('d', 30));
    EXPECT_EQ(string1.rfind('A', 30), g3dstring1.rfind('A', 30));

    // find whole string
    EXPECT_EQ(string1.rfind(findTestString), g3dstring1.rfind(findTestString));
    // find whole string (no match)
    EXPECT_EQ(string1.rfind(findTestString2), g3dstring1.rfind(findTestString2));
    EXPECT_EQ(string1.rfind(findTestString3), g3dstring1.rfind(findTestString3));

    std::string string2(findTestString, 30);
    G3DString<64> g3dstring2(findTestString, 30);
    // test edge case where match is nearby but out of scope
    EXPECT_EQ(string2.rfind('d', 6), g3dstring2.rfind('d', 6));
    EXPECT_EQ(string2.rfind("cd", 6), g3dstring2.rfind("cd", 6));
    EXPECT_EQ(string2.rfind('d', string2.size()), g3dstring2.rfind('d', g3dstring2.size()));
    EXPECT_EQ(string2.rfind("cd", string2.size()), g3dstring2.rfind("cd", g3dstring2.size()));

    // test match empty string
    EXPECT_EQ(string2.rfind(""), g3dstring2.rfind(""));
    EXPECT_EQ(string2.rfind("", 6), g3dstring2.rfind("", 6));
    EXPECT_EQ(string2.rfind("", string2.size()), g3dstring2.rfind("", g3dstring2.size()));

    // test empty string
    G3DString<64> g3dstring3;
    std::string string3;
    EXPECT_EQ(string3.rfind('a'), g3dstring3.rfind('a'));
    EXPECT_EQ(string3.rfind("abcd"), g3dstring3.rfind("abcd"));

    // edge case null char
    G3DString<64> g3dstring4("test");
    g3dstring4 += '\0';
    g3dstring4 += "null";
    std::string string4("test");
    string4 += '\0';
    string4 += "null";
    EXPECT_EQ(string4.rfind('\0'), g3dstring4.rfind('\0'));
}

TEST(G3DStringTest, FindFirstLastOf)
{
    const char* findFirstLastOfTestString = "The quick brown fox jumps over the lazy dog. Sphinx of black quartz, judge my vow.";
    std::string string1(findFirstLastOfTestString);
    G3DString<64> g3dstring1(findFirstLastOfTestString);

    // find first/last of
    EXPECT_EQ(string1.find_first_of('d'), g3dstring1.find_first_of('d'));
    EXPECT_EQ(string1.find_first_of("mnop"), g3dstring1.find_first_of("mnop"));
    EXPECT_EQ(string1.find_last_of('d'), g3dstring1.find_last_of('d'));
    EXPECT_EQ(string1.find_last_of("mnop"), g3dstring1.find_last_of("mnop"));

    // find first/last of, with pos
    EXPECT_EQ(string1.find_first_of('d', 40), g3dstring1.find_first_of('d', 40));
    EXPECT_EQ(string1.find_first_of("mnop", 40), g3dstring1.find_first_of("mnop", 40));
    EXPECT_EQ(string1.find_last_of('d', 40), g3dstring1.find_last_of('d', 40));
    EXPECT_EQ(string1.find_last_of("mnop", 40), g3dstring1.find_last_of("mnop", 40));

    // find edge case, match == pos
    EXPECT_EQ(string1.find_first_of('q', string1.find('q')), g3dstring1.find_first_of('q', g3dstring1.find('q')));
    EXPECT_EQ(string1.find_first_of("mnop", string1.find('m')), g3dstring1.find_first_of("mnop", g3dstring1.find('m')));
    EXPECT_EQ(string1.find_last_of('d', string1.find('d')), g3dstring1.find_last_of('d', g3dstring1.find('d')));
    EXPECT_EQ(string1.find_last_of("jkl", string1.find('j')), g3dstring1.find_last_of("jkl", g3dstring1.find('j')));

    // find first/last of, no match
    EXPECT_EQ(string1.find_first_of('?'), g3dstring1.find_first_of('?'));
    EXPECT_EQ(string1.find_first_of("1234"), g3dstring1.find_first_of("1234"));
    EXPECT_EQ(string1.find_last_of('?'), g3dstring1.find_last_of('?'));
    EXPECT_EQ(string1.find_last_of("1234"), g3dstring1.find_last_of("1234"));

    // find first/last of with pos, no match
    EXPECT_EQ(string1.find_first_of('?', 90), g3dstring1.find_first_of('?', 90));
    EXPECT_EQ(string1.find_first_of("1234", 90), g3dstring1.find_first_of("1234", 90));
    EXPECT_EQ(string1.find_last_of('?', 90), g3dstring1.find_last_of('?', 90));
    EXPECT_EQ(string1.find_last_of("1234", 90), g3dstring1.find_last_of("1234", 90));

    // find first/last of, empty match string
    EXPECT_EQ(string1.find_first_of(""), g3dstring1.find_first_of(""));
    EXPECT_EQ(string1.find_last_of(""), g3dstring1.find_last_of(""));
    EXPECT_EQ(string1.find_first_of("", 90), g3dstring1.find_first_of("", 90));
    EXPECT_EQ(string1.find_last_of("", 90), g3dstring1.find_last_of("", 90));

    // find first/last of, edge case  
    size_t testStringSize = strlen(findFirstLastOfTestString);
    EXPECT_EQ(string1.find_first_of('d', testStringSize), g3dstring1.find_first_of('d', testStringSize));
    EXPECT_EQ(string1.find_first_of("mnop", testStringSize), g3dstring1.find_first_of("mnop", testStringSize));
    EXPECT_EQ(string1.find_last_of('d', testStringSize), g3dstring1.find_last_of('d', testStringSize));
    EXPECT_EQ(string1.find_last_of("mnop", testStringSize), g3dstring1.find_last_of("mnop", testStringSize));

    // buffer alloc
    std::string string2(20, 'a');
    G3DString<64> g3dstring2(20, 'a');
    string2[15] = 'b';
    g3dstring2[15] = 'b';
    string2.resize(11);
    g3dstring2.resize(11);

    // test edge case where match is nearby but out of scope
    EXPECT_EQ(string2.find_first_of('b'), g3dstring2.find_first_of('b'));
    EXPECT_EQ(string2.find_first_of("bc"), g3dstring2.find_first_of("bc"));
    EXPECT_EQ(string2.find_last_of('b'), g3dstring2.find_last_of('b'));
    EXPECT_EQ(string2.find_last_of("bc"), g3dstring2.find_last_of("bc"));
    EXPECT_EQ(string2.find_first_of('b', 6), g3dstring2.find_first_of('b', 6));
    EXPECT_EQ(string2.find_first_of("bc", 6), g3dstring2.find_first_of("bc", 6));
    EXPECT_EQ(string2.find_last_of('b', 6), g3dstring2.find_last_of('b', 6));
    EXPECT_EQ(string2.find_last_of("bc", 6), g3dstring2.find_last_of("bc", 6));

    G3DString<64> g3dstring3;
    std::string string3;
    // test empty string
    EXPECT_EQ(string3.find_first_of('d'), g3dstring3.find_first_of('d'));
    EXPECT_EQ(string3.find_first_of("mnop"), g3dstring3.find_first_of("mnop"));
    EXPECT_EQ(string3.find_last_of('d'), g3dstring3.find_last_of('d'));
    EXPECT_EQ(string3.find_last_of("mnop"), g3dstring3.find_last_of("mnop"));
}

TEST(G3DStringTest, FindFirstLastNotOf)
{
    const char* findFirstLastNotOfTestString = "     a           b     m       c            d            e    t     f         ";
    std::string string1(findFirstLastNotOfTestString);
    G3DString<64> g3dstring1(findFirstLastNotOfTestString);

    // find first/last not of (space in "abcdef " is deliberate) 
    EXPECT_EQ(string1.find_first_not_of(' '), g3dstring1.find_first_not_of(' '));
    EXPECT_EQ(string1.find_first_not_of("abcdef "), g3dstring1.find_first_not_of("abcdef "));
    EXPECT_EQ(string1.find_last_not_of(' '), g3dstring1.find_last_not_of(' '));
    EXPECT_EQ(string1.find_last_not_of("abcdef "), g3dstring1.find_last_not_of("abcdef "));

    // find first/last not of with pos
    EXPECT_EQ(string1.find_first_not_of(' ', 35), g3dstring1.find_first_not_of(' ', 35));
    EXPECT_EQ(string1.find_first_not_of("abcdef ", 35), g3dstring1.find_first_not_of("abcdef ", 35));
    EXPECT_EQ(string1.find_last_not_of(' ', 35), g3dstring1.find_last_not_of(' ', 35));
    EXPECT_EQ(string1.find_last_not_of("abcdef ", 35), g3dstring1.find_last_not_of("abcdef ", 35));

    // find edge case, match == pos
    EXPECT_EQ(string1.find_first_not_of(' ', string1.find('a')), g3dstring1.find_first_not_of(' ', g3dstring1.find('a')));
    EXPECT_EQ(string1.find_first_not_of("abcdef ", string1.find('t')), g3dstring1.find_first_not_of("abcdef ", g3dstring1.find('t')));
    EXPECT_EQ(string1.find_last_not_of(' ', string1.find('f')), g3dstring1.find_last_not_of(' ', g3dstring1.find('f')));
    EXPECT_EQ(string1.find_last_not_of("abcdef ", string1.find('t')), g3dstring1.find_last_not_of("abcdef ", g3dstring1.find('t')));

    // find first/last not of, no match
    EXPECT_EQ(string1.find_first_not_of("abcdefmt "), g3dstring1.find_first_not_of("abcdefmt "));
    EXPECT_EQ(string1.find_last_not_of("abcdefmt "), g3dstring1.find_last_not_of("abcdefmt "));
    EXPECT_EQ(string1.substr(0, 3).find_first_not_of(' '), g3dstring1.substr(0, 3).find_first_not_of(' '));
    EXPECT_EQ(string1.substr(0, 3).find_last_not_of(' '), g3dstring1.substr(0, 3).find_last_not_of(' '));

    // find first/last not of, no match with pos
    EXPECT_EQ(string1.find_first_not_of("abcdeft ", 35), g3dstring1.find_first_not_of("abcdeft ", 35));
    EXPECT_EQ(string1.find_last_not_of("abcdeft ", 35), g3dstring1.find_last_not_of("abcdeft ", 35));
    EXPECT_EQ(string1.find_first_not_of(' ', string1.find('f') + 1), g3dstring1.find_first_not_of(' ', g3dstring1.find('f') + 1));
    EXPECT_EQ(string1.find_last_not_of(' ', string1.find('f') + 1), g3dstring1.find_last_not_of(' ', g3dstring1.find('f') + 1));

    // find first/last of, empty match string
    EXPECT_EQ(string1.find_first_not_of(""), g3dstring1.find_first_not_of(""));
    EXPECT_EQ(string1.find_last_not_of(""), g3dstring1.find_last_not_of(""));
    EXPECT_EQ(string1.find_first_not_of("", 35), g3dstring1.find_first_not_of("", 35));
    EXPECT_EQ(string1.find_last_not_of("", 35), g3dstring1.find_last_not_of("", 35));

    // find first/last of, edge case  
    size_t testStringSize = strlen(findFirstLastNotOfTestString);
    EXPECT_EQ(string1.find_first_not_of('?', testStringSize), g3dstring1.find_first_not_of('?', testStringSize));
    EXPECT_EQ(string1.find_first_not_of("1234", testStringSize), g3dstring1.find_first_not_of("1234", testStringSize));
    EXPECT_EQ(string1.find_last_not_of('?', testStringSize), g3dstring1.find_last_not_of('?', testStringSize));
    EXPECT_EQ(string1.find_last_not_of("1234", testStringSize), g3dstring1.find_last_not_of("1234", testStringSize));

    // buffer alloc
    std::string string2(20, 'a');
    G3DString<64> g3dstring2(20, 'a');
    string2[2] = 'c';
    g3dstring2[2] = 'c';
    string2[15] = 'b';
    g3dstring2[15] = 'b';
    string2.resize(11);
    g3dstring2.resize(11);

    // test edge case where match is nearby but out of scope
    EXPECT_EQ(string2.find_first_not_of('a'), g3dstring2.find_first_not_of('a'));
    EXPECT_EQ(string2.find_first_not_of("ac"), g3dstring2.find_first_not_of("ac"));
    EXPECT_EQ(string2.find_last_not_of('a'), g3dstring2.find_last_not_of('a'));
    EXPECT_EQ(string2.find_last_not_of("ac"), g3dstring2.find_last_not_of("ac"));
    EXPECT_EQ(string2.find_first_not_of('a', 6), g3dstring2.find_first_not_of('a', 6));
    EXPECT_EQ(string2.find_first_not_of("ac", 6), g3dstring2.find_first_not_of("ac", 6));
    EXPECT_EQ(string2.find_last_not_of('a', 6), g3dstring2.find_last_not_of('a', 6));
    EXPECT_EQ(string2.find_last_not_of("ac", 6), g3dstring2.find_last_not_of("ac", 6));

    G3DString<64> g3dstring3;
    std::string string3;
    // test empty string
    EXPECT_EQ(string3.find_first_not_of('d'), g3dstring3.find_first_not_of('d'));
    EXPECT_EQ(string3.find_first_not_of("mnop"), g3dstring3.find_first_not_of("mnop"));
    EXPECT_EQ(string3.find_last_not_of('d'), g3dstring3.find_last_not_of('d'));
    EXPECT_EQ(string3.find_last_not_of("mnop"), g3dstring3.find_last_not_of("mnop"));

    G3DString<64> g3dstring4("test");
    g3dstring4 += '\0';
    g3dstring4 += "null";

    std::string string4("test");
    string4 += '\0';
    string4 += "null";
}

TEST(G3DStringTest, StartsEndsWith)
{
    G3DString<64> g3dstring1(sampleString);

    EXPECT_TRUE(g3dstring1.starts_with("the"));
    EXPECT_FALSE(g3dstring1.starts_with("woah"));
    EXPECT_TRUE(g3dstring1.ends_with("dog"));
    EXPECT_FALSE(g3dstring1.ends_with("woah"));
    EXPECT_TRUE(g3dstring1.starts_with('t'));
    EXPECT_FALSE(g3dstring1.starts_with('w'));
    EXPECT_TRUE(g3dstring1.ends_with('g'));
    EXPECT_FALSE(g3dstring1.ends_with('h'));
    EXPECT_TRUE(g3dstring1.starts_with(sampleString));
    EXPECT_TRUE(g3dstring1.ends_with(sampleString));

    const char* s = "hello world";
    EXPECT_FALSE(G3DString<64>(s + 6).ends_with("hello world")); // Added 64 as input type
}

TEST(G3DStringTest, Contains)
{
    G3DString<64> g3dstring1(sampleString);

    EXPECT_TRUE(g3dstring1.contains("the"));
    EXPECT_FALSE(g3dstring1.contains("woah"));
    EXPECT_FALSE(g3dstring1.contains('T'));
}

TEST(G3DStringTest, CopySubstr)
{
    const char* copySubstrTestString = "0123456789abcdefghijklmnopqrstuvwxyz";
    char tmp[11];
    G3DString<64> g3dstring1(copySubstrTestString);
    tmp[10] = '\0';

    // test copy and substr for first 10 chars
    EXPECT_EQ(10, g3dstring1.copy(tmp, 10));
    EXPECT_STREQ(tmp, "0123456789");
    G3DString<64> substr = g3dstring1.substr(0, 10);
    EXPECT_EQ(strlen(tmp), substr.size());
    EXPECT_STREQ(tmp, substr.c_str());

    // test copy and substr for chars [10, 20)
    EXPECT_EQ(10, g3dstring1.copy(tmp, 10, 10));
    EXPECT_STREQ(tmp, "abcdefghij");
    substr = g3dstring1.substr(10, 10);
    EXPECT_EQ(strlen(tmp), substr.size());
    EXPECT_STREQ(tmp, substr.c_str());

    // test substr for [10, end of string)
    substr = g3dstring1.substr(10);
    EXPECT_EQ(strlen(copySubstrTestString + 10), substr.size());
    EXPECT_STREQ(copySubstrTestString + 10, substr.c_str());

    // Test substr with null chars
    G3DString<64> g3dstring2("test");
    g3dstring2 += '\0';
    g3dstring2 += "null";
    g3dstring2 += '\0';
    g3dstring2 += "ability";

    // STREQ is for const char* only, which we can't test with null chars
    // So look for each word individually and test size
    substr = g3dstring2.substr(5);
    EXPECT_STREQ("null", substr.c_str());
    EXPECT_STREQ("ability", substr.c_str() + 5);
    EXPECT_EQ(g3dstring2.size() - 5, substr.size());

    substr = g3dstring2.substr(10);
    EXPECT_STREQ("ability", substr.c_str());
    EXPECT_EQ(g3dstring2.size() - 10, substr.size());
}

TEST(G3DStringTest, SizeClear)
{
    G3DString<64> g3dstring1(sampleString);
    std::string string1(sampleString);

    // resize bigger, don't set  
    g3dstring1.resize(50);
    string1.resize(50);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    // resize bigger, do set
    g3dstring1.resize(60, '!');
    string1.resize(60, '!');
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    // resize smaller
    g3dstring1.resize(40);
    string1.resize(40);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    // resize smaller, do set
    g3dstring1.resize(30, '!');
    string1.resize(30, '!');
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    // test length (size is used extensively in these tests so we assume it's working right)  
    EXPECT_EQ(g3dstring1.length(), string1.length());

    // test empty (false)
    EXPECT_FALSE(g3dstring1.empty());

    // test clear/empty
    g3dstring1.clear();
    EXPECT_TRUE(g3dstring1.empty());
}

TEST(G3DStringTest, Conversions)
{
    G3DString<64> g3dstring1 = to_string(1234567890);
    std::string string1 = std::to_string(1234567890);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(1234567890u);
    string1 = std::to_string(1234567890u);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(1234567890l);
    string1 = std::to_string(1234567890l);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(1234567890ul);
    string1 = std::to_string(1234567890ul);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(1234567890ll);
    string1 = std::to_string(1234567890ll);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(1234567890ull);
    string1 = std::to_string(1234567890ull);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(INT_MAX);
    string1 = std::to_string(INT_MAX);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(INT_MIN);
    string1 = std::to_string(INT_MIN);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(LONG_MAX);
    string1 = std::to_string(LONG_MAX);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(LONG_MIN);
    string1 = std::to_string(LONG_MIN);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(LLONG_MAX);
    string1 = std::to_string(LLONG_MAX);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(LLONG_MIN);
    string1 = std::to_string(LLONG_MIN);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(UINT_MAX);
    string1 = std::to_string(UINT_MAX);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(ULONG_MAX);
    string1 = std::to_string(ULONG_MAX);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(ULLONG_MAX);
    string1 = std::to_string(ULLONG_MAX);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());


    g3dstring1 = to_string(0X1.BC70A3D70A3D7P+6f);
    string1 = std::to_string(0X1.BC70A3D70A3D7P+6f);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(0X1.BC70A3D70A3D7P+6);
    string1 = std::to_string(0X1.BC70A3D70A3D7P+6);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(0X1.BC70A3D70A3D7P+6l);
    string1 = std::to_string(0X1.BC70A3D70A3D7P+6l);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(123456789012345678901234567890.12345678901234567890123456789012345678901234567890123456789012345678901234567890f);
    string1 = std::to_string(123456789012345678901234567890.12345678901234567890123456789012345678901234567890123456789012345678901234567890f);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567891234567890.12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789);
    string1 = std::to_string(123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567891234567890.12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    g3dstring1 = to_string(123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567891234567890.1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789L);
    string1 = std::to_string(123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567891234567890.1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789L);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(g3dstring1.size(), string1.size());

    G3DString<64> g3dstring2("-1234567890");
    std::string string2("-1234567890");

    EXPECT_EQ(stoi(g3dstring2), std::stoi(string2));
    EXPECT_EQ(stol(g3dstring2), std::stol(string2));
    EXPECT_EQ(stoll(g3dstring2), std::stoll(string2));

    EXPECT_EQ(stoul(g3dstring2), std::stol(string2));
    EXPECT_EQ(stoull(g3dstring2), std::stoll(string2));

    size_t posSimdstring, posString;
    EXPECT_EQ(stoi(g3dstring2, &posSimdstring), std::stoi(string2, &posString));
    EXPECT_EQ(posSimdstring, posString);
    EXPECT_EQ(stol(g3dstring2, &posSimdstring), std::stol(string2, &posString));
    EXPECT_EQ(posSimdstring, posString);
    EXPECT_EQ(stoll(g3dstring2, &posSimdstring), std::stoll(string2, &posString));
    EXPECT_EQ(posSimdstring, posString);

    EXPECT_EQ(stoul(g3dstring2, &posSimdstring), std::stol(string2, &posString));
    EXPECT_EQ(posSimdstring, posString);
    EXPECT_EQ(stoull(g3dstring2, &posSimdstring), std::stoll(string2, &posString));
    EXPECT_EQ(posSimdstring, posString);

    G3DString<64> g3dstring3("-1.52343249");
    std::string string3("-1.52343249");

    EXPECT_EQ(stof(g3dstring3), std::stof(string3));
    EXPECT_EQ(stod(g3dstring3), std::stod(string3));
    EXPECT_EQ(stold(g3dstring3), std::stold(string3));

    EXPECT_EQ(stof(g3dstring3, &posSimdstring), std::stof(string3, &posString));
    EXPECT_EQ(posSimdstring, posString);
    EXPECT_EQ(stod(g3dstring3, &posSimdstring), std::stod(string3, &posString));
    EXPECT_EQ(posSimdstring, posString);
    EXPECT_EQ(stold(g3dstring3, &posSimdstring), std::stold(string3, &posString));
    EXPECT_EQ(posSimdstring, posString);
}

TEST(G3DStringTest, IO) {
    std::istringstream iss1("Hello there!\nWho are you?");
    std::istringstream iss2("Hello there!\nWho are you?");
    std::istringstream iss3("Hello there!\nWho are you?");
    std::istringstream iss4("Hello there!\nWho are you?");

    std::string tmp(1 << 21, '-');
    std::istringstream iss5(tmp);
    std::istringstream iss6(tmp);
    std::istringstream iss7(tmp);
    std::istringstream iss8(tmp);

    G3DString<64> g3dstring1;
    std::string string1;

    // test operator>>
    iss1 >> g3dstring1;
    iss2 >> string1;
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());

    // test getline
    std::getline(iss1, string1);
    getline(iss2, g3dstring1);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());

    // test format for input  
    iss3.setf(std::ios::skipws);
    iss4.setf(std::ios::skipws);
    iss3 >> g3dstring1;
    iss4 >> string1;
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());
    EXPECT_EQ(iss3.get(), iss4.get());

    iss7 >> g3dstring1;
    iss8 >> string1;
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());

    std::getline(iss5, string1);
    getline(iss6, g3dstring1);
    EXPECT_STREQ(g3dstring1.c_str(), string1.c_str());

    // test operator<<
    std::ostringstream oss2;
    std::ostringstream oss1;

    G3DString<64> g3dstring2(sampleString);
    std::string string2(sampleString);

    oss1 << std::setw(50) << g3dstring2 << std::setfill('*') << std::setw(50) << g3dstring2 << std::endl;
    oss2 << std::setw(50) << string2 << std::setfill('*') << std::setw(50) << string2 << std::endl;

    EXPECT_STREQ(oss1.str().c_str(), oss2.str().c_str());
}

TEST(G3DStringTest, Reserve) {
    size_t largeSize = 1 << 21;
    // test that reserve allocates exactly how much is expected + 1
    G3DString<64> g3dstring1;
    g3dstring1.reserve(largeSize);
    EXPECT_LE(largeSize, g3dstring1.capacity());

    // call to reserve with smaller numbers doesn't do anything
    g3dstring1.reserve();
    EXPECT_LE(largeSize, g3dstring1.capacity());
    g3dstring1.reserve(largeSize / 2);
    EXPECT_LE(largeSize, g3dstring1.capacity());

    // call to reserve doesn't change expected size
    g3dstring1 = G3DString<64>(100, 'a');
    g3dstring1.reserve();
    EXPECT_LE(100, g3dstring1.capacity());

    // in buffer, call to reserve with no params, should preserve buffer size
    g3dstring1 = G3DString<64>(10, 'a');
    g3dstring1.reserve();
    EXPECT_EQ(64, g3dstring1.capacity());
    EXPECT_STREQ("aaaaaaaaaa", g3dstring1.c_str());

    // in buffer, call to reserve with larger size, data should move to heap
    g3dstring1 = G3DString<64>(10, 'a');
    g3dstring1.reserve(largeSize);
    EXPECT_LE(largeSize, g3dstring1.capacity());
    EXPECT_STREQ("aaaaaaaaaa", g3dstring1.c_str());

    // when inConst, capacity is exactly as big as the const string
    g3dstring1 = G3DString<64>(sampleString);
    EXPECT_EQ(sampleStringSize, g3dstring1.capacity());
    g3dstring1.reserve();
    EXPECT_EQ(sampleStringSize, g3dstring1.capacity());

    // test that reserve allocates expected amount AND that const string is copied over
    g3dstring1 = G3DString<64>(sampleString);
    g3dstring1.reserve(largeSize);
    EXPECT_LE(largeSize, g3dstring1.capacity());
    EXPECT_STREQ(sampleString, g3dstring1.c_str());
}

//TEST(G3DStringTest, Hash) {
//    G3DString<64> g3dstring1(sampleString);
//    std::string string1(sampleString);
//
//    // only works if G3D Alloc is disabled bc had to hard code in types for hash
//    size_t g3dHash = std::hash<G3DString<64>>{}(g3dstring1);
//    size_t stringHash = std::hash<std::string>{}(string1);
//
//    EXPECT_EQ(g3dHash, stringHash);
//}

TEST(G3DStringTest, RangeLoops) {
    std::string result1, result2;

    G3DString<64> g3dstring1(sampleString);
    std::string string1(sampleString);

    for (char c : g3dstring1) {
        result1.push_back(c);
    }

    for (char c : string1) {
        result2.push_back(c);
    }

    EXPECT_STREQ(result1.c_str(), result2.c_str());
}
} // namespace G3D
