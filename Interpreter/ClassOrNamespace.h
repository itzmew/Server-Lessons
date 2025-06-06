#pragma once

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include "Token.h"
#include "Expr.h"

//namespace expr
//{

struct FuncDefinition;
struct ConstructorInfo;

struct BaseClassInfo
{
    bool             m_isPrivate;
    std::string_view m_name;
};

namespace expr {

struct ClassOrNamespace: public Expression
{
    bool m_isClass = false;

    std::string_view m_name;
    
    ClassOrNamespace*       m_parent = nullptr;
    
    // Map of inner classes or namespaces
    // (for class on inner classes)
    std::map<std::string_view,ClassOrNamespace*>    m_namespaceMap;
    
    std::map<std::string_view,FuncDefinition*>      m_functionMap;
    std::map< std::string_view,VarDeclaration*>     m_variableDeclMap; // global/static variable map

    // Only for classes
    std::vector<BaseClassInfo>                           m_baseClasses;
    std::vector<ConstructorInfo*>                        m_constuctors;

    // runtime initialization
    std::map< std::string_view, bool>                    m_initializationVariableMap;  // true if variable is initializing now
    std::map<std::string_view,ObjectValue>               m_variableValueMap;           // will be used in runtime

public:
    ClassOrNamespace() = default;
    ClassOrNamespace( bool isClass, const std::string_view& name, ClassOrNamespace* parentNamespace )
        : m_isClass(isClass), m_name(name), m_parent(parentNamespace)
    {}

    ~ClassOrNamespace()
    {
        LOG("~ClassOrNamespace()");
        for( auto& [key,value] : m_variableValueMap )
        {
            LOG( "left:?? " << (void*)&value );
        }
        //assert(0);
    }

public:
    
    virtual ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        return nullptr;
    }

    ClassOrNamespace* getTopNamespace()
    {
        if ( m_parent == nullptr )
        {
            return this;
        }
        return m_parent->getTopNamespace();
    }

    bool emplaceVar( const std::string_view& name, VarDeclaration* varDecl )
    {
        auto result = m_variableDeclMap.emplace( name, varDecl );
        return result.second;
    }
    
    bool emplaceClass( const std::string_view& name )
    {
        auto* namespaceOrClass = new ClassOrNamespace( true, name, this );
        auto result = m_namespaceMap.emplace( name, namespaceOrClass );
        if ( !result.second )
        {
            delete namespaceOrClass;
        }
        return result.second;
    }
    
    ClassOrNamespace* emplaceNamespace( const std::string_view& name )
    {
        assert( not m_isClass );

        auto* namespaceOrClass = new ClassOrNamespace( false, name, this );
        auto result = m_namespaceMap.emplace( name, namespaceOrClass );
        assert( result.second );
        return namespaceOrClass;
    }
    
    ClassOrNamespace* getClassOrNamespace( const std::string_view& name )
    {
        if ( auto it = m_namespaceMap.find( name ); it != m_namespaceMap.end() )
        {
            return it->second;
        }
        return nullptr;
    }
    
    FuncDefinition* getFunctionDef( const std::string_view& name )
    {
        if ( auto it = m_functionMap.find( name ); it != m_functionMap.end() )
        {
            return it->second;
        }
        return nullptr;
    }
    
    VarDeclaration* getVarDecl( const std::string_view& name )
    {
        if ( auto it = m_variableDeclMap.find( name ); it != m_variableDeclMap.end() )
        {
            return it->second;
        }
        return nullptr;
    }

    ObjectValue* getVarValue( const std::string_view& name )
    {
        if ( auto it = m_variableValueMap.find( name ); it != m_variableValueMap.end() )
        {
            return &it->second;
        }
        return nullptr;
    }

    template<class T>
    T* getInThisNamespace( const std::string_view& name )
    {
        if constexpr( std::is_same<T, FuncDefinition>::value )
        {
            return getFunctionDef( name );
        }
        if constexpr( std::is_same<T, ClassOrNamespace>::value )
        {
            return getClassOrNamespace( name );
        }
        if constexpr( std::is_same<T, VarDeclaration>::value )
        {
            return getVarDecl( name );
        }
        if constexpr( std::is_same<T, ObjectValue>::value )
        {
            return getVarValue( name );
        }
        return nullptr;
    }
    
    ClassOrNamespace* getClassOrNamespace( std::vector<std::string_view>::iterator begin, std::vector<std::string_view>::iterator end )
    {
        assert( begin != end );

        auto* namespacePtr0 = this;

        if ( begin->empty() )
        {
            namespacePtr0 = getTopNamespace();
            begin++;

            if ( begin == end )
            {
                return namespacePtr0;
            }
        }

        for( auto* namespacePtr = namespacePtr0;;)
        {
            if ( auto it = namespacePtr->m_namespaceMap.find( *begin ); it == namespacePtr->m_namespaceMap.end() )
            {
                return nullptr;
            }
            else
            {
                if ( begin+1 == end )
                {
                    return it->second;
                }
                begin++;
                namespacePtr = it->second;
            }
        }
        
        return nullptr;
    }
    
    FuncDefinition* getFunctionDef( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
    {
        return get<FuncDefinition>( name, namespaceSpec );
    }
    
    ClassOrNamespace* getClassDef( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
    {
        return get<ClassOrNamespace>( name, namespaceSpec );
    }
    
    VarDeclaration* getVarDecl( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
    {
        return get<VarDeclaration>( name, namespaceSpec );
    }

    ObjectValue* getVarValue( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
    {
        return get<ObjectValue>( name, namespaceSpec );
    }

    template<class T>
    T* get( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
    {
        auto* namespacePtr = this;

        if ( not namespaceSpec.empty() )
        {
            namespacePtr = getClassOrNamespace( namespaceSpec.begin(), namespaceSpec.end() );
        }

        if ( namespacePtr != nullptr )
        {
            if ( auto* obj = namespacePtr->getInThisNamespace<T>( name ); obj != nullptr )
            {
                return obj;
            }
        }
        
        // If our namespace is not top namespace, then try to find in top namespace
        if ( m_parent != nullptr )
        {
            auto* topNamespace = m_parent->getTopNamespace();
            
            auto* namespacePtr = topNamespace;
            if ( not namespaceSpec.empty() )
            {
                auto* namespacePtr = topNamespace->getClassOrNamespace( namespaceSpec.begin(), namespaceSpec.end() );
            }
            if ( namespacePtr != nullptr )
            {
                if ( auto* obj = namespacePtr->getInThisNamespace<T>( name ); obj != nullptr )
                {
                    return obj;
                }
            }
        }
        
        return nullptr;
    }
};

}

