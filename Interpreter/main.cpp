#include <iostream>
#include "Lexer.h"
#include "Parser.h"
#include "Executor.h"
#include "Runtime.h"






//var x1 = f2( func1( a, b, c+d ) );
//var x2 = (1+2)*(n<<2)+--y+0;
//y = x+1;

//   var x1 = ( a + b*(c + d/2)/m )*2;

//    var x1 = f1( 1+ff( (a)+(1) ), f2(u(3),d(4))+5);

//var x1 = f1( 1+ff( (a)+(2) ), f2(u(3),d(4))+5) + [](){ return 0;};

//const char * test = R"V0G0N(
//
//func test()
//{
//   var x1 = f3( (0), 1+f1( f0()+(2) ), f2(f1(3),f1(4))+5);
//
//   print( "x1=\(x1+f(x2))d\(x)" );
//}
//)V0G0N";

const char * test = R"V0G0N(

func f0()
{
    return 10;
}

func f1( a:Int )
{
    return a*10;
}

func f2( a1:Int, a2:Int )
{
    return a1-a2;
}

func f3( a1:Int, a2:Int, a3:Int )
{
    return a1+a2+a3;
}

var x1 = f3( (0), 1+f1( f0()+(2) ), f2(f1(3),f1(4))+5);

print( "x1=\(x1);" );

)V0G0N";

// std::cout <<"x1=" << (x1+f(x2)) << "d"

int main()
{

    std::string testStr{test};
    Lexer lexer{ testStr.data(), testStr.data()+testStr.size() };
    lexer.run();
    
    for( auto& token: lexer.tokens() )
    {
        LOG( gTokenTypeStrings[token.type] << " " << token.lexeme )
//        LOG( "#" << gTokenTypeStrings[token.type] << "# " << token.line << " " << token.pos << " "<< token.endPos )
    }
    
    Runtime runtime;
    Parser  parser{runtime};
    parser.parseProgram( testStr.data(), lexer.tokens() );
    
    std::vector<expr::Expression*> program = std::move(parser.m_program);
    runtime.run( program );

    return 0;
}
