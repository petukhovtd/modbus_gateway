#ifndef MODBUS_GATEWAY_SYNCHRONIZED_H
#define MODBUS_GATEWAY_SYNCHRONIZED_H

#include <mutex>

namespace modbus_gateway
{

/// @brief Реализация потокобезопасного доступа к объекту
/// @tparam T тип объекта синхронизации
template< typename T >
class Synchronized
{
public:
     /// @brief Конструктор класса
     /// @param[in] initial
     explicit Synchronized( T initial = T() )
               : value( std::move( initial ) )
     {}

     /// @brief Структура доступа
     /// Реализует блокировку при обращении к объекту
     struct Access
     {

     private:
          std::unique_lock< std::mutex > lock;
     public:
          T& ref;

          /// @brief Конструктор доступа
          /// @param[in,out] m
          /// @param[in,out] value
          Access( std::mutex& m, T& value )
                    : lock( m )
                    , ref( value )
          {}
     };

     /// @brief Получение доступа к объекту
     /// Предоставляет уникальную блокировку доступа
     /// @return
     Access GetAccess()
     {
          return Access( m, value );
     }

private:
     T value;
     std::mutex m;
};

}

#endif
