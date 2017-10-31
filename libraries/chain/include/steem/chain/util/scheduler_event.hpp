#pragma once

#include <boost/interprocess/containers/flat_map.hpp>
#include <fc/static_variant.hpp>
#include <steem/chain/smt_objects.hpp>


namespace steem { namespace chain {

class database;

namespace util {
 
class launch_action
{
   private:
   protected:
   public:

      launch_action();
      ~launch_action();
      int run( database& db ) const;
};

class refund_action
{
   private:
   protected:
   public:

      refund_action();
      ~refund_action();
      int run( database& db ) const;
};

class scheduler_event
{
   protected:

      const asset_symbol_type symbol;

      const smt_token_object* get_smt_token_object( database& db ) const;
      void change_phase( database& db, const smt_token_object* token, smt_token_object::smt_phase phase ) const;

   public:

      scheduler_event( const asset_symbol_type& _symbol );
      ~scheduler_event();
      virtual int run( database& db ) const = 0;
};

class scheduler_proxy: public scheduler_event
{
   private:
   protected:

      launch_action launcher;

      bool is_min_steem_reached( database& db ) const;
      bool is_hard_cap_revealed( database& db ) const;

   public:

      scheduler_proxy( const asset_symbol_type& _symbol );
      ~scheduler_proxy();
      virtual int run( database& db ) const = 0;
};

class contribution_begin_scheduler_event: public scheduler_event
{
   public:

      contribution_begin_scheduler_event( const asset_symbol_type& _symbol );
      ~contribution_begin_scheduler_event();
      int run( database& db ) const override;
};

class contribution_end_scheduler_event: public scheduler_event
{
   public:

      contribution_end_scheduler_event( const asset_symbol_type& _symbol );
      ~contribution_end_scheduler_event();
      int run( database& db ) const override;
};

class launch_scheduler_event: public scheduler_proxy
{
   private:
   public:

      launch_scheduler_event( const asset_symbol_type& _symbol );
      ~launch_scheduler_event();
      int run( database& db ) const override;
};

class launch_expiration_scheduler_event: public scheduler_proxy
{
   private:

      refund_action refunder;

   public:

      launch_expiration_scheduler_event( const asset_symbol_type& _symbol );
      ~launch_expiration_scheduler_event();
      int run( database& db ) const override;
};

typedef fc::static_variant<
   contribution_begin_scheduler_event,
   contribution_end_scheduler_event,
   launch_scheduler_event,
   launch_expiration_scheduler_event
   > timed_event_object;


struct scheduler_event_visitor
{
   database& db;

   typedef void result_type;

   scheduler_event_visitor( database& _db );

   void operator()( const contribution_begin_scheduler_event& contribution_begin ) const;
   void operator()( const contribution_end_scheduler_event& contribution_end ) const;
   void operator()( const launch_scheduler_event& launch ) const;
   void operator()( const launch_expiration_scheduler_event& expiration_launch ) const;

   template < typename T >
   void operator()( const T& obj ) const;
};

} } }

FC_REFLECT(
   steem::chain::util::scheduler_event,
   (symbol)
   )

FC_REFLECT_EMPTY(
   steem::chain::util::scheduler_proxy
   )

FC_REFLECT_EMPTY(
   steem::chain::util::contribution_begin_scheduler_event
   )

FC_REFLECT_EMPTY(
   steem::chain::util::contribution_end_scheduler_event
   )

FC_REFLECT_EMPTY(
   steem::chain::util::launch_scheduler_event
   )

FC_REFLECT_EMPTY(
   steem::chain::util::launch_expiration_scheduler_event
   )
