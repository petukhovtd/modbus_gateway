#ifndef MODBUS_GATEWAY_LIMIT_QUEUE_H
#define MODBUS_GATEWAY_LIMIT_QUEUE_H

#include <queue>

namespace modbus_gateway
{

template< typename T >
class LimitQueue
{
public:
     explicit LimitQueue( size_t maxSize )
     : queue_()
     , maxSize_( maxSize )
     {
          assert( maxSize_ > 0 );
     }

     LimitQueue()
     : LimitQueue( std::numeric_limits< size_t >::max() )
     {}

     void Push( const T& val )
     {
          if( queue_.size() > maxSize_ )
          {
               queue_.pop();
          }
          queue_.push( val );
     }

     void Pop()
     {
          return queue_.pop();
     }

     const T& Front() const
     {
          return queue_.front();
     }

     size_t Size() const
     {
          return queue_.size();
     }

     bool Empty() const
     {
          return queue_.empty();
     }

     size_t MaxSize() const
     {
          return maxSize_;
     }

private:
     std::queue< T > queue_;
     size_t maxSize_;
};

}

#endif //MODBUS_GATEWAY_LIMIT_QUEUE_H
