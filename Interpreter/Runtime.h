#pragma once

#include "TokenType.h"
#include "TokenTypeStrings.h"
#include "ObjectValue.h"
#include "getLineAndPos.h"
#include <map>
#include <stack>
#include <vector>

namespace expr
{
    struct FuncDefinition;
    struct VarDeclaration;
    struct ClassDefinition;
    struct Expression;
    struct ClassOrNamespace;
}

struct circular_reference_error: public std::runtime_error
{
    circular_reference_error( const Token& token1, const Token& token2 ) : std::runtime_error("circular reference error"),
        m_token1(token1),
        m_token2(token2)
    {}

    const Token& m_token1;
    const Token& m_token2;
};

struct Runtime
{
    const std::string_view m_programText;

    expr::ClassOrNamespace&   m_topLevelNamespace; // must be namespace with empty name

    expr::ClassOrNamespace*   m_currentNamespace2 = nullptr;

    //???
    using LocalVarStack = std::vector< std::map<std::string_view,ObjectValue> >;
    //using LocalVarStack = std::vector< std::map<std::string_view,void*> >;
    LocalVarStack       m_localVarStack;

    Runtime( const std::string_view& programText, expr::ClassOrNamespace& globaNamespace ) : m_programText(programText), m_topLevelNamespace(globaNamespace) {}

    void initGlobalVariables();

    void run( const std::vector<expr::Expression*>& code, const std::string& sourceCode );

    void getLineAndPos( const Token& token, int& line, int& pos, int& endPos ) const
    {
        ::getLineAndPos( m_programText, token, line, pos, endPos );
    }

    ObjectValue* getLocalVarValue( const std::string_view& varName )
    {
        for( auto localVarMapIt = m_localVarStack.rbegin();
             localVarMapIt != m_localVarStack.rend();
             localVarMapIt++ )
        {
            if ( auto it = localVarMapIt->find( varName ); it != localVarMapIt->end() )
            {
                return &it->second;
            }
        }
        return nullptr;
    }

    void printRuntimeError( const std::string& error, expr::Expression& e );

    void dbgPrintLine( const std::string& label, expr::Expression& e );

private:
    void initGlobalVariablesR( expr::ClassOrNamespace& namespaceRef );
};
