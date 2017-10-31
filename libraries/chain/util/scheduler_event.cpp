
#include <steem/chain/util/scheduler_event.hpp>

#include <steem/chain/database.hpp>

namespace steem { namespace chain { namespace util {

launch_action::launch_action()
{

}

launch_action::~launch_action()
{

}

int launch_action::run( database& db ) const
{
   //Main action - SMT launching.
   return 0;
}

refund_action::refund_action()
{

}

refund_action::~refund_action()
{

}

int refund_action::run( database& db ) const
{
   //Second main action - SMT refunding.
   return 0;
}

scheduler_event::scheduler_event( const asset_symbol_type& _symbol )
               : symbol( _symbol )
{

}

scheduler_event::~scheduler_event()
{

}

const smt_token_object* scheduler_event::get_smt_token_object( database& db ) const
{
   const smt_token_object* ret = db.find< smt_token_object, by_symbol >( symbol );
   FC_ASSERT( ret );

   return ret;
}

void scheduler_event::change_phase( database& db, const smt_token_object* token, smt_token_object::smt_phase phase ) const
{
   FC_ASSERT( token );

   db.create< smt_token_object >( [&]( smt_token_object& token )
   {
      token.phase = phase;
   });
}

scheduler_proxy::scheduler_proxy( const asset_symbol_type& _symbol )
               : scheduler_event( _symbol )
{

}

scheduler_proxy::~scheduler_proxy()
{

}

bool scheduler_proxy::is_min_steem_reached( database& db ) const
{
   //Check if we have enough STEEM-s.
   return true;
}

bool scheduler_proxy::is_hard_cap_revealed( database& db ) const
{
   //Check if hard cap is revealed.
   return true;
}

contribution_begin_scheduler_event::contribution_begin_scheduler_event( const asset_symbol_type& _symbol )
                              : scheduler_event( _symbol )
{

}

contribution_begin_scheduler_event::~contribution_begin_scheduler_event()
{

}

int contribution_begin_scheduler_event::run( database& db ) const
{
   const smt_token_object* obj = scheduler_event::get_smt_token_object( db );
   FC_ASSERT( obj );

   //From now contributions are allowed.
   change_phase( db, obj, smt_token_object::smt_phase::contribution_begin_time_completed );
   db.add_scheduler_event( obj->generation_end_time, util::contribution_end_scheduler_event( symbol ) );

   return 0;
}

contribution_end_scheduler_event::contribution_end_scheduler_event( const asset_symbol_type& _symbol )
                              : scheduler_event( _symbol )
{

}

contribution_end_scheduler_event::~contribution_end_scheduler_event()
{

}

int contribution_end_scheduler_event::run( database& db ) const
{
   const smt_token_object* obj = scheduler_event::get_smt_token_object( db );
   FC_ASSERT( obj );

   //From now contributions are forbidden.
   change_phase( db, obj, smt_token_object::smt_phase::contribution_end_time_completed );
   db.add_scheduler_event( obj->announced_launch_time, util::launch_scheduler_event( symbol ) );

   return 0;
}

launch_scheduler_event::launch_scheduler_event( const asset_symbol_type& _symbol )
                              : scheduler_proxy( _symbol )
{

}

launch_scheduler_event::~launch_scheduler_event()
{

}

int launch_scheduler_event::run( database& db ) const
{
   int ret = 0;

   const smt_token_object* obj = scheduler_event::get_smt_token_object( db );
   FC_ASSERT( obj );

   change_phase( db, obj, smt_token_object::smt_phase::launch_time_completed );

   bool check_min_steems = is_min_steem_reached( db );
   if( !check_min_steems )
   {
      db.add_scheduler_event( obj->launch_expiration_time, util::launch_expiration_scheduler_event( symbol ) );
   }
   else
   {
      bool check_hard_cap = is_hard_cap_revealed( db );
      if( check_hard_cap )
         ret = launcher.run( db );
      else
      {
         db.add_scheduler_event( obj->launch_expiration_time, util::launch_expiration_scheduler_event( symbol ) );
      }
   }

   return ret;
}

launch_expiration_scheduler_event::launch_expiration_scheduler_event( const asset_symbol_type& _symbol )
                              : scheduler_proxy( _symbol )
{

}

launch_expiration_scheduler_event::~launch_expiration_scheduler_event()
{

}

int launch_expiration_scheduler_event::run( database& db ) const
{
   int ret = 0;

   const smt_token_object* obj = scheduler_event::get_smt_token_object( db );
   FC_ASSERT( obj );

   change_phase( db, obj, smt_token_object::smt_phase::launch_expiration_time_completed );

   bool check_min_steems = is_min_steem_reached( db );
   if( !check_min_steems )
      ret = refunder.run( db );
   else
   {
      bool check_hard_cap = is_hard_cap_revealed( db );
      if( check_hard_cap )
         ret = launcher.run( db );
      else
         refunder.run( db );
   }

   return ret;
}

scheduler_event_visitor::scheduler_event_visitor( database& _db )
                     : db( _db )
{
}

void scheduler_event_visitor::operator()( const contribution_begin_scheduler_event& contribution_begin ) const
{
   int status = contribution_begin.run( db );
   FC_ASSERT( status == 0 );
}

void scheduler_event_visitor::operator()( const contribution_end_scheduler_event& contribution_end ) const
{
   int status = contribution_end.run( db );
   FC_ASSERT( status == 0 );
}

void scheduler_event_visitor::operator()( const launch_scheduler_event& launch ) const
{
   int status = launch.run( db );
   FC_ASSERT( status == 0 );
}

void scheduler_event_visitor::operator()( const launch_expiration_scheduler_event& expiration_launch ) const
{
   int status = expiration_launch.run( db );
   FC_ASSERT( status == 0 );
}

template < typename T >
void scheduler_event_visitor::operator()( const T& obj ) const
{
   FC_ASSERT("Not supported yet.");
}

} } } // steem::chain::util
