#pragma once

#include <string>
#include <iostream>
#include <assert.h>

#include "Logs.h"

namespace expr
{
    struct FuncDefinition;
    struct VarDeclaration;
    struct ClassDefinition;
    struct Expression;
}

struct runtime_ex : std::runtime_error
{
    runtime_ex( const std::string& message ) : std::runtime_error(message) {}
};

struct Token;
struct runtime_ex3 : std::runtime_error
{
    const Token& m_token;
    runtime_ex3( const std::string& message, const Token& tokenRef ) : std::runtime_error(message), m_token(tokenRef) {}
};

#define RUNTIME_EX throw runtime_ex( std::string("runtime exception: in ") + __func__ );
#define RUNTIME_EX2(message) throw runtime_ex( message );
#define RUNTIME_EX3(message,token) throw runtime_ex3( message,token );

enum ObjectType : uint8_t { ot_null, ot_bool, ot_int, ot_double, ot_string, ot_class_ptr, ot_class_shared_ptr, ot_class_weak_ptr };

struct ClassObject;
struct Runtime;

namespace expr
{
    struct ClassOrNamespace;
}

struct ObjectValue;

void createClassObject( ObjectValue& outValue, Runtime&, bool isGlobal, expr::ClassOrNamespace& );

//class SomeClass
//{
//    ClassA      m_a;      // value
//    ClassA&     m_aPtr;   // shared ptr
//    ClassA&&    m_aPtr;   // weak ptr
//}

// Variable value
//
struct ObjectValue
{
    ObjectType m_type;
    uint8_t    m_isReturned = 0;

    union
    {
        bool            m_boolValue;
        uint64_t        m_intValue;
        double          m_doubleValue;

        std::string                   m_stringValue;

        std::shared_ptr<ClassObject>  m_classSharedPtr;
        std::weak_ptr<ClassObject>    m_classWeakPtr;

        // TODO: for C++ support
        ClassObject*                  m_classObjPtr;

    };

    ObjectValue()
    {
        m_type = ot_null;
        m_intValue = 0;
    }

    ~ObjectValue();

    ObjectValue( ObjectValue& val )
    {
        *this = val;
    }

    ObjectValue( ObjectValue&& val )
    {
        *this = std::move(val);
    }

    ObjectValue& operator=( ObjectValue& val )
    {
        if ( this == &val )
        {
            return *this;
        }

        if ( m_type != val.m_type )
        {
            this->~ObjectValue();
            m_type = val.m_type;
            if ( m_type == ot_class_shared_ptr )
            {
                new (&m_classSharedPtr) std::shared_ptr<ClassObject>();
            }
            else if ( m_type == ot_class_weak_ptr )
            {
                new (&m_classWeakPtr) std::weak_ptr<ClassObject>();
            }
            else if ( m_type == ot_string )
            {
                new (&m_stringValue) std::string();
            }
        }

        switch( m_type )
        {
            case ot_null:   break;
            case ot_bool:   m_boolValue   = val.m_boolValue;   break;
            case ot_int:    m_intValue    = val.m_intValue;    break;
            case ot_double: m_doubleValue = val.m_doubleValue; break;
            case ot_string: m_stringValue = val.m_stringValue; break;
            
            case ot_class_shared_ptr: m_classSharedPtr = val.m_classSharedPtr; break;
            case ot_class_weak_ptr:   m_classWeakPtr   = val.m_classWeakPtr;   break;
        }
        return *this;
    }

    ObjectValue& operator=( ObjectValue&& val )
    {
        if ( this == &val )
        {
            return *this;
        }

        if ( m_type != val.m_type )
        {
            this->~ObjectValue();
            m_type = val.m_type;
            if ( m_type == ot_class_shared_ptr )
            {
                new (&m_classSharedPtr) std::shared_ptr<ClassObject>();
            }
            else if ( m_type == ot_class_weak_ptr )
            {
                new (&m_classWeakPtr) std::weak_ptr<ClassObject>();
            }
            else if ( m_type == ot_string )
            {
                new (&m_stringValue) std::string();
            }
        }

        switch( m_type )
        {
            case ot_null:   break;
            case ot_bool:   m_boolValue = val.m_boolValue; break;
            case ot_int:    m_intValue = val.m_intValue; break;
            case ot_double: m_doubleValue = val.m_doubleValue; break;
            case ot_string: m_stringValue = std::move(val.m_stringValue); break;
            case ot_class_ptr:  assert(0); break;
            case ot_class_shared_ptr:
                m_classSharedPtr = std::move(val.m_classSharedPtr);
                val.m_classSharedPtr.reset();
                break;
            case ot_class_weak_ptr:  
                m_classWeakPtr = std::move(val.m_classWeakPtr);
                val.m_classWeakPtr.reset();
                break;
        }
        return *this;
    }

    std::string pstring()
    {
        switch( m_type )
        {
            case ot_null: return "<null>";
            case ot_bool: return m_boolValue ? "<bool:true>" : "<bool:false>";
            case ot_int:  return std::string("<Int:") + std::to_string(m_intValue) + ">";
            case ot_double: return std::string("<Double:") + std::to_string(m_doubleValue) + ">";
            case ot_string: return std::string("<String:'") + m_stringValue + "'>";;
        }
        return "<unknown-type>";
    }

    void toStream( std::ostream& stream )
    {
        switch( m_type )
        {
            case ot_null: stream << "<null>"; break;
            case ot_bool: stream << m_boolValue; break;
            case ot_int: stream << m_intValue; break;
            case ot_double: stream << m_doubleValue; break;
            case ot_string: stream << m_stringValue; break;
        }
    }

    bool isNull() const { return m_type == ot_null; };

    bool boolValue() const
    {
        if ( m_type == ot_bool )
        {
            return m_boolValue;
        }
        RUNTIME_EX;
    };

    uint64_t intValue() const
    {
        if ( m_type == ot_int )
        {
            return m_intValue;
        }
        RUNTIME_EX;
    };

    double doubleValue() const
    {
        if ( m_type == ot_int )
        {
            return m_intValue;
        }
        if ( m_type == ot_double )
        {
            return m_doubleValue;
        }
        RUNTIME_EX;
    };

    const std::string& stringValue() const
    {
        if ( m_type == ot_string )
        {
            return m_stringValue;
        }
        RUNTIME_EX;
    };


    void setBool( bool newValue )
    {
        if ( m_type != ot_bool )
        {
            this->~ObjectValue();
        }
        m_type = ot_bool;
        m_boolValue = newValue;
    }

    void setInt( int newValue )
    {
        if ( m_type != ot_int )
        {
            this->~ObjectValue();
        }
        m_type = ot_int;
        m_intValue = newValue;
    }

    void setDouble( int newValue )
    {
        if ( m_type != ot_double )
        {
            this->~ObjectValue();
        }
        m_type = ot_double;
        m_doubleValue = newValue;
    }

    void setString( const std::string& newValue )
    {
        if ( m_type == ot_string )
        {
            m_stringValue = newValue;
        }
        else
        {
            this->~ObjectValue();
            m_type = ot_string;
            new (&m_stringValue) std::string(newValue);
        }
    }

    void setString( const std::string_view& newValue )
    {
        if ( m_type == ot_string )
        {
            m_stringValue = std::string(newValue);
        }
        else
        {
            this->~ObjectValue();
            m_type = ot_string;
            new (&m_stringValue) std::string(newValue);
        }
    }
};




