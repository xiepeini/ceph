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

#ifndef __OSD_TYPES_H
#define __OSD_TYPES_H

#include "include/types.h"
#include "include/reqid.h"

#define PG_INO 1


// osd types
typedef __uint64_t coll_t;        // collection id

// pg stuff
typedef __uint16_t ps_t;
typedef __uint8_t pruleset_t;     // hmm what is this for?  -sage


// crush rule ids
#define CRUSH_REP_RULE(nrep) (100+nrep)  // replication
#define CRUSH_RAID_RULE(num) (200+num)   // raid



// placement group id
struct pg_t {
public:
  static const int TYPE_REP   = 1;
  static const int TYPE_RAID4 = 2;

private:
  union {
    struct {
      __uint8_t   type:3;       //  2 
      __uint8_t   size:5;       //  5
      ps_t        ps:16;        // 16
      pruleset_t  ruleset:8;    //  8
      __uint32_t  preferred:32; // 32
    } fields;
    __uint64_t val;          // 64
  } u;

public:
  pg_t() { u.val = 0; }
  pg_t(const pg_t& o) { u.val = o.u.val; }
  pg_t(int type, int size, ps_t seed, int pref, pruleset_t r=0) {
    u.fields.type = type;
    u.fields.size = size;
    u.fields.ps = seed;
    u.fields.preferred = pref+1;   // hack: avoid negative.
    u.fields.ruleset = r;
  }
  pg_t(__uint64_t v) { u.val = v; }

  int type()      { return u.fields.type; }
  bool is_rep()   { return type() == TYPE_REP; }
  bool is_raid4() { return type() == TYPE_RAID4; }

  int size() { return u.fields.size; }
  ps_t ps() { return u.fields.ps; }
  pruleset_t ruleset() { return u.fields.ruleset; }
  __uint32_t preferred() { return u.fields.preferred-1; }   // hack: avoid negative.
  
  /*
  pg_t operator=(__uint64_t v) { u.val = v; return *this; }
  pg_t operator&=(__uint64_t v) { u.val &= v; return *this; }
  pg_t operator+=(pg_t o) { u.val += o.val; return *this; }
  pg_t operator-=(pg_t o) { u.val -= o.val; return *this; }
  pg_t operator++() { ++u.val; return *this; }
  */
  operator __uint64_t() const { return u.val; }

  object_t to_object() const { return object_t(PG_INO, u.val >> 32, u.val & 0xffffffff); }
};

inline ostream& operator<<(ostream& out, pg_t pg) 
{
  //return out << hex << pg.val << dec;

  if (pg.is_rep()) 
    out << pg.size() << 'x';
  else if (pg.is_raid4()) 
    out << pg.size() << 'r';
  else 
    out << pg.size() << '?';
  
  if (pg.ruleset())
    out << (int)pg.ruleset() << 's';
  
  if (pg.preferred())
    out << pg.preferred() << 'p';
  out << hex << pg.ps() << dec;

  out << "=" << hex << (__uint64_t)pg << dec;
  return out;
}

namespace __gnu_cxx {
  template<> struct hash< pg_t >
  {
    size_t operator()( const pg_t& x ) const
    {
      static hash<__uint64_t> H;
      return H(x);
    }
  };
}






// compound rados version type
class eversion_t {
public:
  epoch_t epoch;
  version_t version;
  eversion_t(epoch_t e=0, version_t v=0) : epoch(e), version(v) {}
};

inline bool operator==(const eversion_t& l, const eversion_t& r) {
  return (l.epoch == r.epoch) && (l.version == r.version);
}
inline bool operator!=(const eversion_t& l, const eversion_t& r) {
  return (l.epoch != r.epoch) || (l.version != r.version);
}
inline bool operator<(const eversion_t& l, const eversion_t& r) {
  return (l.epoch == r.epoch) ? (l.version < r.version):(l.epoch < r.epoch);
}
inline bool operator<=(const eversion_t& l, const eversion_t& r) {
  return (l.epoch == r.epoch) ? (l.version <= r.version):(l.epoch <= r.epoch);
}
inline bool operator>(const eversion_t& l, const eversion_t& r) {
  return (l.epoch == r.epoch) ? (l.version > r.version):(l.epoch > r.epoch);
}
inline bool operator>=(const eversion_t& l, const eversion_t& r) {
  return (l.epoch == r.epoch) ? (l.version >= r.version):(l.epoch >= r.epoch);
}
inline ostream& operator<<(ostream& out, const eversion_t e) {
  return out << e.epoch << "'" << e.version;
}





// -----------------------------------------

class ObjectExtent {
 public:
  object_t    oid;       // object id
  off_t       start;     // in object
  size_t      length;    // in object

  objectrev_t rev;       // which revision?
  pg_t        pgid;      // where to find the object

  map<size_t, size_t>  buffer_extents;  // off -> len.  extents in buffer being mapped (may be fragmented bc of striping!)
  
  ObjectExtent() : start(0), length(0), rev(0), pgid(0) {}
  ObjectExtent(object_t o, off_t s=0, size_t l=0) : oid(o), start(s), length(l), rev(0), pgid(0) { }
};

inline ostream& operator<<(ostream& out, ObjectExtent &ex)
{
  return out << "extent(" 
             << ex.oid << " in " << hex << ex.pgid << dec
             << " " << ex.start << "~" << ex.length
             << ")";
}



// ---------------------------------------

class OSDSuperblock {
public:
  const static __uint64_t MAGIC = 0xeb0f505dULL;
  __uint64_t magic;
  __uint64_t fsid;      // unique fs id (random number)
  int        whoami;    // my role in this fs.
  epoch_t    current_epoch;             // most recent epoch
  epoch_t    oldest_map, newest_map;    // oldest/newest maps we have.
  OSDSuperblock(__uint64_t f=0, int w=0) : 
    magic(MAGIC), fsid(f), whoami(w), 
    current_epoch(0), oldest_map(0), newest_map(0) {}
};

inline ostream& operator<<(ostream& out, OSDSuperblock& sb)
{
  return out << "sb(fsid " << sb.fsid
             << " osd" << sb.whoami
             << " e" << sb.current_epoch
             << " [" << sb.oldest_map << "," << sb.newest_map
             << "])";
}


#endif
