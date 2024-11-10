#pragma once

#include <forward_list>
#include <list>
#include <vector>
#include "Object.h"

class Executor;

const char* getLineEnd( const char* text )
{
    for( const char* ptr = text; ; ptr++ )
    {
        if ( *ptr == '\n' || *ptr == '\r' || *ptr == 0 )
        {
            return ptr;
        }
    }
}


std::string getLine( const char* text, int lineNumber )
{
    if ( lineNumber == 0)
    {
        return std::string( text, getLineEnd(text) );
    }
    
    int number = 0;
    for( const char* ptr = text; *ptr != 0; ptr++ )
    {
        if ( *ptr == 0 )
        {
            return "";
        }
        
        if ( *ptr == '\n' || *ptr == '\r' )
        {
            number++;
            while( *ptr == '\n' || *ptr == '\r' )
            {
                ptr++;
                if ( *ptr == 0 )
                {
                    return "";
                }
            }
            if ( number == lineNumber )
            {
                return std::string( ptr, getLineEnd(ptr) );
            }
        }
    }
}

namespace expr {

struct Expression
{
//    Object evaluate( Executor& ) = 0;
};

struct ExpressionPtr
{
    ExpressionPtr( Expression* expr ) : m_expr(expr) {}
    
    ExpressionPtr( ExpressionPtr&& exprPtr ) : m_expr(exprPtr.m_expr)
    {
        if ( this != &exprPtr )
            exprPtr.m_expr=nullptr;
    }
    
    ExpressionPtr& operator=( ExpressionPtr&& exprPtr )
    {
        m_expr = exprPtr.m_expr;
        if ( this != &exprPtr )
            exprPtr.m_expr = nullptr;
    }

    ~ExpressionPtr() {
        delete m_expr;
    }
    
    Expression* m_expr = nullptr;
};

struct ExpressionList: public Expression
{
    std::list<ExpressionPtr> m_list;
};

struct Argument
{
    std::string m_name;
    std::string m_type;
    std::string m_defaultValue;
};

struct Func : public Expression
{
    std::string             m_name;
    std::vector<Argument>   m_argList;
    ExpressionList          m_body;
};

struct If : public Expression
{
    ExpressionPtr           m_condition;
    ExpressionList          m_yesBlock;
    ExpressionList          m_noBlock;
};

struct VariableDefinition : public Expression
{
    std::string     m_name;
    std::string     m_type;
    ExpressionPtr   m_value;
};

struct For : public Expression
{
    std::string     m_iteratorName;
    ExpressionPtr   m_condition;
    ExpressionPtr   m_getNext;
};

struct Return : public Expression
{
    ExpressionPtr   m_returnValue;
};

struct Print : public Expression
{
    ExpressionList  m_printParameters;
};

}
