#include "Runtime.h"
#include "Expr.h"
#include "ClassOrNamespace.h"

void Runtime::initGlobalVariablesR( expr::ClassOrNamespace& namespaceRef )
{
    m_currentNamespace2 = &namespaceRef;

    // go through all variables of this namespace
    for( auto& [variableName,var]: namespaceRef.m_variableDeclMap )
    {
        namespaceRef.m_initializationVariableMap[variableName] = true;
        {
            ObjectValue value;
            var->execute( value, *this, true);
            namespaceRef.m_variableValueMap.emplace( var->m_identifierName, std::move(value) );
        }
        namespaceRef.m_initializationVariableMap[variableName] = false;
    }

    // init nested namespaces
    for( auto& [name,namespaceRef]: namespaceRef.m_namespaceMap )
    {
        initGlobalVariablesR( *namespaceRef );
    }

    m_currentNamespace2 = &namespaceRef;
}

void prepareVarMapToInitializationR( expr::ClassOrNamespace& namespaceRef )
{
    for( auto& [variableName,varDecl]: namespaceRef.m_variableDeclMap )
    {
        namespaceRef.m_initializationVariableMap[variableName] = false;
    }

    for( auto& [namespaceName,namespaceRef]:  namespaceRef.m_namespaceMap )
    {
        prepareVarMapToInitializationR( *namespaceRef );
    }
}

void Runtime::initGlobalVariables()
{
    initGlobalVariablesR( m_topLevelNamespace );
}

void Runtime::run( const std::vector<expr::Expression*>& code, const std::string& sourceCode )
{
    auto mainRef = m_topLevelNamespace.m_functionMap.find("main");
    if ( mainRef == m_topLevelNamespace.m_functionMap.end() )
    {
        throw runtime_ex( "undefined function main" );
    }

    auto mainCall = expr::FunctionCall( mainRef->second->m_token );
    mainCall.m_functionName = "main";
    ObjectValue retValue;
    mainCall.execute( retValue, *this, false );
}

void Runtime::printRuntimeError( const std::string& error, expr::Expression& e )
{
    int line=0;
    int pos, endPos;
    ::getLineAndPos( m_programText, e.m_token, line, pos, endPos );

    std::cerr << error << ":" << std::endl;
    std::cerr << ::getLine( m_programText.data(), line ) << std::endl;
    for( int i=0; i<pos-1; i++ )
    {
        std::cerr << ' ';
    }
    for( int i=pos; i<endPos; i++ )
    {
        std::cerr << '^';
    }
    std::cerr << std::endl;
}

void Runtime::dbgPrintLine( const std::string& label, expr::Expression& e )
{
    int line=0;
    int pos, endPos;
    ::getLineAndPos( m_programText, e.m_token, line, pos, endPos );

    std::cerr << "+++dbgPrintLine: " << label << std::endl;
    std::cerr << ::getLine( m_programText.data(), line ) << std::endl;
    for( int i=0; i<pos-1; i++ )
    {
        std::cerr << ' ';
    }
    for( int i=pos; i<endPos; i++ )
    {
        std::cerr << '^';
    }
    std::cerr << std::endl;
}
