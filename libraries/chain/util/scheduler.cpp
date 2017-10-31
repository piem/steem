
#include <steem/chain/util/scheduler.hpp>

#include <steem/chain/database.hpp>

namespace steem { namespace chain { namespace util {

class timed_event_scheduler_impl
{
   public:

      using pair = std::pair< time_point_sec, timed_event_object >;
      using scheduler_allocator = chainbase::allocator< pair >;
      using items = fc::flat_map< time_point_sec, timed_event_object, std::less< time_point_sec >, scheduler_allocator >;
      using pitems = std::unique_ptr< items >;

   private:
   protected:
   public:

      pitems events;

      database& db;

      timed_event_scheduler_impl( database& _db );
      ~timed_event_scheduler_impl();

      void add( const time_point_sec& key, const timed_event_object& value );
      void run( const time_point_sec& head_block_time );
};

timed_event_scheduler_impl::timed_event_scheduler_impl( database& _db )
                        : db( _db )
{
}

timed_event_scheduler_impl::~timed_event_scheduler_impl()
{
   
}

void timed_event_scheduler_impl::add( const time_point_sec& key, const timed_event_object& value )
{
   if( !events )
      events = pitems( new items( scheduler_allocator( db.get_segment_manager() ) ) );
   events->emplace( std::make_pair( key, value ) );
}

void timed_event_scheduler_impl::run( const time_point_sec& head_block_time )
{
   const auto& begin = events->upper_bound( head_block_time );
   const auto& it = begin;

   const auto& end = events->end();

   scheduler_event_visitor visitor( db );

   while( it!=end )
   {
      it->second.visit( visitor );
   }

   events->erase( begin, end );
}

timed_event_scheduler::timed_event_scheduler( database& _db )
                  : impl( new timed_event_scheduler_impl( _db ) )
{

}

timed_event_scheduler::~timed_event_scheduler()
{

}

void timed_event_scheduler::add( const time_point_sec& key, const timed_event_object& value )
{
   FC_ASSERT( impl );
   impl->add( key, value );
}

void timed_event_scheduler::run( const time_point_sec& head_block_time )
{
   FC_ASSERT( impl );
   impl->run( head_block_time );
}

} } } // steem::chain::util
