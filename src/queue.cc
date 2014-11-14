/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <cstdio>
#include <limits>
#include <cassert>
#include <cmath>

#include "packet.hh"
#include "queue.hh"

using namespace std;

Queue::QueuedPacket::QueuedPacket( const Packet & p, const double & s_arrival_tick )
    : bytes_to_transmit( PACKET_SIZE ),
      contents( p ),
      arrival_time( s_arrival_tick )
{}

/* construct a fixed schedule based on the supplied link rate  */
Queue::Queue( void )     
    : next_delivery_( 0 ),
      schedule_(),
      base_timestamp_( 0 ),
      packet_queue_(),
      repeat_( true )
{
}

/* construct a fixed schedule based on the supplied link rate  */
LinkQueue::LinkQueue( const double & link_packets_per_ms )     
    : Queue()
{
    const double ms = 1.0/link_packets_per_ms;
    if( ms < 1 ) {
        double counter = 0;
        while( counter < 1 ) {
            schedule_.emplace_back( 1 );
            counter += ms;
        }
    } else {
        schedule_.emplace_back( ms );
    }
}

void Queue::accept( const Packet & p, const double & tickno )
{
    const unsigned int now = tickno;

    /* pop wasted PDOs */
    while ( next_scheduled_time() <= now
            and (packet_queue_.empty() or packet_queue_.front().arrival_time > next_event_time( tickno )) ) {
        use_a_delivery_opportunity();
    }

    packet_queue_.emplace( p, now );
}

double Queue::next_event_time( const double & tickno __attribute ((unused)) ) const
{
    if( packet_queue_.empty() ) {
        return std::numeric_limits<double>::max();
    }
    return schedule_.at( next_delivery_ ) + base_timestamp_;
}

double Queue::next_scheduled_time( void ) const
{
    return schedule_.at( next_delivery_ ) + base_timestamp_;
}

void LinkQueue::use_a_delivery_opportunity( void )
{
    next_delivery_ = (next_delivery_ + 1) % schedule_.size();

    /* wraparound */
    if ( next_delivery_ == 0 ) {
        if ( repeat_ ) {
            base_timestamp_ += schedule_.back();
        } else {
            assert( false );
            //throw Exception( "LinkQueue", "reached end of link recording" );
        }
    }
}