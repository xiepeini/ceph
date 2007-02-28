// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software 
 * Foundation.  See file COPYING.
 * 
 */

#ifndef __REPLICATEDPG_H
#define __REPLICATEDPG_H


#include "PG.h"


class ReplicatedPG : public PG {
  
  /*
   * gather state on the primary/head while replicating an osd op.
   */
  class Gather {
  public:
    class MOSDOp *op;
    tid_t rep_tid;

    ObjectStore::Transaction t;
    bool applied;

    set<int>  waitfor_ack;
    set<int>  waitfor_commit;
    
    utime_t   start;

    bool sent_ack, sent_commit;
    
    set<int>         osds;
    eversion_t       new_version;

    eversion_t       pg_local_last_complete;
    map<int,eversion_t> pg_complete_thru;
    
    Gather(MOSDOp *o, tid_t rt, eversion_t nv, eversion_t lc) :
      op(o), rep_tid(rt),
      applied(false),
      sent_ack(false), sent_commit(false),
      new_version(nv), 
      pg_local_last_complete(lc) { }

    bool can_send_ack() { 
      return !sent_ack && !sent_commit &&
        waitfor_ack.empty(); 
    }
    bool can_send_commit() { 
      return !sent_commit &&
        waitfor_ack.empty() && waitfor_commit.empty(); 
    }
    bool can_delete() { 
      return waitfor_ack.empty() && waitfor_commit.empty(); 
    }
  };

  // replica ops
  // [primary|tail]
  map<tid_t, Gather*>          repop_gather;
  map<tid_t, list<class Message*> > waiting_for_repop;

  void get_repop_gather(Gather*);
  void apply_repop(Gather *repop);
  void put_repop_gather(Gather*);
  void issue_repop(MOSDOp *op, int osd);
  Gather *new_repop_gather(MOSDOp *op);
  void repop_ack(Gather *repop,
                 int result, bool commit,
                 int fromosd, eversion_t pg_complete_thru=0);

public:
  void op_stat(MOSDOp *op);
  int op_read(MOSDOp *op);
  void op_modify(MOSDOp *op);
  
};


inline ostream& operator<<(ostream& out, PG::Gather& repop)
{
  out << "repop(" << &repop << " rep_tid=" << repop.rep_tid 
      << " wfack=" << repop.waitfor_ack
      << " wfcommit=" << repop.waitfor_commit;
  out << " pct=" << repop.pg_complete_thru;
  out << " op=" << *(repop.op);
  out << " repop=" << &repop;
  out << ")";
  return out;
}


#endif
