/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


#include "../src/tokenize.h"
#include "../src/checkbufferoverrun.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestBufferOverrun : public TestFixture
{
public:
    TestBufferOverrun() : TestFixture("TestBufferOverrun")
    { }

private:



    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Assign variable ids
        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        // Fill function list
        tokenizer.fillFunctionList();

        // Clear the error buffer..
        errout.str("");

        // Check for buffer overruns..
        Settings settings;
        settings._showAll = true;
        CheckBufferOverrunClass checkBufferOverrun(&tokenizer, settings, this);
        checkBufferOverrun.bufferOverrun();
    }

    void run()
    {
        TEST_CASE(noerr1);
        TEST_CASE(noerr2);
        TEST_CASE(noerr3);
        TEST_CASE(noerr4);

        TEST_CASE(sizeof1);
        TEST_CASE(sizeof2);

        TEST_CASE(array_index_1);
        TEST_CASE(array_index_2);
        TEST_CASE(array_index_3);
        TEST_CASE(array_index_4);
        TEST_CASE(array_index_5);
        TEST_CASE(array_index_6);
        TEST_CASE(array_index_7);
        TEST_CASE(array_index_8);
        TEST_CASE(array_index_9);
        TEST_CASE(array_index_10);
        TEST_CASE(array_index_11);
        TEST_CASE(array_index_12);

        TEST_CASE(buffer_overrun_1);
        TEST_CASE(buffer_overrun_2);
        TEST_CASE(buffer_overrun_3);

        TEST_CASE(sprintf1);
        TEST_CASE(snprintf1);
        TEST_CASE(snprintf2);
        TEST_CASE(snprintf3);

        TEST_CASE(strncat1);
        TEST_CASE(strncat2);

        TEST_CASE(cin1);

        TEST_CASE(varid1);
        TEST_CASE(varid2);

        TEST_CASE(assign1);
    }



    void noerr1()
    {
        check("void f()\n"
              "{\n"
              "    if (ab)\n"
              "    {\n"
              "        char str[50];\n"
              "    }\n"
              "    if (ab)\n"
              "    {\n"
              "        char str[50];\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void noerr2()
    {
        check("void f1(char *str)\n"
              "{\n"
              "    strcpy(buf,str);\n"
              "}\n"
              "void f2(char *str)\n"
              "{\n"
              "    strcat(buf,str);\n"
              "}\n"
              "void f3(char *str)\n"
              "{\n"
              "    sprintf(buf,\"%s\",str);\n"
              "}\n"
              "void f4(const char str[])\n"
              "{\n"
              "    strcpy(buf, str);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void noerr3()
    {
        check("static void f()\n"
              "{\n"
              "    char data[1];\n"
              "    return abc.data[1];\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void noerr4()
    {
        // The memory isn't read or written and therefore there is no error.
        check("static void f()\n"
              "{\n"
              "    char data[100];\n"
              "    const char *p = &data[100];\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }




    void sizeof1()
    {
        check("static void f()\n"
              "{\n"
              "    char data[10];\n"
              "    data[ sizeof(*data) ] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void sizeof2()
    {
        check("static void f()\n"
              "{\n"
              "    char data[10];\n"
              "    data[ sizeof(data[0]) ] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        check("static void f()\n"
              "{\n"
              "    int data[2];\n"
              "    data[ sizeof(data[0]) ] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: (all) Array index out of bounds\n"), errout.str());
    }




    void array_index_1()
    {
        check("void f()\n"
              "{\n"
              "    char str[0x10];\n"
              "    str[15] = 0;\n"
              "    str[16] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: (all) Array index out of bounds\n"), errout.str());
    }


    void array_index_2()
    {
        check("void f()\n"
              "{\n"
              "    char *str = new char[0x10];\n"
              "    str[15] = 0;\n"
              "    str[16] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: (all) Array index out of bounds\n"), errout.str());
    }


    void array_index_3()
    {
        check("void f()\n"
              "{\n"
              "    int val[50];\n"
              "    for (i = 0; i < 100; i++)\n"
              "        sum += val[i];\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: (all) Buffer overrun\n"), errout.str());
    }


    void array_index_4()
    {
        check("const int SIZE = 10;\n"
              "void f()\n"
              "{\n"
              "    int i[SIZE];\n"
              "    i[SIZE] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: (all) Array index out of bounds\n"), errout.str());
    }


    void array_index_5()
    {
        check("void f()\n"
              "{\n"
              "    int i[10];\n"
              "    i[ sizeof(i) - 1 ] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: (all) Array index out of bounds\n"), errout.str());
    }


    void array_index_6()
    {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    abc.str[10] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:9]: (all) Array index out of bounds\n"), errout.str());
    }


    void array_index_7()
    {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void f(ABC *abc)\n"
              "{\n"
              "    abc->str[10] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:8]: (all) Array index out of bounds\n"), errout.str());
    }


    void array_index_8()
    {
        check("const int SIZE = 10;\n"
              "\n"
              "struct ABC\n"
              "{\n"
              "    char str[SIZE];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    abc.str[SIZE] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:11]: (all) Array index out of bounds\n"), errout.str());
    }

    void array_index_9()
    {
        check("static void memclr( char *data )\n"
              "{\n"
              "    data[10] = 0;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char str[5];\n"
              "    memclr( str );   // ERROR\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:9] -> [test.cpp:3]: (all) Array index out of bounds\n"), errout.str());
    }


    void array_index_10()
    {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void memclr( char *data )\n"
              "{\n"
              "    data[10] = 0;\n"
              "}\n"
              "\n"
              "static void f(ABC *abc)\n"
              "{\n"
              "    memclr(abc->str);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:13] -> [test.cpp:8]: (all) Array index out of bounds\n"), errout.str());
    }


    void array_index_11()
    {
        check("class ABC\n"
              "{\n"
              "public:\n"
              "    ABC();\n"
              "    char *str[10];\n"
              "    struct ABC *next;"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    for ( ABC *abc = abc1; abc; abc = abc->next() )\n"
              "    {\n"
              "        abc->str[10] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:12]: (all) Array index out of bounds\n"), errout.str());
    }


    void array_index_12()
    {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char str[10];\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{\n"
              "    str[10] = 0;\n"
              "}\n");
        std::string err(errout.str());
        ASSERT_EQUALS(std::string("[test.cpp:10]: (all) Array index out of bounds\n"), err);
    }





    void buffer_overrun_1()
    {
        check("void f()\n"
              "{\n"
              "    char str[3];\n"
              "    strcpy(str, \"abc\");\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: (all) Buffer overrun\n"), errout.str());
    }


    void buffer_overrun_2()
    {
        check("struct ABC\n"
              "{\n"
              "    char str[5];\n"
              "};\n"
              "\n"
              "static void f(ABC *abc)\n"
              "{\n"
              "    strcpy( abc->str, \"abcdef\" );\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:8]: (all) Buffer overrun\n"), errout.str());
    }


    void buffer_overrun_3()
    {
        check("int a[10];\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    int i;\n"
              "    for (i = 0; i <= 10; ++i)\n"
              "        a[i] = 0;\n"
              "}\n");
        std::string err(errout.str());
        ASSERT_EQUALS(std::string("[test.cpp:7]: (all) Buffer overrun\n"), err);
    }





    void sprintf1()
    {
        check("void f()\n"
              "{\n"
              "    char str[3];\n"
              "    sprintf(str, \"%s\", \"abc\");\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: (all) Buffer overrun\n"), errout.str());
    }

    void snprintf1()
    {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, 10, \"%s\", \"abc\");\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: (error) snprintf size is out of bounds\n"), errout.str());
    }

    void snprintf2()
    {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, 5, \"%s\", \"abc\");\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void snprintf3()
    {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, sizeof str, \"%s\", \"abc\");\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }




    void strncat1()
    {
        check("void f()\n"
              "{\n"
              "    char str[16];\n"
              "    strncpy(str, a, 10);\n"
              "    strncat(str, b, 10);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: (all) Dangerous usage of strncat, possible buffer overrun\n"), errout.str());
    }

    void strncat2()
    {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    strncat(str, a, 5);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: (all) Dangerous usage of strncat, possible buffer overrun\n"), errout.str());
    }



    void cin1()
    {
        check("void f()\n"
              "{\n"
              "    char str[10];\n"
              "    cin >> str;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: (all) Buffer overrun\n"), errout.str());
    }



    void varid1()
    {
        check("void foo()\n"
              "{\n"
              "    char str[10];\n"
              "    if (str[0])\n"
              "    {\n"
              "        char str[50];\n"
              "        str[30] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void varid2()
    {
        check("void foo()\n"
              "{\n"
              "    char str[10];\n"
              "    if (str[0])\n"
              "    {\n"
              "        char str[50];\n"
              "        memset(str,0,50);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void assign1()
    {
        check("char str[3] = {'a', 'b', 'c'};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    str[3] = 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: (all) Array index out of bounds\n"), errout.str());
    }
};

REGISTER_TEST(TestBufferOverrun)


